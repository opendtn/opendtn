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
        @file           dtn_item_json.h
        @author         Töpfer, Markus

        @date           2025-11-29


        ------------------------------------------------------------------------
*/
#ifndef dtn_item_json_h
#define dtn_item_json_h

#include "dtn_item.h"
#include "dtn_json_grammar.h"

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_from_json(const char *string);
char *dtn_item_to_json(const dtn_item *self);

dtn_item *dtn_item_from_json_string(const char *string, size_t len);

/*
 *      ------------------------------------------------------------------------
 *
 *      STRINGIFY CONFIGS
 *
 *      ... full customizable stringify setup
 *
 *      ------------------------------------------------------------------------
 */

typedef struct dtn_item_config {

    struct entry {

        char *intro;  //      ... item intro e.g. space, quote
        char *outro;  //      ... item intro e.g. space, quote
        bool depth;   //      ... enable depth
        char *indent; //      ... whitespace indent (e.g. tab)
    } entry;

    struct item {

        char *intro;     // {    ... intro after depth
        char *out;       // \n   ... out before depth
        char *outro;     // }    ... outro after depth
        char *separator; // ,    ... separation of collection items
        char *delimiter; // :    ... delimiter within object

    } item;

} dtn_item_json_config;

/*----------------------------------------------------------------------------*/

typedef struct dtn_item_json_stringify_config {

    char *intro;

    dtn_item_json_config literal;
    dtn_item_json_config number;
    dtn_item_json_config string;
    dtn_item_json_config array;
    dtn_item_json_config object;

    char *outro;

} dtn_item_json_stringify_config;

/*----------------------------------------------------------------------------*/

dtn_item_json_stringify_config dtn_item_json_config_stringify_minimal();
dtn_item_json_stringify_config dtn_item_json_config_stringify_default();

/*----------------------------------------------------------------------------*/

char *dtn_item_to_json_with_config(const dtn_item *self,
                                   dtn_item_json_stringify_config config);

bool dtn_item_json_write_file(const char *path, const dtn_item *value);
dtn_item *dtn_item_json_read_dir(const char *path, const char *extension);
dtn_item *dtn_item_json_read_file(const char *path);

#endif /* dtn_item_json_h */
