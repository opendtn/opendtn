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
        @file           dtn_router_app.h
        @author         Töpfer, Markus

        @date           2025-12-21


        ------------------------------------------------------------------------
*/
#ifndef dtn_router_app_h
#define dtn_router_app_h

#include <dtn_base/dtn_event_loop.h>
#include <dtn_core/dtn_io.h>
#include <dtn_core/dtn_webserver.h>
#include <dtn_core/dtn_password.h>

/*---------------------------------------------------------------------------*/

typedef struct dtn_router_app dtn_router_app;

/*---------------------------------------------------------------------------*/

typedef struct dtn_router_app_config {

    dtn_event_loop *loop;
    dtn_io *io;
    dtn_webserver *server;

    char name[PATH_MAX];
    char route_config_path[PATH_MAX];

    dtn_socket_configuration socket; // command & control socket

    dtn_password password;

    struct {

        uint64_t threadlock_timeout_usec;
        uint64_t message_queue_capacity;
        uint64_t threads;
        uint64_t link_check;

    } limits;

} dtn_router_app_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_router_app *dtn_router_app_create(dtn_router_app_config config);
dtn_router_app *dtn_router_app_free(dtn_router_app *self);
dtn_router_app *dtn_router_app_cast(const void *data);

/*---------------------------------------------------------------------------*/

dtn_router_app_config dtn_router_app_config_from_item(
    const dtn_item *config);

/*---------------------------------------------------------------------------*/

void dtn_router_app_websocket_callback(
    void *userdata, int socket, dtn_item *item);

/*---------------------------------------------------------------------------*/

bool dtn_router_enable_ip_interfaces(
    dtn_router_app *self, const dtn_item *config);


#endif /* dtn_router_app_h */
