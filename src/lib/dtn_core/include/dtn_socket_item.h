/***
        ------------------------------------------------------------------------

        Copyright (c) 2024 German Aerospace Center DLR e.V. (GSOC)

        Licensed under the Apache License, Version 2.0 (the "License");
        you may not use this file except in compliance with the License.
        You may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

        Unless required by applicable law or agreed to in writing, software
        distributed under the License is distributed on an "AS IS" BASIS,
        WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
        See the License for the specific language governing permissions and
        limitations under the License.

        This file is part of the openvocs project. https://openvocs.org

        ------------------------------------------------------------------------
*//**
        @file           dtn_socket_item.h
        @author         Markus

        @date           2024-10-11


        ------------------------------------------------------------------------
*/
#ifndef dtn_socket_item_h
#define dtn_socket_item_h

#include <dtn_base/dtn_event_loop.h>
#include <dtn_base/dtn_item.h>

/*----------------------------------------------------------------------------*/

typedef struct dtn_socket_item dtn_socket_item;

/*----------------------------------------------------------------------------*/

typedef struct dtn_socket_item_config {

  dtn_event_loop *loop;

  struct {

    uint64_t threadlock_timeout_usec;

  } limits;

} dtn_socket_item_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_socket_item *dtn_socket_item_create(dtn_socket_item_config config);
dtn_socket_item *dtn_socket_item_cast(const void *data);
dtn_socket_item *dtn_socket_item_free(dtn_socket_item *self);

/**
 *  This function will return a copy of the current data set at socket.
 *  NOTE outgoing value is unconnected with socket data, to store or restore
 *  any changed data use dtn_socket_item_set.
 */
dtn_item *dtn_socket_item_get(dtn_socket_item *self, int socket);

/*----------------------------------------------------------------------------*/

/**
 *  Set value at slot position of socket. Will override any existing data at
 *  socket. Do not use value anymore after set.
 */
bool dtn_socket_item_set(dtn_socket_item *self, int socket,
                        dtn_item **value);

/*----------------------------------------------------------------------------*/

/**
 *  Drop all data stored at socket.
 */
bool dtn_socket_item_drop(dtn_socket_item *self, int socket);

/*----------------------------------------------------------------------------*/

bool dtn_socket_item_for_each_set_data(dtn_socket_item *self, dtn_item *out);

/*----------------------------------------------------------------------------*/

bool dtn_socket_item_for_each(dtn_socket_item *self, void *data,
                             bool (*function)(const void *key, void *val,
                                              void *data));

#endif /* dtn_socket_item_h */
