/***

  Copyright   2018,2020       German Aerospace Center DLR e.V.,
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

 ***/
/**
 *
 *   \file               dtn_thread_loop.c
 *   \author             Michael J. Beer, DLR/GSOC
 *   \date               2020-03-20
 *
 */
/*---------------------------------------------------------------------------*/

#include "../include/dtn_thread_loop.h"
#include "../include/dtn_thread_pool.h"
#include "../include/dtn_utils.h"

/*---------------------------------------------------------------------------*/

static const uint32_t MAGIC_BYTES = 0x61705450;

/******************************************************************************
 *                           DEFAULT CONFIG VALUES
 ******************************************************************************/

static const uint64_t LOCK_TIMEOUT_USECS_DEFAULT = 100 * 1000;
static const uint64_t MESSAGE_QUEUE_CAPACITY_DEFAULT = 100;
static const size_t NUM_THREADS_DEFAULT = 4;

/******************************************************************************
 *                                CONFIG KEYS
 ******************************************************************************/

#define CONFIG_KEY_LOCK_TIMEOUT_USECS "lock_timeout_usecs"
#define CONFIG_KEY_MESSAGE_QUEUE_CAPACITY "message_queue_capacity"
#define CONFIG_KEY_NUM_THREADS "num_threads"
#define CONFIG_KEY_DISABLE_TO_LOOP_QUEUE "disable_to_loop_queue"

/******************************************************************************
 *                             INTERNAL TYPEDEFS
 ******************************************************************************/

struct dtn_thread_loop {

  uint32_t magic_bytes;

  dtn_event_loop *event_loop;

  struct {
    dtn_ringbuffer *queue;
    dtn_thread_lock lock;
  } to_threads;

  dtn_thread_pool *pool;

  dtn_thread_loop_callbacks callbacks;

  struct to_loop {
    int trigger;
    int catch;
    dtn_thread_lock lock;

    /*
     * If this pointer is null, the message pointers will
     * be forwarded over the trigger socket only.
     * Otherwise, they will be enqueued in this queue as well.
     * @see dtn_thread_loop_config
     * Synchronization is done using lock.
     */
    dtn_ringbuffer *queue;
  } to_loop;

  void *data;
};

/******************************************************************************
 *                             INTERNAL FUNCTIONS
 ******************************************************************************/

static dtn_thread_loop *as_thread_pool_process(void *vptr) {

  if (0 == vptr)
    return 0;

  dtn_thread_loop *tpp = vptr;

  if (MAGIC_BYTES != tpp->magic_bytes) {

    return 0;
  }

  return tpp;
}

/******************************************************************************
 *                                   create
 ******************************************************************************/

static bool msg_to_loop_handler(int fd, uint8_t events, void *userdata) {

  /*
   * This method is actually called by the event loop -
   * hence executed in the event loop thread.
   */
  UNUSED(fd);
  UNUSED(events);

  dtn_thread_message *message = 0;

  dtn_thread_loop *tpp = as_thread_pool_process(userdata);

  if (sizeof(message) != read(tpp->to_loop.catch, &message, sizeof(message))) {

    /* Since the socket buffer has not been emptied,
     * we will be called again */
    dtn_log_error("Could not read from catch socket");

    goto finish;
  }

  dtn_ringbuffer *queue = tpp->to_loop.queue;

  if (0 == queue) {

    /* Queue has been disabled, use message pointer from
     * socket directly */

    goto handle_message;
  }

  dtn_thread_lock *lock = &tpp->to_loop.lock;

  if (!dtn_thread_lock_try_lock(lock)) {

    dtn_log_error("Could not lock down on trigger socket");
    goto finish;
  }

  message = queue->pop(queue);

  if (!dtn_thread_lock_unlock(lock)) {

    dtn_log_error("Could not unlock trigger socket");
    DTN_ASSERT(!"MUST NEVER EVER HAPPEN - DEADLOCK!!!");
  }

handle_message:

  DTN_ASSERT(tpp->callbacks.handle_message_in_loop);

  if (0 != message) {
    tpp->callbacks.handle_message_in_loop(tpp, message);
  }

finish:

  return 0;
}

/*----------------------------------------------------------------------------*/

static bool setup_msg_to_loop_signalling(dtn_thread_loop *self) {

  int sp[2] = {0};

  if (0 == self) {

    dtn_log_error("No thread pool process given");
    goto error;
  }

  if (0 == self->event_loop) {

    dtn_log_error("No loop attached to thread pool process");
    goto error;
  }

  /* Create LOCAL socket pair */

  if (0 > socketpair(AF_UNIX, SOCK_STREAM, 0, sp)) {

    dtn_log_error("Could not create socket pair: %s", strerror(errno));

    goto error;
  }

  /* and register with server
   * BEWARE: MUST be level-triggered as the handler will process
   * one message AT MOST
   */
  bool register_successful = self->event_loop->callback.set(
      self->event_loop, sp[1], DTN_EVENT_IO_IN, self, msg_to_loop_handler);

  if (!register_successful) {

    dtn_log_error("Could not register signal handler");
    goto error;
  }

  self->to_loop.trigger = sp[0];
  self->to_loop.catch = sp[1];

  return true;

error:

  if (sp[0] > 0)
    close(sp[0]);
  if (sp[1] > 0)
    close(sp[1]);

  return false;
}

/*----------------------------------------------------------------------------*/

dtn_thread_loop *dtn_thread_loop_create(dtn_event_loop *event_loop,
                                      dtn_thread_loop_callbacks callbacks,
                                      void *thread_pool_process_data) {

  dtn_thread_loop *self = 0;

  if (0 == event_loop) {

    dtn_log_error("Require event loop");
    goto error;
  }

  if ((0 == callbacks.handle_message_in_thread) ||
      (0 == callbacks.handle_message_in_loop)) {

    dtn_log_error("No callbacks set");
    goto error;
  }

  self = calloc(1, sizeof(dtn_thread_loop));

  DTN_ASSERT(0 != self);

  if (0 == self) {

    dtn_log_error("FUBAR: Could not allocate memory");
    goto error;
  }

  self->magic_bytes = MAGIC_BYTES;

  self->data = thread_pool_process_data;

  self->event_loop = event_loop;

  self->callbacks = callbacks;

  /* Set the trigger sockets invalid */
  self->to_loop.trigger = -1;
  self->to_loop.catch = -1;

  if (!setup_msg_to_loop_signalling(self)) {

    dtn_log_error("Could not set up message signalling mechanism "
                 "to loop");

    goto error;
  }

  dtn_thread_loop_reconfigure(self, (dtn_thread_loop_config){0});

  return self;

error:

  if (0 != self) {
    free(self);
    self = 0;
  }

  return 0;
}

/*----------------------------------------------------------------------------*/

dtn_thread_loop *dtn_thread_loop_free(dtn_thread_loop *self) {

  if (0 == self)
    goto error;

  if (0 != self->pool) {

    self->pool = self->pool->free(self->pool);
  }

  self->to_threads.queue = dtn_ringbuffer_free(self->to_threads.queue);
  self->to_loop.queue = dtn_ringbuffer_free(self->to_loop.queue);

  dtn_thread_lock_clear(&self->to_threads.lock);
  dtn_thread_lock_clear(&self->to_loop.lock);

  if (-1 < self->to_loop.catch) {
    close(self->to_loop.catch);
    self->to_loop.catch = -1;
  }

  if (-1 < self->to_loop.trigger) {
    close(self->to_loop.trigger);
    self->to_loop.trigger = -1;
  }

  free(self);

  self = 0;

  return 0;

error:

  return self;
}

/*----------------------------------------------------------------------------*/

void *dtn_thread_loop_get_data(dtn_thread_loop *self) {

  if (0 == self) {
    dtn_log_error("got 0 pointer");
    goto error;
  }

  return self->data;

error:

  return 0;
}

/******************************************************************************
 *                                reconfigure
 ******************************************************************************/

static void free_message(void *additional_arg, void *element_to_free) {

  UNUSED(additional_arg);

  if (0 == element_to_free)
    goto error;

  dtn_thread_message *msg = element_to_free;
  msg = msg->free(msg);

error:

  return;
}

/*----------------------------------------------------------------------------*/

static bool handle_in_thread(void *tpp_void, void *element) {

  dtn_thread_message *message = dtn_thread_message_cast(element);

  if (0 == message) {
    dtn_log_error("No message given");
    goto error;
  }

  dtn_thread_loop *tpp = as_thread_pool_process(tpp_void);

  if (0 == tpp) {
    dtn_log_error("No thread pool process given");
    goto error;
  }

  DTN_ASSERT(0 != tpp->callbacks.handle_message_in_thread);

  return tpp->callbacks.handle_message_in_thread(tpp, message);

error:

  if (message) {
    message = message->free(message);
  }

  return 0;
}

/*---------------------------------------------------------------------------*/

bool dtn_thread_loop_reconfigure(dtn_thread_loop *tpp,
                                dtn_thread_loop_config config) {

  if (0 == tpp) {

    dtn_log_error("Got 0 pointer");
    goto error;
  }

  if (0 == config.message_queue_capacity) {
    config.message_queue_capacity = MESSAGE_QUEUE_CAPACITY_DEFAULT;
  }

  if (0 == config.lock_timeout_usecs) {
    config.lock_timeout_usecs = LOCK_TIMEOUT_USECS_DEFAULT;
  }

  if (0 == config.num_threads) {
    config.num_threads = NUM_THREADS_DEFAULT;
  }

  const uint64_t timeout_usecs = config.lock_timeout_usecs;
  const uint64_t max_queue_length = config.message_queue_capacity;
  const size_t num_threads = config.num_threads;

  /* First of all, if threads are there, free them */

  if (0 != tpp->pool) {

    tpp->pool = tpp->pool->free(tpp->pool);
  }

  if (0 != tpp->to_threads.queue) {

    tpp->to_threads.queue = tpp->to_threads.queue->free(tpp->to_threads.queue);
  }

  if (0 != tpp->to_loop.queue) {

    tpp->to_loop.queue = tpp->to_loop.queue->free(tpp->to_loop.queue);
  }

  DTN_ASSERT(0 == tpp->pool);
  DTN_ASSERT(0 == tpp->to_threads.queue);
  DTN_ASSERT(0 == tpp->to_loop.queue);

  /* 1st: Recreate queues */

  tpp->to_threads.queue =
      dtn_ringbuffer_create(max_queue_length, free_message, 0);

  if (!config.disable_to_loop_queue) {

    tpp->to_loop.queue =
        dtn_ringbuffer_create(max_queue_length, free_message, 0);
  }

  dtn_thread_lock_clear(&tpp->to_threads.lock);
  dtn_thread_lock_clear(&tpp->to_loop.lock);
  dtn_thread_lock_init(&tpp->to_threads.lock, timeout_usecs);
  dtn_thread_lock_init(&tpp->to_loop.lock, timeout_usecs);

  /* 2nd: Recreate thread pool */

  tpp->pool = dtn_thread_pool_create(
      (dtn_thread_queue){
          .queue = tpp->to_threads.queue,
          .lock = &tpp->to_threads.lock,
      },
      handle_in_thread,
      (dtn_thread_pool_config){.num_threads = num_threads, .userdata = tpp});

  tpp->pool->user_data = tpp;

  return true;

error:

  return false;
}

/******************************************************************************
 *                    dtn_thread_loop_send_message
 ******************************************************************************/

static bool put_into_queue(dtn_ringbuffer *restrict queue,
                           dtn_thread_lock *restrict lock, void *element) {

  bool retval = false;

  DTN_ASSERT(0 != queue);
  DTN_ASSERT(0 != element);

  if ((0 != lock) && (!dtn_thread_lock_try_lock(lock))) {

    dtn_log_error("Could not lock down on queue");
    lock = 0;
    goto finish;
  }

  retval = queue->insert(queue, element);

  if (!retval) {

    dtn_log_error("Could not insert into queue");
    goto finish;
  }

  element = 0;

  DTN_ASSERT(retval);

  if ((0 != lock) && (!dtn_thread_lock_notify(lock))) {

    dtn_log_error("Could not notify");
  }

finish:

  if ((0 != lock) && (!dtn_thread_lock_unlock(lock))) {

    DTN_ASSERT(!"Could not unlock queue - we will end in a dead LOCK");
  }

  return retval;
}

/*----------------------------------------------------------------------------*/

static bool send_message_pointer_to_loop(dtn_thread_loop *restrict tpp,
                                         dtn_thread_message *msg) {

  bool sent = false;

  if (0 == tpp) {
    dtn_log_error("No thread pool process given (0 pointer)");
    goto error;
  }

  if (!dtn_thread_lock_try_lock(&tpp->to_loop.lock)) {

    dtn_log_error("Could not lock down on trigger socket");
    goto error;
  }

  errno = EBADF; /* assume no socket available */
  size_t bytes_written = 0;

  dtn_ringbuffer *queue = tpp->to_loop.queue;

  if (0 != queue) {

    sent = queue->insert(queue, msg);
  }

  if (!dtn_thread_lock_unlock(&tpp->to_loop.lock)) {

    dtn_log_error("Could not unlock to_loop socket");
    DTN_ASSERT(!"MUST NEVER EVER HAPPEN - DEADLOCK!!!");
  }

  if (!sent)
    goto error;

  /* Byte order unimportant since we remain on the same machine by
   * design (after all, we transfer POINTERS to process memory !!!)
   */
  if (0 < tpp->to_loop.trigger) {
    bytes_written = write(tpp->to_loop.trigger, &msg, sizeof(msg));
  }

  if (sizeof(msg) != bytes_written) {
    dtn_log_error("Could not write to trigger socket: %s", strerror(errno));
    goto error;
  }

  return true;

error:

  return false;
}

/*---------------------------------------------------------------------------*/

bool dtn_thread_loop_send_message(dtn_thread_loop *self,
                                 dtn_thread_message *message,
                                 dtn_thread_receiver receiver) {

  bool retval = false;

  if (0 == self)
    goto error;
  if (0 == message)
    goto error;

  DTN_ASSERT(message);
  DTN_ASSERT(self);

  switch (receiver) {

  case DTN_RECEIVER_EVENT_LOOP:

    retval = send_message_pointer_to_loop(self, message);
    break;

  case DTN_RECEIVER_THREAD:

    retval =
        put_into_queue(self->to_threads.queue, &self->to_threads.lock, message);

    break;

  default:

    DTN_ASSERT(!"SHOULD NEVER EVER HAPPEN!");
  };

  return retval;

error:
  return false;
}

/*----------------------------------------------------------------------------*/

/******************************************************************************
 *                             Start/Stop threads
 ******************************************************************************/

bool dtn_thread_loop_start_threads(dtn_thread_loop *self) {

  if (0 == self) {

    dtn_log_error("No thread pool process given(0 pointer)");
    goto error;
  }

  return self->pool->start(self->pool);

error:

  return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_thread_loop_stop_threads(dtn_thread_loop *self) {

  if (0 == self) {

    dtn_log_error("Not thread_pool_process given (0 pointer)");
    goto error;
  }

  return self->pool->stop(self->pool);

error:

  return false;
}

/******************************************************************************
 *                                    JSON
 ******************************************************************************/

dtn_item *dtn_thread_loop_config_to_json(const dtn_thread_loop_config config,
                                             dtn_item *target) {

  dtn_item *val_to_add = 0;

  dtn_item *json = target;
  if (0 == json) {
    json = dtn_item_object();
  }

  int64_t lock_timeout_usecs = (int64_t)LOCK_TIMEOUT_USECS_DEFAULT;
  int64_t message_queue_capacity = (int64_t)MESSAGE_QUEUE_CAPACITY_DEFAULT;

  int64_t num_threads = (int64_t)NUM_THREADS_DEFAULT;

  if ((0 < config.lock_timeout_usecs) &&
      (INT64_MAX >= config.lock_timeout_usecs)) {

    lock_timeout_usecs = (int64_t)config.lock_timeout_usecs;
  }

  if ((0 < config.message_queue_capacity) &&
      (INT64_MAX >= config.message_queue_capacity)) {

    message_queue_capacity = (int64_t)config.message_queue_capacity;
  }

  if ((0 < config.num_threads) && (SIZE_MAX >= config.num_threads)) {

    num_threads = (size_t)config.num_threads;
  }

  dtn_item_object_delete(json, CONFIG_KEY_LOCK_TIMEOUT_USECS);

  val_to_add = dtn_item_number(lock_timeout_usecs);

  if ((0 == val_to_add) ||
      !dtn_item_object_set(json, CONFIG_KEY_LOCK_TIMEOUT_USECS, val_to_add)) {

    dtn_log_error("Could not add %s to JSON", CONFIG_KEY_LOCK_TIMEOUT_USECS);
    goto error;
  }

  val_to_add = 0;

  dtn_item_object_delete(json, CONFIG_KEY_MESSAGE_QUEUE_CAPACITY);

  val_to_add = dtn_item_number(message_queue_capacity);

  if ((0 == val_to_add) ||
      !dtn_item_object_set(json, CONFIG_KEY_MESSAGE_QUEUE_CAPACITY,
                          val_to_add)) {
    dtn_log_error("Could not add %s to JSON", CONFIG_KEY_MESSAGE_QUEUE_CAPACITY);
    goto error;
  }

  val_to_add = 0;

  dtn_item_object_delete(json, CONFIG_KEY_NUM_THREADS);

  val_to_add = dtn_item_number(num_threads);

  if ((0 == val_to_add) ||
      !dtn_item_object_set(json, CONFIG_KEY_NUM_THREADS, val_to_add)) {

    dtn_log_error("Could not add %s to JSON", CONFIG_KEY_NUM_THREADS);

    goto error;
  }

  val_to_add = 0;

  dtn_item_object_delete(json, CONFIG_KEY_DISABLE_TO_LOOP_QUEUE);

  if (config.disable_to_loop_queue) {
    val_to_add = dtn_item_true();
  }

  if ((config.disable_to_loop_queue) && (0 == val_to_add)) {

    dtn_log_error("Could not create JSON 'true'");
    goto error;
  }

  if ((0 != val_to_add) &&
      (!dtn_item_object_set(json, CONFIG_KEY_DISABLE_TO_LOOP_QUEUE,
                           val_to_add))) {

    dtn_log_error("Could not add %s to JSON", CONFIG_KEY_DISABLE_TO_LOOP_QUEUE);
    goto error;
  }

  val_to_add = 0;

  return json;

error:

  if ((0 == target) && (0 != json)) {
    json = dtn_item_free(json);
  }

  if (0 != val_to_add) {
    val_to_add = dtn_item_free(val_to_add);
  }

  DTN_ASSERT(0 == val_to_add);

  return 0;
}

/*----------------------------------------------------------------------------*/

dtn_thread_loop_config
dtn_thread_loop_config_from_json(dtn_item const *json) {

  dtn_item const *val = 0;
  dtn_thread_loop_config config = {0};

  if (!dtn_ptr_valid(json, "No JSON given")) {
    goto finish;
  }

  bool disable_to_loop_queue = false;
  int64_t message_queue_capacity = 0;
  int64_t lock_timeout_usecs = 0;
  int64_t num_threads = 0;

  val = dtn_item_get(json, "/" CONFIG_KEY_MESSAGE_QUEUE_CAPACITY);

  double dval = dtn_item_get_number(val);
  message_queue_capacity = (int64_t)dval;

  if (0 >= message_queue_capacity) {

    dtn_log_warning("No or invalid message queue capacity "
                   "given: "
                   "%" PRIi64,
                   message_queue_capacity);

    message_queue_capacity = MESSAGE_QUEUE_CAPACITY_DEFAULT;
  }

  val = dtn_item_get(json, "/" CONFIG_KEY_LOCK_TIMEOUT_USECS);
  dval = dtn_item_get_number(val);
  lock_timeout_usecs = (int64_t)dval;

  if (0 >= lock_timeout_usecs) {

    dtn_log_warning("No or invalid lock timeout given: "
                   "%" PRIi64,
                   lock_timeout_usecs);

    lock_timeout_usecs = LOCK_TIMEOUT_USECS_DEFAULT;
  }

  val = dtn_item_get(json, "/" CONFIG_KEY_NUM_THREADS);
  dval = dtn_item_get_number(val);
  num_threads = (int64_t)dval;

  if ((0 >= num_threads) || (SIZE_MAX < (uint64_t)num_threads)) {

    dtn_log_warning("No or invalid number of threads: "
                   "%" PRIi64,
                   num_threads);

    num_threads = NUM_THREADS_DEFAULT;
  }

  val = dtn_item_object_get(json, CONFIG_KEY_DISABLE_TO_LOOP_QUEUE);
  disable_to_loop_queue = dtn_item_is_true(val);

  config.message_queue_capacity = (uint64_t)message_queue_capacity;
  config.lock_timeout_usecs = (uint64_t)lock_timeout_usecs;
  config.num_threads = (size_t)num_threads;
  config.disable_to_loop_queue = disable_to_loop_queue;

finish:

  return config;
}

/*---------------------------------------------------------------------------*/
