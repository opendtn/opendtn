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
        @file           dtn_tunnel_app.c
        @author         Töpfer, Markus

        @date           2025-12-25


        ------------------------------------------------------------------------
*/
#include "../include/dtn_tunnel_app.h"



#include "../include/dtn_tunnel_app.h"
#include "../include/dtn_tunnel_core.h"

#include <dtn/dtn_cbor.h>

#include <dtn_base/dtn_string.h>
#include <dtn_core/dtn_app.h>
#include <dtn_core/dtn_event_api.h>
#include <dtn_core/dtn_socket_item.h>

#define DTN_TUNNEL_APP_MAGIC_BYTES 0xAFEE

/*---------------------------------------------------------------------------*/

struct dtn_tunnel_app {

    uint16_t magic_byte;
    dtn_tunnel_app_config config;

    int socket;

    dtn_app *app;

    dtn_socket_item *connections;
    dtn_tunnel_core *core;

};

/*---------------------------------------------------------------------------*/

static void cb_close(void *userdata, int socket){

    dtn_tunnel_app *self = dtn_tunnel_app_cast(userdata);
    if (!self) goto error;

    dtn_socket_item_drop(self->connections, socket);
error:  
    return;
}

/*---------------------------------------------------------------------------*/

static bool cb_login(dtn_tunnel_app *self, int socket, const dtn_item *msg, 
    dtn_item **out){

    dtn_item *answer = NULL;

    if (!self || socket < 1 || !msg || !out) goto error;

    const char *pass = dtn_item_get_string(dtn_item_get(msg, 
        "/parameter/password"));

    answer = dtn_event_message_create_response(msg);

    if (!pass){
        
        dtn_event_set_error(answer, 
            DTN_EVENT_ERROR_CODE_INPUT, DTN_EVENT_ERROR_DESC_INPUT);

    } else if (!dtn_password_check(pass, self->config.password)){

        dtn_event_set_error(answer, 
            DTN_EVENT_ERROR_CODE_AUTH, DTN_EVENT_ERROR_DESC_AUTH);
    
    } else {

        dtn_item *conn = dtn_item_object();
        dtn_item_object_set(conn, "auth", dtn_item_true());
        if (!dtn_socket_item_set(self->connections, socket, &conn)){
            conn = dtn_item_free(conn);
            dtn_event_set_error(answer, 
                DTN_EVENT_ERROR_CODE_AUTH, DTN_EVENT_ERROR_DESC_AUTH);
        }
    } 

    if (0 == dtn_event_get_error_code(answer)){
        dtn_log_info("LOGIN at socket %i", socket);
    } else {
        dtn_log_error("LOGIN failed at socket %i", socket);
    }

    *out = answer;
    return true;
error:
    
    return false;
}

/*---------------------------------------------------------------------------*/

static bool cb_load_routes(dtn_tunnel_app *self, int socket, const dtn_item *msg, 
    dtn_item **out){

    dtn_item *data = NULL;
    dtn_item *answer = NULL;

    if (!self || socket < 1 || !msg || !out) goto error;

    const char *path = dtn_item_get_string(dtn_item_get(msg, 
        "/parameter/path"));

    answer = dtn_event_message_create_response(msg);

    data = dtn_socket_item_get(self->connections, socket);
    if (!data){

        dtn_event_set_error(answer, 
            DTN_EVENT_ERROR_CODE_AUTH, DTN_EVENT_ERROR_DESC_AUTH);

        goto response;
    }

    if (!dtn_item_is_true(dtn_item_object_get(data, "auth"))){

        dtn_event_set_error(answer, 
            DTN_EVENT_ERROR_CODE_AUTH, DTN_EVENT_ERROR_DESC_AUTH);

        goto response;

    }

    if (!path){
        
        dtn_event_set_error(answer, 
            DTN_EVENT_ERROR_CODE_INPUT, DTN_EVENT_ERROR_DESC_INPUT);

    } else {

        if (!dtn_tunnel_core_enable_routes(self->core, path)){

            dtn_event_set_error(answer, 
                DTN_EVENT_ERROR_CODE_PROCESSING, 
                DTN_EVENT_ERROR_DESC_PROCESSING);
        }
    } 

    if (0 == dtn_event_get_error_code(answer)){
        dtn_log_info("load routes at socket %i", socket);
    } else {
        dtn_log_error("load routes failed at socket %i", socket);
    }

response:
    *out = answer;
    data = dtn_item_free(data);
    return true;
error:
    data = dtn_item_free(data);
    return false;
}

/*---------------------------------------------------------------------------*/

static bool cb_shutdown(dtn_tunnel_app *self, int socket, const dtn_item *msg, 
    dtn_item **out){

    dtn_item *data = NULL;
    dtn_item *answer = NULL;

    if (!self || socket < 1 || !msg || !out) goto error;

    data = dtn_socket_item_get(self->connections, socket);
    if (!data){

        dtn_event_set_error(answer, 
            DTN_EVENT_ERROR_CODE_AUTH, DTN_EVENT_ERROR_DESC_AUTH);

        goto response;
    }

    if (!dtn_item_is_true(dtn_item_object_get(data, "auth"))){

        dtn_event_set_error(answer, 
            DTN_EVENT_ERROR_CODE_AUTH, DTN_EVENT_ERROR_DESC_AUTH);

        goto response;

    }

    dtn_log_warning("GOING TO SHUTDOWN ON REQUEST.");
    dtn_event_loop_stop(self->config.loop);

response:
    *out = answer;
    data = dtn_item_free(data);
    return true;
error:
    data = dtn_item_free(data);
    return false;
}

/*---------------------------------------------------------------------------*/

static bool cb_app_generic(
    void *userdata, 
    int socket, 
    dtn_item *msg, 
    bool (*function)(dtn_tunnel_app *self, int socket, 
        const dtn_item *msg, dtn_item **out)){

    dtn_item *out = NULL;
    dtn_tunnel_app *self = dtn_tunnel_app_cast(userdata);
    if (!self || socket < 1 || !msg) goto error;

    bool result = function(self, socket, msg, &out);
    if (!result) goto error;
    
    if (out) dtn_app_send_json(self->app, socket, out);
    
    out = dtn_item_free(out);
    msg = dtn_item_free(msg);
    return true;
error:
    out = dtn_item_free(out);
    msg = dtn_item_free(msg);
    return false;
}

/*---------------------------------------------------------------------------*/

static bool cb_app_login(void *userdata, int socket, dtn_item *msg){

    return cb_app_generic(userdata, socket, msg, cb_login);
}

/*---------------------------------------------------------------------------*/

static bool cb_app_load_routes(void *userdata, int socket, dtn_item *msg){

    return cb_app_generic(userdata, socket, msg, cb_load_routes);
}

/*---------------------------------------------------------------------------*/

static bool cb_app_shutdown(void *userdata, int socket, dtn_item *msg){

    return cb_app_generic(userdata, socket, msg, cb_shutdown);
}

/*---------------------------------------------------------------------------*/

static bool register_app_callback(dtn_tunnel_app *self){

    if (!dtn_app_register(self->app,
        "login",
        cb_app_login,
        self)) goto error;

    if (!dtn_app_register(self->app,
        "load_routes",
        cb_app_load_routes,
        self)) goto error;

    if (!dtn_app_register(self->app,
        "shutdown",
        cb_app_shutdown,
        self)) goto error;

    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

static bool init_config(dtn_tunnel_app_config *config){

    if (!config || !config->loop || !config->io) goto error;

    if (0 == config->limits.threadlock_timeout_usec)
        config->limits.threadlock_timeout_usec = 100000;

    if (0 == config->limits.message_queue_capacity)
        config->limits.message_queue_capacity = 10000;

    if (0 == config->limits.cbor.string_size)
        config->limits.cbor.string_size = 1500;

    if (0 == config->limits.cbor.utf8_string_size)
        config->limits.cbor.utf8_string_size = 1500;

    if (0 == config->limits.cbor.array_size)
        config->limits.cbor.array_size = 1500;

    if (0 == config->limits.cbor.undef_length_array)
        config->limits.cbor.undef_length_array = 1500;

    if (0 == config->limits.cbor.map_size)
        config->limits.cbor.map_size = 1500;

    if (0 == config->limits.cbor.undef_length_map)
        config->limits.cbor.undef_length_map = 1500;

    if (0 == config->limits.threads){

        long numofcpus = sysconf(_SC_NPROCESSORS_ONLN);
        config->limits.threads = numofcpus;

    }

    if (0 == config->name[0]) strncpy(config->name, "localhost", 10);

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

dtn_tunnel_app *dtn_tunnel_app_create(dtn_tunnel_app_config config){

    dtn_tunnel_app *self = NULL;
    if (!init_config(&config)) goto error;

    self = calloc(1, sizeof(dtn_tunnel_app));
    if (!self) goto error;

    self->magic_byte = DTN_TUNNEL_APP_MAGIC_BYTES;
    self->config = config;

    dtn_socket_item_config conn = (dtn_socket_item_config){
        .loop = config.loop,
        .limits.threadlock_timeout_usec = config.limits.threadlock_timeout_usec
    };

    self->connections = dtn_socket_item_create(conn);
    if (!self->connections) goto error;

    dtn_app_config app_config = (dtn_app_config){
        .loop = config.loop,
        .io = config.io,
        .register_client = false,
        .limits.threadlock_timeout_usec = config.limits.threadlock_timeout_usec,
        .limits.message_queue_capacity = config.limits.message_queue_capacity,
        .limits.threads = config.limits.threads
    };
    strncpy(app_config.name, config.name, PATH_MAX);

    self->app = dtn_app_create(app_config);
    if (!self->app) goto error;

    self->socket = dtn_app_open_listener(self->app, config.socket);

    if (-1 == self->socket){
        
        dtn_log_error("Failed to open command and control socket %s:%i",
            self->config.socket.host,
            self->config.socket.port);

        goto error;
    
    } else {

        dtn_log_info("created C&C listener at %s:%i",
            self->config.socket.host,
            self->config.socket.port);
    }

    if (!register_app_callback(self)) goto error;
    dtn_app_set_debug(self->app, false);

    if (!dtn_app_register_close(self->app,
        self, cb_close)) goto error;

    dtn_tunnel_core_config core = (dtn_tunnel_core_config){
        .loop = config.loop,
        .tunnel = config.tunnel,
        .remote = config.remote,
        .limits.threadlock_timeout_usec = config.limits.threadlock_timeout_usec,
        .limits.message_queue_capacity = config.limits.message_queue_capacity,
        .limits.threads = config.limits.threads,
        .limits.buffer_time_cleanup_usecs = config.limits.buffer_time_cleanup_usecs,
        .limits.max_buffer_time_secs = config.limits.max_buffer_time_secs,
        .limits.history_secs = config.limits.history_secs
    };

    self->core = dtn_tunnel_core_create(core);
    if (!self->core) goto error;

    if (0 != self->config.uri[0])
        dtn_tunnel_core_set_source_uri(self->core, self->config.uri);

    if (0 != self->config.destination_uri[0])
        dtn_tunnel_core_set_destination_uri(self->core, self->config.destination_uri);

    dtn_cbor_config cbor_config = (dtn_cbor_config){

        .limits.string_size = config.limits.cbor.string_size,
        .limits.utf8_string_size = config.limits.cbor.utf8_string_size,
        .limits.array_size = config.limits.cbor.array_size,
        .limits.undef_length_array = config.limits.cbor.undef_length_array,
        .limits.map_size = config.limits.cbor.map_size,
        .limits.undef_length_map = config.limits.cbor.undef_length_map
    };

    dtn_cbor_configure(cbor_config);
    
    return self;
error:
    dtn_tunnel_app_free(self);
    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_tunnel_app *dtn_tunnel_app_free(dtn_tunnel_app *self){

    if (!dtn_tunnel_app_cast(self)) return self;

    self->core = dtn_tunnel_core_free(self->core);
    self->app = dtn_app_free(self->app);
    self->connections = dtn_socket_item_free(self->connections);
    self = dtn_data_pointer_free(self);
    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_tunnel_app *dtn_tunnel_app_cast(const void *data){

    if (!data) return NULL;

    if (*(uint16_t *)data != DTN_TUNNEL_APP_MAGIC_BYTES)
        return NULL;

    return (dtn_tunnel_app *)data;
}

/*---------------------------------------------------------------------------*/

dtn_tunnel_app_config dtn_tunnel_app_config_from_item(const dtn_item *input){

    dtn_tunnel_app_config config = {0};

    const dtn_item *conf = dtn_item_get(input, "/dtn/node");
    if (!conf) conf = input;

    config.limits.threadlock_timeout_usec = dtn_item_get_number(
        dtn_item_get(conf, "threadlock_timeout_usec"));

    config.limits.message_queue_capacity = dtn_item_get_number(
        dtn_item_get(conf, "message_queue_capacity"));

    config.limits.link_check = dtn_item_get_number(
        dtn_item_get(conf, "link_check_usec"));

    config.limits.threads = dtn_item_get_number(
        dtn_item_get(conf, "threads"));

    config.limits.buffer_time_cleanup_usecs = dtn_item_get_number(
        dtn_item_get(conf, "buffer_time_cleanup_usecs"));

    config.limits.max_buffer_time_secs = dtn_item_get_number(
        dtn_item_get(conf, "max_buffer_time_secs"));

    config.limits.history_secs = dtn_item_get_number(
        dtn_item_get(conf, "history_secs"));

    const dtn_item *cbor = dtn_item_get(conf, "/cbor");

    config.limits.cbor.string_size = dtn_item_get_number(
        dtn_item_get(cbor, "string_size"));

    config.limits.cbor.utf8_string_size = dtn_item_get_number(
        dtn_item_get(cbor, "utf8_string_size"));

    config.limits.cbor.array_size = dtn_item_get_number(
        dtn_item_get(cbor, "array_size"));

    config.limits.cbor.undef_length_array = dtn_item_get_number(
        dtn_item_get(cbor, "undef_length_array"));

    config.limits.cbor.map_size = dtn_item_get_number(
        dtn_item_get(cbor, "map_size"));

    config.limits.cbor.undef_length_map = dtn_item_get_number(
        dtn_item_get(cbor, "undef_length_map"));

    config.socket = dtn_socket_configuration_from_item(
        dtn_item_object_get(conf, "socket"));

    config.password = dtn_password_from_item(
        dtn_item_object_get(conf, "password"));

    dtn_item *tunnel = dtn_item_get(input, "/dtn/tunnel");

    config.tunnel = dtn_socket_configuration_from_item(
        dtn_item_object_get(tunnel, "socket"));

    config.remote = dtn_socket_configuration_from_item(
        dtn_item_object_get(tunnel, "remote"));

    const char *str = dtn_item_get_string(dtn_item_get(tunnel, "/destination"));
    if (str) strncpy(config.destination_uri, str, PATH_MAX);

    str = dtn_item_get_string(dtn_item_get(conf, "/uri"));
    if (str) strncpy(config.uri, str, PATH_MAX);

    return config;
}

/*---------------------------------------------------------------------------*/

bool dtn_tunnel_app_enable_ip_interfaces(
    dtn_tunnel_app *self, const dtn_item *input){

    return dtn_tunnel_core_enable_ip_interfaces(
        self->core, input);
}

/*---------------------------------------------------------------------------*/

bool dtn_tunnel_app_enable_routes(
    dtn_tunnel_app *self, const char *path){

    return dtn_tunnel_core_enable_routes(
        self->core, path);
}