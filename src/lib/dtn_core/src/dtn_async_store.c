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
        @file           dtn_async_store.c
        @author         Töpfer, Markus

        @date           2025-12-06


        ------------------------------------------------------------------------
*/
#include "../include/dtn_async_store.h"


#include <dtn_base/dtn_thread_lock.h>
#include <dtn_base/dtn_string.h>
#include <dtn_base/dtn_linked_list.h>
#include <dtn_base/dtn_dict.h>
#include <dtn_base/dtn_time.h>

/*----------------------------------------------------------------------------*/

#define DTN_ASYNC_STORE_MAGIC_BYTES 0xd123

/*----------------------------------------------------------------------------*/

struct dtn_async_store {

    uint16_t magic_bytes;
    dtn_async_store_config config;

    struct {

        dtn_thread_lock lock;
        dtn_dict *dict;

    } data;

    uint32_t invalidate_timer;
};

/*----------------------------------------------------------------------------*/

typedef struct {

  uint64_t created_usec;
  uint64_t max_lifetime_usec;

  dtn_async_data data;

} internal_session_data;

/*----------------------------------------------------------------------------*/

static void *free_internal_session_data(void *self) {

  if (!self)
    return NULL;

  internal_session_data *d = (internal_session_data *)self;
  d->data.message = dtn_item_free(d->data.message);
  d = dtn_data_pointer_free(d);

  return NULL;
}

/*----------------------------------------------------------------------------*/

struct container {

  dtn_list *list;
  uint64_t now;
};


/*----------------------------------------------------------------------------*/

static bool delete_keys_in_dict(void *item, void *data) {

  dtn_dict *dict = dtn_dict_cast(data);

  internal_session_data *entry =
      (internal_session_data *)dtn_dict_remove(dict, item);

  if (entry && entry->data.timedout.callback) {

    entry->data.timedout.callback(entry->data.timedout.userdata, entry->data);

    entry->data.message = NULL;
  }

  entry = free_internal_session_data(entry);
  return true;
}

/*----------------------------------------------------------------------------*/

static bool search_timed_out_keys(const void *key, void *item, void *data) {

  if (!key)
    return true;

  if (!item || !data)
    goto error;

  struct container *c = (struct container *)data;
  internal_session_data *d = (internal_session_data *)item;

  uint64_t lifetime = d->created_usec + d->max_lifetime_usec;

  if (lifetime < c->now)
    dtn_list_push(c->list, (void *)key);

  return true;
error:
  return false;
}

/*----------------------------------------------------------------------------*/

static bool invalidate_run(uint32_t id, void *data) {

    dtn_async_store *self = dtn_async_store_cast(data);
    if (!self) goto error;
    UNUSED(id);

    self->invalidate_timer = DTN_TIMER_INVALID;

    if (!dtn_thread_lock_try_lock(&self->data.lock)) goto reschedule;

    struct container container = (struct container){

        .list = dtn_linked_list_create((dtn_list_config){0}),
        .now = dtn_time_get_current_time_usecs()};

    if (!dtn_dict_for_each(self->data.dict, &container, search_timed_out_keys)) {
        dtn_log_error("Failed to search timedout items");
        DTN_ASSERT(1 == 0);
    }

    if (!dtn_list_for_each(container.list, self->data.dict, delete_keys_in_dict)) {
        dtn_log_error("Failed to delete timedout ID items");
        DTN_ASSERT(1 == 0);
    }

    container.list = dtn_list_free(container.list);

    /* unlock */

    if (!dtn_thread_lock_unlock(&self->data.lock)) {
        dtn_log_error("Failed to unlock session store");
        DTN_ASSERT(1 == 0);
    }

reschedule:
    
    self->invalidate_timer = dtn_event_loop_timer_set(
        self->config.loop,
        self->config.limits.invalidate_check_usec,
        self,
        invalidate_run);

    if (DTN_TIMER_INVALID == self->invalidate_timer){
        dtn_log_warning("Failed to reset invalidate run, dataloss may occur.");
    }

    return true;
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_async_store *dtn_async_store_create(dtn_async_store_config config){

    dtn_async_store *self = NULL;

    if (!config.loop) goto error;

    if (0 == config.limits.threadlock_timeout_usec)
        config.limits.threadlock_timeout_usec = 100000;

    if (0 == config.limits.invalidate_check_usec)
        config.limits.invalidate_check_usec = 1000000;

    self = calloc(1, sizeof(dtn_async_store));
    if (!self) goto error;

    self->magic_bytes = DTN_ASYNC_STORE_MAGIC_BYTES;
    self->config = config;

    if (!dtn_thread_lock_init(&self->data.lock, 
        self->config.limits.threadlock_timeout_usec)) goto error;

    dtn_dict_config dconfig = dtn_dict_string_key_config(255);
    dconfig.value.data_function.free = free_internal_session_data;

    self->data.dict = dtn_dict_create(dconfig);
    if (!self->data.dict) goto error;

    self->invalidate_timer = dtn_event_loop_timer_set(
        self->config.loop,
        self->config.limits.invalidate_check_usec,
        self,
        invalidate_run);

    if (DTN_TIMER_INVALID == self->invalidate_timer) goto error;

    return self;
error:
    dtn_async_store_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_async_store *dtn_async_store_cast(const void *data){

    if (!data) return NULL;

    if (*(uint16_t *)data != DTN_ASYNC_STORE_MAGIC_BYTES)
        return NULL;

    return (dtn_async_store *)data;
}

/*----------------------------------------------------------------------------*/

dtn_async_store *dtn_async_store_free(dtn_async_store *self){

    if (!dtn_async_store_cast(self)) return self;

    int i = 0;
    int max = 100;

    for (i = 0; i < max; i++) {

        if (dtn_thread_lock_try_lock(&self->data.lock))
            break;
    }

    if (i >= max){
        dtn_log_error("Failed to log store on delete. Dataloss may occur.");
        return self;
    }

    if (DTN_TIMER_INVALID != self->invalidate_timer){

        dtn_event_loop_timer_unset(
            self->config.loop,
            self->invalidate_timer,
            NULL);

    }

    self->data.dict = dtn_dict_free(self->data.dict);

    dtn_thread_lock_unlock(&self->data.lock);
    dtn_thread_lock_clear(&self->data.lock);

    self = dtn_data_pointer_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

void dtn_async_data_clear(dtn_async_data *data){

    if (!data) return;

    data->message = dtn_item_free(data->message);
    *data = (dtn_async_data){0};
    return;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      ASYNC STORE FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_async_set(dtn_async_store *self,
                         const char *id,
                         dtn_async_data data,
                         uint64_t min_lifetime_usec){

    char *key = NULL;

    internal_session_data *val = NULL;

    if (!self || !id) goto error;

    key = dtn_string_dup(id);
    val = calloc(1, sizeof(internal_session_data));

    if (!key || !val) goto error;

    val->created_usec = dtn_time_get_current_time_usecs();
    val->max_lifetime_usec = min_lifetime_usec;

    if (!dtn_thread_lock_try_lock(&self->data.lock)) goto error;

    bool result = dtn_dict_set(self->data.dict, key, val, NULL);

    if (result) val->data = data;

    if (!dtn_thread_lock_unlock(&self->data.lock)){
        dtn_log_warning("Failed to unlock async data");
        goto error;
    }

    if (!result) goto error;
    return true;
error:
    key = dtn_data_pointer_free(key);
    val = dtn_data_pointer_free(val);
    return false;
}

/*---------------------------------------------------------------------------*/


dtn_async_data dtn_async_get(dtn_async_store *self,
                                   const char *id){

    if (!self || !id) goto error;

    if (!dtn_thread_lock_try_lock(&self->data.lock)) goto error;

    internal_session_data *internal = dtn_dict_remove(self->data.dict, id);

    if (!dtn_thread_lock_unlock(&self->data.lock)){
        dtn_log_warning("Failed to unlock async data");
        goto error; 
    }

    if (!internal) goto error;

    dtn_async_data data = internal->data;
    internal->data = (dtn_async_data){0};
    internal = free_internal_session_data(internal);

    return data;
error:
    return (dtn_async_data){0};
}

/*----------------------------------------------------------------------------*/

struct container_socket {

  int socket;
  dtn_list *list;
};

/*----------------------------------------------------------------------------*/

static bool search_ids_of_socket(const void *key, void *val, void *data) {

  if (!key)
    return true;

  struct container_socket *c = (struct container_socket *)data;
  internal_session_data *d = (internal_session_data *)val;

  if (c->socket == d->data.socket)
    return dtn_list_push(c->list, (void *)key);

  return true;
}

/*----------------------------------------------------------------------------*/

static bool drop_entry(void *val, void *data) {

  dtn_async_store *store = (dtn_async_store *)data;
  return dtn_dict_del(store->data.dict, val);
}

/*---------------------------------------------------------------------------*/

bool dtn_async_drop(dtn_async_store *self, int socket){

    dtn_list *list = NULL;
    bool result = false;

    if (!self) goto error;

    list = dtn_linked_list_create((dtn_list_config){0});

    if (!dtn_thread_lock_try_lock(&self->data.lock)) goto error;

    struct container_socket c =
      (struct container_socket){.socket = socket, .list = list};

    result = dtn_dict_for_each(self->data.dict, &c, search_ids_of_socket);

    if (!result)
        goto done;

    result &= dtn_list_for_each(list, self, drop_entry);

done:
    if (!dtn_thread_lock_unlock(&self->data.lock)){
        dtn_log_warning("Failed to unlock async data");
        goto error;
    }

    dtn_list_free(list);
    return result;
error:
    dtn_list_free(list);
    return false;
}