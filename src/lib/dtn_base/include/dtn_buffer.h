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
        @file           dtn_buffer.h
        @author         Markus Toepfer
        @author         Michael Beer

        @date           2019-01-25

        @ingroup        dtn_buffer

        @brief          Definition of a standard buffer.

                        @NOTE @CONVENTION

                        if capacity == 0 start has not been allocated by us
                        if capacity  > 0 start is allocated and
                                         length is current used size

                        if capacity != 0 dtn_buffer_free will free the pointer
                        at start

        ------------------------------------------------------------------------
*/
#ifndef dtn_buffer_h
#define dtn_buffer_h

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "dtn_data_function.h"
#include "dtn_utils.h"

/*----------------------------------------------------------------------------*/

#define dtn_BUFFER_MAGIC_BYTE 0xBBBB

/*----------------------------------------------------------------------------*/

typedef struct {

    uint16_t magic_byte;

    uint8_t *start;  // start of data
    size_t length;   // length of data
    size_t capacity; // allocated capacity (if 0 start is pointer)

} dtn_buffer;

ssize_t dtn_buffer_len(dtn_buffer const *self);
uint8_t const *dtn_buffer_data(dtn_buffer const *self);
uint8_t *dtn_buffer_data_mutable(dtn_buffer *self);

/*
 *      ------------------------------------------------------------------------
 *
 *      PUBLIC FUNCTION IMPLEMENTATION
 *
 *      ------------------------------------------------------------------------
 */

/**
        Create a buffer with given size.
        @param size     defines size allocated for data
*/
dtn_buffer *dtn_buffer_create(size_t size);

/*----------------------------------------------------------------------------*/

/**
        Create a buffer and copy data with length size to buffer.
*/
dtn_buffer *dtn_buffer_from_string(const char *string);

/*----------------------------------------------------------------------------*/

/**
 * Appends b2 at the end of b1
 * The result is placed within b1 - no new buffer is created
 */
dtn_buffer *dtn_buffer_concat(dtn_buffer *b1, dtn_buffer const *b2);

/*----------------------------------------------------------------------------*/

/**
 * Creates a zero-terminated buffer containing a concatenation of all strings
 */
#define dtn_buffer_from_strlist(...)                                           \
    dtn_buffer_from_strlist_internal((char const *[]){__VA_ARGS__, 0})

/*----------------------------------------------------------------------------*/
/**
        Cast to dtn_buffer
*/
dtn_buffer *dtn_buffer_cast(const void *data);

/*----------------------------------------------------------------------------*/

/**
        Copy some data to buffer start.
        This function will take into account current buffer capacity and
        adjust the capacity to the new content length, if required.

        @param buffer   buffer to use
        @param data     start of data to copy
        @param length   length of data to copy
*/
bool dtn_buffer_set(dtn_buffer *buffer, const void *data, size_t length);

/*----------------------------------------------------------------------------*/

/**
        PUSH some data to the end of the buffer.

        @param buffer   buffer to use
        @param data     data to write behind buffer->length
        @param size     required size
*/
bool dtn_buffer_push(dtn_buffer *buffer, void *data, size_t size);

/*----------------------------------------------------------------------------*/

/**
        Shift buffer start to next.

        This function will delete some memory area at the start of the
        buffer.

        @param buffer   buffer to use
        @param next     pointer to next byte in buffer
*/
bool dtn_buffer_shift(dtn_buffer *buffer, uint8_t *next);

/*----------------------------------------------------------------------------*/

bool dtn_buffer_equals(dtn_buffer const *self, char const *refstr);

/*----------------------------------------------------------------------------*/

/**
        Shift buffer start by length

        This function will delete some memory area at the start of the
        buffer.

        @param buffer   buffer to use
        @param length   length of the buffer to cut at start
*/
bool dtn_buffer_shift_length(dtn_buffer *buffer, size_t length);

/*----------------------------------------------------------------------------*/

/**
        Increases the buffer->capacity by additional_bytes

        @param buffer       buffer to use
        @param add_bytes    additional bytes to add to capacity
*/
bool dtn_buffer_extend(dtn_buffer *buffer, size_t add_bytes);

/*
 *      ------------------------------------------------------------------------
 *
 *      DEFAULT DATA FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_buffer_clear(void *self);
void *dtn_buffer_free(void *self);
void *dtn_buffer_copy(void **destination, const void *self);
bool dtn_buffer_dump(FILE *stream, const void *self);

void *dtn_buffer_free_uncached(void *self);

dtn_data_function dtn_buffer_data_functions();

/******************************************************************************
 *                                  CACHING
 ******************************************************************************/

/**
 * Enables caching.
 * BEWARE: Call dtn_registered_cache_free_all() before exiting your process to
 * avoid memleaks!
 */
void dtn_buffer_enable_caching(size_t capacity);

/*----------------------------------------------------------------------------*/

/*****************************************************************************
                                    INTERNAL
 ****************************************************************************/

dtn_buffer *dtn_buffer_from_strlist_internal(char const **strlist);

#endif /* dtn_buffer_h */
