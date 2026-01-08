/***
        ------------------------------------------------------------------------

        Copyright (c) 2024 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_socket_item.c
        @author         Markus

        @date           2024-10-11


        ------------------------------------------------------------------------
*/
#include "../include/dtn_socket_item.h"

#define DTN_SOCKET_ITEM_MAGIC_BYTES 0x5234

#include <dtn_base/dtn_convert.h>
#include <dtn_base/dtn_dict.h>
#include <dtn_base/dtn_thread_lock.h>

/*----------------------------------------------------------------------------*/

struct dtn_socket_item {

    uint16_t magic_bytes;
    dtn_socket_item_config config;

    dtn_thread_lock lock;

    dtn_dict *data;
};

/*----------------------------------------------------------------------------*/

dtn_socket_item *dtn_socket_item_create(dtn_socket_item_config config) {

    dtn_socket_item *self = NULL;

    if (!config.loop)
        goto error;

    if (0 == config.limits.threadlock_timeout_usec)
        config.limits.threadlock_timeout_usec = 100000;

    self = calloc(1, sizeof(dtn_socket_item));
    if (!self)
        goto error;

    self->magic_bytes = DTN_SOCKET_ITEM_MAGIC_BYTES;
    self->config = config;

    dtn_dict_config d_config = dtn_dict_intptr_key_config(255);
    d_config.value.data_function.free = dtn_item_free;

    self->data = dtn_dict_create(d_config);
    if (!dtn_thread_lock_init(&self->lock,
                              config.limits.threadlock_timeout_usec))
        goto error;

    return self;
error:
    dtn_socket_item_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_socket_item *dtn_socket_item_cast(const void *self) {

    if (!self)
        goto error;

    if (*(uint16_t *)self == DTN_SOCKET_ITEM_MAGIC_BYTES)
        return (dtn_socket_item *)self;

error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_socket_item *dtn_socket_item_free(dtn_socket_item *self) {

    if (!dtn_socket_item_cast(self))
        return NULL;

    dtn_thread_lock_clear(&self->lock);

    self->data = dtn_dict_free(self->data);
    self = dtn_data_pointer_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_item *dtn_socket_item_get(dtn_socket_item *self, int socket) {

    dtn_item *out = NULL;

    if (!self)
        goto error;
    if (!dtn_thread_lock_try_lock(&self->lock))
        goto error;

    dtn_item *data = dtn_dict_get(self->data, (void *)(intptr_t)socket);

    if (!data) {

        out = dtn_item_object();

    } else {

        dtn_item_copy((void **)&out, data);
    }

    dtn_thread_lock_unlock(&self->lock);
    return out;
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_socket_item_set(dtn_socket_item *self, int socket, dtn_item **value) {

    if (!self || !socket || !value)
        goto error;
    if (!dtn_thread_lock_try_lock(&self->lock))
        goto error;

    dtn_item *data = *value;

    intptr_t key = socket;
    bool result = dtn_dict_set(self->data, (void *)key, data, NULL);

    if (result)
        *value = NULL;

    dtn_thread_lock_unlock(&self->lock);
    return result;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_socket_item_drop(dtn_socket_item *self, int socket) {

    if (!self)
        goto error;
    if (!dtn_thread_lock_try_lock(&self->lock))
        goto error;

    bool result = dtn_dict_del(self->data, (void *)(intptr_t)socket);
    dtn_thread_lock_unlock(&self->lock);

    return result;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool add_data_to_out(const void *key, void *val, void *data) {

    char *k = NULL;
    size_t l = 0;

    if (!key)
        return true;

    dtn_item *out = dtn_item_cast(data);
    dtn_item *self = dtn_item_cast(val);

    dtn_item *copy = NULL;
    if (!dtn_item_copy((void **)&copy, self))
        goto error;

    intptr_t p = (intptr_t)key;

    if (!dtn_convert_int64_to_string((int64_t)p, &k, &l))
        goto error;
    if (!dtn_item_object_set(out, k, copy))
        goto error;

    return true;
error:
    copy = dtn_item_free(copy);
    k = dtn_data_pointer_free(k);
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_socket_item_for_each_set_data(dtn_socket_item *self, dtn_item *out) {

    if (!self)
        goto error;
    if (!dtn_thread_lock_try_lock(&self->lock))
        goto error;

    bool result = dtn_dict_for_each(self->data, out, add_data_to_out);

    dtn_thread_lock_unlock(&self->lock);

    return result;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_socket_item_for_each(dtn_socket_item *self, void *data,
                              bool (*function)(const void *key, void *val,
                                               void *data)) {

    if (!self || !function)
        goto error;

    if (!dtn_thread_lock_try_lock(&self->lock))
        goto error;

    bool result = dtn_dict_for_each(self->data, data, function);

    dtn_thread_lock_unlock(&self->lock);

    return result;
error:
    return false;
}