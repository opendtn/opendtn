/***

  Copyright   2018       German Aerospace Center DLR e.V.,
  German Space Operations Center (GSOC)

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

This file is part of the openvocs project. http://openvocs.org

 ***/ /**

      \file               dtn_thread_pool_tests.c
      \author             Michael J. Beer, DLR/GSOC <michael.beer@dlr.de>
      \date               2018-01-18

      \ingroup            empty

     **/
/*---------------------------------------------------------------------------*/

#include "../include/dtn_utils.h"
#include "../include/testrun.h"
#include "dtn_thread_pool.c"
#include <signal.h>

/*---------------------------------------------------------------------------*/

static const int SLEEP_MSECS = 1000;

/*---------------------------------------------------------------------------*/

int sleep_usec(uint64_t usecs) {

    struct timespec t = {

        .tv_sec = usecs / 1000 / 1000,

    };

    t.tv_nsec = 1000 * (usecs - t.tv_sec * 1000 * 1000);

    if (0 != nanosleep(&t, 0)) {

        return -1;
    }

    return 0;
}

/*---------------------------------------------------------------------------*/

static bool put_into_buffer(void *queue, void *element) {

    testrun(queue);

    dtn_thread_queue *q = queue;

    dtn_ringbuffer *rb = q->queue;
    dtn_thread_lock *lock = q->lock;

    testrun(rb);

    if (0 != lock) {
        testrun(dtn_thread_lock_try_lock(lock));
    }

    bool res = rb->insert(rb, element);

    if (0 != lock) {
        testrun(dtn_thread_lock_unlock(lock));
    }

    return res;
}

/*---------------------------------------------------------------------------*/

dtn_thread_pool *create_test_pool(dtn_ringbuffer **b1, dtn_thread_lock *l1,
                                  dtn_thread_queue *queue) {

    DTN_ASSERT(0 != queue);

    dtn_ringbuffer *buffer1 = 0;
    dtn_ringbuffer *buffer2 = 0;

    if (b1) {

        buffer1 = *b1;
    }

    if (!buffer1) {

        buffer1 = dtn_ringbuffer_create(3, 0, 0);
    }

    if (b1) {

        *b1 = buffer1;
    }

    if (queue->queue) {

        buffer2 = queue->queue;
    }

    if (!buffer2) {

        buffer2 = dtn_ringbuffer_create(3, 0, 0);
    }

    queue->queue = buffer2;

    dtn_thread_queue incoming = {
        .queue = buffer1,
        .lock = l1,
    };

    return dtn_thread_pool_create(incoming, put_into_buffer,
                                  (dtn_thread_pool_config){
                                      .userdata = queue,
                                  });
}

/*---------------------------------------------------------------------------*/

static void count_frees(void *additional_arg, void *element) {

    UNUSED(element);

    size_t *count = additional_arg;
    ++*count;
}

/*---------------------------------------------------------------------------*/

static void *dummy_process_function_passed_element = 0;

static bool dummy_process_function(void *pool, void *element) {

    UNUSED(pool);
    dummy_process_function_passed_element = element;

    return true;
}

/******************************************************************************
 *  TESTS
 ******************************************************************************/

static int test_dtn_thread_pool_create() {

    dtn_thread_queue incoming = {0};

    dtn_thread_pool_config config = {0};

    dtn_thread_pool *thread = dtn_thread_pool_create(incoming, 0, config);

    testrun(!thread);

    dtn_ringbuffer *buf1 = dtn_ringbuffer_create(3, 0, 0);
    testrun(buf1);

    incoming.queue = buf1;

    testrun(0 == dtn_thread_pool_create(incoming, 0, config));

    dtn_ringbuffer *buf2 = dtn_ringbuffer_create(3, 0, 0);
    testrun(buf2);
    config.userdata = buf2;

    thread = dtn_thread_pool_create(incoming, put_into_buffer, config);

    testrun(thread);

    testrun(thread_start == thread->start);
    testrun(thread_stop == thread->stop);
    testrun(thread_free == thread->free);

    internal_pool *internal = (internal_pool *)thread;

    testrun(internal->incoming.queue == buf1);
    testrun(put_into_buffer == internal->process_func);

    thread = thread->free(thread);

    config = (dtn_thread_pool_config){.num_threads = 3};
    thread = dtn_thread_pool_create(incoming, put_into_buffer, config);

    testrun(thread);

    testrun(thread_start == thread->start);
    testrun(thread_stop == thread->stop);
    testrun(thread_free == thread->free);

    internal = (internal_pool *)thread;

    testrun(put_into_buffer == internal->process_func);

    testrun(internal->incoming.queue == buf1);

    thread = thread->free(thread);

    thread = dtn_thread_pool_create(incoming, dummy_process_function, config);

    testrun(thread);

    testrun(thread_start == thread->start);
    testrun(thread_stop == thread->stop);
    testrun(thread_free == thread->free);

    internal = (internal_pool *)thread;

    testrun(dummy_process_function == internal->process_func);

    testrun(internal->incoming.queue == buf1);

    thread = thread->free(thread);

    thread = dtn_thread_pool_create(incoming, dummy_process_function, config);

    testrun(thread);

    testrun(thread_start == thread->start);
    testrun(thread_stop == thread->stop);
    testrun(thread_free == thread->free);

    internal = (internal_pool *)thread;

    testrun(dummy_process_function == internal->process_func);

    testrun(internal->incoming.queue == buf1);

    thread = thread->free(thread);

    thread = dtn_thread_pool_create(incoming, dummy_process_function, config);

    testrun(thread);

    testrun(thread_start == thread->start);
    testrun(thread_stop == thread->stop);
    testrun(thread_free == thread->free);

    internal = (internal_pool *)thread;

    testrun(dummy_process_function == internal->process_func);

    testrun(internal->incoming.queue == buf1);

    thread = thread->free(thread);
    buf1 = buf1->free(buf1);
    buf2 = buf2->free(buf2);
    testrun(0 == thread);
    testrun(0 == buf1);
    testrun(0 == buf2);

    return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

static int test_thread_start() {

    testrun(!thread_start(0));

    dtn_ringbuffer *b1 = 0;
    dtn_ringbuffer *b2 = 0;
    dtn_thread_lock l1;
    dtn_thread_lock l2;

    testrun(dtn_thread_lock_init(&l1, 1000000));
    testrun(dtn_thread_lock_init(&l2, 1000000));

    dtn_thread_queue queue = {
        .lock = &l2,
    };

    dtn_thread_pool *thread = create_test_pool(&b1, &l1, &queue);
    testrun(thread);
    testrun(b1);

    b2 = queue.queue;

    testrun(b2);

    internal_pool *internal = (internal_pool *)thread;

    internal->pool_state = RUNNING;
    testrun(!thread_start(thread));

    internal->pool_state = TO_STOP;
    testrun(!thread_start(thread));

    internal->pool_state = STOPPED;
    testrun(thread_start(thread));
    testrun(RUNNING == internal->pool_state);

    /* Check that the thread is running for real */

    int testValue = 13;

    testrun(dtn_thread_lock_try_lock(&l1));

    testrun(b1->insert(b1, &testValue));

    testrun(dtn_thread_lock_unlock(&l1));

    sched_yield();
    sleep_usec(SLEEP_MSECS * 1000);

    int *received = b2->pop(b2);

    testrun(received);
    testrun(testValue == *received);
    testrun(0 == b2->pop(b2));

    thread = thread->free(thread);

    b1->free(b1);
    b2->free(b2);

    return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

static int test_thread_stop() {

    testrun(!thread_stop(0));

    dtn_ringbuffer *b1 = 0;
    dtn_ringbuffer *b2 = 0;
    dtn_thread_queue queue = {0};

    dtn_thread_pool *thread = create_test_pool(&b1, 0, &queue);
    testrun(thread);
    testrun(b1);

    b2 = queue.queue;
    testrun(b2);

    internal_pool *internal = (internal_pool *)thread;

    testrun(thread->start(thread));

    internal->pool_state = TO_STOP;
    testrun(!thread_stop(thread));

    /* A stopped thread technically cannot be stopped. */
    internal->pool_state = STOPPED;
    testrun(!thread_stop(thread));

    internal->pool_state = RUNNING;
    testrun(thread_stop(thread));
    testrun(STOPPED == internal->pool_state);

    /* stopping an already stopped thread keeps it stopped ... */
    testrun(!thread_stop(thread));
    testrun(STOPPED == internal->pool_state);

    thread = thread->free(thread);
    b1 = b1->free(b1);
    b2 = b2->free(b2);

    return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

static int test_thread_run() {

    dtn_thread_lock in_lock;
    dtn_thread_lock_init(&in_lock, 1000000);

    dtn_ringbuffer *in_buf = dtn_ringbuffer_create(3, 0, 0);
    dtn_ringbuffer *out_buf = out_buf = dtn_ringbuffer_create(3, 0, 0);
    testrun(in_buf);
    testrun(out_buf);

    dtn_thread_lock out_lock;
    dtn_thread_lock_init(&out_lock, 1000000);

    dtn_thread_queue queue = {
        .queue = out_buf,
        .lock = &out_lock,
    };

    dtn_thread_pool_config config = {
        .userdata = &queue,
    };

    dtn_thread_pool *thread = dtn_thread_pool_create(
        (dtn_thread_queue){
            .lock = &in_lock,
            .queue = in_buf,
        },
        put_into_buffer, config);

    testrun(thread);

    testrun(thread->start(thread));

    testrun(0 == out_buf->pop(out_buf));

    while (!dtn_thread_lock_try_lock(&in_lock)) {

        /* NOP */
    };

    /* Now input queue should be locked */
    int testValue = 13;

    in_buf->insert(in_buf, &testValue);

    for (size_t repeats = 0; repeats < 10; ++repeats) {

        /* All in all, we will wait 1sec, that should be enough for the
         * thread to grab the element out of the queue -
         * however, it SHOULD NOT, since we locked it  -
         * Thus, our testvalue should stay in the in queue
         */
        sleep_usec(100 * SLEEP_MSECS);
        testrun(&testValue == in_buf->pop(in_buf));
        in_buf->insert(in_buf, &testValue);
    }

    /* Still locked - now unlock and see whether the queue gets emptied */
    testrun(dtn_thread_lock_unlock(&in_lock));
    /* Now processing should proceed */

    sched_yield();
    sleep_usec(1000 * SLEEP_MSECS);

    testrun(0 == in_buf->pop(in_buf));

    /* Element should have appeared in out_buf */
    testrun(&testValue == out_buf->pop(out_buf));

    /* Repeat, but with out_buf locked */
    testrun(0 == out_buf->pop(out_buf));

    while (!dtn_thread_lock_try_lock(&in_lock)) {

        /* NOP */
    };

    in_buf->insert(in_buf, &testValue);
    for (size_t repeats = 0; repeats < 10; ++repeats) {

        /* All in all, we will wait 1sec, that should be enough for the
         * thread to grab the element out of the queue */
        sleep_usec(100 * SLEEP_MSECS);
        testrun(&testValue == in_buf->pop(in_buf));
        in_buf->insert(in_buf, &testValue);
    }

    /* Still locked - now unlock and see whether the queue gets emptied */
    dtn_thread_lock_unlock(&in_lock);

    sched_yield();
    sleep_usec(1000 * SLEEP_MSECS);

    testrun(0 == in_buf->pop(in_buf));

    thread = thread->free(thread);
    in_buf = in_buf->free(in_buf);

    /* Check whether a given process function will be called if provided */

    in_buf = dtn_ringbuffer_create(3, 0, 0);
    testrun(in_buf);

    config.userdata = out_buf;

    thread = dtn_thread_pool_create(
        (dtn_thread_queue){
            .lock = &in_lock,
            .queue = in_buf,
        },
        dummy_process_function, config);

    testrun(thread);
    testrun(thread->start(thread));

    while (!dtn_thread_lock_try_lock(&in_lock)) {

        /* NOP */
    };

    in_buf->insert(in_buf, &testValue);

    /* Still locked - now unlock and see whether the queue gets emptied */
    testrun(dtn_thread_lock_notify(&in_lock));
    testrun(dtn_thread_lock_unlock(&in_lock));

    sched_yield();
    sleep_usec(1000 * SLEEP_MSECS);

    // testrun(0 == in_buf->pop(in_buf));

    /* dummy_process_function should have been called... */
    testrun(&testValue == dummy_process_function_passed_element);

    testrun(dtn_thread_lock_unlock(&in_lock));

    thread = thread->free(thread);
    in_buf = in_buf->free(in_buf);
    out_buf = out_buf->free(out_buf);

    /* Check several threads */

    /* Create ringbuffers with a slighty higher capacity ... */

    size_t free_in = 0;
    size_t free_out = 0;

    in_buf = dtn_ringbuffer_create(57, count_frees, &free_in);
    out_buf = dtn_ringbuffer_create(57, count_frees, &free_out);

    testrun(in_buf);
    testrun(out_buf);

    queue.queue = out_buf;

    thread = dtn_thread_pool_create(
        (dtn_thread_queue){
            .lock = &in_lock,
            .queue = in_buf,
        },
        put_into_buffer,
        (dtn_thread_pool_config){
            .num_threads = 10,
            .userdata = &queue,
        });

    testrun(thread);
    testrun(thread->start(thread));

    const size_t in_buf_capacity = in_buf->capacity(in_buf);

    for (uintptr_t i = 1; i < in_buf_capacity; ++i) {

        while (!dtn_thread_lock_try_lock(&in_lock)) {
            /* NOP */
        };

        testrun(in_buf->insert(in_buf, (void *)i));

        testrun(dtn_thread_lock_notify(&in_lock));
        testrun(dtn_thread_lock_unlock(&in_lock));
    }

    sched_yield();
    sleep_usec(SLEEP_MSECS * 1000);

    thread = thread->free(thread);
    testrun(0 == thread);

    bool *found = calloc(1, in_buf_capacity);

    for (size_t i = 0; i < in_buf_capacity; ++i) {
        found[i] = false;
    }

    /* Now all testvalues' addresses should have been put into out_buf */
    for (size_t i = 1; i < in_buf_capacity; ++i) {

        uintptr_t content = (uintptr_t)out_buf->pop(out_buf);
        testrun(0 != content);
        // printf("content is %i\n", content);
        testrun(in_buf_capacity > content);

        found[content] = true;
    }

    for (size_t i = 1; i < in_buf_capacity; ++i) {
        testrun(found[i]);
    }

    free(found);
    found = 0;

    in_buf = in_buf->free(in_buf);
    out_buf = out_buf->free(out_buf);

    return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

static int test_dtn_thread_pool_free() {

    testrun(0 == thread_free(0));

    dtn_ringbuffer *buf1 = dtn_ringbuffer_create(3, 0, 0);
    testrun(buf1);

    dtn_thread_queue incoming = {
        .queue = buf1,
    };

    dtn_thread_pool *thread = dtn_thread_pool_create(
        incoming, put_into_buffer, (dtn_thread_pool_config){0});

    testrun(thread);

    thread = thread->free(thread);
    testrun(0 == thread);

    buf1 = buf1->free(buf1);

    testrun(0 == buf1);

    return testrun_log_success();
}

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CLUSTER                                                    #CLUSTER
 *
 *      ------------------------------------------------------------------------
 */

int all_tests() {

    testrun_init();
    testrun_test(test_dtn_thread_pool_create);
    testrun_test(test_thread_start);
    testrun_test(test_thread_stop);
    testrun_test(test_thread_run);
    testrun_test(test_dtn_thread_pool_free);

    return testrun_counter;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST EXECUTION                                                  #EXEC
 *
 *      ------------------------------------------------------------------------
 */

testrun_run(all_tests);