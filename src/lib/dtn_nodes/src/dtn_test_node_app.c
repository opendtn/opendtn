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
        @file           dtn_test_node_app.c
        @author         Töpfer, Markus

        @date           2025-12-18


        ------------------------------------------------------------------------
*/
#include "../include/dtn_test_node_app.h"
#include "../include/dtn_test_node_core.h"
#include <dtn/dtn_cbor.h>

#include <dtn_base/dtn_string.h>
#include <dtn_core/dtn_app.h>
#include <dtn_core/dtn_event_api.h>
#include <dtn_core/dtn_socket_item.h>

#define DTN_TEST_NODE_APP_MAGIC_BYTE 0x934A

/*---------------------------------------------------------------------------*/

struct dtn_test_node_app {

    uint16_t magic_byte;
    dtn_test_node_app_config config;

    int socket;

    dtn_app *app;

    dtn_socket_item *connections;
    dtn_test_node_core *core;

};

/*---------------------------------------------------------------------------*/

static void cb_close(void *userdata, int socket){

    dtn_test_node_app *self = dtn_test_node_app_cast(userdata);
    if (!self) goto error;

    dtn_socket_item_drop(self->connections, socket);
error:  
    return;
}

/*---------------------------------------------------------------------------*/

static bool cb_login(dtn_test_node_app *self, int socket, const dtn_item *msg, 
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

static uint64_t get_uint_from_string(const dtn_item *source){

    uint64_t out = 0;
    const char *str = dtn_item_get_string(source);
    if (!str) goto error;

    dtn_convert_hex_string_to_uint64(str, strlen(str), &out);

    return out;
error:
    return 0;
}

/*---------------------------------------------------------------------------*/

static void set_primary(dtn_cbor *cbor, const dtn_item *source){

    uint64_t hex = 0;
    dtn_cbor *val = NULL;
    const char *str = NULL;
    dtn_item *item = NULL;

    if (!cbor || !source) goto error;

    dtn_cbor *arr = dtn_cbor_array();
    dtn_cbor_array_push(cbor, arr);

    item = dtn_item_object_get(source, "version");
    if (item){
        hex = get_uint_from_string(item);
        val = dtn_cbor_uint(hex);
        dtn_cbor_array_push(arr, val);
    }

    item = dtn_item_object_get(source, "flags");
    if (item){
        hex = get_uint_from_string(item);
        val = dtn_cbor_uint(hex);
        dtn_cbor_array_push(arr, val);
    }

    item = dtn_item_object_get(source, "crc_type");
    if (item){
        hex = get_uint_from_string(item);
        val = dtn_cbor_uint(hex);
        dtn_cbor_array_push(arr, val);
    }

    item = dtn_item_object_get(source, "dest");
    if (item){
        str = dtn_item_get_string(item);
        val = dtn_cbor_string(str);
        dtn_cbor_array_push(arr, val);
    }

    item = dtn_item_object_get(source, "source");
    if (item){
        str = dtn_item_get_string(item);
        val = dtn_cbor_string(str);
        dtn_cbor_array_push(arr, val);
    }

    item = dtn_item_object_get(source, "report");
    if (item){
        str = dtn_item_get_string(item);
        val = dtn_cbor_string(str);
        dtn_cbor_array_push(arr, val);
    }

    dtn_cbor *timestamp = dtn_cbor_array();
    dtn_cbor_array_push(arr, timestamp);

    item = dtn_item_object_get(source, "timestamp");
    if (item){
        hex = get_uint_from_string(item);
        val = dtn_cbor_uint(hex);
        dtn_cbor_array_push(timestamp, val);
    }

    item = dtn_item_object_get(source, "sequence");
    if (item){
        hex = get_uint_from_string(item);
        val = dtn_cbor_uint(hex);
        dtn_cbor_array_push(timestamp, val);
    }

    item = dtn_item_object_get(source, "lifetime");
    if (item){
        hex = get_uint_from_string(item);
        val = dtn_cbor_uint(hex);
        dtn_cbor_array_push(arr, val);
    }

    item = dtn_item_object_get(source, "fragment");
    if (item){
        hex = get_uint_from_string(item);
        val = dtn_cbor_uint(hex);
        dtn_cbor_array_push(arr, val);
    }

    item = dtn_item_object_get(source, "total_data");
    if (item){
        hex = get_uint_from_string(item);
        val = dtn_cbor_uint(hex);
        dtn_cbor_array_push(arr, val);
    }

    item = dtn_item_object_get(source, "crc");
    if (item){
        hex = get_uint_from_string(item);
        val = dtn_cbor_uint(hex);
        dtn_cbor_array_push(arr, val);
    }

error:
    return;
}

/*---------------------------------------------------------------------------*/

static bool add_block(void *val, void *data){

    uint64_t hex = 0;
    dtn_item *item = NULL;
    const char *str = NULL;

    dtn_item *source = (dtn_item*) val;

    dtn_cbor *cbor = (dtn_cbor*) data;
    dtn_cbor *arr = dtn_cbor_array();
    dtn_cbor_array_push(cbor, arr);

    item = dtn_item_object_get(source, "code");
    if (item){
        hex = get_uint_from_string(item);
        val = dtn_cbor_uint(hex);
        dtn_cbor_array_push(arr, val);
    }

    item = dtn_item_object_get(source, "num");
    if (item){
        hex = get_uint_from_string(item);
        val = dtn_cbor_uint(hex);
        dtn_cbor_array_push(arr, val);
    }

    item = dtn_item_object_get(source, "flags");
    if (item){
        hex = get_uint_from_string(item);
        val = dtn_cbor_uint(hex);
        dtn_cbor_array_push(arr, val);
    }

    item = dtn_item_object_get(source, "crc_type");
    if (item){
        hex = get_uint_from_string(item);
        val = dtn_cbor_uint(hex);
        dtn_cbor_array_push(arr, val);
    }

    item = dtn_item_object_get(source, "data");
    if (item){
        str = dtn_item_get_string(item);
        val = dtn_cbor_string(str);
        dtn_cbor_array_push(arr, val);
    }

    item = dtn_item_object_get(source, "crc");
    if (item){
        hex = get_uint_from_string(item);
        val = dtn_cbor_uint(hex);
        dtn_cbor_array_push(arr, val);
    }

    return true;
}

/*---------------------------------------------------------------------------*/

static void set_blocks(dtn_cbor *cbor, const dtn_item *source){

    if (!cbor || !source) goto error;

    dtn_item_array_for_each((dtn_item*)source, cbor, add_block);

error:
    return;
}

/*---------------------------------------------------------------------------*/

static void set_payload(dtn_cbor *cbor, const dtn_item *source){

    if (!cbor || !source) goto error;

    add_block((void*)source, cbor);

error:
    return;
}

/*---------------------------------------------------------------------------*/

static dtn_cbor *get_cbor_from_json(const dtn_item *item){

    dtn_cbor *cbor = NULL;
    if (!item) goto error;

    cbor = dtn_cbor_array();

    const dtn_item *primary = dtn_item_object_get(item, "primary");
    const dtn_item *payload = dtn_item_object_get(item, "payload");
    const dtn_item *blocks = dtn_item_object_get(item, "blocks");

    set_primary(cbor, primary);
    set_blocks(cbor, blocks);
    set_payload(cbor, payload);

    return cbor;
error:
    return NULL;
}

/*---------------------------------------------------------------------------*/

static bool cb_bundle_send_request(dtn_test_node_app *self, int socket, const dtn_item *msg, 
    dtn_item **out){

    dtn_item *answer = NULL;

    if (!self || socket < 1 || !msg || !out) goto error;

    answer = dtn_event_message_create_response(msg);

    dtn_socket_configuration socket_config = dtn_socket_configuration_from_item(
        dtn_item_get(msg, "/parameter/socket"));
    
    socket_config.type = UDP;

    dtn_cbor *cbor = get_cbor_from_json(dtn_item_get(msg, "/parameter"));

    if (0 == socket_config.host[0]){

        dtn_event_set_error(answer, 
            DTN_EVENT_ERROR_CODE_INPUT, DTN_EVENT_ERROR_DESC_INPUT);
        
        goto done;
    }

    if (!cbor){

        dtn_event_set_error(answer, 
            DTN_EVENT_ERROR_CODE_PROCESSING, DTN_EVENT_ERROR_DESC_PROCESSING);
        
        goto done;
    }

    if (!dtn_test_node_core_send_raw(self->core, socket_config, cbor)){

        dtn_event_set_error(answer, 
            DTN_EVENT_ERROR_CODE_SEND, DTN_EVENT_ERROR_DESC_SEND);

        goto done;
    }

done:
    
    cbor = dtn_cbor_free(cbor);

    if (0 == dtn_event_get_error_code(answer)){
        dtn_log_info("SUCCESS bundle send request at socket %i", socket);
    } else {
        dtn_log_error("FAILURE bundle send request at socket %i", socket);
    }

    *out = answer;
    return true;
error:
    
    return false;
}
/*---------------------------------------------------------------------------*/

static bool cb_app_generic(
    void *userdata, 
    int socket, 
    dtn_item *msg, 
    bool (*function)(dtn_test_node_app *self, int socket, 
        const dtn_item *msg, dtn_item **out)){

    dtn_item *out = NULL;
    dtn_test_node_app *self = dtn_test_node_app_cast(userdata);
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

static bool cb_app_bundle_send_request(void *userdata, int socket, dtn_item *msg){

    return cb_app_generic(userdata, socket, msg, cb_bundle_send_request);
}

/*---------------------------------------------------------------------------*/

static bool register_app_callback(dtn_test_node_app *self){

    if (!dtn_app_register(self->app,
        "login",
        cb_app_login,
        self)) goto error;

    if (!dtn_app_register(self->app,
        "bundle_send_request",
        cb_app_bundle_send_request,
        self)) goto error;

    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

static bool init_config(dtn_test_node_app_config *config){

    if (!config || !config->loop || !config->io) goto error;

    if (!config->server){
        dtn_log_info("NO webserver input from node, disable web interface.");
    }

    if (0 == config->limits.threadlock_timeout_usec)
        config->limits.threadlock_timeout_usec = 100000;

    if (0 == config->limits.message_queue_capacity)
        config->limits.message_queue_capacity = 10000;

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

dtn_test_node_app *dtn_test_node_app_create(dtn_test_node_app_config config){

    dtn_test_node_app *self = NULL;
    if (!init_config(&config)) goto error;

    self = calloc(1, sizeof(dtn_test_node_app));
    if (!self) goto error;

    self->magic_byte = DTN_TEST_NODE_APP_MAGIC_BYTE;
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

    if (!dtn_webserver_register_close(self->config.server,
        self, cb_close)) goto error;

    if (!dtn_app_register_close(self->app,
        self, cb_close)) goto error;

    dtn_test_node_core_config core = (dtn_test_node_core_config){
        .loop = config.loop,
        .limits.threadlock_timeout_usec = config.limits.threadlock_timeout_usec,
        .limits.message_queue_capacity = config.limits.message_queue_capacity,
        .limits.threads = config.limits.threads
    };

    self->core = dtn_test_node_core_create(core);
    if (!self->core) goto error;
    
    return self;
error:
    dtn_test_node_app_free(self);
    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_test_node_app *dtn_test_node_app_free(dtn_test_node_app *self){

    if (!dtn_test_node_app_cast(self)) return self;

    self->core = dtn_test_node_core_free(self->core);
    self->app = dtn_app_free(self->app);
    self->connections = dtn_socket_item_free(self->connections);
    self = dtn_data_pointer_free(self);
    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_test_node_app *dtn_test_node_app_cast(const void *data){

    if (!data) return NULL;

    if (*(uint16_t *)data != DTN_TEST_NODE_APP_MAGIC_BYTE)
        return NULL;

    return (dtn_test_node_app *)data;
}

/*---------------------------------------------------------------------------*/

dtn_test_node_app_config dtn_test_node_app_config_from_item(const dtn_item *input){

    dtn_test_node_app_config config = {0};

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

    config.socket = dtn_socket_configuration_from_item(
        dtn_item_object_get(conf, "socket"));

    config.password = dtn_password_from_item(
        dtn_item_object_get(conf, "password"));

    return config;
}

/*---------------------------------------------------------------------------*/

void dtn_test_node_app_websocket_callback(
    void *userdata, int socket, dtn_item *message){

    // Websocket enabled events NOTE send must be done via webserver
    dtn_item *out = NULL;

    dtn_test_node_app *self = dtn_test_node_app_cast(userdata);
    if (!self || socket < 1 || !message) goto error;

    if (!self->config.server){

        // NOTE we need a webserver to answer. 
        // So without that we just ignore

        dtn_log_error("WEB IO received but no server configured - ignoring");
        goto error;
    }

    bool result = false;

    if (dtn_event_is(message, "login")){

        result = cb_login(self, socket, message, &out);

    } else if (dtn_event_is(message, "bundle_send_request")){

        result = cb_bundle_send_request(self, socket, message, &out);

    }

    if (!result) goto error;

    result &= dtn_webserver_send(self->config.server, socket, out);

error:
    dtn_item_free(out);
    dtn_item_free(message);
    return;
}

/*---------------------------------------------------------------------------*/

bool dtn_test_node_enable_ip_interfaces(
    dtn_test_node_app *self, const dtn_item *input){

    return dtn_test_node_core_enable_ip_interfaces(
        self->core, input);
}

   