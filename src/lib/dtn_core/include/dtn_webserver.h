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
        @file           dtn_webserver.h
        @author         Töpfer, Markus

        @date           2025-12-17

        This is a minimal implementation of a webserver able to 
        process HTTP GET requests and websocket connections of 
        some domains. 

        ------------------------------------------------------------------------
*/
#ifndef dtn_webserver_h
#define dtn_webserver_h

#include <dtn_base/dtn_event_loop.h>
#include "dtn_io.h"
#include "dtn_http_pointer.h"
#include "dtn_websocket_pointer.h"

/*----------------------------------------------------------------------------*/

typedef struct dtn_webserver dtn_webserver;

/*----------------------------------------------------------------------------*/

typedef struct dtn_webserver_config{

    dtn_event_loop *loop;
    dtn_io *io;

    char name[PATH_MAX];

    dtn_socket_configuration socket;

    dtn_http_message_config http;
    dtn_websocket_frame_config frame;

} dtn_webserver_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_webserver *dtn_webserver_create(dtn_webserver_config config);
dtn_webserver *dtn_webserver_free(dtn_webserver *self);

/*----------------------------------------------------------------------------*/

bool dtn_webserver_set_debug(dtn_webserver *self, bool on);

/*----------------------------------------------------------------------------*/

bool dtn_webserver_enable_callback(
        dtn_webserver *self,
        const char *domain,
        void *userdata,
        void (*callback)(void *userdata, int socket, dtn_item *msg));

/*----------------------------------------------------------------------------*/

bool dtn_webserver_enable_domains(dtn_webserver *self, const dtn_item *config);

/*----------------------------------------------------------------------------*/

dtn_webserver_config dtn_webserver_config_from_item(const dtn_item *config);

/*----------------------------------------------------------------------------*/

bool dtn_webserver_send(dtn_webserver *self, int socket, const dtn_item *msg);

/*----------------------------------------------------------------------------*/

dtn_event_loop *dtn_webserver_get_eventloop(const dtn_webserver *self);

#endif /* dtn_webserver_h */
