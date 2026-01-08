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
        @file           dtn_aes_key_wrap.h
        @author         Töpfer, Markus

        @date           2025-12-30


        ------------------------------------------------------------------------
*/
#ifndef dtn_aes_key_wrap_h
#define dtn_aes_key_wrap_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*----------------------------------------------------------------------------*/

bool dtn_aes_key_wrap(uint8_t *input, size_t in_size, uint8_t *out,
                      size_t *out_size, uint8_t *key, size_t key_size);

/*----------------------------------------------------------------------------*/

bool dtn_aes_key_unwrap(uint8_t *input, size_t in_size, uint8_t *out,
                        size_t *out_size, uint8_t *key, size_t key_size);

#endif /* dtn_aes_key_wrap_h */
