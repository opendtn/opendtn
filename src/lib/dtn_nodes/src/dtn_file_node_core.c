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
        @file           dtn_file_node_core.c
        @author         Töpfer, Markus

        @date           2025-12-23


        ------------------------------------------------------------------------
*/
#include "../include/dtn_file_node_core.h"

#include <libgen.h>
#include <limits.h>
#include <stdlib.h>

#include <dtn/dtn_bundle_buffer.h>
#include <dtn/dtn_dtn_uri.h>
#include <dtn/dtn_interface_ip.h>
#include <dtn/dtn_routing.h>

#include <dtn_base/dtn_dict.h>
#include <dtn_base/dtn_dir.h>
#include <dtn_base/dtn_dump.h>
#include <dtn_base/dtn_file.h>
#include <dtn_base/dtn_garbadge_colloctor.h>
#include <dtn_base/dtn_item.h>
#include <dtn_base/dtn_item_json.h>
#include <dtn_base/dtn_linked_list.h>
#include <dtn_base/dtn_string.h>
#include <dtn_base/dtn_thread_lock.h>
#include <dtn_base/dtn_thread_loop.h>
#include <dtn_base/dtn_thread_message.h>
#include <dtn_base/dtn_time.h>
#include <dtn_base/dtn_utils.h>

#include <dtn_core/dtn_key_store.h>

/*---------------------------------------------------------------------------*/

#define DTN_FILE_NODE_CORE_MAGIC_BYTE 0xc053

typedef enum ThreadMessageType {

    BUNDLE_IO = 0,
    STATE_CHANGE

} ThreadMessageType;

/*---------------------------------------------------------------------------*/

struct dtn_file_node_core {

    uint16_t magic_byte;
    dtn_file_node_core_config config;

    dtn_garbadge_colloctor *garbadge;

    uint64_t sequence;

    dtn_dtn_uri *uri;
    char *path;

    dtn_thread_loop *tloop;

    dtn_bundle_buffer *buffer;

    dtn_routing *routing;

    dtn_key_store *keys;

    struct {

        dtn_thread_lock lock_ip;
        dtn_dict *ip;

    } interfaces;
};

/*----------------------------------------------------------------------------*/

typedef struct Interface {

    dtn_thread_lock lock;
    char name[DTN_HOST_NAME_MAX];
    dtn_ip_link_state state;

    dtn_interface_ip *interface;

} Interface;

/*----------------------------------------------------------------------------*/

static void *interface_free(void *data) {

    if (!data)
        return NULL;

    Interface *self = (Interface *)data;
    if (!dtn_thread_lock_try_lock(&self->lock))
        goto error;
    self->interface = dtn_interface_ip_free(self->interface);
    if (!dtn_thread_lock_unlock(&self->lock)) {
        dtn_log_error("failed to unlock interface.");
    }
    dtn_thread_lock_clear(&self->lock);
    self = dtn_data_pointer_free(self);
error:
    return self;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      THREADED FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

typedef struct Threadmessage {

    dtn_thread_message generic;
    ThreadMessageType type;

    dtn_ip_link_state state;
    dtn_bundle *bundle;
    dtn_socket_data remote;
    char *interface;

} Threadmessage;

/*----------------------------------------------------------------------------*/

static dtn_thread_message *thread_message_free(dtn_thread_message *self) {

    if (self->type != 1)
        return self;

    Threadmessage *msg = (Threadmessage *)self;
    msg->bundle = dtn_bundle_free(msg->bundle);
    msg->interface = dtn_data_pointer_free(msg->interface);
    msg = dtn_data_pointer_free(msg);
    return NULL;
}

/*----------------------------------------------------------------------------*/

static dtn_thread_message *
thread_message_state_create(const char *name, dtn_ip_link_state state) {

    Threadmessage *msg = calloc(1, sizeof(Threadmessage));
    if (!msg)
        return NULL;

    msg->generic.magic_bytes = DTN_THREAD_MESSAGE_MAGIC_BYTES;
    msg->generic.type = 1;
    msg->generic.free = thread_message_free;

    msg->type = STATE_CHANGE;
    msg->state = state;
    msg->interface = dtn_string_dup(name);

    return dtn_thread_message_cast(msg);
}

/*----------------------------------------------------------------------------*/

static dtn_thread_message *thread_message_create(dtn_bundle *bundle,
                                                 const dtn_socket_data *remote,
                                                 const char *name) {

    Threadmessage *msg = calloc(1, sizeof(Threadmessage));
    if (!msg)
        return NULL;

    msg->generic.magic_bytes = DTN_THREAD_MESSAGE_MAGIC_BYTES;
    msg->generic.type = 1;
    msg->generic.free = thread_message_free;

    msg->type = BUNDLE_IO;
    msg->bundle = bundle;
    msg->remote = *remote;
    msg->interface = dtn_string_dup(name);
    return dtn_thread_message_cast(msg);
}

/*----------------------------------------------------------------------------*/

static bool handle_in_loop(dtn_thread_loop *tloop, dtn_thread_message *msg) {

    dtn_file_node_core *self = dtn_thread_loop_get_data(tloop);
    if (!self || !msg)
        goto error;

    TODO("... to be implemented.");

    dtn_thread_message_free(msg);
    return true;
error:
    dtn_thread_message_free(msg);
    return false;
}

/*---------------------------------------------------------------------------*/

static void cb_payload(void *userdata, const uint8_t *payload, size_t size,
                       const char *source, const char *destination) {

    dtn_dtn_uri *dest = NULL;
    char inpath[PATH_MAX];
    memset(inpath, 0, PATH_MAX);

    char cleanpath[PATH_MAX];
    memset(cleanpath, 0, PATH_MAX);

    char path[2 * PATH_MAX];
    memset(path, 0, 2 * PATH_MAX);

    char rpath[PATH_MAX];
    memset(rpath, 0, PATH_MAX);

    dtn_file_node_core *self = dtn_file_node_core_cast(userdata);
    if (!self || !payload || size < 1 || !source || !destination)
        goto error;

    dtn_log_debug("GOT PAYLOAD from %s for %s", source, destination);

    dest = dtn_dtn_uri_decode(destination);

    if (0 != dtn_string_compare(dest->scheme, self->uri->scheme))
        goto error;

    if (0 != dtn_string_compare(dest->name, self->uri->name))
        goto error;

    // prepare path
    snprintf(inpath, PATH_MAX, "%s", dest->demux);
    if (!dtn_dtn_uri_path_remove_dot_segments(inpath, cleanpath))
        goto error;
    snprintf(path, 2 * PATH_MAX, "%s/%s", self->path, cleanpath);
    strncpy(rpath, path, PATH_MAX);

    char *dir = dirname(rpath);
    if (!dtn_dir_tree_create(dir))
        goto error;

    if (DTN_FILE_SUCCESS != dtn_file_write(path, payload, size, "w+")) {
        dtn_log_error("failed to write file %s", path);
    } else {
        dtn_log_info("new file received %s", path);
    }

error:
    dtn_dtn_uri_free(dest);
    return;
}

/*---------------------------------------------------------------------------*/

static bool message_bundle_process(dtn_file_node_core *self,
                                   Threadmessage *msg) {

    if (!self || !msg)
        goto error;

    /*
        dtn_log_debug("THREAD IO at %s from %s:%i", msg->interface,
            msg->remote.host, msg->remote.port);
    */

    if (!dtn_bundle_buffer_push(self->buffer, msg->bundle)) {
        dtn_log_error("failed to push bundle to buffer - dropping.");
    }

    msg->bundle = NULL;

    dtn_thread_message_free(dtn_thread_message_cast(msg));
    return true;

error:
    dtn_thread_message_free(dtn_thread_message_cast(msg));
    return false;
}

/*---------------------------------------------------------------------------*/

static bool message_state_change_process(dtn_file_node_core *self,
                                         Threadmessage *msg) {

    if (!self || !msg)
        goto error;

    const char *string = NULL;

    switch (msg->state) {

    case DTN_IP_LINK_ERROR:
        string = "ERROR";
        break;

    case DTN_IP_LINK_DOWN:
        string = "DOWN";
        break;

    case DTN_IP_LINK_UP:
        string = "UP";
        break;
    }

    dtn_log_debug("THREAD IO STATE CHANGE at %s to %s", msg->interface, string);

    TODO("... process state change");

    dtn_thread_message_free(dtn_thread_message_cast(msg));
    return true;

error:
    dtn_thread_message_free(dtn_thread_message_cast(msg));
    return false;
}

/*---------------------------------------------------------------------------*/

static bool handle_in_thread(dtn_thread_loop *tloop, dtn_thread_message *msg) {

    dtn_file_node_core *self = dtn_thread_loop_get_data(tloop);
    if (!self || !msg)
        goto error;

    if (msg->type != 1)
        goto error;

    Threadmessage *message = (Threadmessage *)msg;

    switch (message->type) {

    case BUNDLE_IO:
        return message_bundle_process(self, message);
        break;

    case STATE_CHANGE:
        return message_state_change_process(self, message);
        break;
    }

    dtn_thread_message_free(msg);
    return true;
error:
    dtn_thread_message_free(msg);
    return false;
}

/*---------------------------------------------------------------------------*/

static dtn_key_store *cb_get_keys(void *data) {

    dtn_file_node_core *self = dtn_file_node_core_cast(data);

    return self->keys;
}

/*---------------------------------------------------------------------------*/

static bool init_config(dtn_file_node_core_config *config) {

    if (!config || !config->loop)
        goto error;

    if (0 == config->limits.threadlock_timeout_usec)
        config->limits.threadlock_timeout_usec = 100000;

    if (0 == config->limits.message_queue_capacity)
        config->limits.message_queue_capacity = 10000;

    if (0 == config->limits.link_check)
        config->limits.link_check = 1000000;

    if (0 == config->limits.threads) {

        long numofcpus = sysconf(_SC_NPROCESSORS_ONLN);
        config->limits.threads = numofcpus;
    }

    return true;
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_file_node_core *
dtn_file_node_core_create(dtn_file_node_core_config config) {

    dtn_file_node_core *self = NULL;

    if (!init_config(&config))
        goto error;

    self = calloc(1, sizeof(dtn_file_node_core));
    if (!self)
        goto error;

    self->magic_byte = DTN_FILE_NODE_CORE_MAGIC_BYTE;
    self->config = config;

    self->tloop =
        dtn_thread_loop_create(self->config.loop,
                               (dtn_thread_loop_callbacks){
                                   .handle_message_in_thread = handle_in_thread,
                                   .handle_message_in_loop = handle_in_loop},
                               self);

    if (!self->tloop)
        goto error;
    if (!dtn_thread_loop_reconfigure(
            self->tloop,
            (dtn_thread_loop_config){
                .disable_to_loop_queue = false,
                .message_queue_capacity = config.limits.message_queue_capacity,
                .lock_timeout_usecs = config.limits.threadlock_timeout_usec,
                .num_threads = config.limits.threads}))
        goto error;

    if (!dtn_thread_loop_start_threads(self->tloop))
        goto error;

    dtn_dict_config d_config = dtn_dict_string_key_config(255);
    d_config.value.data_function.free = interface_free;

    self->interfaces.ip = dtn_dict_create(d_config);
    if (!self->interfaces.ip)
        goto error;

    if (!dtn_thread_lock_init(&self->interfaces.lock_ip,
                              self->config.limits.threadlock_timeout_usec))
        goto error;

    self->garbadge = dtn_garbadge_colloctor_create((
        dtn_garbadge_colloctor_config){
        .loop = config.loop,
        .limits.threadlock_timeout_usec = config.limits.threadlock_timeout_usec,
        .limits.run_cleanup_usec = 1000000 // 1 second
    });

    if (!self->garbadge)
        goto error;

    self->buffer = dtn_bundle_buffer_create((dtn_bundle_buffer_config){
        .loop = self->config.loop,
        .limits.buffer_time_cleanup_usecs =
            self->config.limits.buffer_time_cleanup_usecs,
        .limits.history_secs = self->config.limits.history_secs,
        .limits.max_buffer_time_secs = self->config.limits.max_buffer_time_secs,
        .limits.threadlock_timeout_usecs =
            self->config.limits.threadlock_timeout_usec,
        .callbacks.userdata = self,
        .callbacks.payload = cb_payload,
        .callbacks.get_keys = cb_get_keys});

    dtn_routing_config routing = (dtn_routing_config){

        .loop = config.loop,
        .limits.threadlock_timeout_usecs =
            config.limits.threadlock_timeout_usec};

    self->routing = dtn_routing_create(routing);
    if (!self->routing)
        goto error;

    dtn_key_store_config keys =
        (dtn_key_store_config){.limits.threadlock_timeout_usec =
                                   config.limits.threadlock_timeout_usec};

    if (0 != config.keys[0])
        strncpy(keys.path, config.keys, PATH_MAX);

    self->keys = dtn_key_store_create(keys);
    if (!self->keys)
        goto error;

    dtn_key_store_load(self->keys, NULL);

    return self;
error:
    dtn_file_node_core_free(self);
    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_file_node_core *dtn_file_node_core_free(dtn_file_node_core *self) {

    if (!dtn_file_node_core_cast(self))
        return self;

    dtn_thread_lock_clear(&self->interfaces.lock_ip);

    self->keys = dtn_key_store_free(self->keys);
    self->buffer = dtn_bundle_buffer_free(self->buffer);
    self->uri = dtn_dtn_uri_free(self->uri);
    self->path = dtn_data_pointer_free(self->path);
    self->routing = dtn_routing_free(self->routing);
    self->garbadge = dtn_garbadge_colloctor_free(self->garbadge);
    self->interfaces.ip = dtn_dict_free(self->interfaces.ip);
    self->tloop = dtn_thread_loop_free(self->tloop);
    self = dtn_data_pointer_free(self);
    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_file_node_core *dtn_file_node_core_cast(const void *data) {

    if (!data)
        return NULL;

    if (*(uint16_t *)data != DTN_FILE_NODE_CORE_MAGIC_BYTE)
        return NULL;

    return (dtn_file_node_core *)data;
}

/*---------------------------------------------------------------------------*/

static void interface_io(void *userdata, const dtn_socket_data *remote,
                         dtn_bundle *bundle, const char *name) {

    dtn_file_node_core *self = dtn_file_node_core_cast(userdata);
    if (!self || !remote || !bundle || !name)
        return;

    // dtn_log_debug("IO at %s from %s:%i", name, remote->host, remote->port);

    dtn_thread_message *msg = thread_message_create(bundle, remote, name);
    if (!msg)
        goto error;

    bundle = NULL;

    if (!dtn_thread_loop_send_message(self->tloop, msg, DTN_RECEIVER_THREAD)) {
        msg = dtn_thread_message_free(msg);
    }

error:
    dtn_bundle_free(bundle);
    return;
}

/*---------------------------------------------------------------------------*/

static void interface_state(void *userdata, dtn_ip_link_state state,
                            const char *name) {

    dtn_file_node_core *self = dtn_file_node_core_cast(userdata);
    if (!self || !name)
        return;

    const char *string = NULL;

    switch (state) {

    case DTN_IP_LINK_ERROR:
        string = "ERROR";
        break;

    case DTN_IP_LINK_DOWN:
        string = "DOWN";
        break;

    case DTN_IP_LINK_UP:
        string = "UP";
        break;
    }

    dtn_log_debug("state change at %s to %s", name, string);

    Interface *in = dtn_dict_get(self->interfaces.ip, name);
    if (!in)
        goto done;

    if (!dtn_thread_lock_try_lock(&in->lock))
        goto done;
    in->state = state;
    if (!dtn_thread_lock_unlock(&in->lock)) {
        dtn_log_error("failed to unlock interface.");
    }

    dtn_thread_message *msg = thread_message_state_create(name, state);
    if (!msg)
        goto done;

    if (!dtn_thread_loop_send_message(self->tloop, msg, DTN_RECEIVER_THREAD)) {
        msg = dtn_thread_message_free(msg);
    }

done:
    return;
}

/*---------------------------------------------------------------------------*/

static void interface_close(void *userdata, const char *name) {

    dtn_file_node_core *self = dtn_file_node_core_cast(userdata);
    if (!userdata || !name)
        return;

    dtn_log_error("Interface %s lost", name);
    if (!dtn_thread_lock_try_lock(&self->interfaces.lock_ip)) {
        dtn_log_error("Failed to lock IP interfaces.");
        goto error;
    }

    Interface *in = dtn_dict_remove(self->interfaces.ip, name);

    if (!dtn_thread_lock_unlock(&self->interfaces.lock_ip)) {
        dtn_log_warning("Failed to unlock IP interfaces.");
    }

    if (in) {

        in = interface_free(in);

        if (in) {
            dtn_garbadge_colloctor_push(self->garbadge, in, interface_free);
        }
    }
error:
    return;
}

/*---------------------------------------------------------------------------*/

static bool open_interface(dtn_socket_configuration socket,
                           dtn_file_node_core *self) {

    dtn_interface_ip_config config = (dtn_interface_ip_config){
        .loop = self->config.loop,
        .socket = socket,
        .limits.link_check = self->config.limits.link_check,
        .callbacks.userdata = self,
        .callbacks.io = interface_io,
        .callbacks.state = interface_state,
        .callbacks.close = interface_close};

    dtn_interface_ip *interface = dtn_interface_ip_create(config);

    if (!interface) {

        dtn_log_error("Failed to create interface |%s:%i - continue",
                      socket.host, socket.port);

        return true;
    }

    char *name = dtn_string_dup(socket.host);

    Interface *in = calloc(1, sizeof(Interface));
    if (!in)
        goto error;

    strncpy(in->name, name, DTN_HOST_NAME_MAX);
    in->interface = interface;

    if (!dtn_thread_lock_try_lock(&self->interfaces.lock_ip)) {

        dtn_log_error("Failed to look IP interfaces.");
        name = dtn_data_pointer_free(name);
        interface = dtn_interface_ip_free(interface);
        goto error;
    }

    bool result = dtn_dict_set(self->interfaces.ip, name, in, NULL);

    if (!dtn_thread_lock_unlock(&self->interfaces.lock_ip)) {
        dtn_log_error("Failed to unlook IP interfaces.");
    }

    if (!result) {

        name = dtn_data_pointer_free(name);
        in = interface_free(in);

        dtn_log_error("Failed to create interface %s:%i - continue",
                      socket.host, socket.port);

        return true;
    }

    dtn_log_info("created interface %s:%i", socket.host, socket.port);

error:
    return true;
}
/*---------------------------------------------------------------------------*/

struct container {

    const char *key;
    dtn_file_node_core *app;
};

/*---------------------------------------------------------------------------*/

static bool open_ip_interface(void *item, void *userdata) {

    struct container *container = (struct container *)userdata;

    const char *ip = dtn_item_get_string(item);
    if (!ip)
        goto error;

    dtn_socket_configuration socket = {0};
    strncpy(socket.host, ip, DTN_HOST_NAME_MAX);
    socket.port = 4556;
    socket.type = UDP;

    dtn_log_debug("opening ip interface at %s|%s:%i", container->key,
                  socket.host, socket.port);

    if (!open_interface(socket, container->app))
        goto error;
    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

static bool open_socket_interface(void *item, void *userdata) {

    dtn_file_node_core *self = dtn_file_node_core_cast(userdata);
    if (!item || !self)
        goto error;

    dtn_socket_configuration socket = dtn_socket_configuration_from_item(item);
    if (0 == socket.host[0])
        goto error;

    return open_interface(socket, self);
error:
    return false;
}

/*---------------------------------------------------------------------------*/

static bool open_local_interface(const char *key, dtn_item const *val,
                                 void *userdata) {

    if (!key)
        return true;

    dtn_file_node_core *self = dtn_file_node_core_cast(userdata);

    struct container container = (struct container){.key = key, .app = self};

    if (dtn_item_count(val) == 0) {
        dtn_log_debug("Skipping interface %s - no ip data", key);
        return true;
    }
    return dtn_item_array_for_each((dtn_item *)val, &container,
                                   open_ip_interface);
}

/*---------------------------------------------------------------------------*/

static bool open_all_interfaces(dtn_file_node_core *self) {

    dtn_item *interfaces = NULL;

    if (!self)
        goto error;

    interfaces = dtn_io_link_get_all_interfaces();

    if (!interfaces) {
        dtn_log_error("could not get host interfaces.");
        goto error;
    }

    bool result =
        dtn_item_object_for_each(interfaces, open_local_interface, self);

error:
    interfaces = dtn_item_free(interfaces);
    return result;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      CONFIG FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_file_node_core_enable_ip_interfaces(dtn_file_node_core *self,
                                             const dtn_item *input) {

    bool result = false;

    if (!self || !input)
        goto error;

    const dtn_item *config = dtn_item_get(input, "/dtn/node");
    if (!config)
        config = input;

    dtn_item *sockets = dtn_item_object_get(config, "sockets");
    if (!dtn_item_is_array(sockets)) {

        result = open_all_interfaces(self);

    } else {

        result = dtn_item_array_for_each(sockets, self, open_socket_interface);
    }

error:
    return result;
}

/*---------------------------------------------------------------------------*/

bool dtn_file_node_core_enable_routes(dtn_file_node_core *self,
                                      const char *path) {

    if (!self || !path)
        goto error;

    return dtn_routing_load(self->routing, path);
error:
    return false;
}

/*---------------------------------------------------------------------------*/

bool dtn_file_node_core_set_source_uri(dtn_file_node_core *self,
                                       const char *uri) {

    if (!self)
        goto error;

    self->uri = dtn_dtn_uri_free(self->uri);
    self->uri = dtn_dtn_uri_decode(uri);

    dtn_log_info("SET SOURCE URI TO %s", uri);

    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

bool dtn_file_node_core_set_reception_path(dtn_file_node_core *self,
                                           const char *path) {

    if (!self)
        goto error;

    self->path = dtn_data_pointer_free(self->path);
    self->path = dtn_string_dup(path);

    dtn_log_info("SET RECEPTION PATH TO %s", path);
    return true;
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      SEND FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------*/

static dtn_list *create_bundles_for_file(dtn_file_node_core *self,
                                         const dtn_dtn_uri *uri,
                                         const char *path, uint8_t *buffer,
                                         size_t size) {

    dtn_list *queue = NULL;
    char *source = NULL;
    dtn_cbor *payload = NULL;
    dtn_bundle *bundle = NULL;
    dtn_cbor *primary = NULL;
    dtn_cbor *bib = NULL;
    dtn_cbor *bcb = NULL;
    dtn_buffer *key = NULL;

    if (!self || !path || !buffer || size < 1)
        goto error;

    queue = dtn_linked_list_create(
        (dtn_list_config){.item.free = dtn_bundle_free_void});

    if (!queue)
        goto error;

    uint8_t *ptr = buffer;
    size_t chunk = 1000; // use a chunk size smaller Ethernet MTU
    size_t open = size;

    self->sequence++;

    uint64_t timestamp = dtn_time_get_current_time_usecs();
    uint64_t lifetime = 24 * 60 * 60 * 1000; // 24h

    char destination[2 * PATH_MAX];
    memset(destination, 0, 2 * PATH_MAX);

    char *uri_dest = dtn_dtn_uri_encode(uri);

    if (path[0] == '/') {

        snprintf(destination, 2 * PATH_MAX, "%s%s", uri_dest, path);

    } else {

        snprintf(destination, 2 * PATH_MAX, "%s/%s", uri_dest, path);
    }

    uri_dest = dtn_data_pointer_free(uri_dest);
    source = dtn_dtn_uri_encode(self->uri);

    char key_source[PATH_MAX];
    memset(key_source, 0, PATH_MAX);
    snprintf(key_source, PATH_MAX, "%s/%s", self->uri->name, self->uri->demux);

    key = dtn_key_store_get(self->keys, key_source);

    while (open - chunk > 0) {

        bundle = dtn_bundle_create();
        bib = NULL;
        bcb = NULL;

        if (!bundle)
            goto error;
        if (!dtn_list_queue_push(queue, bundle))
            goto error;

        primary = dtn_bundle_add_primary_block(
            bundle, 0x01, 0x00, destination, source, source, timestamp,
            self->sequence, lifetime, ptr - buffer, size);

        if (!primary)
            goto error;

        if (self->config.sec.bib.protect.header) {

            if (!key)
                goto error;

            bib = dtn_bundle_add_block(bundle, 11, 2, 0, 0,
                                       dtn_cbor_string("text"));
        }

        if (self->config.sec.bcb.protect.bib ||
            self->config.sec.bcb.protect.payload) {

            if (!key)
                goto error;

            bcb = dtn_bundle_add_block(bundle, 12, 3, 0, 0,
                                       dtn_cbor_string("text"));
        }

        payload = dtn_cbor_string("test");
        if (!dtn_cbor_set_byte_string(payload, ptr, chunk))
            goto error;

        dtn_cbor *payload_block =
            dtn_bundle_add_block(bundle, 0x01, 0x01, 0x00, 0x00, payload);

        if (!payload_block)
            goto error;

        payload = NULL;
        ptr = ptr + chunk;

        if (bib) {

            if (!dtn_bundle_bib_protect(bundle, bib, primary, key,
                                        self->config.sec.bib.aad_flags,
                                        self->config.sec.bib.sha, self->uri,
                                        self->config.sec.bib.new_key))
                goto error;
        }

        if (bcb) {

            if (self->config.sec.bcb.protect.bib) {

                if (!dtn_bundle_bcb_protect(bundle, bcb, bib, key, self->uri,
                                            self->config.sec.bcb.aad_flags,
                                            self->config.sec.bcb.aes,
                                            self->config.sec.bcb.new_key))
                    goto error;
            }

            if (self->config.sec.bcb.protect.payload) {

                if (!dtn_bundle_bcb_protect(
                        bundle, bcb, payload_block, key, self->uri,
                        self->config.sec.bcb.aad_flags,
                        self->config.sec.bcb.aes, self->config.sec.bcb.new_key))
                    goto error;
            }
        }

        int64_t check = open - chunk;
        if (check < 0)
            break;

        open = open - chunk;
        self->sequence++;
    }

    // add last block

    bib = NULL;
    bcb = NULL;

    self->sequence++;

    bundle = dtn_bundle_create();
    if (!bundle)
        goto error;
    if (!dtn_list_queue_push(queue, bundle))
        goto error;

    primary = dtn_bundle_add_primary_block(
        bundle, 0x01, 0x00, destination, source, source, timestamp,
        self->sequence, lifetime, ptr - buffer, size);

    if (!primary)
        goto error;

    if (self->config.sec.bib.protect.header) {

        if (!key)
            goto error;

        bib = dtn_bundle_add_block(bundle, 11, 2, 0, 0, dtn_cbor_string(NULL));
    }

    if (self->config.sec.bcb.protect.bib ||
        self->config.sec.bcb.protect.payload) {

        if (!key)
            goto error;

        bcb = dtn_bundle_add_block(bundle, 12, 3, 0, 0, dtn_cbor_string(NULL));
    }

    payload = dtn_cbor_string("test");
    if (!dtn_cbor_set_byte_string(payload, ptr, open))
        goto error;

    dtn_cbor *payload_block =
        dtn_bundle_add_block(bundle, 0x01, 0x01, 0x00, 0x00, payload);

    if (!payload_block)
        goto error;

    payload = NULL;

    if (bib) {

        if (!dtn_bundle_bib_protect(bundle, bib, primary, key,
                                    self->config.sec.bib.aad_flags,
                                    self->config.sec.bib.sha, self->uri,
                                    self->config.sec.bib.new_key))
            goto error;
    }

    if (bcb) {

        if (self->config.sec.bcb.protect.bib) {

            if (!dtn_bundle_bcb_protect(bundle, bcb, bib, key, self->uri,
                                        self->config.sec.bcb.aad_flags,
                                        self->config.sec.bcb.aes,
                                        self->config.sec.bcb.new_key))
                goto error;
        }

        if (self->config.sec.bcb.protect.payload) {

            if (!dtn_bundle_bcb_protect(
                    bundle, bcb, payload_block, key, self->uri,
                    self->config.sec.bcb.aad_flags, self->config.sec.bcb.aes,
                    self->config.sec.bcb.new_key))
                goto error;
        }
    }

    dtn_data_pointer_free(source);
    dtn_buffer_free(key);
    return queue;

error:
    dtn_bundle_free(bundle);
    dtn_list_free(queue);
    dtn_data_pointer_free(source);
    dtn_buffer_free(key);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_file_node_core_send_file(dtn_file_node_core *self, const char *uri,
                                  const char *source_path,
                                  const char *dest_path) {

    uint8_t out[2048];
    size_t out_size = 2048;
    memset(out, 0, out_size);

    dtn_list *queue = NULL;
    uint8_t *buffer = NULL;
    size_t size = 0;
    dtn_list *list = NULL;
    dtn_dtn_uri *destination = NULL;

    if (!self || !uri || !source_path || !dest_path)
        goto error;

    if (DTN_FILE_SUCCESS != dtn_file_read(source_path, &buffer, &size))
        goto error;

    destination = dtn_dtn_uri_decode(uri);
    if (!destination) {
        dtn_log_error("%s not a DTN uri", uri);
        goto error;
    }

    list = dtn_routing_get_info_for_uri(self->routing, destination);
    if (!list)
        goto error;

    queue = create_bundles_for_file(self, destination, dest_path, buffer, size);
    if (!queue)
        goto error;

    dtn_bundle *bundle = dtn_list_queue_pop(queue);

    while (bundle) {

        uint8_t *next = NULL;

        memset(out, 0, out_size);

        if (!dtn_bundle_encode(bundle, out, out_size, &next))
            goto error;

        dtn_routing_info *info = NULL;

        void *nxt = list->iter(list);

        while (nxt) {

            nxt = list->next(list, nxt, (void **)&info);

            if (!dtn_thread_lock_try_lock(&self->interfaces.lock_ip))
                continue;

            Interface *in = dtn_dict_get(self->interfaces.ip, info->interface);
            if (in) {

                dtn_thread_lock_try_lock(&in->lock);
            }

            dtn_thread_lock_unlock(&self->interfaces.lock_ip);

            bool result = dtn_interface_ip_send(in->interface, info->remote,
                                                out, next - out);

            dtn_thread_lock_unlock(&in->lock);

            if (result)
                dtn_log_debug("send bundle at %s to %s:%i", info->interface,
                              info->remote.host, info->remote.port);
        }

        bundle = dtn_bundle_free(bundle);
        bundle = dtn_list_queue_pop(queue);
    }

    dtn_log_info("Send file %s", source_path);

    queue = dtn_list_free(queue);
    buffer = dtn_data_pointer_free(buffer);
    destination = dtn_dtn_uri_free(destination);
    list = dtn_list_free(list);
    return true;
error:
    bundle = dtn_bundle_free(bundle);
    queue = dtn_list_free(queue);
    buffer = dtn_data_pointer_free(buffer);
    destination = dtn_dtn_uri_free(destination);
    list = dtn_list_free(list);

    return false;
}