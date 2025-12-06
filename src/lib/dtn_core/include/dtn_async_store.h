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
        @file           dtn_async_store.h
        @author         Töpfer, Markus

        @date           2025-12-06


        ------------------------------------------------------------------------
*/
#ifndef dtn_async_store_h
#define dtn_async_store_h

#include <dtn_base/dtn_event_loop.h>
#include <dtn_base/dtn_item.h>

/*----------------------------------------------------------------------------*/

typedef struct dtn_async_store dtn_async_store;

/*----------------------------------------------------------------------------*/

typedef struct dtn_async_store_config {

    dtn_event_loop *loop;

    struct {

        uint64_t threadlock_timeout_usec;
        uint64_t invalidate_check_usec;

    } limits;

} dtn_async_store_config;

/*----------------------------------------------------------------------------*/

typedef struct dtn_async_data dtn_async_data;

/*----------------------------------------------------------------------------*/

struct dtn_async_data {

    int socket;
    dtn_item *message;

    struct {

        void *userdata;
        void (*callback)(void *userdata, dtn_async_data data);

    } timedout;

    struct {

        void *userdata;
        void (*callback)(void *userdata, int socket, dtn_item *message);

    } callback;

};

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_async_store *dtn_async_store_create(dtn_async_store_config config);
dtn_async_store *dtn_async_store_free(dtn_async_store *self);
dtn_async_store *dtn_async_store_cast(const void *self);

void dtn_async_data_clear(dtn_async_data *data);

/*
 *      ------------------------------------------------------------------------
 *
 *      ASYNC STORE FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

/**
 *  Set some data for a min lifetime.
 *  @NOTE the message item set MUST NOT BE FREED
 */
bool dtn_async_set(dtn_async_store *self,
                         const char *id,
                         dtn_async_data data,
                         uint64_t min_lifetime_usec);

/*---------------------------------------------------------------------------*/

/**
 *  Get some asnyc data for id
 */
dtn_async_data dtn_async_get(dtn_async_store *self,
                                   const char *id);

/*---------------------------------------------------------------------------*/

/**
 *  Drop all async data for some socket
 */
bool dtn_async_drop(dtn_async_store *self, int socket);


#endif /* dtn_async_store_h */
