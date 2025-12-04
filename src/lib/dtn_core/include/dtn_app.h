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
        @file           dtn_app.h
        @author         Töpfer, Markus

        @date           2025-12-04


        ------------------------------------------------------------------------
*/
#ifndef dtn_app_h
#define dtn_app_h

/*----------------------------------------------------------------------------*/

#include <dtn_base/dtn_event_loop.h>
#include "dtn_io.h"

/*----------------------------------------------------------------------------*/

typedef struct dtn_app dtn_app;

/*----------------------------------------------------------------------------*/

typedef struct dtn_app_config {

    dtn_event_loop *loop;
    dtn_io *io;

    char name[PATH_MAX];

    struct {

        uint64_t threadlock_timeout_usec;
        uint64_t message_queue_capacity;
        uint64_t threads;

    } limits;

} dtn_app_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_app *dtn_app_create(dtn_app_config config);
dtn_app *dtn_app_free(dtn_app *self);
dtn_app *dtn_app_cast(const void *data);

void dtn_app_set_debug(dtn_app *self, bool value);

/*
 *      ------------------------------------------------------------------------
 *
 *      SOCKET FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

int dtn_app_open_listener(dtn_app *self, dtn_io_socket_config config);
int dtn_app_open_connection(dtn_app *self, dtn_io_socket_config config);
bool dtn_app_close(dtn_app *self, int socket);

/*
 *      ------------------------------------------------------------------------
 *
 *      SEND FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_app_send(dtn_app *self, int socket, const uint8_t *buffer, size_t size);
bool dtn_app_send_json(dtn_app *self, int socket, const dtn_item *output);

/*
 *      ------------------------------------------------------------------------
 *
 *      CALLBACKS FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_app_register(dtn_app *self, 
        const char *event,
        bool (*function)(void *userdata, int socket, dtn_item *input),
        void *userdata);

bool dtn_app_deregister(dtn_app *self,
        const char *event);




#endif /* dtn_app_h */
