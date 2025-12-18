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
        @file           dtn_base_node.h
        @author         Töpfer, Markus

        @date           2025-12-18


        ------------------------------------------------------------------------
*/
#ifndef dtn_base_node_h
#define dtn_base_node_h

#include <dtn_base/dtn_event_loop.h>
#include <dtn_core/dtn_io.h>
#include <dtn_core/dtn_webserver.h>
#include <dtn_core/dtn_password.h>

/*---------------------------------------------------------------------------*/

typedef struct dtn_base_node dtn_base_node;

/*---------------------------------------------------------------------------*/

typedef struct dtn_base_node_config {

    dtn_event_loop *loop;
    dtn_io *io;
    dtn_webserver *server;

    char name[PATH_MAX];

    dtn_socket_configuration socket; // command & control socket

    dtn_password password;

    struct {

        uint64_t threadlock_timeout_usec;
        uint64_t message_queue_capacity;
        uint64_t threads;

    } limits;

} dtn_base_node_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_base_node *dtn_base_node_create(dtn_base_node_config config);
dtn_base_node *dtn_base_node_free(dtn_base_node *self);
dtn_base_node *dtn_base_node_cast(const void *data);

/*---------------------------------------------------------------------------*/

dtn_base_node_config dtn_base_node_config_from_item(const dtn_item *config);

/*---------------------------------------------------------------------------*/

void dtn_base_node_websocket_callback(
    void *userdata, int socket, dtn_item *item);

#endif /* dtn_base_node_h */
