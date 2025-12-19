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
        @file           dtn_test_node_core.h
        @author         Töpfer, Markus

        @date           2025-12-19


        ------------------------------------------------------------------------
*/
#ifndef dtn_test_node_core_h
#define dtn_test_node_core_h

#include <dtn_base/dtn_event_loop.h>

/*---------------------------------------------------------------------------*/

typedef struct dtn_test_node_core dtn_test_node_core;

/*---------------------------------------------------------------------------*/

typedef struct dtn_test_node_core_config{

    dtn_event_loop *loop;

    struct {

        uint64_t threadlock_timeout_usec;
        uint64_t message_queue_capacity;
        uint64_t threads;

    } limits;

} dtn_test_node_core_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_test_node_core *dtn_test_node_core_create(dtn_test_node_core_config config);
dtn_test_node_core *dtn_test_node_core_free(dtn_test_node_core *self);
dtn_test_node_core *dtn_test_node_core_cast(const void *data);

bool dtn_test_node_core_open_interface_ip(
        dtn_test_node_core *self,
        dtn_socket_configuration socket);

#endif /* dtn_test_node_core_h */
