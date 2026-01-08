/***
        ------------------------------------------------------------------------

        Copyright (c) 2026 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_key_store.h
        @author         Töpfer, Markus

        @date           2026-01-03


        ------------------------------------------------------------------------
*/
#ifndef dtn_key_store_h
#define dtn_key_store_h

#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>

#include <dtn_base/dtn_buffer.h>

/*----------------------------------------------------------------------------*/

#define DTN_DEFAULT_KEY_PATH "/etc/opendtn/keys"

/*----------------------------------------------------------------------------*/

typedef struct dtn_key_store dtn_key_store;

/*----------------------------------------------------------------------------*/

typedef struct dtn_key_store_config {

    char path[PATH_MAX];

    struct {

        uint64_t threadlock_timeout_usec;

    } limits;

} dtn_key_store_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_key_store *dtn_key_store_create(dtn_key_store_config config);
dtn_key_store *dtn_key_store_free(dtn_key_store *self);

/*
 *      ------------------------------------------------------------------------
 *
 *      Persistance FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_key_store_load(dtn_key_store *self, const char *optional_path);
bool dtn_key_store_save(dtn_key_store *self, const char *optional_path);

/*
 *      ------------------------------------------------------------------------
 *
 *      KEY ACCESS FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

/**
 *  Get a key for some dtn_destination of form:
 *      reg_name/node_name
 *
 *  @param self         instance pointer,
 *  @param destination  destination key
 *
 *  @returns copy of the key, buffer MUST be deleted by the caller
 */
dtn_buffer *dtn_key_store_get(dtn_key_store *self, const char *destination);

/*----------------------------------------------------------------------------*/

/**
 *  Set a key for some destination of form:
 *      reg_name/node_name
 *
 *  @param self         instance pointer
 *  @param destiantion  destination key
 *  @param key          key buffer to be set (will be consumed do not delete)
 */
bool dtn_key_store_set(dtn_key_store *self, const char *destination,
                       dtn_buffer *key);

#endif /* dtn_key_store_h */
