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
        @file           dtn_aes_key.c
        @author         Töpfer, Markus

        @date           2026-01-03


        ------------------------------------------------------------------------
*/
#include "../include/dtn_aes_key.h"

#include <openssl/rand.h>

dtn_buffer *dtn_aes_key_generate(aes_key_gcm_algorithm algorithm) {

    dtn_buffer *buffer = NULL;

    size_t len = 256;

    switch (algorithm) {

    case AES_128_GCM:

        len = 128;
        break;

    case AES_192_GCM:
        len = 192;
        break;

    case AES_256_GCM:
        len = 256;
        break;

    default:
        goto error;
    }

    buffer = dtn_buffer_create(len);
    if (!buffer)
        goto error;

    if (1 != RAND_bytes(buffer->start, len))
        goto error;

    buffer->length = len;
    return buffer;
error:
    dtn_buffer_free(buffer);
    return NULL;
}