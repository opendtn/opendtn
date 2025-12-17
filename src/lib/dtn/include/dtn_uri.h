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
        @file           dtn_uri.h
        @author         Töpfer, Markus

        @date           2025-12-17


        ------------------------------------------------------------------------
*/
#ifndef dtn_uri_h
#define dtn_uri_h

#include <stdbool.h>
#include <stdio.h>

typedef struct dtn_uri {

    char *scheme;
    char *name;
    char *demux;

} dtn_uri;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_uri *dtn_uri_create();

void *dtn_uri_free(void *self);
bool dtn_uri_clear(void *self);
bool dtn_uri_dump(FILE *stream, void *self);
void *dtn_uri_copy(void** destination, void *source);

/*
 *      ------------------------------------------------------------------------
 *
 *      DE/ENCODER
 *
 *      ------------------------------------------------------------------------
 */

dtn_uri *dtn_uri_decode(const char *string);
char *dtn_uri_encode(const dtn_uri *self);

#endif /* dtn_uri_h */
