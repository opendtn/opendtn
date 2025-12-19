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

#include <dtn_base/dtn_utils.h>
#include <dtn_base/dtn_thread_loop.h>

/*---------------------------------------------------------------------------*/

#define DTN_TEST_NODE_CORE_MAGIC_BYTE 0xc053

/*---------------------------------------------------------------------------*/

struct dtn_test_node_core {

    uint16_t magic_byte;
    dtn_test_node_core_config config;

    dtn_thread_loop *tloop;
};

/*
 *      ------------------------------------------------------------------------
 *
 *      THREADED FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

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

static bool handle_in_thread(dtn_thread_loop *tloop, dtn_thread_message *msg){

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

static bool init_config(dtn_test_node_core_config *config){

    if (!config || !config->loop) goto error;

    if (0 == config->limits.threadlock_timeout_usec)
        config->limits.threadlock_timeout_usec = 100000;

    if (0 == config->limits.message_queue_capacity)
        config->limits.message_queue_capacity = 10000;

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

    return self;
error:
    dtn_test_node_core_free(self);
    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_test_node_core *dtn_test_node_core_free(dtn_test_node_core *self){

    if (!dtn_test_node_core_cast(self)) return self;

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

bool dtn_test_node_core_open_interface_ip(
        dtn_test_node_core *self,
        dtn_socket_configuration socket){

    if (!self) goto error;
    if (socket.type != UDP) goto error;



    return true;
error:
    return false;
}