/***
        ------------------------------------------------------------------------

        Copyright (c) 2026 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_aes_key.h
        @author         Töpfer, Markus

        @date           2026-01-03


        ------------------------------------------------------------------------
*/
#ifndef dtn_aes_key_h
#define dtn_aes_key_h

#include <dtn_base/dtn_buffer.h>

/*----------------------------------------------------------------------------*/

typedef enum aes_key_gcm_algorithm {

    AES_128_GCM = 1,
    AES_192_GCM = 2,
    AES_256_GCM = 3

} aes_key_gcm_algorithm;

/*----------------------------------------------------------------------------*/

/**
 *  This function generates a random buffer, which may be used as an AES key
 */
dtn_buffer *dtn_aes_key_generate(aes_key_gcm_algorithm algorithm);

#endif /* dtn_aes_key_h */
