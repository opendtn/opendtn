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
        @file           dtn_tunnel_app.h
        @author         Töpfer, Markus

        @date           2025-12-25


        ------------------------------------------------------------------------
*/
#ifndef dtn_tunnel_app_h
#define dtn_tunnel_app_h


#include <dtn_base/dtn_event_loop.h>
#include <dtn_core/dtn_io.h>
#include <dtn_core/dtn_webserver.h>
#include <dtn_core/dtn_password.h>

/*---------------------------------------------------------------------------*/

typedef struct dtn_tunnel_app dtn_tunnel_app;

/*---------------------------------------------------------------------------*/

typedef struct dtn_tunnel_app_config {

    dtn_event_loop *loop;
    dtn_io *io;

    char name[PATH_MAX];
    char uri[PATH_MAX];
    char destination_uri[PATH_MAX];

    dtn_socket_configuration socket; // command & control socket
    dtn_socket_configuration tunnel; // tunnel socket
    dtn_socket_configuration remote; // remote tunnel socket

    dtn_password password;

    struct {

        uint64_t threadlock_timeout_usec;
        uint64_t message_queue_capacity;
        uint64_t threads;
        uint64_t link_check;
        uint64_t buffer_time_cleanup_usecs;
        uint64_t max_buffer_time_secs;
        uint64_t history_secs;

        struct {

            uint64_t string_size;
            uint64_t utf8_string_size;
            uint64_t array_size;
            uint64_t undef_length_array;
            uint64_t map_size;
            uint64_t undef_length_map; 

        } cbor;

    } limits;

} dtn_tunnel_app_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_tunnel_app *dtn_tunnel_app_create(dtn_tunnel_app_config config);
dtn_tunnel_app *dtn_tunnel_app_free(dtn_tunnel_app *self);
dtn_tunnel_app *dtn_tunnel_app_cast(const void *ptr);

/*
 *      ------------------------------------------------------------------------
 *
 *      CONFIG FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_tunnel_app_config dtn_tunnel_app_config_from_item(
        const dtn_item *config);

/*---------------------------------------------------------------------------*/

bool dtn_tunnel_app_enable_ip_interfaces(
        dtn_tunnel_app *self, const dtn_item *config);

/*---------------------------------------------------------------------------*/

bool dtn_tunnel_app_enable_routes(
        dtn_tunnel_app *self, const char *path);

#endif /* dtn_tunnel_app_h */
