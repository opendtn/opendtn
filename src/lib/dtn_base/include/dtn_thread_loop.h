/***

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

***/ /**

     \file               dtn_thread_loop.h
     \author             Michael J. Beer, DLR/GSOC
     \date               2018-07-11

     \ingroup            empty

     Ties together an event loop with a thread pool.
     Provides for means to send information from the callbacks of the loop
     to the pool and back.

 **/
/*---------------------------------------------------------------------------*/

#ifndef dtn_thread_loop_h
#define dtn_thread_loop_h

#include "dtn_event_loop.h"
#include "dtn_thread_message.h"
#include "dtn_item.h"


/******************************************************************************
 *                                  TYPEDEFS
 ******************************************************************************/

typedef enum {

  RESERVED_DTN_RECEIVER_ENSURE_SIGNED_INT = INT_MIN,
  DTN_RECEIVER_EVENT_LOOP = 0,
  DTN_RECEIVER_THREAD

} dtn_thread_receiver;

/*----------------------------------------------------------------------------*/

typedef struct dtn_thread_loop dtn_thread_loop;

/**
 * Configuration.
 * Values that are zero will be replaced by default values.
 */
typedef struct {

  /**
   * If true, no queue will be used to send messages (from the threads) to
   * the event loop.
   * Using a queue incurs some performance penalty, (using the queue,
   * having to lock down on mutex in EL -> 2 additional context
   * switches).
   * On the other hand, if no queue is used, messages that are lost cause
   * mem leaks.
   * Thus if there is no chance that the EL might be too slow to process
   * all the messages from the threads, it might be useful to switch
   * using the queue off.
   * If in doubt, use the queue.
   */
  bool disable_to_loop_queue;

  uint64_t message_queue_capacity;

  uint64_t lock_timeout_usecs;

  size_t num_threads;

} dtn_thread_loop_config;

/*----------------------------------------------------------------------------*/

/**
 * The functions that are called whenever a message has been sent.
 *
 * BEWARE: All handlers need to free the messages themselves.
 */
typedef struct {

  /**
   * Executed in the threads.
   * Do the processing.
   * Avoid I/O here.
   */
  bool (*handle_message_in_thread)(dtn_thread_loop *, dtn_thread_message *);

  /** Executed in the loop thread - called by the loop.
   * Avoid processing.
   * Do I/O here.
   */
  bool (*handle_message_in_loop)(dtn_thread_loop *, dtn_thread_message *);

} dtn_thread_loop_callbacks;

/******************************************************************************
 *
 *  FUNCTIONS
 *
 ******************************************************************************/

/**
 * Create a new thread_pool_process.
 * In here, only a minimalistic config is done.
 * The actual configuration is done via a call to dtn_thread_loop_reconfigure.
 *
 * @param event_loop the event loop to use
 * @param thread_pool_process_data additional data that can be retrieved
 * in the handlers via dtn_thread_loop_get_data()
 */
dtn_thread_loop *dtn_thread_loop_create(dtn_event_loop *event_loop,
                                      dtn_thread_loop_callbacks callbacks,
                                      void *thread_pool_process_data);

/*----------------------------------------------------------------------------*/

/**
 * Stop thread pool, free resources.
 */
dtn_thread_loop *dtn_thread_loop_free(dtn_thread_loop *self);

/**
 * Get the thread_pool_process specific data that was supplied to
 * dtn_thread_loop_create()
 *
 * BEWARE: The data is NOT synced!
 */
void *dtn_thread_loop_get_data(dtn_thread_loop *self);

/*----------------------------------------------------------------------------*/

bool dtn_thread_loop_reconfigure(dtn_thread_loop *self,
                                dtn_thread_loop_config config);

/*----------------------------------------------------------------------------*/

/**
 * Send messages along.
 * @param self the threaded thread_pool_process.
 * @param message message to be sent.
 * @param receiver denotes the receiver of the message.
 * @return true on success, false else
 */
bool dtn_thread_loop_send_message(dtn_thread_loop *self,
                                 dtn_thread_message *message,
                                 dtn_thread_receiver receiver);

/*----------------------------------------------------------------------------*/

bool dtn_thread_loop_start_threads(dtn_thread_loop *self);

/**
 * Stops all threads of the thread pool.
 * Must be done before e.g. resetting the thread_pool_process_data if the
 * threads access it.
 */
bool dtn_thread_loop_stop_threads(dtn_thread_loop *self);

/*----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*/
/**
 * Serialize dtn_threads_pool_process_config as JSON.
 * Configuration will be overwritten.
 * @target Target JSON object. If null, a new JSON object will be created.
 */
dtn_item *dtn_thread_loop_config_to_json(const dtn_thread_loop_config config,
                                             dtn_item *target);

/*----------------------------------------------------------------------------*/

/**
 * Deserialize dtn_thread_loop_config from JSON
 */
dtn_thread_loop_config
dtn_thread_loop_config_from_json(const dtn_item *restrict json);

/*----------------------------------------------------------------------------*/
#endif /* dtn_thread_loop_h */
