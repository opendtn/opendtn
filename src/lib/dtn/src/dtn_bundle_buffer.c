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
        @file           dtn_bundle_buffer.c
        @author         Töpfer, Markus

        @date           2025-12-23


        ------------------------------------------------------------------------
*/
#include "../include/dtn_bundle_buffer.h"

#include <dtn_base/dtn_dict.h>
#include <dtn_base/dtn_thread_lock.h>
#include <dtn_base/dtn_linked_list.h>
#include <dtn_base/dtn_time.h>
#include <dtn_base/dtn_string.h>


/*---------------------------------------------------------------------------*/

struct dtn_bundle_buffer {

    dtn_bundle_buffer_config config;

    struct {

        dtn_thread_lock lock;
        dtn_dict *dict;

    } data;

    struct {

        dtn_thread_lock lock;
        dtn_dict *dict;

    } history;
    

    struct {

        uint32_t cleanup;

    } timer;
};

/*---------------------------------------------------------------------------*/

typedef struct Data {

    uint64_t created;
    dtn_list *queue;

} Data;

/*---------------------------------------------------------------------------*/

static Data *data_create(){

    Data *self = calloc(1, sizeof(Data));
    self->created = dtn_time_get_current_time_usecs();
    self->queue = dtn_linked_list_create((dtn_list_config){
        .item.free = dtn_bundle_free_void
    });

    return self;
}

/*---------------------------------------------------------------------------*/

static void *data_free(void *self){

    if (!self) return NULL;

    Data *data = (Data*) self;
    data->queue = dtn_list_free(data->queue);
    data = dtn_data_pointer_free(data);
    return NULL;
}

/*---------------------------------------------------------------------------*/

static bool init_config(dtn_bundle_buffer_config *config){

    if (!config || !config->loop) goto error;


    if (0 == config->limits.max_buffer_time_secs)
        config->limits.max_buffer_time_secs =  24 * 60 * 60; // 24h

    if (0 == config->limits.threadlock_timeout_usecs)
        config->limits.threadlock_timeout_usecs = 100000; // 100ms

    if (0 == config->limits.buffer_time_cleanup_usecs)
        config->limits.buffer_time_cleanup_usecs = 5000000; // 5min

    if (0 == config->limits.history_secs)
        config->limits.history_secs = 24 * 60 * 60; // 24h

    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

struct container {

    uint64_t now;
    dtn_bundle_buffer *self;
    dtn_list *list;
};

/*---------------------------------------------------------------------------*/

static bool search_expired_keys(const void *key, void *val, void *data){

    if (!key) return true;

    struct container *container = (struct container*) data;
    Data *self = (Data*) val;

    if (container->now - self->created > 
        1000000 *container->self->config.limits.max_buffer_time_secs){

        dtn_list_push(container->list, (void*) key);
    }

    return true;
}

/*---------------------------------------------------------------------------*/

static bool drop_expired_keys(void *item, void *data){

    dtn_dict *dict = dtn_dict_cast(data);
    dtn_dict_del(dict, item);
    return true;
}

/*---------------------------------------------------------------------------*/

static bool search_expired_keys_history(const void *key, void *val, void *data){

    if (!key) return true;

    struct container *container = (struct container*) data;

    uint64_t created = (uintptr_t)val;

    if (container->now - created > 
        1000000 * container->self->config.limits.history_secs){

        dtn_list_push(container->list, (void*) key);
    }

    return true;
}


/*---------------------------------------------------------------------------*/

static bool run_cleanup(uint32_t id, void *data){

    UNUSED(id);
    dtn_bundle_buffer *self = (dtn_bundle_buffer*) data;

    if (!dtn_thread_lock_try_lock(&self->data.lock)) goto reschedule;

    struct container container = (struct container){

        .now = dtn_time_get_current_time_usecs(),
        .self = self,
        .list = dtn_linked_list_create((dtn_list_config){0})
    };

    dtn_dict_for_each(self->data.dict, 
                      &container, 
                      search_expired_keys);

    dtn_list_for_each(container.list, 
                     self->data.dict,
                     drop_expired_keys);

    container.list = dtn_list_free(container.list);

    if (!dtn_thread_lock_unlock(&self->data.lock)){
        dtn_log_error("failed to unlock data");
    }

    if (!dtn_thread_lock_try_lock(&self->history.lock)) goto reschedule;

    container = (struct container){

        .now = dtn_time_get_current_time_usecs(),
        .self = self,
        .list = dtn_linked_list_create((dtn_list_config){0})
    };

    dtn_dict_for_each(self->history.dict, 
                      &container, 
                      search_expired_keys_history);

    dtn_list_for_each(container.list, 
                     self->history.dict,
                     drop_expired_keys);

    container.list = dtn_list_free(container.list);

    if (!dtn_thread_lock_unlock(&self->history.lock)){
        dtn_log_error("failed to unlock data");
    }


reschedule:
    
    self->timer.cleanup = dtn_event_loop_timer_set(
        self->config.loop,
        self->config.limits.buffer_time_cleanup_usecs,
        self, 
        run_cleanup);

    return true;

}

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_bundle_buffer *dtn_bundle_buffer_create(dtn_bundle_buffer_config config){

    dtn_bundle_buffer *self = NULL;

    if (!init_config(&config)) goto error;

    self = calloc(1, sizeof(dtn_bundle_buffer));
    if (!self) goto error;

    self->config = config;

    dtn_dict_config d_config = dtn_dict_string_key_config(255);
    d_config.value.data_function.free = data_free;

    self->data.dict = dtn_dict_create(d_config);
    if (!self->data.dict) goto error;

    d_config = dtn_dict_string_key_config(255);
    d_config.value.data_function.free = NULL;

    self->history.dict = dtn_dict_create(d_config);
    if (!self->history.dict) goto error;

    if (!dtn_thread_lock_init(&self->data.lock, 
        config.limits.threadlock_timeout_usecs)) goto error;

    if (!dtn_thread_lock_init(&self->history.lock, 
        config.limits.threadlock_timeout_usecs)) goto error;

    self->timer.cleanup = dtn_event_loop_timer_set(
        self->config.loop,
        self->config.limits.buffer_time_cleanup_usecs,
        self, 
        run_cleanup);

    return self;
error:
    dtn_bundle_buffer_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_bundle_buffer *dtn_bundle_buffer_free(dtn_bundle_buffer *self){

    if (!self) return self;

    if (DTN_TIMER_INVALID != self->timer.cleanup){

        dtn_event_loop_timer_unset(
            self->config.loop, self->timer.cleanup, NULL);
    }

    dtn_thread_lock_clear(&self->data.lock);
    self->data.dict = dtn_dict_free(self->data.dict);
    self = dtn_data_pointer_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

static bool unfragmented_bundle(dtn_bundle_buffer *self, dtn_bundle *bundle){

    const char *dest = dtn_bundle_primary_get_destination(bundle);
    const char *source = dtn_bundle_primary_get_source(bundle);

    dtn_cbor *payload = dtn_bundle_get_block(bundle, 1);
    if (!payload) goto error;

    dtn_cbor *data = dtn_bundle_get_data(payload);

    uint8_t *payload_data = NULL;
    size_t size = 0;

    if (!dtn_cbor_get_byte_string(data, &payload_data, &size)) goto error;

    if (self->config.callbacks.payload)
        self->config.callbacks.payload(
            self->config.callbacks.userdata,
            payload_data,
            size, 
            source, 
            dest);

    // payload callback done, delete bundle
    bundle = dtn_bundle_free(bundle);
    return true;
error:
    bundle = dtn_bundle_free(bundle);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool add_payload_to_buffer(void *val, void *data){

    dtn_buffer *buffer = (dtn_buffer*)data;
    dtn_bundle *bundle = (dtn_bundle*)val;

    dtn_cbor *payload = dtn_bundle_get_block(bundle, 1);
    dtn_cbor *pdata = dtn_bundle_get_data(payload);

    uint8_t *start = NULL;
    size_t length = 0;

    if (!dtn_cbor_get_byte_string(pdata, &start, &length)) goto error;
    if (!dtn_buffer_push(buffer, start, length)) goto error;

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static dtn_buffer *get_output_buffer(dtn_list *queue){

    dtn_buffer *buffer = dtn_buffer_create(2048);

    if (!dtn_list_for_each(queue, buffer, add_payload_to_buffer))
        goto error;

    return buffer;
error:
    dtn_buffer_free(buffer);
    return NULL;
}

/*----------------------------------------------------------------------------*/

static bool bundle_history_contained(dtn_bundle_buffer *self, 
    const char *source, uint64_t timestamp, uint64_t sequence){

    char buffer[2048];
    ssize_t size = 2048;
    memset(buffer, 0, size);

    snprintf(buffer, size, "%s|%"PRIu64"|%"PRIu64, source, timestamp, sequence);

    if (!dtn_thread_lock_try_lock(&self->history.lock)) goto error;

    bool result = dtn_dict_is_set(self->history.dict, buffer);

    if (!result){

        uint64_t now = dtn_time_get_current_time_usecs();
        char *key = dtn_string_dup(buffer);

        if (!dtn_dict_set(self->history.dict, key, (void*)(uintptr_t)now, NULL)){
            key = dtn_data_pointer_free(key);
        }
    }

    if (!dtn_thread_lock_unlock(&self->history.lock)){
        dtn_log_error("failed to unlock history");
    }

    return result;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_buffer_push(dtn_bundle_buffer *self, dtn_bundle *bundle){

    char buffer[2048];
    ssize_t size = 2048;
    memset(buffer, 0, size);

    dtn_buffer *out = NULL;

    if (!self || !bundle) goto error;

    uint64_t flags = dtn_bundle_primary_get_flags(bundle);
    if (! flags & 0x01)
        return unfragmented_bundle(self, bundle);

    const char *source = dtn_bundle_primary_get_source(bundle);
    const char *destination = dtn_bundle_primary_get_destination(bundle);

    uint64_t timestamp = 0;
    uint64_t sequence = 0;

    if (!dtn_bundle_primary_get_timestamp(
        bundle, &timestamp, &sequence)) goto error;

    if (bundle_history_contained(self, source, timestamp, sequence))
        goto drop;

    ssize_t bytes = snprintf(buffer, size, "%s|%"PRIu64, source, timestamp);
    if (bytes >= size) goto error;
    if (bytes < 0) goto error;

    uint64_t size_payload = 0;
    uint64_t size_all = dtn_bundle_primary_get_totel_data_length(bundle);

    if (!dtn_thread_lock_try_lock(&self->data.lock)) goto error;

    Data *data = dtn_dict_get(self->data.dict, buffer);
    if (!data){

        data = data_create();
        if (!data) goto error;
            
        char *key = dtn_string_dup(buffer);
        if (!key) {
            data = data_free(data); 
            goto error;
        }

        if (!dtn_dict_set(self->data.dict, key, data, NULL)){
            data = data_free(data);
            key = dtn_data_pointer_free(key);
            goto error;
        }

        if (!dtn_list_push(data->queue, bundle)) goto error;

        goto done;
    }

    void *item = NULL;
    
    dtn_bundle *current = NULL;
    dtn_cbor *payload_data = NULL;
    dtn_cbor *payload = NULL;

    uint8_t *buf = NULL;
    size_t buf_size = 0;

    uint64_t current_time;
    uint64_t current_sequence;

    void *next = data->queue->iter(data->queue);
    while(next){

        current = NULL;

        next = data->queue->next(data->queue, next, (void**)&item);

        current = (dtn_bundle*) item;

        if (!dtn_bundle_primary_get_timestamp(
            current, &current_time, &current_sequence)) goto error;

        payload = dtn_bundle_get_block(current, 1);
        if (!payload) goto error;

        payload_data = dtn_bundle_get_data(payload);

        if (!dtn_cbor_get_byte_string(payload_data, &buf, &buf_size))
            goto error;

        size_payload += buf_size;

        if (current_sequence >= sequence)
            goto insert_before_current;

    }

    if (!next){

        dtn_list_push(data->queue, bundle);
    
    } else {

insert_before_current:

        size_t pos = dtn_list_get_pos(data->queue, current);
        dtn_list_insert(data->queue, pos, bundle);

    }

    payload = dtn_bundle_get_block(bundle, 1);
    if (!payload) goto error;

    payload_data = dtn_bundle_get_data(payload);

    dtn_cbor_get_byte_string(payload_data, &buf, &buf_size);
    size_payload += buf_size;

    if (size_payload >= size_all){
        out = get_output_buffer(data->queue);
    }

done:

    if (out){

        char s[strlen(source) + 1];
        memset(s, 0, strlen(source) + 1);
        strcat(s, source);

        char d[strlen(destination) + 1];
        memset(d, 0, strlen(destination) + 1);
        strcat(d, destination);

        dtn_dict_del(self->data.dict, buffer);

        if (!dtn_thread_lock_unlock(&self->data.lock)){
            dtn_log_error("failed to unlock data");
        }

        if (self->config.callbacks.payload){

            self->config.callbacks.payload(
                self->config.callbacks.userdata,
                out->start,
                out->length,
                s,
                d);
        }

        out = dtn_buffer_free(out);

    } else {

        if (!dtn_thread_lock_unlock(&self->data.lock)){
            dtn_log_error("failed to unlock data");
        }
    }

    return true;
drop:
    dtn_bundle_free(bundle);
    return true;
error:
    dtn_bundle_free(bundle);
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_buffer_clear(dtn_bundle_buffer *self){

    if (!self) goto error;

    if (!dtn_thread_lock_try_lock(&self->data.lock)) goto error;

    bool result = dtn_dict_clear(self->data.dict);

    if (!dtn_thread_lock_unlock(&self->data.lock)){
        dtn_log_error("failed to unlock data");
    }

    return result;
error:
    return false;
}