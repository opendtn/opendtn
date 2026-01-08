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
        @file           dtn_router_core.h
        @author         Töpfer, Markus

        @date           2025-12-21


        ------------------------------------------------------------------------
*/
#ifndef dtn_router_core_h
#define dtn_router_core_h

#include <dtn/dtn_cbor.h>
#include <dtn_base/dtn_event_loop.h>

/*---------------------------------------------------------------------------*/

typedef struct dtn_router_core dtn_router_core;

/*---------------------------------------------------------------------------*/

typedef struct dtn_router_core_config {

    dtn_event_loop *loop;

    char name[PATH_MAX];
    char route_config_path[PATH_MAX];

    struct {

        uint64_t threadlock_timeout_usec;
        uint64_t message_queue_capacity;
        uint64_t threads;
        uint64_t link_check;

    } limits;

} dtn_router_core_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_router_core *dtn_router_core_create(dtn_router_core_config config);
dtn_router_core *dtn_router_core_free(dtn_router_core *self);
dtn_router_core *dtn_router_core_cast(const void *data);

/*---------------------------------------------------------------------------*/

bool dtn_router_core_enable_ip_interfaces(dtn_router_core *self,
                                          const dtn_item *config);

/*---------------------------------------------------------------------------*/

bool dtn_router_core_send_raw(dtn_router_core *self,
                              dtn_socket_configuration remote,
                              const dtn_cbor *data);

#endif /* dtn_router_core_h */
