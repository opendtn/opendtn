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
        @file           dtn_sdnv.h
        @author         Töpfer, Markus

        @date           2025-12-08

        RFC 6256 Using Self-Delimiting Numeric Values in Protocols

        Implementation of Self Delimiting Numeric Values up to UIN64_MAX

        ------------------------------------------------------------------------
*/
#ifndef dtn_sdnv_h
#define dtn_sdnv_h

#include <stdbool.h>
#include <inttypes.h>
#include <stddef.h>

/*----------------------------------------------------------------------------*/

/**
 *      Decode a max uint64_t number from a buffer.
 * 
 *      @param buffer   start of the buffer
 *      @param size     size of the input buffer
 *      @param out      pointer to parsed value
 *      @param next     pointer to next byte in buffer after number
 *                      (will be set to next byte after number)
 */
bool dtn_sdnv_decode(
        const uint8_t *buffer, 
        size_t size,
        uint64_t *out,
        uint8_t **next);

/*----------------------------------------------------------------------------*/

/**
 *      Encode a max uint64_t number from a buffer.
 * 
 *      @param number   number to encode
 *      @param buffer   start of the buffer
 *      @param size     size of the buffer
 *      @param next     pointer to next byte in buffer after number
 *                      (will be set to next byte after encoded number)
 */
bool dtn_sdnv_encode(
        const uint64_t number,
        uint8_t *buffer, 
        size_t size,
        uint8_t **next);

#endif /* dtn_sdnv_h */
