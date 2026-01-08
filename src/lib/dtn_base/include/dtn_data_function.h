/***
        ------------------------------------------------------------------------

        Copyright 2018 German Aerospace Center DLR e.V. (GSOC)

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

        ------------------------------------------------------------------------
*//**
        @file           dtn_data_function.h
        @author         Markus Toepfer

        @date           2018-03-23

        @ingroup        dtn_base

        @brief          Definition of standard functions for operations on
                        "unspecific" data structures at a void pointer.

                        These functions are the abstract function set to be
                        implemented by data structures, which shall be handled
                        in some kind of dtn_* data container.

                                e.g. @see dtn_data
                                e.g. @see dtn_list
                                e.g. @see dtn_vector

        ------------------------------------------------------------------------
*/
#ifndef dtn_data_function_h
#define dtn_data_function_h

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/time.h>
#include <time.h>

typedef struct dtn_data_function dtn_data_function;

typedef bool (*dtn_DATA_CLEAR)(void *source);
typedef void *(*dtn_DATA_FREE)(void *source);
typedef void *(*dtn_DATA_COPY)(void **destination, const void *source);
typedef bool (*dtn_DATA_DUMP)(FILE *stream, const void *source);

/*
 *      ------------------------------------------------------------------------
 *
 *                        FUNCTION STRUCTURE DEFINITION
 *
 *      ------------------------------------------------------------------------
 *
 *      dtn_data_functions are used as interface for functions
 *      related to handle some kind of "undefined" void data structure,
 *      within some kind of data structure container.
 *
 *      @NOTE create is not a standard function (MAY use parameter init)
 *      @NOTE clear MUST clear and reinit a structure to default
 *      @NOTE copy  MUST create, or fill the structure destination.
 *      @NOTE free  MUST free the structure and set the pointer to NULL
 */

struct dtn_data_function {

    dtn_DATA_FREE free;   // terminate struct  and content
    dtn_DATA_CLEAR clear; // terminate content of a struct
    dtn_DATA_COPY copy;   // copy a struct including content
    dtn_DATA_DUMP dump;   // default dump a struct (e.g. debug)
};

/*
 *      ------------------------------------------------------------------------
 *
 *      STANDARD FUNCTIONS
 *
 *      Functions defined in this block SHOULD be used for structure
 *      related functionality e.g.
 *
 *              create  a structure (allocate and initialize)
 *              destroy a structure and terminate all of it's content
 *              copy    a structure including all of it's content
 *
 *      ------------------------------------------------------------------------
 */

/**
        Allocate a default empty dtn_data_function structure.
*/
dtn_data_function *dtn_data_function_create();

/*----------------------------------------------------------------------------*/

/**
        Clear a dtn_data_function structure.
*/
bool dtn_data_function_clear(dtn_data_function *func);

/*----------------------------------------------------------------------------*/

/**
        Terminate a dtn_data_function structure.
*/
dtn_data_function *dtn_data_function_free(dtn_data_function *func);

/*----------------------------------------------------------------------------*/

/**
        Copy a dtn_data_function structure.
*/
dtn_data_function *dtn_data_function_copy(dtn_data_function **destination,
                                          const dtn_data_function *source);

/*----------------------------------------------------------------------------*/

/**
        Dump a dtn_data_function structure.
*/
bool dtn_data_function_dump(FILE *stream, const dtn_data_function *func);

/*
 *      ------------------------------------------------------------------------
 *
 *      EXAMPLE FUNCTION IMPLEMENTATIONS (string based)
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_data_string_clear(void *string);
void *dtn_data_string_free(void *string);
void *dtn_data_string_copy(void **destination, const void *string);
bool dtn_data_string_dump(FILE *stream, const void *string);

dtn_data_function dtn_data_string_data_functions();
bool dtn_data_string_data_functions_are_valid(
    const dtn_data_function *functions);

/*
 *      ------------------------------------------------------------------------
 *
 *      EXAMPLE IMPLEMENTATIONS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_data_int64_clear(void *number);
void *dtn_data_int64_free(void *number);
void *dtn_data_int64_copy(void **destination, const void *number);
bool dtn_data_int64_dump(FILE *stream, const void *number);
dtn_data_function dtn_data_int64_data_functions();

void *dtn_data_int64_direct_copy(void **destination, const void *number);
bool dtn_data_int64_direct_dump(FILE *stream, const void *number);

void *dtn_data_uint64_free(void *data);
bool dtn_data_uint64_clear(void *number);
void *dtn_data_uint64_copy(void **destination, const void *number);
bool dtn_data_uint64_dump(FILE *stream, const void *number);
dtn_data_function dtn_data_uint64_data_functions();

void *dtn_data_timeval_free(void *data);
bool dtn_data_timeval_clear(void *number);
void *dtn_data_timeval_copy(void **destination, const void *number);
bool dtn_data_timeval_dump(FILE *stream, const void *number);
dtn_data_function dtn_data_timeval_data_functions();

void *dtn_data_pointer_free(void *data);

/*
 *      ------------------------------------------------------------------------
 *
 *      Standard free
 *
 *      ------------------------------------------------------------------------
 */
void *dtn_data_function_wrapper_free(void *ptr);

/*
 *      ------------------------------------------------------------------------
 *
 *      Additional helper
 *
 *      ------------------------------------------------------------------------
 */

/**
        Create an allocated struct for dtn_data_functions and fill
        the struct using a custom function.
*/
dtn_data_function *
    dtn_data_function_allocated(dtn_data_function (*function_fill_struct)());

#endif /* dtn_data_function_h */
