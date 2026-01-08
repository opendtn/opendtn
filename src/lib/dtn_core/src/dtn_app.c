/***
        ------------------------------------------------------------------------

        Copyright (c) 2025 German Aerospace Center DLR e.V. (GSOC)

        Licensed under the Apache License, Version 2.0 (the "License");
        you may not use this file except in compliance with the License.
        You may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

        Unless required by applicable law or agreed to in writing, software
        distributed under the License is distributed on an "AS IS" BASIS,
        WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
        See the License for the specific language governing permissions and
        limitations under the License.

        This file is part of the opendtn project. https://opendtn.com

        ------------------------------------------------------------------------
*//**
        @file           dtn_app.c
        @author         Töpfer, Markus

        @date           2025-12-04


        ------------------------------------------------------------------------
*/
#include "../include/dtn_app.h"
#include "../include/dtn_socket_item.h"

#include <dtn_base/dtn_dict.h>
#include <dtn_base/dtn_id.h>
#include <dtn_base/dtn_item_json_io_buffer.h>
#include <dtn_base/dtn_log.h>
#include <dtn_base/dtn_socket.h>
#include <dtn_base/dtn_string.h>
#include <dtn_base/dtn_utils.h>

#include <dtn_base/dtn_thread_loop.h>
#include <dtn_base/dtn_thread_pool.h>

/*----------------------------------------------------------------------------*/

#define DTN_APP_MAGIC_BYTES 0xAAAA

/*----------------------------------------------------------------------------*/

struct dtn_app {

    uint16_t magic_bytes;
    dtn_app_config config;

    dtn_id id;

    bool debug;

    dtn_json_io_buffer *io_buffer;

    struct {

        dtn_thread_lock lock;
        dtn_dict *dict;

    } events;

    dtn_thread_loop *thread_loop;

    struct {

        void *userdata;
        void (*callback)(void *userdata, int socket);

    } close;

    struct {

        void *userdata;
        void (*callback)(void *userdata, int socket);

    } connected;
};

/*----------------------------------------------------------------------------*/

struct app_callback {

    void *userdata;
    bool (*function)(void *userdata, int socket, dtn_item *input);
};

/*
 *      ------------------------------------------------------------------------
 *
 *      #THREAD FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

bool handle_in_loop(dtn_thread_loop *loop, dtn_thread_message *msg) {

    if (!loop || !msg)
        goto error;

    dtn_app *self = dtn_app_cast(dtn_thread_loop_get_data(loop));
    if (!self)
        goto error;

    dtn_log_debug("%s unexpected message received in loop", self->config.name);

    msg = dtn_thread_message_free(msg);
    return true;
error:
    msg = dtn_thread_message_free(msg);
    return false;
}

/*----------------------------------------------------------------------------*/

bool handle_in_thread(dtn_thread_loop *loop, dtn_thread_message *msg) {

    struct app_callback callback = (struct app_callback){0};

    if (!loop || !msg)
        goto error;

    dtn_app *self = dtn_app_cast(dtn_thread_loop_get_data(loop));
    if (!self)
        goto error;

    if (!msg->message)
        goto error;

    const char *event =
        dtn_item_get_string(dtn_item_get(msg->message, "/event"));
    if (!event) {

        if (self->debug)
            dtn_log_debug("%s JSON message without event received, ignoring",
                          self->config.name);

        goto done;
    }

    if (!dtn_thread_lock_try_lock(&self->events.lock)) {

        char *string = dtn_item_to_json(msg->message);

        dtn_log_debug("%s failed to unlock event, loosing event %s",
                      self->config.name, string);

        string = dtn_data_pointer_free(string);
        goto done;
    }

    struct app_callback *cb = dtn_dict_get(self->events.dict, event);

    if (cb)
        callback = *cb;

    if (!dtn_thread_lock_unlock(&self->events.lock)) {

        dtn_log_error("%s failed to unlock events dict.", self->config.name);
        goto error;
    }

    if (!callback.function) {

        dtn_log_debug("%s received event %s, event not in DB - ignoring",
                      self->config.name, event);

        goto done;
    }

    if (!callback.function(callback.userdata, msg->socket, msg->message)) {

        dtn_log_error("%s did not accept message", self->config.name);
        msg->message = NULL;

        goto error;
    }

    msg->message = NULL;
done:
    msg = dtn_thread_message_free(msg);
    return true;
error:
    msg = dtn_thread_message_free(msg);
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #JSON FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

static void cb_json_success(void *userdata, int socket, dtn_item *input) {

    dtn_app *self = dtn_app_cast(userdata);
    if (!self || socket < 0 || !input)
        goto error;

    dtn_thread_message *msg =
        dtn_thread_message_standard_create(DTN_GENERIC_MESSAGE, input);

    if (!msg)
        goto error;

    msg->socket = socket;

    if (!dtn_thread_loop_send_message(self->thread_loop, msg,
                                      DTN_RECEIVER_THREAD)) {

        char *string = dtn_item_to_json(input);
        dtn_log_debug("%s lost message %s", self->config.name, string);
        string = dtn_data_pointer_free(string);

        msg = dtn_thread_message_free(msg);
        goto error;
    }

    return;

error:
    if (self)
        dtn_io_close(self->config.io, socket);
    dtn_item_free(input);
    return;
}

/*----------------------------------------------------------------------------*/

static void cb_json_failure(void *userdata, int socket) {

    dtn_app *self = dtn_app_cast(userdata);

    dtn_log_debug("NON JSON IO at %i - dropping connection.", socket);
    dtn_io_close(self->config.io, socket);
    return;
}
/*
 *      ------------------------------------------------------------------------
 *
 *      #GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

static bool init_config(dtn_app_config *config) {

    if (!config || !config->loop || !config->io)
        goto error;

    if (0 == config->limits.threadlock_timeout_usec)
        config->limits.threadlock_timeout_usec = 100000;

    if (0 == config->limits.message_queue_capacity)
        config->limits.message_queue_capacity = 1000;

    if (0 == config->name[0])
        strcat(config->name, "app");

    if (0 == config->limits.threads) {

        long numofcpus = sysconf(_SC_NPROCESSORS_ONLN);
        config->limits.threads = numofcpus;
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

dtn_app *dtn_app_create(dtn_app_config config) {

    dtn_app *self = NULL;
    if (!init_config(&config))
        goto error;

    self = calloc(1, sizeof(dtn_app));
    if (!self)
        goto error;

    self->magic_bytes = DTN_APP_MAGIC_BYTES;
    self->config = config;

    self->io_buffer = dtn_json_io_buffer_create(
        (dtn_json_io_buffer_config){.debug = false,
                                    .objects_only = false,
                                    .callback.userdata = self,
                                    .callback.success = cb_json_success,
                                    .callback.failure = cb_json_failure});

    if (!self->io_buffer)
        goto error;

    dtn_dict_config d_config = dtn_dict_string_key_config(255);
    d_config.value.data_function.free = dtn_data_pointer_free;

    self->events.dict = dtn_dict_create(d_config);
    if (!self->events.dict)
        goto error;

    self->thread_loop =
        dtn_thread_loop_create(self->config.loop,
                               (dtn_thread_loop_callbacks){
                                   .handle_message_in_thread = handle_in_thread,
                                   .handle_message_in_loop = handle_in_loop},
                               self);

    if (!self->thread_loop)
        goto error;

    if (!dtn_thread_loop_reconfigure(
            self->thread_loop,
            (dtn_thread_loop_config){
                .disable_to_loop_queue = false,
                .message_queue_capacity =
                    self->config.limits.message_queue_capacity,
                .lock_timeout_usecs =
                    self->config.limits.threadlock_timeout_usec,
                .num_threads = self->config.limits.threads}))
        goto error;

    if (!dtn_thread_lock_init(&self->events.lock,
                              self->config.limits.threadlock_timeout_usec))
        goto error;

    if (!dtn_thread_loop_start_threads(self->thread_loop))
        goto error;

    dtn_id_fill_with_uuid(self->id);

    return self;
error:
    dtn_app_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

void dtn_app_set_debug(dtn_app *self, bool value) {

    if (!self)
        return;

    self->debug = value;
    return;
}

/*----------------------------------------------------------------------------*/

dtn_app *dtn_app_free(dtn_app *self) {

    if (!dtn_app_cast(self))
        return self;

    self->io_buffer = dtn_json_io_buffer_free(self->io_buffer);

    self->events.dict = dtn_dict_free(self->events.dict);

    self->thread_loop = dtn_thread_loop_free(self->thread_loop);

    dtn_thread_lock_clear(&self->events.lock);

    self = dtn_data_pointer_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_app *dtn_app_cast(const void *data) {

    if (!data)
        return NULL;

    if (*(uint16_t *)data != DTN_APP_MAGIC_BYTES)
        return NULL;

    return (dtn_app *)data;
}

/*----------------------------------------------------------------------------*/

dtn_app_config dtn_app_config_from_item(const dtn_item *item) {

    dtn_app_config out = {0};

    const dtn_item *config = dtn_item_get(item, "/app");
    if (!config)
        config = item;

    const char *name = dtn_item_get_string(dtn_item_get(config, "/name"));
    if (name)
        strncpy(out.name, name, PATH_MAX);

    out.limits.threadlock_timeout_usec = dtn_item_get_number(
        dtn_item_get(config, "/limits/threadlock_timeout_usec"));

    out.limits.message_queue_capacity = dtn_item_get_number(
        dtn_item_get(config, "/limits/message_queue_capacity"));

    out.limits.threads =
        dtn_item_get_number(dtn_item_get(config, "/limits/threads"));

    return out;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #SOCKET FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

static bool cb_accept(void *userdata, int listener, int connection) {

    dtn_socket_data local = {0};
    dtn_socket_data remote = {0};

    dtn_app *app = dtn_app_cast(userdata);
    if (!app)
        goto error;

    dtn_socket_get_data(connection, &local, &remote);

    dtn_log_info("%s accepted socket at %i connection %i from %s:%i",
                 app->config.name, listener, connection, remote.host,
                 remote.port);

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static void cb_connected(void *userdata, int connection) {

    dtn_socket_data local = {0};
    dtn_socket_data remote = {0};

    dtn_app *self = dtn_app_cast(userdata);
    if (!self || connection < 0)
        goto error;

    dtn_socket_get_data(connection, &local, &remote);

    dtn_log_info("%s connected from %s:%i to %s:%i", self->config.name,
                 local.host, local.port, remote.host, remote.port);

    if (self->config.register_client) {

        dtn_item *event = dtn_item_object();
        dtn_item_object_set(event, "event", dtn_item_string("register"));
        dtn_item *par = dtn_item_object();
        dtn_item_object_set(event, "paramater", par);
        dtn_item_object_set(par, "name", dtn_item_string(self->config.name));
        dtn_item_object_set(par, "uuid", dtn_item_string(self->id));

        dtn_app_send_json(self, connection, event);
        event = dtn_item_free(event);
    }

    if (self->connected.callback)
        self->connected.callback(self->connected.userdata, connection);

error:
    return;
}

/*----------------------------------------------------------------------------*/

static bool cb_io(void *userdata, int connection, const char *domain,
                  const dtn_memory_pointer data) {

    dtn_app *self = dtn_app_cast(userdata);
    if (!self)
        goto error;

    UNUSED(domain);

    return dtn_json_io_buffer_push(self->io_buffer, connection, data);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static void cb_close(void *userdata, int connection) {

    dtn_app *self = dtn_app_cast(userdata);

    if (!self)
        return;
    dtn_log_debug("%s closing socket %i", self->config.name, connection);
    dtn_json_io_buffer_drop(self->io_buffer, connection);

    if (self->close.userdata)
        if (self->close.callback)
            self->close.callback(self->close.userdata, connection);

    return;
}

/*----------------------------------------------------------------------------*/

int dtn_app_open_listener(dtn_app *self, dtn_socket_configuration config) {

    if (!self)
        goto error;

    int listener = dtn_io_open_listener(
        self->config.io, (dtn_io_socket_config){.socket = config,
                                                .ssl = (dtn_io_ssl_config){0},
                                                .callbacks.userdata = self,
                                                .callbacks.accept = cb_accept,
                                                .callbacks.io = cb_io,
                                                .callbacks.close = cb_close});

    if (-1 == listener)
        goto error;

    return listener;

error:
    return -1;
}

/*----------------------------------------------------------------------------*/

int dtn_app_open_connection(dtn_app *self, dtn_socket_configuration config,
                            dtn_io_ssl_config ssl) {

    if (!self)
        goto error;

    if (!config.host[0])
        goto error;
    if (0 == config.port)
        goto error;

    int connection = dtn_io_open_connection(
        self->config.io,
        (dtn_io_socket_config){.auto_reconnect = true,
                               .socket = config,
                               .ssl = ssl,
                               .callbacks.userdata = self,
                               .callbacks.accept = cb_accept,
                               .callbacks.io = cb_io,
                               .callbacks.close = cb_close,
                               .callbacks.connected = cb_connected});

    if (-1 == connection) {
        goto error;
    } else {
        cb_connected(self, connection);
    }

    return connection;

error:
    return -1;
}

/*----------------------------------------------------------------------------*/

bool dtn_app_close(dtn_app *self, int socket) {

    if (!self)
        goto error;

    dtn_json_io_buffer_drop(self->io_buffer, socket);

    return dtn_io_close(self->config.io, socket);

error:
    return -1;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      SEND FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_app_send(dtn_app *self, int socket, const uint8_t *buffer,
                  size_t size) {

    if (!self || !buffer)
        goto error;

    if (self->debug)
        dtn_log_debug("%s send at %i %.*s", self->config.name, socket, size,
                      (char *)buffer);

    return dtn_io_send(self->config.io, socket,
                       (dtn_memory_pointer){.start = buffer, .length = size});
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_app_send_json(dtn_app *self, int socket, const dtn_item *output) {

    char *string = NULL;

    if (!self || socket < 0 || !output)
        goto error;

    string = dtn_item_to_json(output);
    if (!string) {
        dtn_log_error("Could not encode JSON");
        goto error;
    }

    if (self->debug)
        dtn_log_debug("%s send at %i %s", self->config.name, socket, string);

    if (!dtn_io_send(self->config.io, socket,
                     (dtn_memory_pointer){.start = (uint8_t *)string,
                                          .length = strlen(string)})) {

        dtn_log_error("%s failed to send at %i", self->config.name, socket);
        goto error;
    }

    string = dtn_data_pointer_free(string);
    return true;
error:
    string = dtn_data_pointer_free(string);
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      IO CALLBACKS FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_app_register(dtn_app *self, const char *event,
                      bool (*function)(void *userdata, int socket,
                                       dtn_item *input),
                      void *userdata) {

    char *key = NULL;
    struct app_callback *cb = NULL;

    if (!self || !event || !function || !userdata)
        goto error;

    key = dtn_string_dup(event);
    cb = calloc(1, sizeof(struct app_callback));
    if (!key || !cb)
        goto error;

    if (!dtn_thread_lock_try_lock(&self->events.lock)) {

        dtn_log_error("%s failed to lock to add event %s", self->config.name,
                      event);

        goto error;
    }

    cb->userdata = userdata;
    cb->function = function;

    bool result = dtn_dict_set(self->events.dict, key, cb, NULL);

    if (result) {
        key = NULL;
        cb = NULL;
    }

    if (!dtn_thread_lock_unlock(&self->events.lock)) {

        dtn_log_error("Failed to unlock events dict after adding event %s",
                      event);

        goto error;
    }

    if (result)
        return result;

error:
    key = dtn_data_pointer_free(key);
    cb = dtn_data_pointer_free(cb);
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_app_deregister(dtn_app *self, const char *event) {

    if (!self || !event)
        goto error;

    if (!dtn_thread_lock_try_lock(&self->events.lock)) {

        dtn_log_error("%s failed to lock to remove event %s", self->config.name,
                      event);

        goto error;
    }

    bool result = dtn_dict_del(self->events.dict, event);

    if (!dtn_thread_lock_unlock(&self->events.lock)) {

        dtn_log_error("Failed to unlock events dict after removing event %s",
                      event);

        goto error;
    }

    if (result)
        return result;

error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_app_register_close(dtn_app *self, void *userdata,
                            void(callback)(void *userdata, int socket)) {

    if (!self)
        goto error;

    if (self->close.userdata) {

        if (!userdata) {
            self->close.userdata = NULL;
            self->close.callback = NULL;
        } else {
            return false;
        }
    }

    self->close.userdata = userdata;
    self->close.callback = callback;
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_app_register_connected(dtn_app *self, void *userdata,
                                void(callback)(void *userdata, int socket)) {

    if (!self)
        goto error;

    if (self->connected.userdata) {

        if (!userdata) {
            self->connected.userdata = NULL;
            self->connected.callback = NULL;
        } else {
            return false;
        }
    }

    self->connected.userdata = userdata;
    self->connected.callback = callback;
    return true;
error:
    return false;
}