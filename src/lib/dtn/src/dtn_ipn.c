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
        @file           dtn_ipn.c
        @author         Töpfer, Markus

        @date           2025-12-17


        ------------------------------------------------------------------------
*/
#include "../include/dtn_ipn.h"

#include <dtn_base/dtn_data_function.h>
#include <dtn_base/dtn_string.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

dtn_ipn *dtn_ipn_create() {

    dtn_ipn *self = calloc(1, sizeof(dtn_ipn));
    return self;
}

/*----------------------------------------------------------------------------*/

void *dtn_ipn_free(void *self) {

    if (!self)
        return self;

    dtn_ipn *uri = (dtn_ipn *)self;
    if (!dtn_ipn_clear(uri))
        goto error;

    uri = dtn_data_pointer_free(uri);
    return NULL;
error:
    return self;
}

/*----------------------------------------------------------------------------*/

bool dtn_ipn_clear(void *self) {

    if (!self)
        return false;
    dtn_ipn *uri = (dtn_ipn *)self;
    uri->scheme = dtn_data_pointer_free(uri->scheme);
    uri->node = dtn_data_pointer_free(uri->node);
    uri->service = dtn_data_pointer_free(uri->service);
    return true;
}

/*----------------------------------------------------------------------------*/

bool dtn_ipn_dump(FILE *stream, void *self) {

    if (!stream || !self)
        return false;

    dtn_ipn *uri = (dtn_ipn *)self;
    char *string = dtn_ipn_encode(uri);
    fprintf(stream, "%s", string);
    string = dtn_data_pointer_free(string);
    return true;
}

/*----------------------------------------------------------------------------*/

void *dtn_ipn_copy(void **destination, void *source) {

    if (!destination || !source)
        goto error;
    if (*destination)
        goto error;

    dtn_ipn *copy = dtn_ipn_create();
    dtn_ipn *self = (dtn_ipn *)source;

    copy->scheme = dtn_string_dup(self->scheme);
    copy->node = dtn_string_dup(self->node);
    copy->service = dtn_string_dup(self->service);

    *destination = copy;
    return copy;
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_ipn *dtn_ipn_decode(const char *string) {

    dtn_ipn *self = NULL;
    if (!string)
        goto error;

    size_t size = strlen(string);

    char *del1 = memchr(string, ':', size);
    if (!del1)
        goto error;

    char *del2 = memchr(del1, '.', size - (del1 - string));
    if (!del2)
        goto error;

    if (del2 - del1 < 2)
        goto error;
    if (size - (del2 - string) == 1)
        goto error;

    char *ptr = del1 + 1;
    while (ptr < del2) {

        if (!isdigit(ptr[0]))
            goto error;
        ptr++;
    }

    ptr = del2 + 1;
    while (size - (ptr - string) > 0) {

        if (!isdigit(ptr[0]))
            goto error;
        ptr++;
    }

    self = dtn_ipn_create();
    if (!self)
        goto error;

    self->scheme = calloc((del1 - string) + 1, sizeof(char));
    if (!self->scheme)
        goto error;
    if (!strncpy(self->scheme, string, (del1 - string)))
        goto error;

    self->node = calloc((del2 - del1) + 1, sizeof(char));
    if (!self->node)
        goto error;
    if (!strncpy(self->node, del1 + 1, (del2 - del1) - 1))
        goto error;

    self->service = calloc(size - (del2 - string) + 1, sizeof(char));
    if (!self->service)
        goto error;
    if (!strncpy(self->service, del2 + 1, (size - (del2 - string) - 1)))
        goto error;

    return self;
error:
    dtn_ipn_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

char *dtn_ipn_encode(const dtn_ipn *self) {

    size_t size = 2 * PATH_MAX;

    char *string = calloc(size, sizeof(char));
    if (!string)
        goto error;

    snprintf(string, size, "%s:%s.%s", self->scheme, self->node, self->service);

    return string;
error:
    return NULL;
}
