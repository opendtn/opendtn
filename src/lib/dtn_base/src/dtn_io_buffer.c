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
        @file           dtn_io_buffer.c
        @author         Töpfer, Markus

        @date           2025-12-19


        ------------------------------------------------------------------------
*/
#include "../include/dtn_io_buffer.h"

#include "../include/dtn_dict.h"

struct dtn_io_buffer {

    dtn_dict *dict;
};

static bool init_config(dtn_io_buffer_config *config) {

    if (!config)
        return false;
    return true;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_io_buffer *dtn_io_buffer_create(dtn_io_buffer_config config) {

    dtn_io_buffer *self = NULL;

    if (!init_config(&config))
        goto error;

    self = calloc(1, sizeof(dtn_io_buffer));
    if (!self)
        goto error;

    dtn_dict_config d_config =
        (dtn_dict_config){.slots = 255,
                          .key.data_function.free = dtn_data_pointer_free,
                          .key.hash = dtn_hash_dtn_socket_data,
                          .key.match = dtn_match_dtn_socket_data,
                          .value.data_function.free = dtn_buffer_free};

    self->dict = dtn_dict_create(d_config);
    if (!self->dict)
        goto error;

    return self;
error:
    dtn_io_buffer_free(self);
    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_io_buffer *dtn_io_buffer_free(dtn_io_buffer *self) {

    if (!self)
        return NULL;

    self->dict = dtn_dict_free(self->dict);
    self = dtn_data_pointer_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_io_buffer_push(dtn_io_buffer *self, dtn_socket_data *remote,
                        const uint8_t *buffer, size_t size) {

    dtn_buffer *val = NULL;
    dtn_socket_data *key = NULL;

    if (!self || !remote || !buffer || size < 1)
        goto error;

    val = dtn_buffer_create(size);
    if (!dtn_buffer_push(val, (uint8_t *)buffer, size))
        goto error;

    key = calloc(1, sizeof(dtn_socket_data));
    if (!key)
        goto error;

    key->port = remote->port;
    memcpy(key->host, remote->host, DTN_HOST_NAME_MAX);

    if (!dtn_dict_set(self->dict, key, val, NULL))
        goto error;
    return true;
error:
    val = dtn_buffer_free(val);
    key = dtn_data_pointer_free(key);
    return false;
}

/*----------------------------------------------------------------------------*/

dtn_buffer *dtn_io_buffer_pop(dtn_io_buffer *self, dtn_socket_data *remote) {

    if (!self || !remote)
        goto error;

    dtn_buffer *out = dtn_dict_remove(self->dict, remote);
    return out;
error:
    return NULL;
}