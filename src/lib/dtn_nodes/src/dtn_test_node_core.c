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
        @file           dtn_test_node_core.c
        @author         Töpfer, Markus

        @date           2025-12-19


        ------------------------------------------------------------------------
*/
#include "../include/dtn_test_node_core.h"
#include <dtn/dtn_interface_ip.h>

#include <dtn_base/dtn_utils.h>
#include <dtn_base/dtn_thread_loop.h>
#include <dtn_base/dtn_thread_lock.h>
#include <dtn_base/dtn_thread_message.h>
#include <dtn_base/dtn_string.h>
#include <dtn_base/dtn_dict.h>
#include <dtn_base/dtn_garbadge_colloctor.h>

/*---------------------------------------------------------------------------*/

#define DTN_TEST_NODE_CORE_MAGIC_BYTE 0xc053

typedef enum ThreadMessageType {

    BUNDLE_IO = 0,
    STATE_CHANGE

}ThreadMessageType;

/*---------------------------------------------------------------------------*/

struct dtn_test_node_core {

    uint16_t magic_byte;
    dtn_test_node_core_config config;

    dtn_garbadge_colloctor *garbadge;

    dtn_thread_loop *tloop;

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

static void *interface_free(void *data){

    if (!data) return NULL;

    Interface *self = (Interface*) data;
    if (!dtn_thread_lock_try_lock(&self->lock)) goto error;
    self->interface = dtn_interface_ip_free(self->interface);
    if (!dtn_thread_lock_unlock(&self->lock)){
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

static dtn_thread_message *thread_message_free(dtn_thread_message *self){

    if (self->type != 1) return self;

    Threadmessage *msg = (Threadmessage*) self;
    msg->bundle = dtn_bundle_free(msg->bundle);
    msg->interface = dtn_data_pointer_free(msg->interface);
    msg = dtn_data_pointer_free(msg);
    return NULL;
}

/*----------------------------------------------------------------------------*/

static dtn_thread_message *thread_message_state_create(const char *name, dtn_ip_link_state state){

    Threadmessage *msg = calloc(1, sizeof(Threadmessage));
    if (!msg) return NULL;
    
    msg->generic.magic_bytes = DTN_THREAD_MESSAGE_MAGIC_BYTES;
    msg->generic.type = 1;
    msg->generic.free = thread_message_free;

    msg->type = STATE_CHANGE;
    msg->state = state;
    msg->interface = dtn_string_dup(name);

    return dtn_thread_message_cast(msg);
}

/*----------------------------------------------------------------------------*/

static dtn_thread_message *thread_message_create(
    dtn_bundle *bundle, const dtn_socket_data *remote, const char *name){

    Threadmessage *msg = calloc(1, sizeof(Threadmessage));
    if (!msg) return NULL;
    
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

static bool handle_in_loop(dtn_thread_loop *tloop, dtn_thread_message *msg){

    dtn_test_node_core *self = dtn_thread_loop_get_data(tloop);
    if (!self || !msg) goto error;

    TODO("... to be implemented.");

    dtn_thread_message_free(msg);
    return true;
error:
    dtn_thread_message_free(msg);
    return false;
}

/*---------------------------------------------------------------------------*/

static bool message_bundle_process(dtn_test_node_core *self, Threadmessage *msg){

    if (!self || !msg) goto error;

    dtn_log_debug("THREAD IO at %s from %s:%i", msg->interface, 
        msg->remote.host, msg->remote.port);

    fprintf(stderr, "DUMP INCOMING BUNDLE\n");
    dtn_bundle_dump(stderr, msg->bundle);
    fprintf(stderr, "\n");

    dtn_thread_message_free(dtn_thread_message_cast(msg));
    return true;

error:
    dtn_thread_message_free(dtn_thread_message_cast(msg));
    return false;

}

/*---------------------------------------------------------------------------*/

static bool message_state_change_process(dtn_test_node_core *self, Threadmessage *msg){

    if (!self || !msg) goto error;

    const char *string = NULL;

    switch(msg->state){

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

static bool handle_in_thread(dtn_thread_loop *tloop, dtn_thread_message *msg){

    dtn_test_node_core *self = dtn_thread_loop_get_data(tloop);
    if (!self || !msg) goto error;

    if (msg->type != 1) goto error;

    Threadmessage *message = (Threadmessage*) msg;

    switch(message->type){

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

static bool init_config(dtn_test_node_core_config *config){

    if (!config || !config->loop) goto error;

    if (0 == config->limits.threadlock_timeout_usec)
        config->limits.threadlock_timeout_usec = 100000;

    if (0 == config->limits.message_queue_capacity)
        config->limits.message_queue_capacity = 10000;

    if (0 == config->limits.link_check)
        config->limits.link_check = 1000000;

    if (0 == config->limits.threads){

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

dtn_test_node_core *dtn_test_node_core_create(dtn_test_node_core_config config){

    dtn_test_node_core *self = NULL;
    
    if (!init_config(&config)) goto error;

    self = calloc(1, sizeof(dtn_test_node_core));
    if (!self) goto error;

    self->magic_byte = DTN_TEST_NODE_CORE_MAGIC_BYTE;
    self->config = config;

    self->tloop = dtn_thread_loop_create(self->config.loop,
        (dtn_thread_loop_callbacks){
            .handle_message_in_thread = handle_in_thread,
            .handle_message_in_loop = handle_in_loop
        },
        self);

    if (!self->tloop) goto error;
    if (!dtn_thread_loop_reconfigure(self->tloop,
        (dtn_thread_loop_config){
            .disable_to_loop_queue = false,
            .message_queue_capacity = config.limits.message_queue_capacity,
            .lock_timeout_usecs = config.limits.threadlock_timeout_usec,
            .num_threads = config.limits.threads
        })) goto error;

    if (!dtn_thread_loop_start_threads(self->tloop)) goto error;

    dtn_dict_config d_config = dtn_dict_string_key_config(255);
    d_config.value.data_function.free = interface_free;

    self->interfaces.ip = dtn_dict_create(d_config);
    if (!self->interfaces.ip) goto error;

    if (!dtn_thread_lock_init(&self->interfaces.lock_ip, 
        self->config.limits.threadlock_timeout_usec))
        goto error;

    self->garbadge = dtn_garbadge_colloctor_create((dtn_garbadge_colloctor_config){
        .loop = config.loop,
        .limits.threadlock_timeout_usec = config.limits.threadlock_timeout_usec,
        .limits.run_cleanup_usec = 1000000 // 1 second
    });

    if (!self->garbadge) goto error;

    return self;
error:
    dtn_test_node_core_free(self);
    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_test_node_core *dtn_test_node_core_free(dtn_test_node_core *self){

    if (!dtn_test_node_core_cast(self)) return self;

    dtn_thread_lock_clear(&self->interfaces.lock_ip);

    self->garbadge = dtn_garbadge_colloctor_free(self->garbadge);
    self->interfaces.ip = dtn_dict_free(self->interfaces.ip);
    self->tloop = dtn_thread_loop_free(self->tloop);
    self = dtn_data_pointer_free(self);
    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_test_node_core *dtn_test_node_core_cast(const void *data){

    if (!data) return NULL;

    if (*(uint16_t *)data != DTN_TEST_NODE_CORE_MAGIC_BYTE)
        return NULL;

    return (dtn_test_node_core *)data;
}

/*---------------------------------------------------------------------------*/

static void interface_io(void *userdata, const dtn_socket_data *remote, 
    dtn_bundle *bundle, const char *name){

    dtn_test_node_core *self = dtn_test_node_core_cast(userdata);
    if (!self || !remote || !bundle || !name) return;

    dtn_log_debug("IO at %s from %s:%i", name, remote->host, remote->port);

    dtn_thread_message *msg = thread_message_create(bundle, remote, name);
    if (!msg) goto error;
    
    bundle = NULL;

    if (!dtn_thread_loop_send_message(self->tloop, msg, DTN_RECEIVER_THREAD)){
        msg = dtn_thread_message_free(msg);
    }    

error:
    dtn_bundle_free(bundle);
    return;
}

/*---------------------------------------------------------------------------*/

static void interface_state(void *userdata, 
    dtn_ip_link_state state, const char *name){

    dtn_test_node_core *self = dtn_test_node_core_cast(userdata);
    if (!self || !name) return;

    const char *string = NULL;

    switch(state){

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
    if (!in) goto done;

    if (!dtn_thread_lock_try_lock(&in->lock)) goto done;
    in->state = state;
    if (!dtn_thread_lock_unlock(&in->lock)){
        dtn_log_error("failed to unlock interface.");
    }

    dtn_thread_message *msg = thread_message_state_create(name, state);
    if (!msg) goto done;

    if (!dtn_thread_loop_send_message(self->tloop, msg, DTN_RECEIVER_THREAD)){
        msg = dtn_thread_message_free(msg);
    }

done:
    return;
}

/*---------------------------------------------------------------------------*/

static void interface_close(void *userdata, const char *name){

    dtn_test_node_core *self = dtn_test_node_core_cast(userdata);
    if (!userdata || !name) return;

    dtn_log_error("Interface %s lost", name);
    if (!dtn_thread_lock_try_lock(&self->interfaces.lock_ip)){
        dtn_log_error("Failed to lock IP interfaces.");
        goto error;
    }

    Interface *in = dtn_dict_remove(self->interfaces.ip, name);

    if (!dtn_thread_lock_unlock(&self->interfaces.lock_ip)){
        dtn_log_warning("Failed to unlock IP interfaces.");
    }

    if(in){

        in = interface_free(in);

        if (in){
            dtn_garbadge_colloctor_push(self->garbadge, in, interface_free);
        }

    }
error:
    return;
}

/*---------------------------------------------------------------------------*/

static bool open_interface(dtn_socket_configuration socket, 
    dtn_test_node_core *self){

    dtn_interface_ip_config config = (dtn_interface_ip_config){
        .loop = self->config.loop,
        .socket = socket,
        .limits.link_check = self->config.limits.link_check,
        .callbacks.userdata = self,
        .callbacks.io = interface_io,
        .callbacks.state = interface_state,
        .callbacks.close = interface_close
    };

    dtn_interface_ip *interface = dtn_interface_ip_create(config);
    
    if (!interface){
    
        dtn_log_error("Failed to create interface |%s:%i - continue",
            socket.host,
            socket.port);

        return true;
    
    }

    char *name = dtn_string_dup(socket.host);

    Interface *in = calloc(1, sizeof(Interface));
    if (!in) goto error;

    strncpy(in->name, name, DTN_HOST_NAME_MAX);
    in->interface = interface;

    if (!dtn_thread_lock_try_lock(&self->interfaces.lock_ip)){

        dtn_log_error("Failed to look IP interfaces.");
        name =dtn_data_pointer_free(name);
        interface = dtn_interface_ip_free(interface);
        goto error;
    }

    bool result = dtn_dict_set(self->interfaces.ip,
        name, in, NULL);

    if (!dtn_thread_lock_unlock(&self->interfaces.lock_ip)){
        dtn_log_error("Failed to unlook IP interfaces.");
    }

    if (!result){

        name =dtn_data_pointer_free(name);
        in = interface_free(in);

        dtn_log_error("Failed to create interface %s:%i - continue",
            socket.host,
            socket.port);

        return true;

    }

    dtn_log_info("created interface %s:%i",
            socket.host,
            socket.port);

error:
    return true;
}
/*---------------------------------------------------------------------------*/

struct container {

    const char *key;
    dtn_test_node_core *app;
};

/*---------------------------------------------------------------------------*/

static bool open_ip_interface(void *item, void *userdata){

    struct container *container = (struct container*) userdata;

    const char *ip = dtn_item_get_string(item);
    if (!ip) goto error;

    dtn_socket_configuration socket = {0};
    strncpy(socket.host, ip, DTN_HOST_NAME_MAX);
    socket.port = 4556;
    socket.type = UDP;

    dtn_log_debug("opening ip interface at %s|%s:%i",
        container->key,
        socket.host,
        socket.port);

    if (!open_interface(socket, container->app)) goto error;
    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

static bool open_socket_interface(void *item, void *userdata){

    dtn_test_node_core *self = dtn_test_node_core_cast(userdata);
    if (!item || !self) goto error;

    dtn_socket_configuration socket = dtn_socket_configuration_from_item(item);
    if (0 == socket.host[0]) goto error;

    return open_interface(socket, self);
error:
    return false;
}

/*---------------------------------------------------------------------------*/

static bool open_local_interface(const char *key, dtn_item const *val, void *userdata){

    if (!key) return true;

    dtn_test_node_core *self = dtn_test_node_core_cast(userdata);

    struct container container = (struct container){
        .key = key,
        .app = self
    };

    if (dtn_item_count(val) == 0){
        dtn_log_debug("Skipping interface %s - no ip data", key);
        return true;
    }
    return dtn_item_array_for_each((dtn_item*) val, &container, open_ip_interface);
}

/*---------------------------------------------------------------------------*/

static bool open_all_interfaces(dtn_test_node_core *self){

    dtn_item *interfaces = NULL;

    if (!self) goto error;

    interfaces = dtn_io_link_get_all_interfaces();

    if (!interfaces){
        dtn_log_error("could not get host interfaces.");
        goto error;
    }

    bool result = dtn_item_object_for_each(
        interfaces, open_local_interface, self);

error:
    interfaces = dtn_item_free(interfaces);
    return result;
}

/*---------------------------------------------------------------------------*/

bool dtn_test_node_core_enable_ip_interfaces(
        dtn_test_node_core *self,
        const dtn_item *input){

    bool result = false;

    if (!self || !input) goto error;

    const dtn_item *config = dtn_item_get(input, "/dtn/node");
    if (!config) config = input;

    dtn_item *sockets = dtn_item_object_get(config, "sockets");
    if (!dtn_item_is_array(sockets)){

        result = open_all_interfaces(self);
    
    } else {

        result = dtn_item_array_for_each(
            sockets, 
            self, 
            open_socket_interface);

    }

error:
    return result;
}

/*---------------------------------------------------------------------------*/

struct container1 {

    dtn_socket_configuration remote;
    const uint8_t *buffer;
    size_t size;
    dtn_test_node_core *self;

};

/*---------------------------------------------------------------------------*/

static bool send_at_interface(const void *key, void *val, void *data){

    if (!key) return true;

    char *name = (char*) key;
    Interface *interface = (Interface*)(val);

    struct container1 *container = (struct container1*) data;
    if (!name || !interface || !container) goto error;

    if (!dtn_thread_lock_try_lock(&interface->lock))
        goto error;

    bool result = dtn_interface_ip_send(
        interface->interface, container->remote, container->buffer, container->size);

    if (!dtn_thread_lock_unlock(&interface->lock)){
        dtn_log_error("failed to unlock interface.");
    }

    if (!result){
        dtn_log_error("Failed to send at interface %s", name);
    } else {
        dtn_log_debug("Send at interface %s", name);
    }

    return true;

error:
    return false;
}

/*---------------------------------------------------------------------------*/

bool dtn_test_node_core_send_raw(
        dtn_test_node_core *self,
        dtn_socket_configuration remote,
        const dtn_cbor *data){

    if (!self || !data) return false;

    uint64_t size = dtn_cbor_encoding_size(data);
    uint8_t buffer[size + 1];
    memset(buffer, 0, size + 1);
    uint8_t *next = NULL;

    if (!dtn_cbor_encode_array_of_indefinite_length(
        data,
        buffer, size, &next)) goto error;

    struct container1 container = (struct container1){

        .remote = remote,
        .buffer = buffer,
        .size = next - buffer,
        .self = self
    };

    return dtn_dict_for_each(
        self->interfaces.ip,
        &container,
        send_at_interface);
error:
    return false;
}