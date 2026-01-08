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
        @file           dtn_data_function.c
        @author         Markus Toepfer

        @date           2018-03-23

        @ingroup        dtn_basics

        @brief          Implementation of dtn_data_function structure functions,
                        as well as some example implementations.


        ------------------------------------------------------------------------
*/
#include "../include/dtn_data_function.h"

dtn_data_function *dtn_data_function_create() {

    return calloc(1, sizeof(dtn_data_function));
}

/*----------------------------------------------------------------------------*/

bool dtn_data_function_clear(dtn_data_function *func) {

    if (!func)
        return false;

    if (!memset(func, 0, sizeof(dtn_data_function)))
        return false;

    return true;
}

/*----------------------------------------------------------------------------*/

dtn_data_function *dtn_data_function_free(dtn_data_function *func) {

    if (!func)
        return NULL;

    if (!dtn_data_function_clear(func))
        return func;

    free(func);
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_data_function *dtn_data_function_copy(dtn_data_function **destination,
                                          const dtn_data_function *source) {

    if (!destination || !source)
        return NULL;

    bool created = false;

    if (!*destination) {

        *destination = calloc(1, sizeof(dtn_data_function));
        if (!*destination)
            return NULL;

        created = true;
    }

    if (memcpy(*destination, source, sizeof(dtn_data_function)))
        return *destination;

    if (created) {

        *destination = dtn_data_function_free(*destination);

    } else {

        dtn_data_function_clear(*destination);
    }

    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_data_function_dump(FILE *stream, const dtn_data_function *func) {

    if (!stream || !func)
        return false;

    fprintf(stream,
            "DATA FUNCTION POINTER:\n"
            "FREE   %p\n"
            "CLEAR  %p\n"
            "COPY   %p\n"
            "DUMP   %p\n",
            func->free, func->clear, func->copy, func->dump);

    return true;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      EXAMPLES
 *
 *      NOTE examples are only done to show the idea and are not error
 *      complete.
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_data_string_clear(void *string) {

    if (!string)
        return false;

    ((char *)string)[0] = 0;
    return true;
}

/*----------------------------------------------------------------------------*/

void *dtn_data_string_free(void *string) {

    if (string)
        free(string);

    return NULL;
}

/*----------------------------------------------------------------------------*/

void *dtn_data_string_copy(void **destination, const void *string) {

    if (!destination || !string)
        return NULL;

    if (*destination)
        dtn_data_string_free(*destination);

    *destination = (void *)strndup((char *)string, SIZE_MAX);
    return *destination;
}

/*----------------------------------------------------------------------------*/

bool dtn_data_string_dump(FILE *stream, const void *string) {

    if (!stream || !string)
        return false;

    if (fprintf(stream, "%s \n", (char *)string))
        return true;

    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      EXAMPLE IMPLEMENTATIONS (int64 pointer based)
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_data_int64_clear(void *data) {

    if (!data)
        return false;

    *(int64_t *)data = 0;
    return true;
}

/*----------------------------------------------------------------------------*/

void *dtn_data_int64_free(void *data) {

    if (!data)
        return NULL;

    free(data);
    return NULL;
}

/*----------------------------------------------------------------------------*/

void *dtn_data_int64_copy(void **destination, const void *source) {

    if (!destination || !source)
        return NULL;

    if (!*destination)
        *destination = calloc(1, sizeof(int64_t));

    int64_t *d = *destination;
    int64_t *s = (int64_t *)source;

    *d = *s;
    return *destination;
}

/*----------------------------------------------------------------------------*/

bool dtn_data_int64_dump(FILE *stream, const void *source) {

    if (!stream || !source)
        return false;

    if (fprintf(stream, "%" PRIu64 " \n", *(int64_t *)source))
        return true;

    return false;
}

/*----------------------------------------------------------------------------*/

void *dtn_data_int64_direct_copy(void **destination, const void *source) {

    if (!destination || !source)
        return NULL;

    *(intptr_t *)destination = (intptr_t)source;
    return *destination;
}

/*----------------------------------------------------------------------------*/

bool dtn_data_int64_direct_dump(FILE *stream, const void *source) {

    if (!stream || !source)
        return false;

    if (fprintf(stream, "%" PRIiPTR " \n", (intptr_t)source))
        return true;

    return false;
}

/*----------------------------------------------------------------------------*/

dtn_data_function dtn_data_int64_data_functions() {

    dtn_data_function function = {

        .clear = dtn_data_int64_clear,
        .free = dtn_data_int64_free,
        .copy = dtn_data_int64_copy,
        .dump = dtn_data_int64_dump};

    return function;
}

/*----------------------------------------------------------------------------*/

bool dtn_data_uint64_clear(void *data) {

    if (!data)
        return false;

    *(uint64_t *)data = 0;
    return true;
}

/*----------------------------------------------------------------------------*/

void *dtn_data_uint64_free(void *data) {

    if (!data)
        return NULL;

    free(data);
    return NULL;
}

/*----------------------------------------------------------------------------*/

void *dtn_data_uint64_copy(void **destination, const void *source) {

    if (!destination || !source)
        return NULL;

    if (!*destination)
        *destination = calloc(1, sizeof(uint64_t));

    uint64_t *d = *destination;
    uint64_t *s = (uint64_t *)source;

    *d = *s;

    return *destination;
}

/*----------------------------------------------------------------------------*/

bool dtn_data_uint64_dump(FILE *stream, const void *source) {

    if (!stream || !source)
        return false;

    if (fprintf(stream, "%" PRIu64 " \n", *(uint64_t *)source))
        return true;

    return false;
}

/*----------------------------------------------------------------------------*/

dtn_data_function dtn_data_uint64_data_functions() {

    dtn_data_function function = {

        .clear = dtn_data_uint64_clear,
        .free = dtn_data_uint64_free,
        .copy = dtn_data_uint64_copy,
        .dump = dtn_data_uint64_dump};

    return function;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      Standard free
 *
 *      ------------------------------------------------------------------------
 */

void *dtn_data_function_wrapper_free(void *ptr) {

    if (ptr)
        free(ptr);

    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_data_function dtn_data_string_data_functions() {

    dtn_data_function function = {

        .clear = dtn_data_string_clear,
        .free = dtn_data_string_free,
        .copy = dtn_data_string_copy,
        .dump = dtn_data_string_dump};

    return function;
}

/*----------------------------------------------------------------------------*/

bool dtn_data_string_data_functions_are_valid(
    const dtn_data_function *functions) {

    if (!functions || (functions->clear != dtn_data_string_clear) ||
        (functions->free != dtn_data_string_free) ||
        (functions->copy != dtn_data_string_copy) ||
        (functions->dump != dtn_data_string_dump))
        return false;

    return true;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      Additional helper
 *
 *      ------------------------------------------------------------------------
 */

dtn_data_function *
dtn_data_function_allocated(dtn_data_function (*function_fill_struct)()) {

    dtn_data_function *func = calloc(1, sizeof(dtn_data_function));
    if (!func)
        return NULL;

    if (function_fill_struct)
        *func = function_fill_struct();

    return func;
}

/*----------------------------------------------------------------------------*/

bool dtn_data_timeval_clear(void *self) {

    if (!self)
        return NULL;

    memset(self, 0, sizeof(struct timeval));
    return true;
}

/*----------------------------------------------------------------------------*/

void *dtn_data_timeval_free(void *self) {

    if (!self)
        return NULL;

    free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

void *dtn_data_timeval_copy(void **destination, const void *self) {

    if (!destination || !self)
        return NULL;

    if (!*destination)
        *destination = calloc(1, sizeof(struct timeval));

    struct timeval *d = *destination;
    struct timeval *s = (struct timeval *)self;

    *d = *s;

    return *destination;
}

/*----------------------------------------------------------------------------*/

bool dtn_data_timeval_dump(FILE *stream, const void *self) {

    if (!stream || !self)
        goto error;

    char time_buf[30] = {0};

    struct timeval tv = *(struct timeval *)self;

    if (!strftime(time_buf, 30, "%FT%TZ", gmtime(&tv.tv_sec)))
        goto error;

    if (fprintf(stream, "%s (micro seconds)\n", time_buf))
        return true;

error:
    return false;
}

/*----------------------------------------------------------------------------*/

dtn_data_function dtn_data_timeval_data_functions() {

    dtn_data_function function = {

        .clear = dtn_data_timeval_clear,
        .free = dtn_data_timeval_free,
        .copy = dtn_data_timeval_copy,
        .dump = dtn_data_timeval_dump};

    return function;
}

/*----------------------------------------------------------------------------*/

void *dtn_data_pointer_free(void *self) {

    if (!self)
        return NULL;

    free(self);
    return NULL;
}
