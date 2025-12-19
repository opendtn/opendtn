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
        @file           dtn_io_buffer.h
        @author         Töpfer, Markus

        @date           2025-12-19

        IO buffer based on remote data usefull for UDP based data buffering. 

        ------------------------------------------------------------------------
*/
#ifndef dtn_io_buffer_h
#define dtn_io_buffer_h

#include "dtn_buffer.h"
#include "dtn_socket.h"

typedef struct dtn_io_buffer dtn_io_buffer;

typedef struct dtn_io_buffer_config {

    struct {

        uint64_t default_size;

    } limits;

} dtn_io_buffer_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_io_buffer *dtn_io_buffer_create(dtn_io_buffer_config config);
dtn_io_buffer *dtn_io_buffer_free(dtn_io_buffer *self);

/*----------------------------------------------------------------------------*/

bool dtn_io_buffer_push(dtn_io_buffer *self, 
    dtn_socket_data *remote, 
    const uint8_t *buffer, 
    size_t size);

/*----------------------------------------------------------------------------*/

dtn_buffer *dtn_io_buffer_pop(dtn_io_buffer *self, 
    dtn_socket_data *remote);

#endif /* dtn_io_buffer_h */
