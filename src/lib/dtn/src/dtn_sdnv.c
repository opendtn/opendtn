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
        @file           dtn_sdnv.c
        @author         Töpfer, Markus

        @date           2025-12-08


        ------------------------------------------------------------------------
*/
#include "../include/dtn_sdnv.h"

#include <dtn_base/dtn_utils.h>

#include <stdio.h>

bool dtn_sdnv_decode(const uint8_t *start, size_t size, uint64_t *out,
                     uint8_t **next) {

    if (!start || size < 1 || !out || !next)
        goto error;

    uint8_t *last = NULL;
    uint8_t *ptr = (uint8_t *)start;

    uint64_t nbr = 0;
    uint64_t count = 0;

    int64_t ssize = size;

    while (!last) {

        ssize--;
        count++;

        if (count > 10) {
            dtn_log_error("Input number > uint64_t max - stopping");
            goto error;
        }

        if (0 > ssize)
            goto error;

        if (ptr[0] & 0x80) {
            ptr++;
            continue;
        }

        last = ptr;
    }

    ptr = (uint8_t *)start;

    if ((last - start) > 9) {

        dtn_log_error("Input number > uint64_t max - stopping");
        goto error;

    } else if (last - start == 9) {

        if (start[0] > 0x81) {

            dtn_log_error("Input number > uint64_t max - stopping");
            goto error;
        }
    }

    while (ptr < last) {

        nbr += ((ptr[0] & 0x7F));
        nbr = nbr << 7;
        ptr++;
    }

    nbr += last[0] & 0x7F;

    *next = last + 1;
    *out = nbr;

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_sdnv_encode(const uint64_t number, uint8_t *buffer, size_t size,
                     uint8_t **next) {

    uint8_t *last = NULL;

    if (!buffer || size < 1 || !next)
        goto error;

    if (number > 0x7fFFffFFffFFffFF) {

        if (size < 10)
            goto error;

        last = buffer + 9;

    } else if (number > 0xFFffFFffFFffFF) {

        if (size < 9)
            goto error;

        last = buffer + 8;

    } else if (number > 0x1FFffFFffFFff) {

        if (size < 8)
            goto error;

        last = buffer + 7;

    } else if (number > 0x3FFffFFffFF) {

        if (size < 7)
            goto error;

        last = buffer + 6;

    } else if (number > 0x7FffFFffFF) {

        if (size < 6)
            goto error;

        last = buffer + 5;

    } else if (number > 0xFffFFff) {

        if (size < 5)
            goto error;

        last = buffer + 4;

    } else if (number > 0x1FffFF) {

        if (size < 4)
            goto error;

        last = buffer + 3;

    } else if (number > 0x3FFF) {

        if (size < 3)
            goto error;

        last = buffer + 2;

    } else if (number > 0x7F) {

        if (size < 2)
            goto error;

        last = buffer + 1;

    } else {

        if (size < 1)
            goto error;

        last = buffer;
    }

    *next = last + 1;

    uint64_t nbr = number;

    last[0] = nbr & 0x7F;

    while (last > buffer) {

        last--;

        nbr = nbr >> 7;

        last[0] = nbr & 0x7F;
        last[0] |= 0x80;
    }

    return true;
error:
    return false;
}