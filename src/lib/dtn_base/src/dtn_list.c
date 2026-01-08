/***
        ------------------------------------------------------------------------

        Copyright 2018 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_list.c
        @author         Markus Toepfer
        @author         Michael Beer

        @date           2018-07-02

        @ingroup        dtn_basics

        @brief


    ------------------------------------------------------------------------
    */

// #include "../../include/dtn_list.h"

#define MAGIC_BYTE dtn_LIST_MAGIC_BYTE

#define DEFAULT_LIST_ITEM_RATE 100
#define DEFAULT_LIST_SHRINK false

/* include default implementation */
#include "../include/dtn_vector_list.h"

/*
 *      ------------------------------------------------------------------------
 *
 *                        PROTOTYPE DEFINITION
 *
 *      ------------------------------------------------------------------------
 */

static bool list_copy_each(void *item, void *destination_list);
static dtn_list *list_copy(const dtn_list *self, dtn_list *destination);
static bool list_dump(const dtn_list *self, FILE *stream);

/*
 *      ------------------------------------------------------------------------
 *
 *                        Default IMPLEMENTATION
 *
 *      ------------------------------------------------------------------------
 */

dtn_list *dtn_list_cast(const void *data) {

    if (!data)
        return NULL;

    if (*(uint16_t *)data != MAGIC_BYTE)
        return NULL;

    return (dtn_list *)data;
}

/*----------------------------------------------------------------------------*/

dtn_list *dtn_list_create(dtn_list_config config) {

    dtn_list *list = dtn_vector_list_create(config);
    if (!list)
        goto error;

    if (!dtn_vector_list_set_shrink(list, DEFAULT_LIST_SHRINK))
        goto error;

    if (!dtn_vector_list_set_rate(list, DEFAULT_LIST_ITEM_RATE))
        goto error;

    return list;

error:
    if (list)
        list->free(list);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_list_clear(void *data) {

    dtn_list *list = dtn_list_cast(data);

    if (!list)
        return false;

    if (list->clear)
        return list->clear(list);

    return false;
}

/*---------------------------------------------------------------------------*/

void *dtn_list_free(void *data) {

    dtn_list *list = dtn_list_cast(data);

    if (!list)
        return data;

    if (!list->clear || !list->free)
        return data;

    if (!list->clear(list))
        return data;

    return list->free(list);
}

/*----------------------------------------------------------------------------*/

void *dtn_list_copy(void **destination, const void *source) {

    if (!destination || !source)
        return NULL;

    dtn_list *orig = dtn_list_cast(source);
    dtn_list *copy = NULL;

    if (!orig || !orig->copy)
        return NULL;

    if (*destination) {

        copy = dtn_list_cast(*destination);
        if (!copy)
            return NULL;

    } else {

        *destination = dtn_list_create(orig->config);
        copy = *destination;
    }

    if (!orig->copy(orig, copy))
        return NULL;

    return copy;
}

/*----------------------------------------------------------------------------*/

bool dtn_list_dump(FILE *stream, const void *data) {

    if (!stream || !data)
        return false;

    dtn_list *list = dtn_list_cast(data);

    if (!list)
        return false;

    if (list_dump(list, stream))
        return true;

    if (!list->count)
        return false;

    if (!fprintf(stream, "LIST %p has %zu items\n", list, list->count(list)))
        return false;

    return true;
}

/*----------------------------------------------------------------------------*/

dtn_data_function dtn_list_data_functions() {

    dtn_data_function function = {

        .clear = dtn_list_clear,
        .free = dtn_list_free,
        .copy = dtn_list_copy,
        .dump = dtn_list_dump

    };

    return function;
}

/*---------------------------------------------------------------------------*/

dtn_list *dtn_list_set_magic_bytes(dtn_list *list) {

    if (0 == list)
        goto error;

    list->magic_byte = MAGIC_BYTE;

    return list;

error:

    return 0;
}

/*---------------------------------------------------------------------------*/

dtn_list_default_implementations dtn_list_get_default_implementations() {

    return (dtn_list_default_implementations){
        .copy = list_copy,
    };
}

/*----------------------------------------------------------------------------*/

static bool list_copy_each(void *item, void *destination_list) {

    dtn_list *dest = dtn_list_cast(destination_list);
    if (!dest)
        return false;

    void *new_item = NULL;

    if (item)
        if (!dest->config.item.copy(&new_item, item))
            return false;

    return dest->push(dest, new_item);
}

/*----------------------------------------------------------------------------*/

static dtn_list *list_copy(const dtn_list *self, dtn_list *destination) {

    if (!self || !destination)
        return NULL;

    if (NULL == self->config.item.copy)
        return NULL;

    destination->config = self->config;

    if (!self->for_each((dtn_list *)self, destination, list_copy_each))
        return NULL;

    return destination;
};

/*----------------------------------------------------------------------------*/

struct dtn_list_dump_data {

    FILE *stream;
    dtn_DATA_DUMP dump;
};

/*----------------------------------------------------------------------------*/

static bool list_dump_each(void *item, void *list_dump_data) {

    if (!list_dump_data)
        return false;

    struct dtn_list_dump_data *d = list_dump_data;
    return d->dump(d->stream, item);
}

/*----------------------------------------------------------------------------*/

static bool list_dump(const dtn_list *list, FILE *stream) {

    if (!list || !stream)
        return false;

    if (NULL == list->config.item.dump)
        return false;

    struct dtn_list_dump_data data = {

        .stream = stream, .dump = list->config.item.dump};

    return list->for_each((dtn_list *)list, &data, list_dump_each);
}

/*
 *      ------------------------------------------------------------------------
 *
 *                        GENERIC FUNCTIONS
 *
 *       ... definition of common generic list functions
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_list_remove_if_included(dtn_list *list, const void *item) {

    if (!list || !dtn_list_cast(list))
        goto error;

    if (!item || list->is_empty(list))
        return true;

    size_t pos = list->get_pos(list, item);

    // was not included ?
    if (pos == 0)
        return true;

    // was removed ?
    if (list->remove(list, pos))
        return true;

error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *                        FUNCTIONS TO INTERNAL POINTERS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_list_is_empty(const dtn_list *list) {

    if (!list || !list->is_empty)
        return false;

    return list->is_empty(list);
}

/*---------------------------------------------------------------------------*/

size_t dtn_list_get_pos(const dtn_list *list, void *item) {

    if (!list || !list->get_pos || !item)
        return 0;

    return list->get_pos(list, item);
}

/*---------------------------------------------------------------------------*/

void *dtn_list_get(const dtn_list *list, size_t pos) {

    if (!list || !list->get)
        return NULL;

    return list->get((dtn_list *)list, pos);
}

/*---------------------------------------------------------------------------*/

bool dtn_list_set(dtn_list *list, size_t pos, void *item, void **replaced) {

    if (!list || !list->set || !item)
        return false;

    return list->set(list, pos, item, replaced);
}

/*---------------------------------------------------------------------------*/

bool dtn_list_insert(dtn_list *list, size_t pos, void *item) {

    if (!list || !list->insert || !item)
        return false;

    return list->insert(list, pos, item);
}

/*---------------------------------------------------------------------------*/

void *dtn_list_remove(dtn_list *list, size_t pos) {

    if (!list || !list->remove)
        return NULL;

    return list->remove(list, pos);
}

/*---------------------------------------------------------------------------*/

bool dtn_list_delete(dtn_list *list, size_t pos) {

    if (!list || !list->remove || !list->config.item.free)
        return false;

    void *item = list->remove(list, pos);
    item = list->config.item.free(item);

    if (item)
        return false;
    return true;
}

/*---------------------------------------------------------------------------*/

bool dtn_list_push(dtn_list *list, void *item) {

    if (!list || !list->push)
        return false;

    return list->push(list, item);
}

/*---------------------------------------------------------------------------*/

void *dtn_list_pop(dtn_list *list) {

    if (!list || !list->pop)
        return NULL;

    return list->pop(list);
}

/*---------------------------------------------------------------------------*/

size_t dtn_list_count(const dtn_list *list) {

    if (!list || !list->count)
        return 0;

    return list->count(list);
}

/*---------------------------------------------------------------------------*/

bool dtn_list_for_each(dtn_list *list, void *data,
                       bool (*function)(void *item, void *data)) {

    if (!list || !list->for_each || !function)
        return false;

    return list->for_each(list, data, function);
}

/*---------------------------------------------------------------------------*/

bool dtn_list_queue_push(dtn_list *list, void *item) {

    return dtn_list_insert(list, 1, item);
}

/*---------------------------------------------------------------------------*/

void *dtn_list_queue_pop(dtn_list *list) { return dtn_list_pop(list); }
