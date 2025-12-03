/***
        ------------------------------------------------------------------------

        Copyright (c) 2018 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_match_functions.h
        @author         Michael Beer
        @author         Markus Toepfer

        @date           2018-08-15

        @ingroup        dtn_base

        @brief          Definition of some default matching functions.


        ------------------------------------------------------------------------
*/
#ifndef dtn_match_functions_h
#define dtn_match_functions_h

#include <inttypes.h>
#include <stdbool.h>

/*---------------------------------------------------------------------------*/

/**
        BYTE MATCH of key and string
        up to \0 termination of string.
*/
bool dtn_match_strict(const void *key, const void *string);

/*---------------------------------------------------------------------------*/

/**
        MATCH a \0 terminated string,
        against another \0 terminated string.
        Case sensitive match.
*/
bool dtn_match_c_string_strict(const void *key, const void *string);

/*---------------------------------------------------------------------------*/

/**
        MATCH a \0 terminated string,
        against another \0 terminated string.
        Case insensitive match.
*/
bool dtn_match_c_string_case_ignore_strict(const void *key, const void *string);

/*---------------------------------------------------------------------------*/

/**
        Intptr match
*/
bool dtn_match_intptr(const void *ptr1, const void *ptr2);

/*---------------------------------------------------------------------------*/

/**
        Uint64_t match
*/
bool dtn_match_uint64(const void *ptr1, const void *ptr2);

#endif /* dtn_match_functions_h */
