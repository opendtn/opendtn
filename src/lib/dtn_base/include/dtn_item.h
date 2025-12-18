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

        This file is part of the openvocs project. https://openvocs.org

        ------------------------------------------------------------------------
*//**
        @file           dtn_item.h
        @author         Töpfer, Markus

        @date           2025-11-28

        Generic implementation for JSON or other datastructure defining
        languages.


        ------------------------------------------------------------------------
*/
#ifndef dtn_item_h
#define dtn_item_h

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

/*----------------------------------------------------------------------------*/

typedef struct dtn_item dtn_item;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_item *dtn_item_cast(const void *self);
dtn_item *dtn_item_get_parent(const dtn_item *self);
/*
 *      ------------------------------------------------------------------------
 *
 *      DATA FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

void *dtn_item_free(void *self);
bool dtn_item_clear(void *self);
void *dtn_item_copy(void **destination, const void *self);
bool dtn_item_dump(FILE *stream, const void *self);

size_t dtn_item_count(const dtn_item *self);
bool dtn_item_is_empty(const dtn_item *self);

/*----------------------------------------------------------------------------*/

/**
        Get a value using pointer based access, e.g.

        for

        {
         "a" : {
           "b" : {
              "cdef" : 14
           }
         }
        }

        dtn_item_get(value, "/a/b/cdef") -> "14"
        dtn_item_get(value, "/b/cdef") -> NULL


        using a \0 terminated pointer string

*/
dtn_item *dtn_item_get(const dtn_item *self, const char *pointer);

/*
 *      ------------------------------------------------------------------------
 *
 *      OBJECT FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_item *dtn_item_object();
bool dtn_item_is_object(const dtn_item *self);

/*---------------------------------------------------------------------------*/

bool dtn_item_object_set(dtn_item *self, const char *string, dtn_item *val);

/*---------------------------------------------------------------------------*/

bool dtn_item_object_delete(dtn_item *self, const char *string);

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_object_remove(dtn_item *self, const char *string);

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_object_get(const dtn_item *self, const char *string);

/*---------------------------------------------------------------------------*/

bool dtn_item_object_for_each(dtn_item *self,
                             bool (*function)(char const *key,
                                              dtn_item const *val,
                                              void *userdata),
                             void *userdata);
/*
 *      ------------------------------------------------------------------------
 *
 *      ARRAY FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_item *dtn_item_array();
bool dtn_item_is_array(const dtn_item *self);

bool dtn_item_array_for_each(dtn_item *self, void *userdata, 
        bool (*function)(void *item, void *userdata));

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_array_get(const dtn_item *self, uint64_t pos);

/*---------------------------------------------------------------------------*/

bool dtn_item_array_set(dtn_item *self, uint64_t pos, dtn_item *val);

/*---------------------------------------------------------------------------*/

bool dtn_item_array_push(dtn_item *self, dtn_item *val);

/*---------------------------------------------------------------------------*/

/**
 *  pop val from end of array from self. LIFO
 */
dtn_item *dtn_item_array_stack_pop(dtn_item *self);
dtn_item *dtn_item_array_lifo(dtn_item *self);

/*---------------------------------------------------------------------------*/

/**
 *  pop val from front of array from self. FIFO
 */
dtn_item *dtn_item_array_queue_pop(dtn_item *self);
dtn_item *dtn_item_array_fifo(dtn_item *self);

/*
 *      ------------------------------------------------------------------------
 *
 *      STRING FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_item *dtn_item_string(const char *string);
bool dtn_item_is_string(const dtn_item *self);
const char *dtn_item_get_string(const dtn_item *self);

/*
 *      ------------------------------------------------------------------------
 *
 *      NUMBER FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_item *dtn_item_number(double number);
bool dtn_item_is_number(const dtn_item *self);
bool dtn_item_set_number(dtn_item *self, double number);
double dtn_item_get_number(const dtn_item *self);
int64_t dtn_item_get_int(const dtn_item *self);

/*
 *      ------------------------------------------------------------------------
 *
 *      LITERAL FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_item *dtn_item_null();
bool dtn_item_is_null(const dtn_item *self);

dtn_item *dtn_item_true();
bool dtn_item_is_true(const dtn_item *self);

dtn_item *dtn_item_false();
bool dtn_item_is_false(const dtn_item *self);

#endif /* dtn_item_h */
