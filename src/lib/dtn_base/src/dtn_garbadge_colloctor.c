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
        @file           dtn_garbadge_colloctor.c
        @author         Töpfer, Markus

        @date           2025-12-21


        ------------------------------------------------------------------------
*/
#include "../include/dtn_garbadge_colloctor.h"
#include "../include/dtn_dict.h"
#include "../include/dtn_linked_list.h"
#include "../include/dtn_thread_lock.h"
#include "../include/dtn_thread_loop.h"
#include "../include/dtn_thread_message.h"
#include "../include/dtn_utils.h"

struct dtn_garbadge_colloctor {

    dtn_garbadge_colloctor_config config;

    dtn_thread_loop *tloop;

    struct {

        dtn_thread_lock lock;
        dtn_list *queue;

    } data;

    struct {

        uint32_t collect;

    } timer;
};

/*------------------------------------------------------------------*/

typedef struct CollectorData {

    void *data;
    void *(*free)(void *data);

} CollectorData;

/*------------------------------------------------------------------*/

static void *collector_data_free(void *data) {

    if (!data)
        return NULL;

    CollectorData *self = (CollectorData *)data;

    if (self->data) {

        self->data = self->free(self->data);

        if (self->data) {
            dtn_log_error("FREE unsuccessfull dataloss may occure.");
        }
    }

    self = dtn_data_pointer_free(self);
    return NULL;
}

/*------------------------------------------------------------------*/

static bool handle_in_thread(dtn_thread_loop *tloop, dtn_thread_message *msg) {

    dtn_garbadge_colloctor *self = dtn_thread_loop_get_data(tloop);
    if (!self)
        goto error;

    msg = dtn_thread_message_free(msg);
    if (!dtn_thread_lock_try_lock(&self->data.lock))
        return true;

    dtn_list *queue = self->data.queue;

    self->data.queue = dtn_linked_list_create(
        (dtn_list_config){.item.free = collector_data_free});

    CollectorData *collect = dtn_list_queue_pop(queue);

    while (collect) {

        collect->data = collect->free(collect->data);

        if (collect->data) {

            // free wasn't successfull, maybe data is still locked
            dtn_list_queue_push(self->data.queue, collect);

        } else {

            collect = collector_data_free(collect);
        }

        collect = dtn_list_queue_pop(queue);
    }

    queue = dtn_list_free(queue);

    if (!dtn_thread_lock_unlock(&self->data.lock)) {
        dtn_log_error("Failed to unlock lock");
    }

    return true;
error:
    dtn_thread_message_free(msg);
    return false;
}

/*------------------------------------------------------------------*/

static bool handle_in_loop(dtn_thread_loop *loop, dtn_thread_message *msg) {

    UNUSED(loop);
    dtn_log_error("unexpected to loop messgae received.");
    msg = dtn_thread_message_free(msg);
    return true;
}

/*------------------------------------------------------------------*/

static bool timer_run_cleanup(uint32_t id, void *data) {

    dtn_garbadge_colloctor *self = (dtn_garbadge_colloctor *)data;
    UNUSED(id);
    if (!self)
        goto error;

    dtn_thread_message *msg =
        dtn_thread_message_standard_create(DTN_GENERIC_MESSAGE, NULL);

    if (!msg)
        goto reschedule;

    if (!dtn_thread_loop_send_message(self->tloop, msg, DTN_RECEIVER_THREAD)) {
        msg = dtn_thread_message_free(msg);
    }

reschedule:

    self->timer.collect = dtn_event_loop_timer_set(
        self->config.loop, self->config.limits.run_cleanup_usec, self,
        timer_run_cleanup);

    return true;
error:
    return false;
}

/*------------------------------------------------------------------*/

dtn_garbadge_colloctor *
dtn_garbadge_colloctor_create(dtn_garbadge_colloctor_config config) {

    dtn_garbadge_colloctor *self = NULL;

    if (!config.loop)
        goto error;

    if (0 == config.limits.run_cleanup_usec)
        config.limits.run_cleanup_usec = 3000000; // 3 sec

    if (0 == config.limits.threadlock_timeout_usec)
        config.limits.threadlock_timeout_usec = 100000; // 100 msec

    self = calloc(1, sizeof(dtn_garbadge_colloctor));
    if (!self)
        goto error;

    self->config = config;

    self->tloop =
        dtn_thread_loop_create(self->config.loop,
                               (dtn_thread_loop_callbacks){
                                   .handle_message_in_thread = handle_in_thread,
                                   .handle_message_in_loop = handle_in_loop},
                               self);

    if (!self->tloop)
        goto error;

    if (!dtn_thread_loop_reconfigure(
            self->tloop, (dtn_thread_loop_config){
                             .disable_to_loop_queue = true,
                             .message_queue_capacity = 1000,
                             .lock_timeout_usecs =
                                 self->config.limits.threadlock_timeout_usec,
                             .num_threads = 1}))
        goto error;

    if (!dtn_thread_loop_start_threads(self->tloop))
        goto error;

    if (!dtn_thread_lock_init(&self->data.lock,
                              self->config.limits.threadlock_timeout_usec))
        goto error;

    self->data.queue = dtn_linked_list_create(
        (dtn_list_config){.item.free = collector_data_free});

    if (!self->data.queue)
        goto error;

    self->timer.collect = dtn_event_loop_timer_set(
        self->config.loop, self->config.limits.run_cleanup_usec, self,
        timer_run_cleanup);

    if (DTN_TIMER_INVALID == self->timer.collect)
        goto error;

    return self;
error:
    dtn_garbadge_colloctor_free(self);
    return NULL;
}

/*------------------------------------------------------------------*/

dtn_garbadge_colloctor *
dtn_garbadge_colloctor_free(dtn_garbadge_colloctor *self) {

    if (!self)
        return self;

    if (DTN_TIMER_INVALID != self->timer.collect)
        dtn_event_loop_timer_unset(self->config.loop, self->timer.collect,
                                   NULL);

    self->data.queue = dtn_list_free(self->data.queue);
    self->tloop = dtn_thread_loop_free(self->tloop);
    dtn_thread_lock_clear(&self->data.lock);
    self = dtn_data_pointer_free(self);
    return NULL;
}

/*------------------------------------------------------------------*/

bool dtn_garbadge_colloctor_push(dtn_garbadge_colloctor *self, void *data,
                                 void *(*free_data)(void *data)) {

    CollectorData *collector = NULL;

    if (!self || !data || !free_data)
        goto error;

    collector = calloc(1, sizeof(CollectorData));
    if (!collector)
        goto error;

    collector->data = data;
    collector->free = free_data;

    if (!dtn_thread_lock_try_lock(&self->data.lock))
        goto error;

    bool result = dtn_list_queue_push(self->data.queue, collector);

    if (!dtn_thread_lock_unlock(&self->data.lock)) {
        dtn_log_error("Failed to unlock data lock");
    }

    if (!result)
        collector = collector_data_free(collector);

    return result;
error:
    collector_data_free(collector);
    return false;
}