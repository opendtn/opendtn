/***

Copyright   2018        German Aerospace Center DLR e.V.,
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

     \file               dtn_thread_pool.h
     \author             Michael J. Beer, DLR/GSOC <michael.beer@dlr.de>
     \date               2018-01-18

     \ingroup            empty

     Provides a thread object that keeps taking elements out of
     queue 1, process each element into some different element and
     putting it into queue 2.

     If locks are provided, they are used to lock down on the queues before
     using them.

     By default, this pool just shifts the elements from queue 1 to
     queue 2 performing no further processing.

     However, by providing a function `process_function` to
     dtn_thread_pool_create(),
     arbitrary processing can take place.
     `process_function` is called for each received element.
     The elements are not managed in any ways, thus the process_function will
     have to free them if necessary.

     Either process_func and notify_func are optional.
     If either one is not supplied, default implementations are set up.
     If no process_func is supplied, the received element will be sent back.
     If no notify_func is supplied, NOTIFY_SIGNAL signal will be sent to the
     thread that
     created the pool.

     Some basic statistical counters are maintained (see
     dtn_thread_pool_statistics).

 */
/*---------------------------------------------------------------------------*/

#ifndef dtn_thread_pool_h
#define dtn_thread_pool_h

/*---------------------------------------------------------------------------*/

#include "dtn_ringbuffer.h"
#include "dtn_thread_lock.h"

/******************************************************************************
 *                                 CONSTANTS
 ******************************************************************************/

extern const int dtn_THREADPOOL_NOTIFY_SIGNAL;

/******************************************************************************
 *
 *  TYPEDEFS
 *
 ******************************************************************************/

/**
 * Statistical counters for dtn_thread_pool.
 */
typedef struct {

    struct {
        uint64_t incoming;
    } lock_blocked;

    struct {
        uint64_t received;
        uint64_t processed;
        uint64_t lost;
    } elements;

} dtn_thread_pool_statistics;

/*---------------------------------------------------------------------------*/

typedef struct {

    /**
     * 0 denotes: Choose at will. Might be cut down to a max number of
     * threads
     */
    size_t num_threads;
    void *userdata;

} dtn_thread_pool_config;

/*---------------------------------------------------------------------------*/

typedef struct dtn_thread_pool_struct dtn_thread_pool;

struct dtn_thread_pool_struct {

    uint32_t magic_bytes;

    bool (*start)(dtn_thread_pool *self);
    bool (*stop)(dtn_thread_pool *self);
    dtn_thread_pool *(*free)(dtn_thread_pool *self);

    dtn_thread_pool_statistics (*get_statistics)(dtn_thread_pool *self);

    void *user_data; /* User might add whatever he wants */
};

/*---------------------------------------------------------------------------*/

typedef struct {

    dtn_thread_lock *lock;
    dtn_ringbuffer *queue;

} dtn_thread_queue;

/*---------------------------------------------------------------------------*/

typedef bool (*dtn_thread_pool_function)(void *userdata, void *element);

/******************************************************************************
 *
 *  FUNCTIONS
 *
 ******************************************************************************/

dtn_thread_pool *dtn_thread_pool_create(dtn_thread_queue incoming_queue,
                                        dtn_thread_pool_function process_func,
                                        dtn_thread_pool_config config);

/*---------------------------------------------------------------------------*/

#endif /* dtn_thread_pool_h */
