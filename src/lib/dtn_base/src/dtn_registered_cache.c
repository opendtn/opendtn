/***
        ------------------------------------------------------------------------

        Copyright (c) 2018 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_cache.c
        @author         Michael Beer
        @author         Markus Toepfer

        ------------------------------------------------------------------------
*/

#include "../include/dtn_registered_cache.h"
#include "../include/dtn_utils.h"
/*----------------------------------------------------------------------------*/

#ifdef __STDC_NO_ATOMICS__
#define DTN_DISABLE_CACHING
#endif

/******************************************************************************
 *                              CACHING ENABLED
 ******************************************************************************/

#ifndef DTN_DISABLE_CACHING

#include "../include/dtn_constants.h"
#include <stdatomic.h>
#include <stdlib.h>

#include "../include/dtn_hashtable.h"
#include "../include/dtn_thread_lock.h"

#include "../include/dtn_buffer.h"
#include "../include/dtn_linked_list.h"
#include "../include/dtn_string.h"
#include "../include/dtn_teardown.h"

/*----------------------------------------------------------------------------*/

struct dtn_registered_cache_struct {

    size_t capacity;
    size_t next_free;
    void **elements;
    atomic_flag in_use;

    bool (*element_checker)(void *);

    void *(*item_free)(void *);
    uint64_t timeout_usec;

    struct {

        size_t elements_put;
        size_t elements_got;

        size_t get_called;
        size_t put_called;

    } stats;
};

static dtn_hashtable *g_registry = 0;
static dtn_thread_lock g_lock;

/*----------------------------------------------------------------------------*/

static dtn_registered_cache *cache_create(size_t capacity) {

    dtn_registered_cache *cache = 0;

    cache = calloc(1, sizeof(dtn_registered_cache));
    cache->elements = calloc(capacity, sizeof(void *));
    cache->capacity = capacity;
    cache->next_free = 0;

    atomic_flag_clear(&cache->in_use);

    // Let's ensure that dtn_teardown() also frees all caches
    dtn_teardown_register(dtn_registered_cache_free_all, "Caches");

    return cache;
}

/*----------------------------------------------------------------------------*/

static dtn_registered_cache *cache_extend(dtn_registered_cache *cache,
                                          size_t capacity) {

    if (0 == capacity) {

        capacity = DTN_DEFAULT_CACHE_SIZE;
    }

    if (0 == cache) {

        return cache_create(capacity);
    }

    capacity += cache->capacity;

    cache->elements = realloc(cache->elements, sizeof(void *) * capacity);

    cache->capacity = capacity;

    return cache;
}

/*----------------------------------------------------------------------------*/

dtn_registered_cache *
dtn_registered_cache_extend(char const *cache_name,
                            dtn_registered_cache_config cfg) {

    bool registry_locked = false;

    if (0 == cache_name) {

        dtn_log_error("No cache name given");
        goto error;
    }

    dtn_log_debug("Enabling caching for %s with %zu buckets", cache_name,
                  cfg.capacity);

    if (0 == g_registry) {

        g_registry = dtn_hashtable_create_c_string(20);
        dtn_thread_lock_init(&g_lock, 1000 * 1000);
    }

    if (!dtn_thread_lock_try_lock(&g_lock)) {

        dtn_log_error("Could not lock cache registry");
        goto error;
    }

    registry_locked = true;

    dtn_registered_cache *cache = dtn_hashtable_get(g_registry, cache_name);

    if ((0 != cache) && (cfg.item_free != cache->item_free)) {

        dtn_log_error("Wont manipulate entry for cache %s = item_free differs",
                      cache_name);
        goto error;
    }

    cache = cache_extend(cache, cfg.capacity);

    DTN_ASSERT(0 != cache);

    cache->item_free = cfg.item_free;
    cache->timeout_usec = cfg.timeout_usec;

    dtn_hashtable_set(g_registry, cache_name, cache);

    dtn_thread_lock_unlock(&g_lock);

    return cache;

error:

    if (registry_locked) {

        dtn_thread_lock_unlock(&g_lock);
    }

    return 0;
}

/*----------------------------------------------------------------------------*/

static dtn_registered_cache *cache_free(dtn_registered_cache *restrict self) {

    if (0 == self)
        goto error;

    uint64_t start = dtn_time_get_current_time_usecs();
    uint64_t current = 0;

    while (atomic_flag_test_and_set(&self->in_use)) {

        current = dtn_time_get_current_time_usecs();

        if (0 == self->timeout_usec)
            continue;

        if (current > self->timeout_usec + start)
            goto error;
    }

    size_t no_elements_freed = 0;

    if ((0 != self->elements) && (self->item_free != 0)) {

        for (size_t i = 0; i < self->next_free; i++) {
            self->item_free(self->elements[i]);

            if (0 != self->elements[i]) {
                ++no_elements_freed;
            }
        }
    }

    dtn_log_debug(
        "Cache Calls: Put: %zu (Put elements: %zu)   Get: %zu(Got elements: "
        "%zu)   Next free at exit: %zu  Freed: %zu",
        self->stats.put_called, self->stats.elements_put,
        self->stats.get_called, self->stats.elements_got, self->next_free,
        no_elements_freed);

    if (0 != self->elements) {

        free(self->elements);
        self->elements = 0;
    }

    free(self);
    self = 0;

error:

    return self;
}

/*----------------------------------------------------------------------------*/

static bool free_hashtable_entry(void const *key, void const *value,
                                 void *arg) {

    UNUSED(key);
    UNUSED(arg);

    dtn_log_debug("Freeing cache %s", dtn_string_sanitize(key));
    cache_free((dtn_registered_cache *)value);

    return true;
}

/*----------------------------------------------------------------------------*/

void dtn_registered_cache_free_all() {

    if (0 == g_registry)
        return;

    if (!dtn_thread_lock_try_lock(&g_lock)) {

        dtn_log_warning("Could not lock cache registry");
        DTN_ASSERT(!"MUST NEVER HAPPEN");
    }

    size_t caches_freed =
        dtn_hashtable_for_each(g_registry, free_hashtable_entry, 0);

    dtn_log_debug("Freed %zu caches", caches_freed);

    dtn_hashtable_free(g_registry);

    g_registry = 0;

    dtn_thread_lock_unlock(&g_lock);
    dtn_thread_lock_clear(&g_lock);
}

/*---------------------------------------------------------------------------*/

static bool element_type_correct(dtn_registered_cache *restrict self,
                                 void *object) {

    if (0 == self) {
        return false;
    } else if (0 == self->element_checker) {
        return true;
    } else {
        return self->element_checker(object);
    }
}

/*---------------------------------------------------------------------------*/

static void *locked_cache_get(dtn_registered_cache *restrict self) {

    if ((0 == self) || (0 == self->elements) || (0 == self->next_free)) {

        return 0;

    } else {

        --self->next_free;
        void *object = self->elements[self->next_free];

        ++self->stats.elements_got;

        DTN_ASSERT(element_type_correct(self, object));

        return object;
    }
}

/*----------------------------------------------------------------------------*/

void *dtn_registered_cache_get(dtn_registered_cache *restrict self) {

    if (0 == self) {

        return 0;

    } else {

        ++self->stats.get_called;

        void *object = 0;

        if (!atomic_flag_test_and_set(&self->in_use)) {

            object = locked_cache_get(self);
            atomic_flag_clear(&self->in_use);
        }

        return object;
    }
}

/*----------------------------------------------------------------------------*/

static void *locked_cache_put(dtn_registered_cache *restrict self,
                              void *object) {

    if (0 == object) {

        return 0;

    } else if ((0 == self->elements) || (self->capacity == self->next_free)) {

        return object;

    } else {

        self->elements[self->next_free] = object;
        ++self->next_free;

        ++self->stats.elements_put;

        return 0;
    }
}

/*----------------------------------------------------------------------------*/

void *dtn_registered_cache_put(dtn_registered_cache *restrict self,
                               void *object) {

    if (0 == self) {

        return object;

    } else {

        ++self->stats.put_called;

        if (!atomic_flag_test_and_set(&self->in_use)) {

            DTN_ASSERT(element_type_correct(self, object));

            object = locked_cache_put(self, object);
            atomic_flag_clear(&self->in_use);
        }

        return object;
    }
}

/*----------------------------------------------------------------------------*/

void dtn_registered_cache_set_element_checker(dtn_registered_cache *self,
                                              bool (*element_checker)(void *)) {

    if (0 == self) {

        return;
    }

    self->element_checker = element_checker;
}

/******************************************************************************
 *                                JSON REPORT
 ******************************************************************************/

struct cache_to_json_arg {
    dtn_item *target;
};

static bool cache_to_json(void const *key, void const *value, void *void_arg) {

    struct cache_to_json_arg *arg = void_arg;

    if ((0 == arg) || (0 == arg->target))
        goto error;

    if (0 == value)
        goto finish;

    dtn_registered_cache *cache = (dtn_registered_cache *)value;

    dtn_item *jcache = dtn_item_object();

    if (0 == jcache) {

        dtn_log_error("Could not create json object for cache");
        goto error;
    }

    dtn_item_object_set(jcache, "capacity", dtn_item_number(cache->capacity));

    dtn_item_object_set(jcache, "in_use", dtn_item_number(cache->next_free));

    dtn_item_object_set(arg->target, key, jcache);

finish:

    return true;

error:

    return false;
}

/*----------------------------------------------------------------------------*/

dtn_item *dtn_registered_cache_report(dtn_item *target) {

    dtn_item *jval = 0;

    if (0 == g_registry) {

        goto finish;
    }

    if (!dtn_thread_lock_try_lock(&g_lock)) {

        dtn_log_error("Could not lock cache registry");
        goto error;
    }

    struct cache_to_json_arg arg = {

        .target = target,

    };

    if (0 == target) {

        jval = dtn_item_object();
        arg.target = jval;
    }

    size_t num_caches = dtn_hashtable_for_each(g_registry, cache_to_json, &arg);

    dtn_thread_lock_unlock(&g_lock);

    dtn_log_debug("Generated report for %zu caches", num_caches);

finish:

    return arg.target;

error:

    jval = dtn_item_free(jval);

    return 0;
}

/*----------------------------------------------------------------------------*/

static bool enable_caching_for(dtn_registered_cache_sizes *cfg,
                               void (*enable_caching)(size_t),
                               char const *cache_name) {

    if (0 == enable_caching)
        goto error;
    if (0 == cache_name)
        goto error;

    size_t size = dtn_registered_cache_size_for(cfg, cache_name);

    if (0 == size) {
        return true;
    }

    enable_caching(size);

    return true;

error:

    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_registered_cache_sizes_configure(dtn_registered_cache_sizes *cfg) {

    bool ok = enable_caching_for(cfg, dtn_buffer_enable_caching, "buffer");

    ok = ok & enable_caching_for(cfg, dtn_linked_list_enable_caching, "list");

    return ok;
}

/******************************************************************************
 *                              CACHING DISABLED
 ******************************************************************************/

#else /* DTN_DISABLE_CACHING */

struct dtn_registered_cache_struct {
    bool dummy : 1;
};

void *dtn_registered_cache_get(dtn_registered_cache *restrict self) {

    UNUSED(self);
    return 0;
}

/*----------------------------------------------------------------------------*/

void *dtn_registered_cache_put(dtn_registered_cache *restrict self,
                               void *object) {

    UNUSED(self);
    return object;
}

/*----------------------------------------------------------------------------*/

void dtn_registered_cache_set_element_checker(dtn_registered_cache *self,
                                              bool (*element_checker)(void *)) {

    UNUSED(self);
    UNUSED(element_checker);
}
/*----------------------------------------------------------------------------*/

dtn_registered_cache *
dtn_registered_cache_extend(char const *cache_name,
                            dtn_registered_cache_config cfg) {

    UNUSED(cache_name);
    UNUSED(cfg);
    return 0;
}

/*----------------------------------------------------------------------------*/

void dtn_registered_cache_free_all() { return; }

/*----------------------------------------------------------------------------*/

dtn_item *dtn_registered_cache_report(dtn_json *json, dtn_item *target) {

    UNUSED(json);
    UNUSED(target);

    return 0;
}

/*----------------------------------------------------------------------------*/

bool dtn_registered_cache_sizes_configure(dtn_registered_cache_sizes *sizes) {
    UNUSED(sizes);
    return true;
}

/*----------------------------------------------------------------------------*/
#endif

/*****************************************************************************
                                 CONFIGURATION
 ****************************************************************************/

struct dtn_registered_cache_sizes {

    bool enabled : 1; // Caching enabled ?
    dtn_hashtable *sizes;
};

/*----------------------------------------------------------------------------*/

struct add_cache_size_arg {
    bool ok;
    dtn_hashtable *tbl;
};

static bool add_cache_size(const char *vkey, const dtn_item *value,
                           void *varg) {

    struct add_cache_size_arg *arg = varg;
    char const *key = vkey;

    DTN_ASSERT(0 != arg);
    DTN_ASSERT(0 != key);

    dtn_hashtable *sizes = arg->tbl;

    DTN_ASSERT(0 != sizes);

    if (!dtn_item_is_number(value)) {

        char *cval = dtn_item_to_json(value);
        dtn_log_error("Malformed config: Expected number for key %s, got %s",
                      key, cval == 0 ? "0" : cval);
        free(cval);

        goto error;
    }

    double dvalue = dtn_item_get_number(value);

    if (0 > dvalue) {
        dtn_log_error("Malformed config: Size for %s negative", key);
        goto error;
    }

    size_t size = dvalue;

    if (0 != dtn_hashtable_set(sizes, key, (void *)size)) {
        dtn_log_warning("Overwriting old value for %s", key);
    }

    return true;

error:

    arg->ok = false;

    return false;
}

/*----------------------------------------------------------------------------*/

dtn_registered_cache_sizes *
dtn_registered_cache_sizes_free(dtn_registered_cache_sizes *self) {

    if (0 == self)
        goto error;

    self->sizes = dtn_hashtable_free(self->sizes);
    DTN_ASSERT(0 == self->sizes);

    free(self);
    self = 0;

error:

    return self;
}

/*----------------------------------------------------------------------------*/

dtn_registered_cache_sizes *
dtn_registered_cache_sizes_from_json(dtn_item const *jval) {

    dtn_registered_cache_sizes *cfg = 0;

    dtn_hashtable *sizes = 0;
    bool caching_enabled = true;

    if (0 == jval) {
        dtn_log_error("Called with 0 pointer");
        goto error;
    }

    dtn_item const *caching = dtn_item_get(jval, "/enabled");

    if ((0 != caching) && (!dtn_item_is_true(caching))) {
        dtn_log_info("Caching disabled");
        caching_enabled = false;
        goto finish;
    }

    sizes = dtn_hashtable_create_c_string(25);

    jval = dtn_item_get(jval, "/sizes");

    if (0 == jval) {
        goto finish;
    }

    DTN_ASSERT(0 != jval);

    struct add_cache_size_arg arg = {
        .ok = true,
        .tbl = sizes,
    };

    dtn_item_object_for_each((dtn_item *)jval, add_cache_size, &arg);
    arg.tbl = 0;

    if (!arg.ok) {
        goto error;
    }

finish:

    cfg = calloc(1, sizeof(dtn_registered_cache_sizes));
    cfg->enabled = caching_enabled;
    cfg->sizes = sizes;
    sizes = 0;

    return cfg;

error:

    sizes = dtn_hashtable_free(sizes);
    DTN_ASSERT(0 == sizes);

    return 0;
}

/*----------------------------------------------------------------------------*/

size_t dtn_registered_cache_size_for(dtn_registered_cache_sizes *cfg,
                                     char const *cache_name) {

    size_t size = 0;

    if (0 == cfg) {
        goto error;
    }

    if (0 == cache_name) {
        goto error;
    }

    if (!cfg->enabled) {
        return 0;
    }

    size = DTN_DEFAULT_CACHE_SIZE;

    if (dtn_hashtable_contains(cfg->sizes, cache_name)) {
        size = (size_t)dtn_hashtable_get(cfg->sizes, cache_name);
    }

error:

    return size;
}
