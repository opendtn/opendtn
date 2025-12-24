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
        @file           dtn_uri.c
        @author         Töpfer, Markus

        @date           2025-12-17


        ------------------------------------------------------------------------
*/
#include "../include/dtn_uri.h"

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <dtn_base/dtn_string.h>
#include <dtn_base/dtn_data_function.h>

dtn_uri *dtn_uri_create(){

    dtn_uri *self = calloc(1, sizeof(dtn_uri));
    return self;
}

/*----------------------------------------------------------------------------*/

void *dtn_uri_free(void *self){

    if (!self) return self;

    dtn_uri *uri = (dtn_uri*) self;
    if (!dtn_uri_clear(uri)) goto error;

    uri = dtn_data_pointer_free(uri);
    return NULL;
error:
    return self;
}

/*----------------------------------------------------------------------------*/

bool dtn_uri_clear(void *self){

    if (!self) return false;
    dtn_uri *uri = (dtn_uri*) self;
    uri->scheme = dtn_data_pointer_free(uri->scheme);
    uri->name = dtn_data_pointer_free(uri->name);
    uri->demux = dtn_data_pointer_free(uri->demux);
    return true;
}

/*----------------------------------------------------------------------------*/

bool dtn_uri_dump(FILE *stream, void *self){

    if (!stream || !self) return false;

    dtn_uri *uri = (dtn_uri*) self;
    char *string = dtn_uri_encode(uri);
    fprintf(stream, "%s", string);
    string = dtn_data_pointer_free(string);
    return true;
}

/*----------------------------------------------------------------------------*/

void *dtn_uri_copy(void** destination, void *source){

    if (!destination || !source) goto error;
    if (*destination) goto error;

    dtn_uri *copy = dtn_uri_create();
    dtn_uri *self = (dtn_uri*) source;

    copy->scheme = dtn_string_dup(self->scheme);
    copy->name = dtn_string_dup(self->name);
    copy->demux = dtn_string_dup(self->demux);

    *destination = copy;
    return copy;
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

static bool check_string(const char *string, size_t size){

    char *ptr = (char*) string;

    while((intptr_t)size > (ptr - string)){

        if (ptr[0] < 0x23) goto error;
        if (ptr[0] > 0x7E) goto error;

        ptr++;
    }
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

dtn_uri *dtn_uri_decode(const char *string){

    dtn_uri *self = NULL;
    if (!string) goto error;

    size_t size = strlen(string);

    if (!check_string(string, size)) goto error;

    self = dtn_uri_create();

    char *scheme_delimiter = memchr(string, ':', size);
    if (!scheme_delimiter) goto error;

    self->scheme = calloc(scheme_delimiter - string + 1, sizeof(char));
    if (!self->scheme) goto error;
    strncpy(self->scheme, string, scheme_delimiter - string);

    if ((int64_t) size < scheme_delimiter - string + 3) goto error;
    if (scheme_delimiter[1] != '/'){

        if (0 != strncmp(scheme_delimiter + 1, "none", 
            size -(scheme_delimiter - string - 1)))
            goto error;

        self->name = dtn_string_dup("none");
        goto done;
    }

    if (scheme_delimiter[1] != '/') goto error;
    if (scheme_delimiter[2] != '/') goto error;

    char *delimiter = memchr(scheme_delimiter + 3, '/', 
        size - (scheme_delimiter - string - 3));

    if (!delimiter){

        self->name = calloc(size - (scheme_delimiter-string) +1 , sizeof(char));
        if (!self->name) goto error;

        strncpy(self->name, scheme_delimiter + 3, 
            size - (scheme_delimiter - string) - 1);
        
        goto done;
    }

    self->name = calloc(size - (delimiter-scheme_delimiter) +1 , sizeof(char));
    self->demux = calloc(size - (delimiter - string) + 1, sizeof(char));
    if (!self->name) goto error;
    if (!self->demux) goto error;
    
    strncpy(self->name, scheme_delimiter + 3, 
            delimiter - scheme_delimiter - 3);

    strncpy(self->demux, delimiter + 1, 
            size - (delimiter - string) -1);

done:
    return self;
error:
    dtn_uri_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

char *dtn_uri_encode(const dtn_uri *self){

    if (!self) return NULL;

    size_t size = 2*PATH_MAX;

    char *string = calloc(size, sizeof(char));
    if (!string) goto error;

    if (!self->name){

         snprintf(string, size, "%s:none",
            self->scheme);

    } else {

        if (self->demux){

            snprintf(string, size, "%s://%s/%s",
            self->scheme,
            self->name,
            self->demux);

        } else {

            snprintf(string, size, "%s://%s/",
            self->scheme,
            self->name);
        }        
    }
    
    return string;
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_uri_is_singleton(const dtn_uri *self){

    if (!self || !self->demux) return false;

    if (self->demux[0] == '~')
        return false;

    return true;
}