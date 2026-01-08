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
        @file           dtn_item.c
        @author         Töpfer, Markus

        @date           2025-11-28


        ------------------------------------------------------------------------
*/
#include "../include/dtn_item.h"

#include <stdlib.h>
#include <string.h>

#include "../include/dtn_data_function.h"
#include "../include/dtn_dict.h"
#include "../include/dtn_linked_list.h"
#include "../include/dtn_log.h"
#include "../include/dtn_string.h"

/*---------------------------------------------------------------------------*/

#define dtn_ITEM_MAGIC_BYTES 0x17e9

/*---------------------------------------------------------------------------*/

typedef enum dtn_item_literal {

    dtn_ITEM_NULL = 0,
    dtn_ITEM_TRUE = 1,
    dtn_ITEM_FALSE = 2,
    dtn_ITEM_ARRAY = 3,
    dtn_ITEM_OBJECT = 4,
    dtn_ITEM_STRING = 6,
    dtn_ITEM_NUMBER = 7

} dtn_item_literal;

/*---------------------------------------------------------------------------*/

typedef struct dtn_item_config {

    dtn_item_literal literal;

    union {
        double number;
        void *data;
    };

} dtn_item_config;

/*---------------------------------------------------------------------------*/

struct dtn_item {

    uint16_t magic_bytes;
    dtn_item_config config;
    dtn_item *parent;
};

/*---------------------------------------------------------------------------*/

static dtn_item *item_create(dtn_item_config config) {

    dtn_item *item = calloc(1, sizeof(dtn_item));
    if (!item)
        goto error;

    item->magic_bytes = dtn_ITEM_MAGIC_BYTES;
    item->config = config;
    item->parent = NULL;

    return item;
error:
    dtn_item_free(item);
    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_cast(const void *self) {

    if (!self)
        return NULL;

    if (*(uint16_t *)self == dtn_ITEM_MAGIC_BYTES)
        return (dtn_item *)self;

    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_get_parent(const dtn_item *self) {

    if (!self)
        return NULL;
    return self->parent;
}

/*---------------------------------------------------------------------------*/

void *dtn_item_free(void *self) {

    dtn_item *item = dtn_item_cast(self);
    if (!item)
        return self;

    switch (item->config.literal) {

    case dtn_ITEM_NULL:
    case dtn_ITEM_TRUE:
    case dtn_ITEM_FALSE:
    case dtn_ITEM_NUMBER:
        break;

    case dtn_ITEM_STRING:

        if (item->config.data)
            item->config.data = dtn_data_pointer_free(item->config.data);

        break;

    case dtn_ITEM_ARRAY:

        if (item->config.data)
            item->config.data = dtn_list_free(item->config.data);

        break;

    case dtn_ITEM_OBJECT:

        if (item->config.data)
            item->config.data = dtn_dict_free(item->config.data);

        break;

    default:
        break;
    }

    item = dtn_data_pointer_free(item);
    return NULL;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_clear(void *self) {

    dtn_item *item = dtn_item_cast(self);
    if (!item)
        return false;

    switch (item->config.literal) {

    case dtn_ITEM_NULL:
    case dtn_ITEM_TRUE:
    case dtn_ITEM_FALSE:
    case dtn_ITEM_NUMBER:
        break;

    case dtn_ITEM_STRING:

        if (item->config.data)
            item->config.data = dtn_data_pointer_free(item->config.data);

        break;

    case dtn_ITEM_ARRAY:

        if (item->config.data)
            item->config.data = dtn_list_free(item->config.data);

        break;

    case dtn_ITEM_OBJECT:

        if (item->config.data)
            item->config.data = dtn_dict_free(item->config.data);

        break;

    default:
        break;
    }

    item->config.literal = dtn_ITEM_NULL;
    item->config.number = 0;

    return true;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_dump(FILE *stream, const void *self) {

    if (!stream || !self)
        return false;
    dtn_item *item = dtn_item_cast(self);
    if (!item)
        goto error;

    switch (item->config.literal) {

    case dtn_ITEM_NULL:

        if (!fprintf(stream, "\nnull"))
            goto error;
        break;

    case dtn_ITEM_TRUE:

        if (!fprintf(stream, "\ntrue"))
            goto error;
        break;

    case dtn_ITEM_FALSE:

        if (!fprintf(stream, "\nfalse"))
            goto error;
        break;

    case dtn_ITEM_NUMBER:

        if (!fprintf(stream, "\n%f", item->config.number))
            goto error;
        break;

    case dtn_ITEM_STRING:

        if (item->config.data)
            if (!fprintf(stream, "\n%s", (char *)item->config.data))
                goto error;

        break;

    case dtn_ITEM_ARRAY:

        if (item->config.data)
            dtn_list_dump(stream, item->config.data);

        break;

    case dtn_ITEM_OBJECT:

        if (item->config.data)
            dtn_dict_dump(stream, item->config.data);

        break;

    default:
        break;
    }

    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

void *dtn_item_copy(void **destination, const void *self) {

    dtn_item *copy = NULL;

    if (!self)
        goto error;

    dtn_item *source = dtn_item_cast(self);
    if (!source)
        goto error;

    copy = NULL;

    bool result = true;

    switch (source->config.literal) {

    case dtn_ITEM_NULL:
        copy = dtn_item_null();
        break;
    case dtn_ITEM_TRUE:
        copy = dtn_item_true();
        break;
    case dtn_ITEM_FALSE:
        copy = dtn_item_false();
        break;

    case dtn_ITEM_NUMBER:

        copy = dtn_item_number(source->config.number);
        break;

    case dtn_ITEM_STRING:

        copy = dtn_item_string(source->config.data);

        break;

    case dtn_ITEM_ARRAY:

        copy = dtn_item_array();

        if (!dtn_list_copy((void **)&copy->config.data, source->config.data)) {

            result = false;
        }

        break;

    case dtn_ITEM_OBJECT:

        copy = dtn_item_object();

        if (!dtn_dict_copy((void **)&copy->config.data, source->config.data)) {

            result = false;
        }

        break;

    default:
        break;
    }

    if (!result)
        goto error;
    *destination = copy;
    return copy;
error:
    copy = dtn_item_free(copy);
    return NULL;
}

/*---------------------------------------------------------------------------*/

size_t dtn_item_count(const dtn_item *self) {

    if (!self)
        goto error;

    uint64_t count = 0;

    switch (self->config.literal) {

    case dtn_ITEM_NULL:
    case dtn_ITEM_TRUE:
    case dtn_ITEM_FALSE:
    case dtn_ITEM_NUMBER:
    case dtn_ITEM_STRING:

        count = 1;
        break;

    case dtn_ITEM_ARRAY:

        count = dtn_list_count(self->config.data);
        break;

    case dtn_ITEM_OBJECT:

        count = dtn_dict_count(self->config.data);
        break;

    default:
        break;
    }

    return count;
error:
    return 0;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_is_empty(const dtn_item *self) {

    size_t count = dtn_item_count(self);
    if (0 == count)
        return true;
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #OBJECT FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_item *dtn_item_object() {

    dtn_item *item = item_create((dtn_item_config){.literal = dtn_ITEM_OBJECT});

    if (!item)
        goto error;

    dtn_dict_config d_config = dtn_dict_string_key_config(255);
    d_config.value.data_function.free = dtn_item_free;
    d_config.value.data_function.clear = dtn_item_clear;
    d_config.value.data_function.dump = dtn_item_dump;
    d_config.value.data_function.copy = dtn_item_copy;

    item->config.data = dtn_dict_create(d_config);
    if (!item->config.data)
        goto error;

    return item;
error:
    dtn_item_free(item);
    return NULL;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_is_object(const dtn_item *self) {

    if (!self)
        goto error;

    if (self->config.literal == dtn_ITEM_OBJECT)
        return true;

error:
    return false;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_object_set(dtn_item *self, const char *string, dtn_item *val) {

    char *key = NULL;

    if (!dtn_item_is_object(self) || !string || !val)
        goto error;

    key = dtn_string_dup(string);

    if (!dtn_dict_set(self->config.data, key, val, NULL))
        goto error;

    val->parent = self;
    return true;

error:
    key = dtn_data_pointer_free(key);
    return false;
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_object_get(const dtn_item *self, const char *string) {

    dtn_item *out = NULL;

    if (!dtn_item_is_object(self) || !string)
        goto error;

    out = dtn_item_cast(dtn_dict_get(self->config.data, (void *)string));

    return out;
error:
    return NULL;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_object_for_each(dtn_item *self,
                              bool (*function)(char const *key,
                                               dtn_item const *val,
                                               void *userdata),
                              void *userdata) {

    if (!dtn_item_is_object(self) || !function)
        goto error;

    bool result =
        dtn_dict_for_each(self->config.data, userdata, (void *)function);

    return result;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_object_delete(dtn_item *self, const char *string) {

    if (!dtn_item_is_object(self) || !string)
        goto error;

    bool result = dtn_dict_del(self->config.data, string);

    return result;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_object_remove(dtn_item *self, const char *string) {

    if (!dtn_item_is_object(self) || !string)
        goto error;

    dtn_item *out = NULL;

    out = dtn_dict_remove(self->config.data, string);

    if (out)
        out->parent = NULL;

    return out;
error:
    return NULL;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #ARRAY FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_item *dtn_item_array() {

    dtn_item *item = item_create((dtn_item_config){.literal = dtn_ITEM_ARRAY});

    if (!item)
        goto error;

    dtn_list_config l_config = (dtn_list_config){.item.free = dtn_item_free,
                                                 .item.clear = dtn_item_clear,
                                                 .item.dump = dtn_item_dump,
                                                 .item.copy = dtn_item_copy};

    item->config.data = dtn_linked_list_create(l_config);
    if (!item->config.data)
        goto error;

    return item;
error:
    dtn_item_free(item);
    return NULL;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_is_array(const dtn_item *self) {

    if (!self)
        goto error;

    if (self->config.literal == dtn_ITEM_ARRAY)
        return true;

error:
    return false;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_array_for_each(dtn_item *self, void *userdata,
                             bool (*function)(void *item, void *userdata)) {

    if (!self || !function)
        return false;
    return dtn_list_for_each(self->config.data, userdata, function);
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_array_get(const dtn_item *self, uint64_t pos) {

    dtn_item *out = NULL;

    if (!self)
        goto error;

    out = dtn_list_get(self->config.data, pos);

    return out;
error:
    return NULL;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_array_set(dtn_item *self, uint64_t pos, dtn_item *val) {

    if (!self)
        goto error;
    dtn_item *out = NULL;

    bool result = dtn_list_set(self->config.data, pos, val, (void **)&out);

    if (out)
        out = dtn_item_free(out);

    val->parent = self;
    return result;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_array_push(dtn_item *self, dtn_item *val) {

    if (!self)
        goto error;

    bool result = dtn_list_push(self->config.data, val);

    val->parent = self;
    return result;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_array_stack_pop(dtn_item *self) {

    dtn_item *out = NULL;

    if (!self)
        goto error;

    out = dtn_list_pop(self->config.data);

    if (out)
        out->parent = NULL;

    return out;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_array_lifo(dtn_item *self) {

    return dtn_item_array_stack_pop(self);
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_array_queue_pop(dtn_item *self) {

    dtn_item *out = NULL;

    if (!self)
        goto error;

    out = dtn_list_remove(self->config.data, 1);
    if (out)
        out->parent = NULL;

    return out;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_array_fifo(dtn_item *self) {

    return dtn_item_array_queue_pop(self);
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #STRING FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_item *dtn_item_string(const char *string) {

    if (!string)
        return NULL;

    dtn_item *item = item_create((dtn_item_config){.literal = dtn_ITEM_STRING});

    if (!item)
        goto error;

    item->config.data = dtn_string_dup(string);

    return item;
error:
    dtn_item_free(item);
    return NULL;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_is_string(const dtn_item *self) {

    if (!self)
        goto error;

    if (self->config.literal == dtn_ITEM_STRING)
        return true;

error:
    return false;
}

/*---------------------------------------------------------------------------*/

const char *dtn_item_get_string(const dtn_item *self) {

    if (!self)
        goto error;
    if (!dtn_item_is_string(self))
        goto error;

    return (const char *)self->config.data;

error:
    return 0;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #NUMBER FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_item *dtn_item_number(double number) {

    dtn_item *item = item_create((dtn_item_config){.literal = dtn_ITEM_NUMBER});

    if (!item)
        goto error;

    item->config.number = number;

    return item;
error:
    dtn_item_free(item);
    return NULL;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_set_number(dtn_item *self, double number) {

    if (dtn_item_is_number(self)) {
        self->config.number = number;
        return true;
    }

    return false;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_is_number(const dtn_item *self) {

    if (!self)
        goto error;

    if (self->config.literal == dtn_ITEM_NUMBER)
        return true;

error:
    return false;
}

/*---------------------------------------------------------------------------*/

double dtn_item_get_number(const dtn_item *self) {

    if (!self)
        goto error;
    if (!dtn_item_is_number(self))
        goto error;

    return self->config.number;

error:
    return 0;
}

/*---------------------------------------------------------------------------*/

int64_t dtn_item_get_int(const dtn_item *self) {

    if (!self)
        goto error;
    if (!dtn_item_is_number(self))
        goto error;

    return (int64_t)self->config.number;

error:
    return 0;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #LITERAL FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_item *dtn_item_null() {

    dtn_item *item = item_create((dtn_item_config){.literal = dtn_ITEM_NULL});

    if (!item)
        goto error;

    return item;
error:
    dtn_item_free(item);
    return NULL;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_is_null(const dtn_item *self) {

    if (!self)
        goto error;

    if (self->config.literal == dtn_ITEM_NULL)
        return true;

error:
    return false;
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_true() {

    dtn_item *item = item_create((dtn_item_config){.literal = dtn_ITEM_TRUE});

    if (!item)
        goto error;

    return item;
error:
    dtn_item_free(item);
    return NULL;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_is_true(const dtn_item *self) {

    if (!self)
        goto error;

    if (self->config.literal == dtn_ITEM_TRUE)
        return true;

error:
    return false;
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_false() {

    dtn_item *item = item_create((dtn_item_config){.literal = dtn_ITEM_FALSE});

    if (!item)
        goto error;

    return item;
error:
    dtn_item_free(item);
    return NULL;
}

/*---------------------------------------------------------------------------*/

bool dtn_item_is_false(const dtn_item *self) {

    if (!self)
        goto error;

    if (self->config.literal == dtn_ITEM_FALSE)
        return true;

error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #POINTER FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

static dtn_item *object_get_with_length(dtn_item *object, const char *key,
                                        size_t length) {

    if (!object || !key || length < 1)
        return NULL;

    char buffer[length + 1];
    memset(buffer, 0, length + 1);

    if (!strncat(buffer, key, length))
        goto error;

    return dtn_item_object_get(object, buffer);
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

static bool pointer_parse_token(const char *start, size_t size, char **token,
                                size_t *length) {

    if (!start || size < 1 || !token || !length)
        return false;

    if (start[0] != '/')
        goto error;

    if (size == 1) {
        *token = (char *)start;
        return true;
    }

    *token = (char *)start + 1;

    size_t len = 0;

    while (len < (size - 1)) {

        switch ((*token + len)[0]) {

        case '/':
        case '\0':
            goto done;
            break;

        default:
            len++;
        }
    }

done:
    *length = len;
    return true;

error:
    *token = NULL;
    *length = 0;
    return false;
}

/*----------------------------------------------------------------------------*/

static bool pointer_replace_special_encoding(char *result, size_t size,
                                             char *source, char *to_replace,
                                             char *replacement) {

    if (!result || !source || !to_replace || !replacement || size < 1)
        return false;

    size_t length_replace = strlen(to_replace);
    size_t length_replacement = strlen(replacement);

    uint64_t i = 0;
    uint64_t k = 0;
    uint64_t r = 0;

    for (i = 0; i < size; i++) {

        if (source[i] != to_replace[0]) {
            result[r] = source[i];
            r++;
            continue;
        }

        if ((i + length_replace) > size)
            goto error;

        if ((i + length_replacement) > size)
            goto error;

        for (k = 0; k < length_replace; k++) {

            if (source[i + k] != to_replace[k])
                break;
        }

        if (k == (length_replace)) {

            if (!memcpy(result + r, replacement, length_replacement))
                goto error;

            i += length_replace - 1;
            r += length_replacement;

        } else {

            // not a full match
            result[r] = source[i];
            r++;
        }
    }

    return true;
error:
    if (result) {
        memset(result, '\0', size);
    }

    return false;
}

/*----------------------------------------------------------------------------*/

static bool pointer_escape_token(char *start, size_t size, char *token,
                                 size_t length) {

    if (!start || size < 1 || !token || length < size)
        return false;

    // replacements ~1 -> \  and ~0 -> ~ (both smaller then source)

    char replace1[size + 1];
    memset(replace1, '\0', size + 1);

    if (!pointer_replace_special_encoding(replace1, size, (char *)start, "~1",
                                          "\\"))
        return false;

    if (!pointer_replace_special_encoding((char *)token, size, replace1, "~0",
                                          "~"))
        return false;

    return true;
}

/*----------------------------------------------------------------------------*/

static dtn_item *pointer_get_token_in_parent(dtn_item *parent, char *token,
                                             size_t length) {

    if (!parent || !token)
        return NULL;

    if (length == 0)
        return parent;

    dtn_item *result = NULL;
    char *ptr = NULL;
    uint64_t number = 0;

    switch (parent->config.literal) {

    case dtn_ITEM_OBJECT:

        // token is the keyname
        result = object_get_with_length(parent, token, length);

        break;

    case dtn_ITEM_ARRAY:

        if (token[0] == '-') {

            // new array member after last array element
            if (strnlen((char *)token, length) == 1) {

                result = dtn_item_null();

                if (!dtn_item_array_push(parent, result))
                    result = dtn_item_free(result);
            }

        } else {

            // parse for INT64
            number = strtoll((char *)token, &ptr, 10);
            if (ptr[0] != '\0')
                return NULL;

            result = dtn_item_array_get(parent, number + 1);
        }

        break;

    default:
        return NULL;
    }

    return result;
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_item_get(const dtn_item *self, const char *pointer) {

    if (!self)
        return NULL;
    if (!pointer)
        return (dtn_item *)self;

    dtn_item *result = NULL;
    size_t size = strlen(pointer);

    if (0 == size)
        return (dtn_item *)self;

    bool parsed = false;

    char *parse = NULL;
    size_t length = 0;

    char token[size + 1];
    memset(token, 0, size + 1);
    char *token_ptr = token;

    const char *ptr = pointer;

    result = (dtn_item *)self;

    while (pointer_parse_token(ptr, size, &parse, &length)) {

        if (length == 0)
            break;

        parsed = true;
        ptr = ptr + length + 1;

        memset(token, 0, size + 1);
        token_ptr = token;

        if (!pointer_escape_token(parse, length, token_ptr, length))
            goto error;

        result = pointer_get_token_in_parent(result, token_ptr, length);

        if (!result)
            goto error;
    }

    if (!parsed)
        goto error;

    return result;

error:
    return NULL;
}