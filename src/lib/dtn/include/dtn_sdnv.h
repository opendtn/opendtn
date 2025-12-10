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
        @file           dtn_sdnv.h
        @author         Töpfer, Markus

        @date           2025-12-08

        RFC 5050 Self Delimiting Numeric Values

        ------------------------------------------------------------------------
*/
#ifndef dtn_sdnv_h
#define dtn_sdnv_h

#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_sdnv_decode(
        const uint8_t *pointer, 
        size_t size,
        uint64_t *out,
        uint8_t **next);

bool dtn_sdnv_encode(
        const uint64_t number,
        uint8_t *buffer, 
        size_t size,
        uint8_t **next);


#endif /* dtn_sdnv_h */
