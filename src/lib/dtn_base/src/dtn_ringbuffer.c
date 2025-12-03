/***
        ------------------------------------------------------------------------

        Copyright (c) 2019 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_ringbuffer.c
        @author         Michael Beer

        @date           2018-01-11

        @ingroup        dtn_ringbuffer

        @brief          Implementation of a ring buffer

        ------------------------------------------------------------------------
*/
#include "../include/dtn_utils.h"
#include <string.h>

#include "../include/dtn_ringbuffer.h"

#include "../include/dtn_constants.h"

#include "../include/dtn_registered_cache.h"
#include "../include/dtn_utils.h"

#define DTN_RINGBUFFER_MAGIC_BYTES 0x52694e00

/*----------------------------------------------------------------------------*/

static dtn_registered_cache *g_cache = 0;

/******************************************************************************
 *
 *  INTERNAL RINGBUFFER STRUCTURE
 *
 ******************************************************************************/

struct internal_ringbuffer {

  dtn_ringbuffer public_stuff;

  size_t capacity;

  /* access indices - point to the content entry to access NEXT */
  /* read == write -> buffer empty */
  size_t read;
  size_t write;

  void **content;

  void *additional_arg;
  void (*free_element)(void *, void *);

  dtn_ringbuffer_statistics statistics;
};

/******************************************************************************
 *
 *  INTERNAL FUNCTIONS
 *
 ******************************************************************************/

static void *ringbuffer_pop(dtn_ringbuffer *self);
static bool ringbuffer_insert(dtn_ringbuffer *self, void *element);
static size_t ringbuffer_capacity(const dtn_ringbuffer *self);
static bool ringbuffer_clear(dtn_ringbuffer *self);
static dtn_ringbuffer *ringbuffer_free(dtn_ringbuffer *self);

static dtn_ringbuffer_statistics
ringbuffer_get_statistics(const dtn_ringbuffer *self);

/*----------------------------------------------------------------------------*/

// static struct internal_ringbuffer *as_internal_ringbuffer(void *vptr) {
//
//     if(0 == vptr) return 0;
//
//     struct internal_ringbuffer *rb = (struct internal_ringbuffer *) vptr;
//
//     if(DTN_RINGBUFFER_MAGIC_BYTES != rb->public_stuff.magic_bytes) return 0;
//
//     return rb;
//
// }

/*----------------------------------------------------------------------------*/

static void free_element_dummy(void *a, void *b) {

  UNUSED(a);
  UNUSED(b);
  /* Dummy function */
}

/*----------------------------------------------------------------------------*/

static void drop_element_nocheck(struct internal_ringbuffer *internal,
                                 size_t index) {

  DTN_ASSERT(0 != internal);
  DTN_ASSERT(index < internal->capacity);
  DTN_ASSERT(1 < internal->capacity);

  void *content = internal->content[index];

  if (0 != content) {
    internal->free_element(internal->additional_arg, content);
    ++internal->statistics.elements_dropped;
  }

  internal->content[index] = 0;
}

/*----------------------------------------------------------------------------*/

static void advance_read(struct internal_ringbuffer *internal) {

  DTN_ASSERT(0 != internal);
  DTN_ASSERT(1 < internal->capacity);

  /* Element we 'read' should have been emptied */
  DTN_ASSERT(0 == internal->content[internal->read]);

  ++internal->read;

  if (internal->read >= internal->capacity) {

    internal->read = 0;
  }
}

/*----------------------------------------------------------------------------*/

static void advance_write(struct internal_ringbuffer *internal) {

  DTN_ASSERT(0 != internal);
  DTN_ASSERT(1 < internal->capacity);

  ++internal->write;

  if (internal->write >= internal->capacity) {

    internal->write = 0;
  }

  drop_element_nocheck(internal, internal->write);

  if (internal->write == internal->read) {

    advance_read(internal);
  }

  DTN_ASSERT(internal->write != internal->read);
}

/******************************************************************************
 *                                Capacity > 1
 ******************************************************************************/

static void *ringbuffer_pop(dtn_ringbuffer *self) {

  if (0 == self)
    goto error;

  struct internal_ringbuffer *internal = (struct internal_ringbuffer *)self;

  if (internal->read == internal->write) {

    return 0;
  }

  void *retval = internal->content[internal->read];
  internal->content[internal->read] = 0;

  advance_read(internal);

  return retval;

error:

  return 0;
}

/*---------------------------------------------------------------------------*/

static bool ringbuffer_insert(dtn_ringbuffer *self, void *element) {

  if (0 == self)
    goto error;

  /* element must not be 0 since 0 indicates an empty slot */
  if (0 == element)
    goto error;

  struct internal_ringbuffer *internal = (struct internal_ringbuffer *)self;

  DTN_ASSERT(1 < internal->capacity);

  const size_t write_index = internal->write;

  DTN_ASSERT(0 == internal->content[write_index]);

  internal->content[write_index] = element;

  advance_write(internal);

  ++internal->statistics.elements_inserted;

  return true;

error:

  return false;
}

/*----------------------------------------------------------------------------*/

static bool ringbuffer_clear(dtn_ringbuffer *self) {

  if (0 == self)
    goto error;

  struct internal_ringbuffer *internal = (struct internal_ringbuffer *)self;

  DTN_ASSERT(1 < internal->capacity);

  for (size_t i = 0; i < internal->capacity; ++i) {

    /* Empty slots ought to be 0 */
    drop_element_nocheck(internal, i);
  }

  return true;

error:

  return false;
}

/*---------------------------------------------------------------------------*/

static struct internal_ringbuffer *new_buffer_create(size_t capacity) {

  struct internal_ringbuffer *buffer = 0;

  if (0 == capacity)
    goto error;

  capacity = 1 + capacity;

  buffer = calloc(1, sizeof(struct internal_ringbuffer));

  buffer->public_stuff = (dtn_ringbuffer){

      .magic_bytes = DTN_RINGBUFFER_MAGIC_BYTES,
      .capacity = ringbuffer_capacity,
      .pop = ringbuffer_pop,
      .insert = ringbuffer_insert,
      .free = ringbuffer_free,
      .clear = ringbuffer_clear,
      .get_statistics = ringbuffer_get_statistics,

  };

  buffer->capacity = capacity;

  /* To be overwritten by caller if required */
  buffer->free_element = free_element_dummy;

  buffer->content = calloc(capacity, sizeof(void *));
  buffer->read = 0;
  buffer->write = 0;

  DTN_ASSERT(1 < buffer->capacity);

  return buffer;

error:

  return 0;
}

/******************************************************************************
 *
 *  PUBLIC FUNCTIONS
 *
 ******************************************************************************/

dtn_ringbuffer *dtn_ringbuffer_create(size_t capacity,
                                    void (*free_element)(void *additional_arg,
                                                         void *element_to_free),
                                    void *additional_arg) {

  struct internal_ringbuffer *buffer = dtn_registered_cache_get(g_cache);

  if (0 == buffer) {

    buffer = new_buffer_create(capacity);
  }

  if (0 == buffer) {
    goto error;
  }

  if (capacity + 1 > buffer->capacity) {

    buffer->content = realloc(buffer->content, (capacity + 1) * sizeof(void *));
    buffer->capacity = 1 + capacity;
    memset(buffer->content, 0, buffer->capacity * sizeof(void *));

    buffer->free_element = free_element_dummy;
  }

  buffer->additional_arg = additional_arg;

  if (0 != free_element) {
    buffer->free_element = free_element;
  }

  return (dtn_ringbuffer *)buffer;

error:

  return 0;
}

/******************************************************************************
 *
 *  INTERNAL FUNCTION IMPLEMENTATIONS
 *
 ******************************************************************************/

static size_t ringbuffer_capacity(const dtn_ringbuffer *self) {

  if (0 == self)
    return 0;

  struct internal_ringbuffer *internal = (struct internal_ringbuffer *)self;

  DTN_ASSERT(1 < internal->capacity);

  return internal->capacity - 1;
}

/*---------------------------------------------------------------------------*/

static dtn_ringbuffer *ringbuffer_free(dtn_ringbuffer *self) {

  if (0 == self)
    goto error;

  if (!self->clear(self))
    goto error;

  /* Free linked list infrastructure */
  struct internal_ringbuffer *internal = (struct internal_ringbuffer *)self;

  if (0 != dtn_registered_cache_put(g_cache, self)) {

    free(internal->content);
    internal->content = 0;

    free(self);
  }

  self = 0;

error:

  return self;
}

/*---------------------------------------------------------------------------*/

static dtn_ringbuffer_statistics
ringbuffer_get_statistics(const dtn_ringbuffer *self) {

  if (0 == self)
    goto error;

  struct internal_ringbuffer *internal = (struct internal_ringbuffer *)self;

  return internal->statistics;

error:

  return (dtn_ringbuffer_statistics){0};
}

/*----------------------------------------------------------------------------*/

void *void_ptr_free(void *void_ptr) {

  dtn_ringbuffer *rb = (dtn_ringbuffer *)void_ptr;

  if (DTN_RINGBUFFER_MAGIC_BYTES != rb->magic_bytes) {

    goto error;
  }

  return rb->free(rb);

error:

  return 0;
}

/*----------------------------------------------------------------------------*/

dtn_ringbuffer *dtn_ringbuffer_free(dtn_ringbuffer *self) {

  if ((0 != self) && (0 != self->free)) {

    return self->free(self);

  } else {

    return self;
  }
}

/*----------------------------------------------------------------------------*/

bool dtn_ringbuffer_insert(dtn_ringbuffer *self, void *element) {

  return dtn_ptr_valid(self, "Cannot insert element - no ringbuffer") &&
         dtn_ptr_valid(self->insert,
                      "Cannot insert element - ringbuffer does not have "
                      "insert method") &&
         self->insert(self, element);
}

/*---------------------------------------------------------------------------*/

void *dtn_ringbuffer_pop(dtn_ringbuffer *self) {

  if (dtn_ptr_valid(self, "Cannot pop element - no ringbuffer") &&
      dtn_ptr_valid(self->insert,
                   "Cannot pop element - ringbuffer does not have "
                   "pop method")) {
    return self->pop(self);

  } else {

    return 0;
  }
}

/*----------------------------------------------------------------------------*/

void dtn_ringbuffer_enable_caching(size_t capacity) {

  dtn_registered_cache_config cfg = {
      .item_free = void_ptr_free,
      .capacity = capacity,
  };

  g_cache = dtn_registered_cache_extend("ringbuffer", cfg);
}

/*----------------------------------------------------------------------------*/
