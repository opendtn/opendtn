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
        @file           dtn_crc16.h
        @author         Töpfer, Markus

        @date           2025-12-16

        
        ------------------------------------------------------------------------
*/
#ifndef dtn_crc16_h
#define dtn_crc16_h

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

// RFC 1662
uint16_t crc16x25(const uint8_t *buffer, size_t size);

#endif /* dtn_crc16_h */
