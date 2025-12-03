/***
        ------------------------------------------------------------------------

        Copyright (c) 2020 German Aerospace Center DLR e.V. (GSOC)

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
        @author         Michael Beer


        ALWAYS RECALL:

        NON-PERFORMANT CACHING DOES NOT MAKE SENSE!


        This is a cache implementation with all caches being registered
        centrally. While the registry is centralized, all cache instances
        of dtn_registered_cache MUST be static global within the implementation
        using this type of cache. (see below)

        This allows for reporting on cache usage and:
        It allows to manage cache sizes locally.

        Use when implementing a data structure that should be cached
        `dtn_cached_datastructure`:

        static dtn_registered_cache *g_cache;

        void dtn_data_structure_enable_caching(size_t capacity) {

            dtn_registered_cache_config cfg = {
                .item_free = data_structure_free,
                .capacity = capacity,
            };

            dtn_registered_cache_extend(g_cache, cfg);

        }

        dtn_data_structure *dtn_data_structure_create() {

            dtn_data_structure *ds = dtn_cache_get(g_cache);

            if(0 == ds) {

                ds == calloc(1, sizeof(dtn_data_structure));

            }

            return ds;

        }

        dtn_data_structure_free(dtn_data_structure *self) {

            if(0 == self) goto finish;

            self = dtn_registry_cache_put(g_cache, self);

            if(0 != self) {

                free(self);
                self = 0;

            }

        finish:

            return self;

        }

        If `dtn_data_structure` contains other data structures internally,
        it should extend their cache accordingly:

        typedef struct {

            // Implemented via `dtn_linked_list`
            dtn_list *elements;

        } dtn_data_structure;

        dtn_data_structure_enable_caching(size_t capacity) {

            dtn_registered_cache_config cfg = {
                .item_free = data_structure_free,
                .capacity = capacity,
            };

            dtn_registered_cache_extend(g_cache, cfg);

            // Adapt linked list cache accordingly:
            // Each dtn_data_structure requires one linked list,
            // thus the cache for linked_list should be extended by
            // the amount of dtn_data_structures to cache
            dtn_linked_list_enable_caching(capacity);

        }

        // Remainder of functions stays the same

        This way, the cache for dtn_linked_list is kept at a appropriate size.

        The caches are freed when `dtn_registered_cache_free_all()` is called.

        This should be done once *AND ONLY ONCE* immediately before terminating
        the process - e.g. before leaving the main method.

        The registered cache must not have functionality to disable created
        caches.
        We must be able to cache the pointer to the cache
        (e.g. dtn_buffer needs to be able to retain its static g_cache pointer).

        Otherwise we cannot cache the cache pointer and always have to retrieve
        the pointer when trying to get / put on the cache like

        // Inflicts heavy performance hit due to hashtable lookup &
        // necessity to lock the cache registry
        my_cache = dtn_registered_cache_get_cache("my_struct");
        dtn_registered_cache_put(my_cache, my_struct);
        ...
        // compressing into a single call does NOT solve the root problem,
        // still a hashtable lookup & and lock is required is needed
        dtn_registered_cache_put("my_struct", my_struct);

        wich causes a hashtable look up.
        Moreover, in order to be thread-safe, for conducting the hashtable
        lookup, the hashtable has to be looked every single time an object is
        put/retrieved .

        This contradicts the very idea to gain performance by caching
        if the caching itself becomes so costly.

        ------------------------------------------------------------------------
*/
#ifndef dtn_registered_cache_h
#define dtn_registered_cache_h

#define DTN_KEY_CACHE_ENABLED "cache"
#define DTN_KEY_CACHE_SIZES "size"

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "dtn_time.h"
#include "dtn_utils.h"

#include "dtn_hashtable.h"
#include "dtn_item.h"
#include "dtn_item_json.h"

/*----------------------------------------------------------------------------*/

typedef struct dtn_registered_cache_struct dtn_registered_cache;

typedef struct {

  uint64_t timeout_usec;
  void *(*item_free)(void *);
  size_t capacity;

} dtn_registered_cache_config;

/*----------------------------------------------------------------------------*/

/**
 * Create / extend a registered cache.
 * Registered caches can be reported on via `dtn_cache_report` and are freed
 * by `dtn_cache_free_all`
 *
 * BEWARE:
 * Call this function *ONLY ONCE* and *ONLY AFTER YOU ARE SURE NOBODY USES
 * ANY OF THE REGISTERED CACHES ANY LONGER!*
 */
dtn_registered_cache *dtn_registered_cache_extend(char const *cache_name,
                                                dtn_registered_cache_config cfg);

/*----------------------------------------------------------------------------*/

void dtn_registered_cache_free_all();

/*----------------------------------------------------------------------------*/

/**
        Get an object out of the cache.
*/
void *dtn_registered_cache_get(dtn_registered_cache *restrict self);

/*----------------------------------------------------------------------------*/

/**
        Put an object out to the cache.

        @returns object in case of error
        @returns NULL in case of success
*/
void *dtn_registered_cache_put(dtn_registered_cache *restrict self, void *object);

/*----------------------------------------------------------------------------*/

/**
 * Set a method to check added pointers for valid type.
 * Required since we must use void pointers and cannot use the compiler to check
 * for us
 */
void dtn_registered_cache_set_element_checker(dtn_registered_cache *cache,
                                             bool (*element_checker)(void *));

/*----------------------------------------------------------------------------*/

/**
 * Creates a report of all registered caches.
 * The report is a json object containig at least capacity & number of
 * cache entries in use for each cache.
 */
dtn_item *dtn_registered_cache_report(dtn_item *target);

/*****************************************************************************
                                 CONFIGURATION
 ****************************************************************************/

typedef struct dtn_registered_cache_sizes dtn_registered_cache_sizes;

/*----------------------------------------------------------------------------*/

dtn_registered_cache_sizes *
dtn_registered_cache_sizes_free(dtn_registered_cache_sizes *self);

/*----------------------------------------------------------------------------*/

/**
 * Parse in cache config from json.
 *
 * JSON looks like
 *
 * {
 * "enable_caching" : true,
 * "cache_sizes" : {
 *    "buffers" : 10
 *    }
 * }
 */
dtn_registered_cache_sizes *
dtn_registered_cache_sizes_from_json(dtn_item const *jval);

/*----------------------------------------------------------------------------*/

/**
 * Get configured cache size for cache `cache_name` out of `sizes`.
 * Returns default cache size if appropriate or 0 if disabled
 */
size_t dtn_registered_cache_size_for(dtn_registered_cache_sizes *sizes,
                                    char const *cache_name);

/*----------------------------------------------------------------------------*/

bool dtn_registered_cache_sizes_configure(dtn_registered_cache_sizes *sizes);

/*----------------------------------------------------------------------------*/

#endif /* dtn_registered_cache_h */
