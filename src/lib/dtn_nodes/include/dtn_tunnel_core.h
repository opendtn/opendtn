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
        @file           dtn_tunnel_core.h
        @author         Töpfer, Markus

        @date           2025-12-25


        ------------------------------------------------------------------------
*/
#ifndef dtn_tunnel_core_h
#define dtn_tunnel_core_h


#include <dtn_base/dtn_event_loop.h>
#include <dtn/dtn_cbor.h>

/*---------------------------------------------------------------------------*/

typedef struct dtn_tunnel_core dtn_tunnel_core;

/*---------------------------------------------------------------------------*/

typedef struct dtn_tunnel_core_config{

    dtn_event_loop *loop;

    dtn_socket_configuration tunnel;
    dtn_socket_configuration remote;

    struct {

        uint64_t threadlock_timeout_usec;
        uint64_t message_queue_capacity;
        uint64_t threads;
        uint64_t link_check;
        uint64_t buffer_time_cleanup_usecs;
        uint64_t max_buffer_time_secs;
        uint64_t history_secs;

    } limits;

} dtn_tunnel_core_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_tunnel_core *dtn_tunnel_core_create(dtn_tunnel_core_config config);
dtn_tunnel_core *dtn_tunnel_core_free(dtn_tunnel_core *self);
dtn_tunnel_core *dtn_tunnel_core_cast(const void *data);

/*
 *      ------------------------------------------------------------------------
 *
 *      CONFIG FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_tunnel_core_enable_ip_interfaces(
        dtn_tunnel_core *self,
        const dtn_item *config);

/*---------------------------------------------------------------------------*/

bool dtn_tunnel_core_enable_routes(
        dtn_tunnel_core *self, 
        const char *path);

/*---------------------------------------------------------------------------*/

bool dtn_tunnel_core_set_source_uri(
        dtn_tunnel_core *self, 
        const char *uri);

/*---------------------------------------------------------------------------*/

bool dtn_tunnel_core_set_destination_uri(
        dtn_tunnel_core *self, 
        const char *uri);

#endif /* dtn_tunnel_core_h */
