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
        @file           dtn_cbor.c
        @author         Töpfer, Markus

        @date           2025-12-12


        ------------------------------------------------------------------------
*/
#include "../include/dtn_cbor.h"

#include <dtn_base/dtn_string.h>
#include <dtn_base/dtn_dict.h>
#include <dtn_base/dtn_linked_list.h>
#include <dtn_base/dtn_utf8.h>
#include <dtn_base/dtn_dump.h>
#include <dtn_base/dtn_buffer.h>

static dtn_cbor_config g_config = {0};

/*----------------------------------------------------------------------------*/

struct dtn_cbor {

    dtn_cbor_type type;

    uint64_t tag;

    union {

        uint8_t *bytes;
        char *string;
        void *data;
    };

    union {

        uint64_t nbr_uint;
        int64_t  nbr_int;

        double nbr_double;
        float  nbr_float;

    };
};

/*----------------------------------------------------------------------------*/

static dtn_cbor *dtn_cbor_create(dtn_cbor_type type);

/*----------------------------------------------------------------------------*/

static void *cbor_free(void *source){

    if (!source) return false;

    dtn_cbor *self = (dtn_cbor*) source;

    return (void*) dtn_cbor_free(self);

}

/*----------------------------------------------------------------------------*/

static bool cbor_clear(void *source){

    if (!source) return false;

    dtn_cbor *self = (dtn_cbor*) source;

    switch (self->type) {

        case DTN_CBOR_MAP:
            self->data = dtn_dict_free(self->data);
            break;

        case DTN_CBOR_ARRAY:
            self->data = dtn_list_free(self->data);
            break;

        case DTN_CBOR_STRING:
            self->string = dtn_data_pointer_free(self->string);
            break;

        case DTN_CBOR_DEC_FRACTION:
        case DTN_CBOR_BIGFLOAT:
            self->data = cbor_free(self->data);
            break;

        case DTN_CBOR_UTF8:
        case DTN_CBOR_SIMPLE:
        case DTN_CBOR_DATE_TIME:
        case DTN_CBOR_UBIGNUM:
        case DTN_CBOR_IBIGNUM:
        
            self->bytes = dtn_data_pointer_free(self->bytes);
            break;
        case DTN_CBOR_TAG:
            
            if (self->data)
                self->data = cbor_free(self->data);
            break;
        default:
            break;

    }

    self->type = DTN_CBOR_UNDEF;
    self->bytes = NULL;
    self->nbr_uint = 0;
    return true;

}

/*----------------------------------------------------------------------------*/

static void *cbor_copy(void **destination, const void *source){

    if (!destination || !source) return false;

    dtn_cbor *copy = NULL;
    dtn_cbor *self = (dtn_cbor*) source;

    bool result = true;
    copy = dtn_cbor_create(self->type);

    switch (self->type) {

        case DTN_CBOR_MAP:
            copy->data = NULL;
            result = dtn_dict_copy((void**)&copy->data, self->data);
            break;

        case DTN_CBOR_ARRAY:
            copy->data = NULL;
            result = dtn_list_copy((void**)&copy->data, self->data);
            break;

        case DTN_CBOR_DEC_FRACTION:
        case DTN_CBOR_BIGFLOAT:
            copy->data = NULL;
            result = cbor_copy((void**)&copy->data, self->data);
            break;

        case DTN_CBOR_STRING:
        case DTN_CBOR_UTF8:
        case DTN_CBOR_DATE_TIME:
        case DTN_CBOR_UBIGNUM:
        case DTN_CBOR_IBIGNUM:
        
            copy->bytes = calloc(1, self->nbr_uint + 1);
            if (!self->bytes) goto error;
            memcpy(copy->bytes, self->bytes, self->nbr_uint);
            break;

        case DTN_CBOR_TAG:
            copy->data = cbor_copy((void**)&copy->data, self->data);
            break;
        default:
            break;

    }

    if (!result) goto error;

    copy->nbr_uint = self->nbr_uint;
    *destination = copy;
    return copy;
error:
    dtn_cbor_free(copy);
    return NULL;
}

/*----------------------------------------------------------------------------*/

static bool cbor_dump(FILE *stream, const void *source){

    if (!stream || !source) return false;

    dtn_cbor *self = (dtn_cbor*) source;

    switch (self->type) {

        case DTN_CBOR_MAP:
            fprintf(stream, "\nDTN_CBOR_MAP\n");
            return dtn_dict_dump(stream, self->data);
            break;

        case DTN_CBOR_ARRAY:
            fprintf(stream, "\nDTN_CBOR_ARRAY\n");
            return dtn_list_dump(stream, self->data);
            break;

        case DTN_CBOR_STRING:
            fprintf(stream, "\nDTN_CBOR_STRING\n");
            fprintf(stream, "%s\n", self->string);
            break;
        case DTN_CBOR_UTF8:
            fprintf(stream, "\nDTN_CBOR_UTF8\n");
            return dtn_dump_binary_as_hex(stream, self->bytes, self->nbr_uint);
            break;
        case DTN_CBOR_DATE_TIME:
            fprintf(stream, "\nDTN_CBOR_DATE_TIME\n");
            return dtn_dump_binary_as_hex(stream, self->bytes, self->nbr_uint);
            break;
        case DTN_CBOR_UBIGNUM:
            fprintf(stream, "\nDTN_CBOR_UBIGNUM\n");
            return dtn_dump_binary_as_hex(stream, self->bytes, self->nbr_uint);
            break;
        case DTN_CBOR_IBIGNUM:
            fprintf(stream, "\nDTN_CBOR_IBIGNUM\n");
            return dtn_dump_binary_as_hex(stream, self->bytes, self->nbr_uint);
            break;
        case DTN_CBOR_DEC_FRACTION:
            fprintf(stream, "\nDTN_CBOR_DEC_FRACTION\n");
            return cbor_dump(stream, self->data);
            break;
        case DTN_CBOR_DATE_TIME_EPOCH:
            fprintf(stream, "\nDTN_CBOR_DATE_TIME_EPOCH\n");
            fprintf(stream, "%"PRIu64, self->nbr_uint);
            break;
        case DTN_CBOR_BIGFLOAT:
            fprintf(stream, "\nDTN_CBOR_BIGFLOAT\n");
            return cbor_dump(stream, self->data);
            break;
        case DTN_CBOR_TAG:
            fprintf(stream, "\nDTN_CBOR_TAG\n");
            fprintf(stream, "%"PRIu64, self->nbr_uint);
            if (self->data)
                cbor_dump(stream, self->data);
            break;
        case DTN_CBOR_FALSE:
            fprintf(stream, "\nDTN_CBOR_FALSE\n");
            fprintf(stream, "false");
            break;
        case DTN_CBOR_TRUE:
            fprintf(stream, "\nDTN_CBOR_TRUE\n");
            fprintf(stream, "true");
            break;
        case DTN_CBOR_UNDEF:
            fprintf(stream, "\nDTN_CBOR_UNDEF\n");
            fprintf(stream, "undef");
            break;
        case DTN_CBOR_NULL:
            fprintf(stream, "\nDTN_CBOR_NULL\n");
            fprintf(stream, "null");
            break;
        case DTN_CBOR_UINT64:
            fprintf(stream, "\nDTN_CBOR_UINT64\n");
            fprintf(stream, "%"PRIu64, self->nbr_uint);
            break;
        case DTN_CBOR_INT64:
            fprintf(stream, "\nDTN_CBOR_INT64\n");
            fprintf(stream, "%"PRIi64, self->nbr_int);
            break;
        case DTN_CBOR_FLOAT:
            fprintf(stream, "\nDTN_CBOR_FLOAT\n");
            fprintf(stream, "%f", self->nbr_float);
            break;
        case DTN_CBOR_DOUBLE:
            fprintf(stream, "\nDTN_CBOR_DOUBLE\n");
            fprintf(stream, "%f", self->nbr_double);
            break;
        case DTN_CBOR_SIMPLE:
            fprintf(stream, "\nDTN_CBOR_SIMPLE\n");
            fprintf(stream, "%"PRIu64, self->nbr_uint);
            break;
    }

    return true;
}

/*----------------------------------------------------------------------------*/

static uint64_t cbor_hash(const void *key){

    if (!key) return 0;

    dtn_cbor *self = (dtn_cbor*) key;

    uint64_t result = 0;
    dtn_buffer *buffer = NULL;

    switch (self->type){

        case DTN_CBOR_UNDEF:
        case DTN_CBOR_FALSE:
        case DTN_CBOR_TRUE:
        case DTN_CBOR_NULL:
            return 0;
            break;
        case DTN_CBOR_UINT64:
            return self->nbr_uint;
            break;
        case DTN_CBOR_INT64:
            return (uint64_t) self->nbr_int;
            break;
        case DTN_CBOR_STRING:
            return dtn_hash_simple_c_string(self->string);
            break;
        case DTN_CBOR_UTF8:
            buffer = dtn_buffer_create(self->nbr_uint);
            dtn_buffer_set(buffer, self->bytes, self->nbr_uint);
            result = dtn_hash_simple_c_string(buffer->start);
            buffer = dtn_buffer_free(buffer);
            return result;
            break;
        case DTN_CBOR_ARRAY:
            return (uint64_t)(uintptr_t)self->data;
            break;
        case DTN_CBOR_MAP:
            return (uint64_t)(uintptr_t)self->data;
            break;
        case DTN_CBOR_DATE_TIME:
            return dtn_hash_simple_c_string(self->string);
            break;
        case DTN_CBOR_DATE_TIME_EPOCH:
            return self->nbr_uint;
            break;
        case DTN_CBOR_UBIGNUM:
            return (uint64_t)(uintptr_t)self->data;
            break;
        case DTN_CBOR_IBIGNUM:
            return (uint64_t)(uintptr_t)self->data;
            break;
        case DTN_CBOR_DEC_FRACTION:
            return (uint64_t)(uintptr_t)self->data;
            break;
        case DTN_CBOR_BIGFLOAT:
            return (uint64_t)(uintptr_t)self->data;
            break;
        case DTN_CBOR_TAG:
            return self->nbr_uint;
            break;
        case DTN_CBOR_SIMPLE:
            return self->nbr_uint;
            break;
        case DTN_CBOR_FLOAT:
            return (uint64_t) self->nbr_float;
            break;
        case DTN_CBOR_DOUBLE:
            return (uint64_t) self->nbr_double;
            break;
    
    }

    return 0;
}

/*----------------------------------------------------------------------------*/

static bool cbor_match(const void *key, const void *value);

/*----------------------------------------------------------------------------*/

static bool key_contained(const void *key, void *val, void *data){

    if (!key) return true;
    dtn_cbor *val1 = (dtn_cbor*) val;
    dtn_dict *two = dtn_dict_cast(data);

    dtn_cbor *val2 = dtn_dict_get(two, key);
    if (!val2) return false;

    if (cbor_match(val1, val2)) return true;

    return false;
}

/*----------------------------------------------------------------------------*/

static bool check_maps_similar(dtn_cbor *one, dtn_cbor *two){

    if (!one || !two) return false;

    if (dtn_dict_for_each(one->data, two->data, key_contained))
        if (dtn_dict_for_each(two->data, one->data, key_contained))
            return true;

    return false;
}

/*----------------------------------------------------------------------------*/

static bool cbor_match(const void *key, const void *value){

    if (!key || !value) goto error;

    dtn_cbor *one = (dtn_cbor*) key;
    dtn_cbor *two = (dtn_cbor*) value;

    size_t count1 = 0;
    size_t count2 = 0;

    if (one->type != two->type) goto error;

    switch(one->type){
        case DTN_CBOR_UNDEF:
        case DTN_CBOR_FALSE:
        case DTN_CBOR_TRUE:
        case DTN_CBOR_NULL:
            return true;
            break;
        case DTN_CBOR_UINT64:
            if (one->nbr_uint == two->nbr_uint)
                return true;
            break;
        case DTN_CBOR_INT64:
            if (one->nbr_int == two->nbr_int)
                return true;
            break;
        case DTN_CBOR_STRING:
            if (0 == dtn_string_compare(one->string, two->string))
                return true;
            break;
        case DTN_CBOR_UTF8:
            if (one->nbr_uint == two->nbr_uint){
                if (0 == memcmp(one->bytes, two->bytes, one->nbr_uint))
                    return true;
            }
            break;
        case DTN_CBOR_ARRAY:
            count1 = dtn_list_count(one->data);
            count2 = dtn_list_count(two->data);
            if (count1 == count2){
                for (size_t i = 1; i <= count1; i++){
                    dtn_cbor *a = dtn_list_get(one->data, i);
                    dtn_cbor *b = dtn_list_get(two->data, i);
                    if (cbor_match(a, b)) continue;
                    return false;
                }
                return true;
            }
            break;
        case DTN_CBOR_MAP:
            count1 = dtn_dict_count(one->data);
            count2 = dtn_dict_count(two->data);
            if (count1 == count2){
                if (check_maps_similar(one, two))
                    return true;
            }
            break;
        case DTN_CBOR_DATE_TIME:
            if (one->nbr_uint == two->nbr_uint){
                if (0 == memcmp(one->bytes, two->bytes, one->nbr_uint))
                    return true;
            }
            break;
        case DTN_CBOR_DATE_TIME_EPOCH:
            if (one->nbr_uint == two->nbr_uint)
                return true;
            break;
        case DTN_CBOR_UBIGNUM:
            if (one->nbr_uint == two->nbr_uint){
                if (0 == memcmp(one->bytes, two->bytes, one->nbr_uint))
                    return true;
            }
            break;
        case DTN_CBOR_IBIGNUM:
            if (one->nbr_uint == two->nbr_uint){
                if (0 == memcmp(one->bytes, two->bytes, one->nbr_uint))
                    return true;
            }
            break;
        case DTN_CBOR_DEC_FRACTION:
            count1 = dtn_cbor_array_count(one->data);
            count2 = dtn_cbor_array_count(two->data);
            if (count1 == count2){
                for (size_t i = 1; i <= count1; i++){
                    const dtn_cbor *a = dtn_cbor_array_get(one->data, i);
                    const dtn_cbor *b = dtn_cbor_array_get(two->data, i);
                    if (cbor_match(a, b)) continue;
                    return false;
                }
                return true;
            }
            break;
            break;
        case DTN_CBOR_BIGFLOAT:
            count1 = dtn_cbor_array_count(one->data);
            count2 = dtn_cbor_array_count(two->data);
            if (count1 == count2){
                for (size_t i = 1; i <= count1; i++){
                    const dtn_cbor *a = dtn_cbor_array_get(one->data, i);
                    const dtn_cbor *b = dtn_cbor_array_get(two->data, i);
                    if (cbor_match(a, b)) continue;
                    return false;
                }
                return true;
            }
            break;
        case DTN_CBOR_TAG:
            if (one->nbr_uint == two->nbr_uint){
                
                if (one->data){

                    if (!two->data)
                        goto error;

                    if (cbor_match(one->data, two->data))
                        return true;

                } else {

                    return true;
                }
            
            }
            break;
        case DTN_CBOR_SIMPLE:
            if (one->nbr_uint == two->nbr_uint){

                if (one->bytes){
                    if (0 == memcmp(one->bytes, two->bytes, one->nbr_uint))
                        return true;
                } else {
                    return true;
                }
            }
            break;
        case DTN_CBOR_FLOAT:
            if (one->nbr_float == two->nbr_float)
                return true;
            break;
        case DTN_CBOR_DOUBLE:
            if (one->nbr_double == two->nbr_double)
                return true;
            break;
    }

error:
    return false;
}

/*----------------------------------------------------------------------------*/

static dtn_dict_config dtn_cbor_dict_config(uint64_t slots){

    return (dtn_dict_config) {
        .slots = slots,
        
        .key.data_function = (dtn_data_function){
                .clear = cbor_clear,
                .copy = cbor_copy,
                .free = cbor_free,
                .dump = cbor_dump
        },
        .key.hash = cbor_hash,
        .key.match = cbor_match,

        .value.data_function = (dtn_data_function){
                .clear = cbor_clear,
                .copy = cbor_copy,
                .free = cbor_free,
                .dump = cbor_dump
            }
    };
}

/*----------------------------------------------------------------------------*/

static dtn_cbor *dtn_cbor_create(dtn_cbor_type type){

    dtn_cbor *self = calloc(1, sizeof(dtn_cbor));
    if (!self) goto error;

    self->type = type;

    // we add a very big default config limitation here

    if (0 == g_config.limits.string_size)
        g_config.limits.string_size = UINT32_MAX;

    if (0 == g_config.limits.utf8_string_size)
        g_config.limits.utf8_string_size = UINT32_MAX;

    if (0 == g_config.limits.array_size)
        g_config.limits.array_size = UINT32_MAX;

    if (0 == g_config.limits.undef_length_array)
        g_config.limits.undef_length_array = UINT32_MAX;

    if (0 == g_config.limits.map_size)
        g_config.limits.map_size = UINT32_MAX;

    if (0 == g_config.limits.undef_length_map)
        g_config.limits.undef_length_map = UINT32_MAX;

    return self;
error:
    return NULL;

}

/*----------------------------------------------------------------------------*/

dtn_cbor_type dtn_cbor_get_type(const dtn_cbor *self){

    return self->type;
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_free(dtn_cbor *self){

    if (!self) return self;

    if (!cbor_clear(self)) return self;

    self = dtn_data_pointer_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

static dtn_cbor_match decode_uint(
    const uint8_t *buffer, 
    size_t size,
    dtn_cbor **out, 
    uint8_t **next){

    dtn_cbor *self = NULL;
    uint64_t local = 0;

    /* NOTE We are using a local variable of 64 bit to support all bitwise
     * operation on all platform, which may redefine int to some smaller size. */

    if (size == 0) return DTN_CBOR_MATCH_PARTIAL;

    size_t len = 0;

    switch (buffer[0]){

        case 0x00:
        case 0x01:
        case 0x02:
        case 0x03:
        case 0x04:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x08:
        case 0x09:
        case 0x0A:
        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x0E:
        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
        case 0x14:
        case 0x15:
        case 0x16:
        case 0x17:
            
            self = dtn_cbor_create(DTN_CBOR_UINT64);
            if (!self) goto error;
            
            self->nbr_uint = buffer[0];
            len = 1;
            goto done;
            break;

        case 0x18:

            if (size < 2)
                goto partial;

            self = dtn_cbor_create(DTN_CBOR_UINT64);
            if (!self) goto error;
            
            self->nbr_uint = buffer[1];
            len = 2;
            goto done;
            break;

        case 0x19:

            if (size < 3)
                goto partial;

            self = dtn_cbor_create(DTN_CBOR_UINT64);
            if (!self) goto error;
            
            local = buffer[1];
            self->nbr_uint = local << 8;
            local = buffer[2];
            self->nbr_uint += local;
            len = 3;
            goto done;
            break;

        case 0x1A:
            
            if (size < 5)
                goto partial;

            self = dtn_cbor_create(DTN_CBOR_UINT64);
            if (!self) goto error;
            
            local = buffer[1];
            self->nbr_uint = local << 24;
            local = buffer[2];
            self->nbr_uint += local << 16;
            local = buffer[3];
            self->nbr_uint += local << 8;
            local = buffer[4];
            self->nbr_uint += local;
            len = 5;
            goto done;
            break;

        case 0x1B:

            if (size < 9)
                goto partial;

            self = dtn_cbor_create(DTN_CBOR_UINT64);
            if (!self) goto error;

            local = buffer[1];
            self->nbr_uint = local << 56;
            local = buffer[2];
            self->nbr_uint += local << 48;
            local = buffer[3];
            self->nbr_uint += local << 40;
            local = buffer[4];
            self->nbr_uint += local << 32;
            local = buffer[5];
            self->nbr_uint += local << 24;
            local = buffer[6];
            self->nbr_uint += local << 16;
            local = buffer[7];
            self->nbr_uint += local << 8;
            local = buffer[8];
            self->nbr_uint += local;
            len = 9;
            goto done;
            break;
            
        default:
            goto error;
    }

done:
    *out = self;
    *next = (uint8_t*) buffer + len;
    return DTN_CBOR_MATCH_FULL;
partial:
    self = dtn_cbor_free(self);
    return DTN_CBOR_MATCH_PARTIAL;
error:
    self = dtn_cbor_free(self);
    return DTN_CBOR_NO_MATCH;
}

/*----------------------------------------------------------------------------*/

static dtn_cbor_match decode_int(
    const uint8_t *buffer, 
    size_t size,
    dtn_cbor **out, 
    uint8_t **next){

    dtn_cbor *self = NULL;
    size_t len = 0;
    uint64_t local = 0;

    if (size == 0) return DTN_CBOR_MATCH_PARTIAL;

    switch (buffer[0]){

        case 0x20:
        case 0x21:
        case 0x22:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
        case 0x29:
        case 0x2A:
        case 0x2B:
        case 0x2C:
        case 0x2D:
        case 0x2E:
        case 0x2F:
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
            
            self = dtn_cbor_create(DTN_CBOR_INT64);
            if (!self) goto error;
            
            self->nbr_int = buffer[0];
            self->nbr_int *= -1;
            len = 1;
            goto done;
            break;

        case 0x38:

            if (size < 2)
                goto partial;

            self = dtn_cbor_create(DTN_CBOR_INT64);
            if (!self) goto error;
            
            self->nbr_int = buffer[1];
            self->nbr_int *= -1;
            len = 2;
            goto done;
            break;

        case 0x39:

            if (size < 3)
                goto partial;

            self = dtn_cbor_create(DTN_CBOR_INT64);
            if (!self) goto error;
            
            local = buffer[1];
            self->nbr_int = local << 8;
            local = buffer[2];
            self->nbr_int += local;
            self->nbr_int *= -1;
            len = 3;
            goto done;
            break;

        case 0x3A:
            
            if (size < 5)
                goto partial;

            self = dtn_cbor_create(DTN_CBOR_INT64);
            if (!self) goto error;
            
            local = buffer[1];
            self->nbr_int =  local << 24;
            local = buffer[2];
            self->nbr_int += local << 16;
            local = buffer[3];
            self->nbr_int += local << 8;
            local = buffer[4];
            self->nbr_int += local;
            self->nbr_int *= -1;
            len = 5;
            goto done;
            break;

        case 0x3B:

            if (size < 9)
                goto partial;

            self = dtn_cbor_create(DTN_CBOR_INT64);
            if (!self) goto error;
            
            local = buffer[1];
            self->nbr_int =  local << 56;
            local = buffer[2];
            self->nbr_int += local << 48;
            local = buffer[3];
            self->nbr_int += local << 40;
            local = buffer[4];
            self->nbr_int += local << 32;
            local = buffer[5];
            self->nbr_int += local << 24;
            local = buffer[6];
            self->nbr_int += local << 16;
            local = buffer[7];
            self->nbr_int += local << 8;
            local = buffer[8];
            self->nbr_int += local;
            self->nbr_int *= -1;
            len = 9;
            goto done;
            break;
            
        default:
            goto error;
    }

done:
    *out = self;
    *next = (uint8_t*)buffer + len;
    return DTN_CBOR_MATCH_FULL;
partial:
    self = dtn_cbor_free(self);
    return DTN_CBOR_MATCH_PARTIAL;
error:
    self = dtn_cbor_free(self);
    return DTN_CBOR_NO_MATCH;
}

/*----------------------------------------------------------------------------*/

static dtn_cbor_match decode_text_string(
    const uint8_t *buffer, 
    size_t size,
    dtn_cbor **out, 
    uint8_t **next){

    if (size == 0) return DTN_CBOR_MATCH_PARTIAL;

    dtn_cbor *self = NULL;

    uint8_t *ptr = NULL;
    size_t len = 0;
    uint64_t str_len = 0;
    uint64_t local = 0;

    switch (buffer[0]){

        case 0x40:
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x47:
        case 0x48:
        case 0x49:
        case 0x4A:
        case 0x4B:
        case 0x4C:
        case 0x4D:
        case 0x4E:
        case 0x4F:
        case 0x50:
        case 0x51:
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
        case 0x56:
        case 0x57:

            str_len = buffer[0] & 0x1F;

            if (size < str_len + 1) goto partial;
            
            self = dtn_cbor_create(DTN_CBOR_STRING);
            if (!self) goto error;

            if (0 != str_len){

                if (str_len > g_config.limits.string_size)
                    goto error;

                self->string = calloc(str_len + 1, sizeof(char));
                if (!self->string) goto error;

                if (!dtn_string_copy(self->string, (char*)buffer + 1, str_len + 1))
                    goto error;

            }

            self->nbr_uint = str_len;
            len = str_len + 1;
            goto done;
            break;

        case 0x58:

            if (size < 2)
                goto partial;

            self = dtn_cbor_create(DTN_CBOR_STRING);
            if (!self) goto error;

            str_len = buffer[1];

            if (size < str_len + 1) goto partial;

            if (0 != str_len){

                if (str_len > g_config.limits.string_size)
                    goto error;

                self->string = calloc(str_len + 1, sizeof(char));
                if (!self->string) goto error;

                if (!dtn_string_copy(self->string, (char*)buffer + 1, str_len + 1))
                    goto error;

            }

            self->nbr_uint = str_len;
            len = str_len + 2;
            goto done;
            break;

        case 0x59:

            if (size < 3)
                goto partial;

            str_len = buffer[1] << 8;
            str_len += buffer[2];

            if (size < str_len + 1) goto partial;

            self = dtn_cbor_create(DTN_CBOR_STRING);
            if (!self) goto error;

            if (0 != str_len){

                if (str_len > g_config.limits.string_size)
                    goto error;

                self->string = calloc(str_len + 1, sizeof(char));
                if (!self->string) goto error;

                if (!dtn_string_copy(self->string, (char*)buffer + 3, str_len + 1))
                    goto error;

            }

            self->nbr_uint = str_len;
            len = str_len + 3;
            goto done;
            break;

        case 0x5A:
            
            if (size < 5)
                goto partial;
            
            local = buffer[1];
            str_len =  local << 24;
            local = buffer[2];
            str_len += local << 16;
            local = buffer[3];
            str_len += local << 8;
            local = buffer[4];
            str_len += local;

            if (size < str_len + 1) goto partial;

            self = dtn_cbor_create(DTN_CBOR_STRING);
            if (!self) goto error;

            if (0 != str_len){

                if (str_len > g_config.limits.string_size)
                    goto error;

                self->string = calloc(str_len + 1, sizeof(char));
                if (!self->string) goto error;

                if (!dtn_string_copy(self->string, (char*)buffer + 5, str_len + 1))
                    goto error;

            }

            self->nbr_uint = str_len;
            len = str_len + 5;
            goto done;
            break;

        case 0x5B:

            if (size < 9)
                goto partial;
            
            local = buffer[1];
            str_len =  local << 56;
            local = buffer[2];
            str_len += local << 48;
            local = buffer[3];
            str_len += local << 40;
            local = buffer[4];
            str_len += local << 32;
            local = buffer[5];
            str_len += local << 24;
            local = buffer[6];
            str_len += local << 16;
            local = buffer[7];
            str_len += local << 8;
            local = buffer[8];
            str_len += local;

            if (str_len == UINT64_MAX) goto error;

            if (size < str_len + 1) goto partial;

            self = dtn_cbor_create(DTN_CBOR_STRING);
            if (!self) goto error;

            if (0 != str_len){

                if (str_len > g_config.limits.string_size)
                    goto error;

                self->string = calloc(str_len + 1, sizeof(char));
                if (!self->string) goto error;

                if (!dtn_string_copy(self->string, (char*)buffer + 9, str_len + 1))
                    goto error;

            }

            self->nbr_uint = str_len;
            len = 9 + str_len;
            goto done;
            break;
            
        case 0x5F:

            ptr = (uint8_t*) buffer;

            while(ptr[0] != 0xFF){

                size--;
                if (size == 0) goto partial;

                ptr++;
            }

            str_len = ptr - (uint8_t*) buffer - 1;

            self = dtn_cbor_create(DTN_CBOR_STRING);
            if (!self) goto error;

            if (0 != str_len){

                if (str_len > g_config.limits.string_size)
                    goto error;

                self->string = calloc(str_len + 1, sizeof(char));
                if (!self->string) goto error;

                if (!dtn_string_copy(self->string, (char*)buffer + 1, str_len + 1))
                    goto error;

            }

            self->nbr_uint = str_len;
            len = str_len + 2;
            break;    
        default:
            goto error;
    }

done:
    *out = self;
    *next = (uint8_t*)buffer + len;
    return DTN_CBOR_MATCH_FULL;
partial:
    self = dtn_cbor_free(self);
    return DTN_CBOR_MATCH_PARTIAL;
error:
    self = dtn_cbor_free(self);
    return DTN_CBOR_NO_MATCH;
}

/*----------------------------------------------------------------------------*/

static dtn_cbor_match decode_utf8_string(
    const uint8_t *buffer, 
    size_t size,
    dtn_cbor **out, 
    uint8_t **next){

    if (size == 0) return DTN_CBOR_MATCH_PARTIAL;

    dtn_cbor *self = NULL;

    uint8_t *ptr = NULL;
    size_t len = 0;
    uint64_t str_len = 0;
    uint64_t local = 0;

    switch (buffer[0]){

        case 0x60:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
        case 0x67:
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6B:
        case 0x6C:
        case 0x6D:
        case 0x6E:
        case 0x6F:
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:

            str_len = buffer[0] & 0x1F;

            if (size < str_len + 1) goto partial;

            if (str_len > g_config.limits.utf8_string_size)
                goto error;
            
            self = dtn_cbor_create(DTN_CBOR_UTF8);
            if (!self) goto error;

            if (0 != str_len){

                self->bytes = calloc(str_len + 1, sizeof(uint8_t));

                if (!memcpy(self->bytes, buffer + 1, str_len))
                    goto error;

            }

            self->nbr_uint = str_len;
            
            len = str_len + 1;
            goto done;
            break;

        case 0x78:

            if (size < 2)
                goto partial;

            str_len = buffer[1];

            if (size < str_len + 1) goto partial;

            if (str_len > g_config.limits.utf8_string_size)
                goto error;

            self = dtn_cbor_create(DTN_CBOR_UTF8);
            if (!self) goto error;

            if (0 != str_len){

                self->bytes = calloc(str_len + 1, sizeof(uint8_t));

                if (!memcpy(self->bytes, buffer + 2, str_len))
                    goto error;
            }

            self->nbr_uint = str_len;

            len = str_len + 2;
            goto done;
            break;

        case 0x79:

            if (size < 3)
                goto partial;

            local = buffer[1];
            str_len = local << 8;
            local = buffer[2];
            str_len += local;

            if (size < str_len + 1) goto partial;

            if (str_len > g_config.limits.utf8_string_size)
                goto error;

            self = dtn_cbor_create(DTN_CBOR_UTF8);
            if (!self) goto error;

            if (0 != str_len){
                
                self->bytes = calloc(str_len + 1, sizeof(uint8_t));

                if (!memcpy(self->bytes, buffer + 3, str_len))
                    goto error;

            }

            self->nbr_uint = str_len;
            len = str_len + 3;
            goto done;
            break;

        case 0x7A:
            
            if (size < 5)
                goto partial;
            
            local = buffer[1];
            str_len =  local << 24;
            local = buffer[2];
            str_len += local << 16;
            local = buffer[3];
            str_len += local << 8;
            local = buffer[4];
            str_len += local;

            if (size < str_len + 1) goto partial;

            if (str_len > g_config.limits.utf8_string_size)
                goto error;

            self = dtn_cbor_create(DTN_CBOR_UTF8);
            if (!self) goto error;

            if (0 != str_len){

                self->bytes = calloc(str_len + 1, sizeof(uint8_t));

                if (!memcpy(self->bytes, buffer + 5, str_len))
                    goto error;
            }

            self->nbr_uint = str_len;

            len = str_len + 5;
            goto done;
            break;

        case 0x7B:

            if (size < 9)
                goto partial;
            
            local = buffer[1];
            str_len =  local << 56;
            local = buffer[2];
            str_len += local << 48;
            local = buffer[3];
            str_len += local << 40;
            local = buffer[4];
            str_len += local << 32;
            local = buffer[5];
            str_len += local << 24;
            local = buffer[6];
            str_len += local << 16;
            local = buffer[7];
            str_len += local << 8;
            local = buffer[8];
            str_len += local;

            if (size < str_len + 1) goto partial;

            if (str_len > g_config.limits.utf8_string_size)
                goto error;

            self = dtn_cbor_create(DTN_CBOR_UTF8);
            if (!self) goto error;

            if (0 != str_len){
                
                self->bytes = calloc(str_len + 1, sizeof(uint8_t));

                if (!memcpy(self->bytes, buffer + 9, str_len))
                    goto error;

            }

            self->nbr_uint = str_len;

            len = 9 + str_len;
            goto done;
            break;
            
        case 0x7F:

            ptr = (uint8_t*) buffer;

            while(ptr[0] != 0xFF){

                size--;
                if (size == 0) goto partial;

                ptr++;
            }

            str_len = ptr - (uint8_t*) buffer - 1;

            if (str_len > g_config.limits.utf8_string_size)
                goto error;

            self = dtn_cbor_create(DTN_CBOR_UTF8);
            if (!self) goto error;

            self->bytes = calloc(str_len + 1, sizeof(uint8_t));

            if (!memcpy(self->bytes, (char*)buffer + 1, str_len))
                goto error;

            len = str_len + 2;

            self->nbr_uint = str_len;
        
            break;    
        default:
            goto error;
    }

done:
    
    if (self->nbr_uint > 0)
        if (!dtn_utf8_validate_sequence(self->bytes, self->nbr_uint)) goto error;

    *out = self;
    *next = (uint8_t*) buffer + len;
    return DTN_CBOR_MATCH_FULL;
partial:
    self = dtn_cbor_free(self);
    return DTN_CBOR_MATCH_PARTIAL;
error:
    self = dtn_cbor_free(self);
    return DTN_CBOR_NO_MATCH;
}

/*----------------------------------------------------------------------------*/

static dtn_cbor_match decode_array(
    const uint8_t *buffer, 
    size_t size,
    dtn_cbor **out, 
    uint8_t **next){

    dtn_cbor *self = NULL;

    if (size == 0) return DTN_CBOR_MATCH_PARTIAL;

    uint8_t *ptr = NULL;
    size_t len = 0;
    uint64_t arr_items = 0;
    uint64_t local = 0;

    switch (buffer[0]){

        case 0x80:
        case 0x81:
        case 0x82:
        case 0x83:
        case 0x84:
        case 0x85:
        case 0x86:
        case 0x87:
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
        case 0x8C:
        case 0x8D:
        case 0x8E:
        case 0x8F:
        case 0x90:
        case 0x91:
        case 0x92:
        case 0x93:
        case 0x94:
        case 0x95:
        case 0x96:
        case 0x97:

            if (size < 1)
                goto partial;

            arr_items = buffer[0] & 0x1F;

            if (arr_items > g_config.limits.array_size)
                goto error;

            if (size < arr_items + 1) goto partial;

            len = 1;
            break;

        case 0x98:

            if (size < 2)
                goto partial;

            arr_items = buffer[1];

            if (arr_items > g_config.limits.array_size)
                goto error;

            if (size < arr_items + 2) goto partial;

            len = 2;
            break;

        case 0x99:

            if (size < 3)
                goto partial;

            local = buffer[1];
            arr_items = local << 8;
            local = buffer[2];
            arr_items += local;

            if (arr_items > g_config.limits.array_size)
                goto error;

            if (size < arr_items + 3) goto partial;

            len = 3;
            break;

        case 0x9A:
            
            if (size < 5)
                goto partial;
            
            local = buffer[1];
            arr_items =  local << 24;
            local = buffer[2];
            arr_items += local << 16;
            local = buffer[3];
            arr_items += local << 8;
            local = buffer[4];
            arr_items += local;

            if (arr_items > g_config.limits.array_size)
                goto error;

            if (size < arr_items + 5) goto partial;

            len = 5;
            break;

        case 0x9B:

            if (size < 9)
                goto partial;
            
            local = buffer[1];
            arr_items =  local << 56;
            local = buffer[2];
            arr_items += local << 48;
            local = buffer[3];
            arr_items += local << 40;
            local = buffer[4];
            arr_items += local << 32;
            local = buffer[5];
            arr_items += local << 24;
            local = buffer[6];
            arr_items += local << 16;
            local = buffer[7];
            arr_items += local << 8;
            local = buffer[8];
            arr_items += local;

            if (arr_items > g_config.limits.array_size)
                goto error;

            if (size < arr_items + 9) goto partial;

            len = 9;
            break;
            
        case 0x9F:

            self = dtn_cbor_create(DTN_CBOR_ARRAY);
            if (!self) goto error;

            ptr = (uint8_t*) buffer;

            self->data = dtn_linked_list_create((dtn_list_config){
                .item.free = cbor_free
            });

            if (!self->data) goto error;

            while(ptr[0] != 0xFF){

                size--;
                if (size == 0) goto partial;

                ptr++;
            }

            uint8_t* last = ptr;
            ptr = (uint8_t*) buffer + 1;

            if ( (uint64_t) (last-ptr) > g_config.limits.undef_length_array)
                goto error;

            while(ptr < last){

                dtn_cbor *item = NULL;

                dtn_cbor_match match = dtn_cbor_decode(
                    ptr, last - ptr, &item, &ptr);
    
                switch (match){

                    case DTN_CBOR_MATCH_FULL:
                
                        if (!item) goto error;

                        if (!dtn_list_push(self->data, item)){
                            item = dtn_cbor_free(item);
                            goto error;
                        }
                
                        break;

                default:
                    goto error;
                }
            }
            
            len = last - (uint8_t*) buffer +1;
            goto done;
            break;    
        default:
            goto error;
    }

    self = dtn_cbor_create(DTN_CBOR_ARRAY);
    if (!self) goto error;

    self->data = dtn_linked_list_create((dtn_list_config){
            .item.free = cbor_free
    });

    if (!self->data) goto error;

    ptr = (uint8_t*) buffer + 1;

    for (uint64_t i = 0; i < arr_items; i++){

        dtn_cbor *item = NULL;

        dtn_cbor_match match = dtn_cbor_decode(
            ptr, size - (ptr - buffer) +1, &item, &ptr);
    
        switch (match){

            case DTN_CBOR_MATCH_FULL:
                
                if (!item) goto error;

                if (!dtn_list_push(self->data, item)){
                    item = dtn_cbor_free(item);
                    goto error;
                }
                
                break;

            default:
                return match;
        }
    }
    
    len += ptr - (uint8_t*) buffer - 1;

done:
    
    *out = self;
    *next = (uint8_t*) buffer + len;
    return DTN_CBOR_MATCH_FULL;
partial:
    self = dtn_cbor_free(self);
    return DTN_CBOR_MATCH_PARTIAL;
error:
    self = dtn_cbor_free(self);
    return DTN_CBOR_NO_MATCH;
}

/*----------------------------------------------------------------------------*/

static dtn_cbor_match decode_map(
    const uint8_t *buffer, 
    size_t size,
    dtn_cbor **out, 
    uint8_t **next){

    dtn_cbor *self = NULL;

    if (size == 0) return DTN_CBOR_MATCH_PARTIAL;

    uint8_t *ptr = NULL;
    size_t len = 0;
    uint64_t map_items = 0;
    uint64_t local = 0;

    switch (buffer[0]){

        case 0xA0:
        case 0xA1:
        case 0xA2:
        case 0xA3:
        case 0xA4:
        case 0xA5:
        case 0xA6:
        case 0xA7:
        case 0xA8:
        case 0xA9:
        case 0xAA:
        case 0xAB:
        case 0xAC:
        case 0xAD:
        case 0xAE:
        case 0xAF:
        case 0xB0:
        case 0xB1:
        case 0xB2:
        case 0xB3:
        case 0xB4:
        case 0xB5:
        case 0xB6:
        case 0xB7:

            map_items = buffer[0] & 0x1F;

            if (map_items > g_config.limits.map_size)
                goto error;

            len = 1;
            ptr = (uint8_t*) buffer + 1;
            break;

        case 0xB8:

            if (size < 2)
                goto partial;

            map_items = buffer[1];

            if (map_items > g_config.limits.map_size)
                goto error;

            len = 2;
            ptr = (uint8_t*) buffer + 2;
            break;

        case 0xB9:

            if (size < 3)
                goto partial;

            local = buffer[1];
            map_items = local << 8;
            local = buffer[2];
            map_items += local;

            len = 3;
            ptr = (uint8_t*) buffer + 3;
            break;

        case 0xBA:
            
            if (size < 5)
                goto partial;
            
            local = buffer[1];
            map_items =  local << 24;
            local = buffer[2];
            map_items += local << 16;
            local = buffer[3];
            map_items += local << 8;
            local = buffer[4];
            map_items += local;

            if (map_items > g_config.limits.map_size)
                goto error;

            len = 5;
            ptr = (uint8_t*) buffer + 5;
            break;

        case 0xBB:

            if (size < 9)
                goto partial;
            
            local = buffer[1];
            map_items =  local << 56;
            local = buffer[2];
            map_items += local << 48;
            local = buffer[3];
            map_items += local << 40;
            local = buffer[4];
            map_items += local << 32;
            local = buffer[5];
            map_items += local << 24;
            local = buffer[6];
            map_items += local << 16;
            local = buffer[7];
            map_items += local << 8;
            local = buffer[8];
            map_items += local;

            if (map_items > g_config.limits.map_size)
                goto error;

            len = 9;
            ptr = (uint8_t*) buffer + 9;
            break;
            
        case 0xBF:

            self = dtn_cbor_create(DTN_CBOR_MAP);
            if (!self) goto error;

            ptr = (uint8_t*) buffer;

            self->data = dtn_dict_create(dtn_cbor_dict_config(255));
            if (!self->data) goto error;

            while(ptr[0] != 0xFF){

                size--;
                if (size == 0) goto partial;

                ptr++;
            }

            uint8_t* last = ptr;
            ptr = (uint8_t*) buffer + 1;

            if ((uint64_t)(last-ptr) > g_config.limits.undef_length_map)
                goto error;

            while(ptr < last){

                dtn_cbor *key = NULL;
                dtn_cbor *val = NULL;

                dtn_cbor_match match = dtn_cbor_decode(
                    ptr, last - ptr, &key, &ptr);
    
                switch (match){

                    case DTN_CBOR_MATCH_FULL:
                
                        if (!key) goto error;
                        break;

                    default:
                        key = dtn_cbor_free(key);
                        goto error;
                }

                if (ptr >= last) {
                    key = dtn_cbor_free(key);
                    goto error;
                }

                match = dtn_cbor_decode(
                    ptr, last - ptr, &val, &ptr);
    
                switch (match){

                    case DTN_CBOR_MATCH_FULL:
                
                        if (!val){
                            key = dtn_cbor_free(key);
                            goto error;
                        } 
                        break;

                    default:
                        key = dtn_cbor_free(key);
                        val = dtn_cbor_free(val);
                        self = dtn_cbor_free(self);
                        goto error;
                }

                if (!dtn_dict_set(self->data, key, val, NULL)){
                    key = dtn_cbor_free(key);
                    val = dtn_cbor_free(val);
                    self = dtn_cbor_free(self);
                    goto error;
                }
            }
            len = ptr - buffer + 1;
            goto done;
            break;    
        default:
            goto error;
    }
        
    self = dtn_cbor_create(DTN_CBOR_MAP);
    if (!self) goto error;

    self->data = dtn_dict_create(dtn_cbor_dict_config(255));
    if (!self->data) goto error;

    for (uint64_t i = 1; i <= map_items; i++){

        dtn_cbor *key = NULL;
        dtn_cbor *val = NULL;

        dtn_cbor_match match = dtn_cbor_decode(
            ptr, size - (ptr - buffer), &key, &ptr);
        
        switch (match){

            case DTN_CBOR_MATCH_FULL:
        
                if (!key) goto error;
                break;

            default:
                key = dtn_cbor_free(key);
                self = dtn_cbor_free(self);
                return match;
        }

        match = dtn_cbor_decode(
            ptr, size - (ptr - buffer), &val, &ptr);
        
        switch (match){

            case DTN_CBOR_MATCH_FULL:
        
                if (!val){
                    key = dtn_cbor_free(key);
                    goto error;
                } 
                break;

            default:
                key = dtn_cbor_free(key);
                val = dtn_cbor_free(val);
                self = dtn_cbor_free(self);
                return match;
        }

        if (!dtn_dict_set(self->data, key, val, NULL)){
            key = dtn_cbor_free(key);
            val = dtn_cbor_free(val);
            goto error;
        }
    }

    if(map_items > 0)
        len = ptr - (uint8_t*) buffer;

done:
    *out = self;
    *next = (uint8_t*) buffer + len;
    return DTN_CBOR_MATCH_FULL;
partial:
    self = dtn_cbor_free(self);
    return DTN_CBOR_MATCH_PARTIAL;
error:
    self = dtn_cbor_free(self);
    return DTN_CBOR_NO_MATCH;
}

/*----------------------------------------------------------------------------*/

static dtn_cbor_match decode_tag(
    const uint8_t *buffer, 
    size_t size,
    dtn_cbor **out, 
    uint8_t **next){

    if (size == 0) return DTN_CBOR_MATCH_PARTIAL;

    uint64_t local = 0;
    dtn_cbor *self = NULL;
    dtn_cbor *child = NULL;
    dtn_cbor_match match = DTN_CBOR_NO_MATCH;

    switch (buffer[0]){

        case 0xc0:  
            match = decode_text_string(buffer + 1, size - 1, &self, next);
            switch(match){
                case DTN_CBOR_MATCH_FULL:
                    self->type = DTN_CBOR_DATE_TIME;
                    goto done;
                default:
                    return match;
            }
            self->nbr_uint = buffer[0];
            break;

        case 0xc1:
            match = decode_uint(buffer + 1, size - 1, &self, next);
            switch(match){
                case DTN_CBOR_MATCH_FULL:
                    self->type = DTN_CBOR_DATE_TIME_EPOCH;
                    goto done;
                default:
                    return match;
            }
            self->nbr_uint = buffer[0];
            break;

        case 0xc2:
            match = decode_text_string(buffer + 1, size - 1, &self, next);
            switch(match){
                case DTN_CBOR_MATCH_FULL:
                    self->type = DTN_CBOR_UBIGNUM;
                    goto done;
                default:
                    return match;
            }
            self->nbr_uint = buffer[0];
            break;

        case 0xc3:
            match = decode_text_string(buffer + 1, size - 1, &self, next);
            switch(match){
                case DTN_CBOR_MATCH_FULL:
                    self->type = DTN_CBOR_IBIGNUM;
                    goto done;
                default:
                    return match;
            }
            self->nbr_uint = buffer[0];
            break;

        case 0xc4:
            match = decode_array(buffer + 1, size - 1, &child, next);
            switch(match){
                case DTN_CBOR_MATCH_FULL:
                    self = dtn_cbor_create(DTN_CBOR_DEC_FRACTION);
                    self->data = child;
                    goto done;
                default:
                    return match;
            }
            self->nbr_uint = buffer[0];
            break;

        case 0xc5:
            match = decode_array(buffer + 1, size - 1, &child, next);
            switch(match){
                case DTN_CBOR_MATCH_FULL:
                    self = dtn_cbor_create(DTN_CBOR_BIGFLOAT);
                    self->data = child;
                    goto done;
                default:
                    return match;
            }
            self->nbr_uint = buffer[0];
            break;

        case 0xc6:
        case 0xc7:
        case 0xc8:
        case 0xc9:
        case 0xca:
        case 0xcb:
        case 0xcc:
        case 0xcd:
        case 0xce:
        case 0xcf:
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:

            self = dtn_cbor_create(DTN_CBOR_TAG);
            self->nbr_uint = buffer[0] & 0x1F;
            self->tag = buffer[0];
            *next = (uint8_t*) buffer + 1;
            break;

        case 0xD5:
        case 0xD6:
        case 0xD7:
            match = dtn_cbor_decode(buffer + 1, size - 1, &child, next);
            switch(match){
                case DTN_CBOR_MATCH_FULL:
                    self = dtn_cbor_create(DTN_CBOR_TAG);
                    self->data = child;
                    self->nbr_uint = buffer[0];
                    break;
                default:
                    return match;
            }
            break;

        case 0xd8:
            // 1 byte tag
            if (size < 2)
                goto partial;
            self = dtn_cbor_create(DTN_CBOR_TAG);
            if (!self) goto error;
            self->nbr_uint = buffer[1];
            *next = (uint8_t*) buffer + 2;
            break;

        case 0xd9:
            if (size < 3)
                goto partial;

            match = dtn_cbor_decode(buffer + 3, size - 3, &child, next);
            switch(match){
                case DTN_CBOR_MATCH_FULL:
                    self = dtn_cbor_create(DTN_CBOR_TAG);
                    self->data = child;
                    break;
                default:
                    return match;
            }

            local = buffer[1];
            self->nbr_uint = local << 8;
            local = buffer[2];
            self->nbr_uint += local;
            break;

        case 0xdA:
            if (size < 5)
                goto partial;

            match = dtn_cbor_decode(buffer + 5, size - 5, &child, next);
            switch(match){
                case DTN_CBOR_MATCH_FULL:
                    self = dtn_cbor_create(DTN_CBOR_TAG);
                    self->data = child;
                    break;
                default:
                    return match;
            }

            local = buffer[1];
            self->nbr_uint =  local << 24;
            local = buffer[2];
            self->nbr_uint += local << 16;
            local = buffer[3];
            self->nbr_uint += local << 8;
            local = buffer[4];
            self->nbr_uint += local;
            break;

        case 0xdB:

            if (size < 9)
                goto partial;

            match = dtn_cbor_decode(buffer + 9, size - 9, &child, next);
            switch(match){
                case DTN_CBOR_MATCH_FULL:
                    self = dtn_cbor_create(DTN_CBOR_TAG);
                    self->data = child;
                    break;
                default:
                    return match;
            }

            local = buffer[1];
            self->nbr_uint = local << 56;
            local = buffer[2];
            self->nbr_uint += local << 48;
            local = buffer[3];
            self->nbr_uint += local << 40;
            local = buffer[4];
            self->nbr_uint += local << 32;
            local = buffer[5];
            self->nbr_uint += local << 24;
            local = buffer[6];
            self->nbr_uint += local << 16;
            local = buffer[7];
            self->nbr_uint += local << 8;
            local = buffer[8];
            self->nbr_uint += local;
            break;
        default:
            goto error;
    }
done:
    self->tag = buffer[0];
    *out = self;
    return DTN_CBOR_MATCH_FULL;
partial:
    self = dtn_cbor_free(self);
    return DTN_CBOR_MATCH_PARTIAL;
error:
    self = dtn_cbor_free(self);
    return DTN_CBOR_NO_MATCH;
}

/*----------------------------------------------------------------------------*/

static dtn_cbor_match decode_simple(
    const uint8_t *buffer, 
    size_t size,
    dtn_cbor **out, 
    uint8_t **next){

    char buf[30] = {0};
    dtn_cbor *self = NULL;
    double nbr = 0;

    if (size == 0) return DTN_CBOR_MATCH_PARTIAL;

    switch (buffer[0]){

        case 0xE0:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:
            self = dtn_cbor_create(DTN_CBOR_SIMPLE);
            self->nbr_uint = buffer[0] & 0x1F;
            *next = (uint8_t*) buffer +1;
            break;
        case 0xF4:
            self = dtn_cbor_create(DTN_CBOR_FALSE);
            *next = (uint8_t*) buffer +1;
            break;
        case 0xF5:
            self = dtn_cbor_create(DTN_CBOR_TRUE);
            *next = (uint8_t*) buffer +1;
            break;
        case 0xF6:
            self = dtn_cbor_create(DTN_CBOR_NULL);
            *next = (uint8_t*) buffer +1;
            break;
        case 0xF7:
            self = dtn_cbor_create(DTN_CBOR_UNDEF);
            *next = (uint8_t*) buffer +1;
            break;
        case 0xF8:
            if (size < 2)
                goto partial;

            self = dtn_cbor_create(DTN_CBOR_SIMPLE);
            self->nbr_uint = buffer[1];
            *next = (uint8_t*) buffer + 2;
            break;

        case 0xF9:
            
            if (size < 3)
                goto partial;

            memcpy(buf, buffer + 1, 2);
            nbr = atof(buf);
            self = dtn_cbor_create(DTN_CBOR_FLOAT);
            self->nbr_float = nbr;

            *next = (uint8_t*) buffer + 3;
            break;

        case 0xFA:

            if (size < 5) goto partial;

            memcpy(buf, buffer + 1, 4);
            nbr = atof(buf);
            self = dtn_cbor_create(DTN_CBOR_FLOAT);
            self->nbr_float = nbr;

            *next = (uint8_t*) buffer + 5;
            break;

        case 0xfB:

            if (size < 9) goto partial;

            memcpy(buf, buffer + 1, 8);
            nbr = atof(buf);
            self = dtn_cbor_create(DTN_CBOR_DOUBLE);
            self->nbr_float = nbr;

            *next = (uint8_t*) buffer + 9;
            break;

        default:
            goto error;

        }

        self->tag = buffer[0];

    *out = self;
    return DTN_CBOR_MATCH_FULL;
partial:
    return DTN_CBOR_MATCH_PARTIAL;
error:
    return DTN_CBOR_NO_MATCH;
}

/*----------------------------------------------------------------------------*/

dtn_cbor_match dtn_cbor_decode(
    const uint8_t *buffer, 
    size_t size,
    dtn_cbor **out, 
    uint8_t **next){

    if (!buffer || !out || !next) goto error;

    if (size < 1) return DTN_CBOR_MATCH_PARTIAL;

    uint8_t header = buffer[0];
    uint8_t major = header >> 5;

    switch (major){

        case 0:
            // unsigned integer
            return decode_uint(buffer, size, out, next);
            break;
        case 1:
            // negative integer
            return decode_int(buffer, size, out, next);
            break;
        case 2:
            // byte string
            return decode_text_string(buffer, size, out, next);
            break;
        case 3:
            // UTF8 text string
            return decode_utf8_string(buffer, size, out, next);
            break;
        case 4:
            // array
            return decode_array(buffer, size, out, next);
            break;
        case 5:
            // map
            return decode_map(buffer, size, out, next);
            break;
        case 6:
            // tag time
            return decode_tag(buffer, size, out, next);
            break;
        case 7: 
            // floating point numbers, simple values, break
            return decode_simple(buffer, size, out, next);
            break;
    }

error:
    return DTN_CBOR_NO_MATCH;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_configure(dtn_cbor_config config){

    g_config = config;
    return true;
}

/*----------------------------------------------------------------------------*/

static bool count_item_size(const void *key, void *val, void *data){

    if (!key) return true;

    dtn_cbor *k = (dtn_cbor*) key;
    dtn_cbor *v = (dtn_cbor*) val;

    uint64_t *counter = (uint64_t*) data;

    uint64_t size = dtn_cbor_encoding_size(k);
    size += dtn_cbor_encoding_size(v);

    *counter = *counter + size;
    return true;
}

/*----------------------------------------------------------------------------*/

static uint64_t cbor_encoding_size(const dtn_cbor *self);

/*----------------------------------------------------------------------------*/

static uint64_t cbor_uint_encoding_size(const dtn_cbor *self){

    uint64_t size = 0;

    if (!self) goto error;

    if (self->nbr_uint <= 0x17){
        size = 1;
    } else if (self->nbr_uint <= 0xFF){
        size = 2;
    } else if (self->nbr_uint <= 0xFFFF){
        size = 3;
    } else if (self->nbr_uint <= 0xFFFFffff){
        size = 5;
    } else {
        size = 9;
    }

    return size;
error:
    return 0;
}

/*----------------------------------------------------------------------------*/

static uint64_t cbor_int_encoding_size(const dtn_cbor *self){

    uint64_t size = 0;
    int64_t nbr = 0;

    if (!self) goto error;

    nbr = self->nbr_int;
    if (nbr < 0) nbr *= -1;

    if (nbr <= 0x17){
        size = 1;
    } else if (nbr <= 0xFF){
        size = 2;
    } else if (nbr <= 0xFFFF){
        size = 3;
    } else if (nbr <= 0xFFFFffff){
        size = 5;
    } else {
        size = 9;
    }

    return size;
error:
    return 0;
}

/*----------------------------------------------------------------------------*/

static uint64_t cbor_string_encoding_size(const dtn_cbor *self){

    uint64_t size = 0;
    uint64_t nbr = 0;

    if (!self) goto error;

    nbr = self->nbr_int;

    if (nbr <= 0x17){
        size = 1;
    } else if (nbr <= 0xFF){
        size = 2;
    } else if (nbr <= 0xFFFF){
        size = 3;
    } else if (nbr <= 0xFFFFffff){
        size = 5;
    } else {
        size = 9;
    }

    size += nbr;
    return size;
error:
    return 0;
}

/*----------------------------------------------------------------------------*/

static uint64_t cbor_array_encoding_size(const dtn_cbor *self){

    uint64_t size = 0;
    uint64_t items = 0;

    if (!self) goto error;

    if (!self->data) return 1;

    if (dtn_list_is_empty(self->data))
                return 1;

    items = 0;
    for (uint64_t i = 1; i <= 
        dtn_list_count(self->data); i++){

        items += dtn_cbor_encoding_size(
            dtn_list_get(self->data, i));

    }

    if (items <= 0x17){
        size = 1;
    } else if (items <= 0xFF){
        size = 2;
    } else if (items <= 0xFFFF){
        size = 3;
    } else if (items <= 0xFFFFffff){
        size = 5;
    } else {
        size = 9;
    }

    size += items;
    return size;
error:
    return 0;
}

/*----------------------------------------------------------------------------*/

static uint64_t cbor_map_encoding_size(const dtn_cbor *self){

    uint64_t size = 0;
    uint64_t items = 0;

    if (!self) goto error;

    if (dtn_dict_is_empty(self->data))
        return 1;
    
    items = 0;
    if (!dtn_dict_for_each(
        self->data,
        &items,
        count_item_size)) goto error;

    if (items <= 0x17){
        size = 1;
    } else if (items <= 0xFF){
        size = 2;
    } else if (items <= 0xFFFF){
        size = 3;
    } else if (items <= 0xFFFFffff){
        size = 5;
    } else {
        size = 9;
    }

    size += items;
    return size;
error:
    return 0;
}

/*----------------------------------------------------------------------------*/

static uint64_t cbor_tag_encoding_size(const dtn_cbor *self){

    uint64_t size = 0;
    uint64_t nbr = 0;

    if (!self) goto error;

    nbr = self->nbr_uint;

    if (nbr <= 0x17){
        size = 1;
    } else if (nbr <= 0xFF){
        size = 2;
    } else if (nbr <= 0xFFFF){
        size = 3;
    } else if (nbr <= 0xFFFFffff){
        size = 5;
    } else {
        size = 9;
    }

    if (self->data)
        size += cbor_encoding_size(self->data);

    return size;
error:
    return 0;
}

/*----------------------------------------------------------------------------*/

static uint64_t cbor_float_encoding_size(const dtn_cbor *self){

    uint64_t size = 0;
    uint8_t buffer[100] = {0};

    if (!self) goto error;

    memset(buffer, 0, 100);
    size = snprintf((char*)buffer, 100, "%f", self->nbr_float);
    size += 1;

    return size;
error:
    return 0;
}

/*----------------------------------------------------------------------------*/

static uint64_t cbor_double_encoding_size(const dtn_cbor *self){

    uint64_t size = 0;
    uint8_t buffer[100] = {0};

    if (!self) goto error;

    memset(buffer, 0, 100);
    size = snprintf((char*)buffer, 100, "%g", self->nbr_double);
    size += 1;
    return size;
error:
    return 0;
}

/*----------------------------------------------------------------------------*/

static uint64_t cbor_encoding_size(const dtn_cbor *self){

    uint64_t size = 0;

    if (!self) goto error;

    switch(self->type){

        case DTN_CBOR_UNDEF:
            size = 1;
            break;

        case DTN_CBOR_FALSE:

            size = 1;
            break;
        case DTN_CBOR_TRUE:

            size = 1;
            break;

        case DTN_CBOR_NULL:

            size = 1;
            break;

        case DTN_CBOR_UINT64:

            return cbor_uint_encoding_size(self);
            break;

        case DTN_CBOR_INT64:

            return cbor_int_encoding_size(self);
            break;

        case DTN_CBOR_STRING:

            return cbor_string_encoding_size(self);
            break;

        case DTN_CBOR_UTF8:

            return cbor_string_encoding_size(self);
            break;

        case DTN_CBOR_ARRAY:

            return cbor_array_encoding_size(self);
            break;

        case DTN_CBOR_MAP:

            return cbor_map_encoding_size(self);
            break;

        case DTN_CBOR_DATE_TIME:

            return cbor_string_encoding_size(self);
            break;

        case DTN_CBOR_DATE_TIME_EPOCH:

            return cbor_uint_encoding_size(self);
            break;

        case DTN_CBOR_UBIGNUM:

            return cbor_string_encoding_size(self);
            break;

        case DTN_CBOR_IBIGNUM:

            return cbor_string_encoding_size(self);
            break;

        case DTN_CBOR_DEC_FRACTION:

            return 1 + cbor_array_encoding_size(self->data);
            break;

        case DTN_CBOR_BIGFLOAT:

            return 1 + cbor_array_encoding_size(self->data);
            break;

        case DTN_CBOR_TAG:

            return cbor_tag_encoding_size(self);
            break;

        case DTN_CBOR_SIMPLE:

            return cbor_uint_encoding_size(self);
            break;

        case DTN_CBOR_FLOAT:

            return cbor_float_encoding_size(self);
            break;

        case DTN_CBOR_DOUBLE:

            return cbor_double_encoding_size(self);
            break;

    }

    return size;
error:
    return 0;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_encoding_size(const dtn_cbor *self){

    return cbor_encoding_size(self);
}

/*----------------------------------------------------------------------------*/

static bool encode_uint(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    if (!self || !buffer || size < 1 || !next) goto error;

    if (self->type != DTN_CBOR_UINT64) goto error;

    if (self->nbr_uint <= 0x17){
        
        if (size < 1) goto error;
        buffer[0] = self->nbr_uint;
        *next = buffer + 1;

    } else if (self->nbr_uint <= 0xFF){
        
        if (size < 2) goto error;
        buffer[0] = 0x18;
        buffer[1] = self->nbr_uint;
        *next = buffer + 2;

    } else if (self->nbr_uint <= 0xFFFF){
        
        if (size < 3) goto error;
        buffer[0] = 0x19;
        buffer[1] = self->nbr_uint >> 8;
        buffer[2] = self->nbr_uint;
        *next = buffer + 3;


    } else if (self->nbr_uint <= 0xFFFFffff){
        
        if (size < 5) goto error;
        buffer[0] = 0x1A;
        buffer[1] = self->nbr_uint >> 24;
        buffer[2] = self->nbr_uint >> 16;
        buffer[3] = self->nbr_uint >> 8;
        buffer[4] = self->nbr_uint;
        *next = buffer + 5;
    
    } else {
        
        if (size < 9) goto error;
        buffer[0] = 0x1B;
        buffer[1] = self->nbr_uint >> 56;
        buffer[2] = self->nbr_uint >> 48;
        buffer[3] = self->nbr_uint >> 40;
        buffer[4] = self->nbr_uint >> 32;
        buffer[5] = self->nbr_uint >> 24;
        buffer[6] = self->nbr_uint >> 16;
        buffer[7] = self->nbr_uint >> 8;
        buffer[8] = self->nbr_uint;
        *next = buffer + 9;
    }
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool encode_int(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    if (!self || !buffer || size < 1 || !next) goto error;

    if (self->type != DTN_CBOR_INT64) goto error;

    int64_t nbr = self->nbr_int;
    if (nbr < 0) nbr *= -1;

    if (nbr <= 0x0F){
        
        if (size < 1) goto error;
        buffer[0] = nbr;
        buffer[0] |= 0x20;
        *next = buffer + 1;

    } else if (nbr <= 0x17){

        if (size < 1) goto error;
        buffer[0] = nbr;
        buffer[0] |= 0x30;
        *next = buffer + 1;

    } else if (nbr <= 0xFF){
        
        if (size < 2) goto error;
        buffer[0] = 0x38;
        buffer[1] = nbr;
        *next = buffer + 2;

    } else if (nbr <= 0xFFFF){
        
        if (size < 3) goto error;
        buffer[0] = 0x39;
        buffer[1] = nbr >> 8;
        buffer[2] = nbr;
        *next = buffer + 3;


    } else if (nbr <= 0xFFFFffff){
        
        if (size < 5) goto error;
        buffer[0] = 0x3A;
        buffer[1] = nbr >> 24;
        buffer[2] = nbr >> 16;
        buffer[3] = nbr >> 8;
        buffer[4] = nbr;
        *next = buffer + 5;
    
    } else {
        
        if (size < 9) goto error;
        buffer[0] = 0x3B;
        buffer[1] = nbr >> 56;
        buffer[2] = nbr >> 48;
        buffer[3] = nbr >> 40;
        buffer[4] = nbr >> 32;
        buffer[5] = nbr >> 24;
        buffer[6] = nbr >> 16;
        buffer[7] = nbr >> 8;
        buffer[8] = nbr;
        *next = buffer + 9;
    }
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool encode_string(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    if (!self || !buffer || size < 1 || !next) goto error;

    if (self->type != DTN_CBOR_STRING) goto error;

    uint64_t len = 0;

    if (self->nbr_uint <= 0x0F){
        
        len = 1 + self->nbr_uint;
        if (size < len) goto error;

        buffer[0] = self->nbr_uint;
        buffer[0] |= 0x40;

        memcpy(buffer + 1, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0x17){
        
        len = 1 + self->nbr_uint;
        if (size < len) goto error;

        buffer[0] = self->nbr_uint;
        buffer[0] |= 0x50;

        memcpy(buffer + 1, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFF){

        len = 2 + self->nbr_uint;
        if (size < len) goto error;

        buffer[0] = 0x58;
        buffer[1] = self->nbr_uint;

        memcpy(buffer + 2, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFFFF){

        len = 3 + self->nbr_uint;
        if (size < len) goto error;

        buffer[0] = 0x59;
        buffer[1] = self->nbr_uint >> 8;
        buffer[2] = self->nbr_uint;

        memcpy(buffer + 3, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFFFFffff){

        len = 5 + self->nbr_uint;
        if (size < len) goto error;

        buffer[0] = 0x5A;
        buffer[1] = self->nbr_uint >> 24;
        buffer[2] = self->nbr_uint >> 16;
        buffer[3] = self->nbr_uint >> 8;
        buffer[4] = self->nbr_uint;

        memcpy(buffer + 5, self->string, self->nbr_uint);
        *next = buffer + len;
    
    } else {
        
        len = 9 + self->nbr_uint;
        if (size < len) goto error;

        buffer[0] = 0x5B;
        buffer[1] = self->nbr_uint >> 56;
        buffer[2] = self->nbr_uint >> 48;
        buffer[3] = self->nbr_uint >> 40;
        buffer[4] = self->nbr_uint >> 32;
        buffer[5] = self->nbr_uint >> 24;
        buffer[6] = self->nbr_uint >> 16;
        buffer[7] = self->nbr_uint >> 8;
        buffer[8] = self->nbr_uint;

        memcpy(buffer + 9, self->string, self->nbr_uint);
        *next = buffer + len;
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool encode_uft8(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    if (!self || !buffer || size < 1 || !next) goto error;

    if (self->type != DTN_CBOR_UTF8) goto error;

    uint64_t len = 0;

    if (self->nbr_uint <= 0x0F){
        
        len = 1 + self->nbr_uint;
        if (size < len) goto error;

        buffer[0] = self->nbr_uint;
        buffer[0] |= 0x60;

        memcpy(buffer + 1, self->bytes, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0x17){
        
        len = 1 + self->nbr_uint;
        if (size < len) goto error;

        buffer[0] = self->nbr_uint;
        buffer[0] |= 0x70;

        memcpy(buffer + 1, self->bytes, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFF){

        len = 2 + self->nbr_uint;
        if (size < len) goto error;

        buffer[0] = 0x78;
        buffer[1] = self->nbr_uint;

        memcpy(buffer + 2, self->bytes, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFFFF){

        len = 3 + self->nbr_uint;
        if (size < len) goto error;

        buffer[0] = 0x79;
        buffer[1] = self->nbr_uint >> 8;
        buffer[2] = self->nbr_uint;

        memcpy(buffer + 3, self->bytes, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFFFFffff){

        len = 5 + self->nbr_uint;
        if (size < len) goto error;

        buffer[0] = 0x7A;
        buffer[1] = self->nbr_uint >> 24;
        buffer[2] = self->nbr_uint >> 16;
        buffer[3] = self->nbr_uint >> 8;
        buffer[4] = self->nbr_uint;

        memcpy(buffer + 5, self->bytes, self->nbr_uint);
        *next = buffer + len;
    
    } else {
        
        len = 9 + self->nbr_uint;
        if (size < len) goto error;

        buffer[0] = 0x7B;
        buffer[1] = self->nbr_uint >> 56;
        buffer[2] = self->nbr_uint >> 48;
        buffer[3] = self->nbr_uint >> 40;
        buffer[4] = self->nbr_uint >> 32;
        buffer[5] = self->nbr_uint >> 24;
        buffer[6] = self->nbr_uint >> 16;
        buffer[7] = self->nbr_uint >> 8;
        buffer[8] = self->nbr_uint;

        memcpy(buffer + 9, self->bytes, self->nbr_uint);
        *next = buffer + len;
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool encode_array(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    if (!self || !buffer || size < 1 || !next) goto error;

    if (self->type != DTN_CBOR_ARRAY) goto error;

    uint64_t len = 0;
    uint8_t *ptr = NULL;

    if (dtn_list_count(self->data) == 0){
        
        if (size < 1) goto error;   
        buffer[0] = 0x80;
        *next = buffer + 1;
        goto done;
    
    }

    uint64_t items = 0;
    for (uint64_t i = 1; i <= dtn_list_count(self->data); i++){

        items += dtn_cbor_encoding_size(
            dtn_list_get(self->data, i));

    }

    if (size < items) goto error;

    if (items <= 0x0F){

        len = 1 + items;
        if (size < len) goto error;

        buffer[0] = items;
        buffer[0] |= 0x80;

        ptr = buffer + 1;

    } else if (items <= 0x17){

        len = 1 + items;
        if (size < len) goto error;

        buffer[0] = items;
        buffer[0] |= 0x90;

        ptr = buffer + 1;

    } else if (items <= 0xFF){

        len = 2 + items;
        if (size < len) goto error;

        buffer[0] = 0x98;
        buffer[1] = items;

        ptr = buffer + 2;

    } else if (items <= 0xFFFF){
        
        len = 3 + items;
        if (size < len) goto error;

        buffer[0] = 0x99;
        buffer[1] = items >> 8;
        buffer[2] = items;
        
        ptr = buffer + 3;

    } else if (items <= 0xFFFFffff){
        
        len = 5 + items;
        if (size < len) goto error;

        buffer[0] = 0x9A;
        buffer[1] = items >> 24;
        buffer[2] = items >> 16;
        buffer[3] = items >> 8;
        buffer[4] = items;
        
        ptr = buffer + 5;

    } else {
        
        len = 9 + items;
        if (size < len) goto error;

        buffer[0] = 0x9B;
        buffer[1] = items >> 56;
        buffer[2] = items >> 48;
        buffer[3] = items >> 40;
        buffer[4] = items >> 32;
        buffer[5] = items >> 24;
        buffer[6] = items >> 16;
        buffer[7] = items >> 8;
        buffer[8] = items;
        
        ptr = buffer + 9;
    }

    for (uint64_t i = 1; i <= dtn_list_count(self->data); i++){

        dtn_cbor *item = dtn_list_get(self->data, i);

        if (!dtn_cbor_encode(
            item, 
            ptr, 
            size - (ptr -buffer),
            &ptr)) goto error;


    }
    
    *next = ptr;
   
done:
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

struct container {

    uint8_t *ptr;
    size_t size;
};

/*----------------------------------------------------------------------------*/

static bool write_items(const void *key, void *val, void *data){

    if (!key) return true;

    dtn_cbor *k = (dtn_cbor*)key;
    dtn_cbor *v = (dtn_cbor*)val;

    struct container *c = (struct container*) data;

    if (!dtn_cbor_encode(
            k, 
            c->ptr, 
            c->size,
            &c->ptr)) goto error;

    if (!dtn_cbor_encode(
            v, 
            c->ptr, 
            c->size,
            &c->ptr)) goto error;

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool encode_map(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    if (!self || !buffer || size < 1 || !next) goto error;

    if (self->type != DTN_CBOR_MAP) goto error;

    uint64_t len = 0;
    uint8_t *ptr = NULL;

    if (!self->data){

        if (size < 1) goto error;   
        buffer[0] = 0xA0;
        *next = buffer + 1;
        goto done;

    }

    if (dtn_dict_count(self->data) == 0){
        
        if (size < 1) goto error;   
        buffer[0] = 0xA0;
        *next = buffer + 1;
        goto done;
    }

    uint64_t items = 0;

    if (!dtn_dict_for_each(
        self->data,
        &items,
        count_item_size)) goto error;

    if (size < items) goto error;

    if (items <= 0x0F){

        len = 1 + items;
        if (size < len) goto error;

        buffer[0] = items;
        buffer[0] |= 0xA0;

        ptr = buffer + 1;

    } else if (items <= 0x17){

        len = 1 + items;
        if (size < len) goto error;

        buffer[0] = items;
        buffer[0] |= 0xB0;

        ptr = buffer + 1;

    } else if (items <= 0xFF){

        len = 2 + items;
        if (size < len) goto error;

        buffer[0] = 0xB8;
        buffer[1] = items;

        ptr = buffer + 2;

    } else if (items <= 0xFFFF){
        
        len = 3 + items;
        if (size < len) goto error;

        buffer[0] = 0xB9;
        buffer[1] = items >> 8;
        buffer[2] = items;
        
        ptr = buffer + 3;

    } else if (items <= 0xFFFFffff){
        
        len = 5 + items;
        if (size < len) goto error;

        buffer[0] = 0xBA;
        buffer[1] = items >> 24;
        buffer[2] = items >> 16;
        buffer[3] = items >> 8;
        buffer[4] = items;
        
        ptr = buffer + 5;

    } else {
        
        len = 9 + items;
        if (size < len) goto error;

        buffer[0] = 0xBB;
        buffer[1] = items >> 56;
        buffer[2] = items >> 48;
        buffer[3] = items >> 40;
        buffer[4] = items >> 32;
        buffer[5] = items >> 24;
        buffer[6] = items >> 16;
        buffer[7] = items >> 8;
        buffer[8] = items;
        
        ptr = buffer + 9;
    }

    struct container container = (struct container){

        .ptr = ptr, 
        .size = size
    };

    if (!dtn_dict_for_each(
        self->data,
        &container,
        write_items)) goto error;
    
    *next = container.ptr;
   
done:
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool encode_date_time(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    if (!self || !buffer || size < 1 || !next) goto error;

    if (self->type != DTN_CBOR_DATE_TIME) goto error;

    uint64_t len = 0;

    buffer[0] = 0xc0;

    if (self->nbr_uint <= 0x0F){
        
        len = 2 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = self->nbr_uint;
        buffer[1] |= 0x40;

        memcpy(buffer + 2, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0x17){
        
        len = 2 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = self->nbr_uint;
        buffer[1] |= 0x50;

        memcpy(buffer + 2, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFF){

        len = 3 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = 0x58;
        buffer[2] = self->nbr_uint;

        memcpy(buffer + 3, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFFFF){

        len = 4 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = 0x59;
        buffer[2] = self->nbr_uint >> 8;
        buffer[3] = self->nbr_uint;

        memcpy(buffer + 4, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFFFFffff){

        len = 6 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = 0x5A;
        buffer[2] = self->nbr_uint >> 24;
        buffer[3] = self->nbr_uint >> 16;
        buffer[4] = self->nbr_uint >> 8;
        buffer[5] = self->nbr_uint;

        memcpy(buffer + 6, self->string, self->nbr_uint);
        *next = buffer + len;
    
    } else {
        
        len = 10 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = 0x5B;
        buffer[2] = self->nbr_uint >> 56;
        buffer[3] = self->nbr_uint >> 48;
        buffer[4] = self->nbr_uint >> 40;
        buffer[5] = self->nbr_uint >> 32;
        buffer[6] = self->nbr_uint >> 24;
        buffer[7] = self->nbr_uint >> 16;
        buffer[8] = self->nbr_uint >> 8;
        buffer[9] = self->nbr_uint;

        memcpy(buffer + 10, self->string, self->nbr_uint);
        *next = buffer + len;
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool encode_date_time_epoch(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    if (!self || !buffer || size < 1 || !next) goto error;

    if (self->type != DTN_CBOR_DATE_TIME_EPOCH) goto error;

    buffer[0] = 0xc1;

    if (self->nbr_uint <= 0x17){
        
        if (size < 2) goto error;
        buffer[1] = self->nbr_uint;
        *next = buffer + 2;

    } else if (self->nbr_uint <= 0xFF){
        
        if (size < 3) goto error;
        buffer[1] = 0x18;
        buffer[2] = self->nbr_uint;
        *next = buffer + 3;

    } else if (self->nbr_uint <= 0xFFFF){
        
        if (size < 4) goto error;
        buffer[1] = 0x19;
        buffer[2] = self->nbr_uint >> 8;
        buffer[3] = self->nbr_uint;
        *next = buffer + 4;


    } else if (self->nbr_uint <= 0xFFFFffff){
        
        if (size < 6) goto error;
        buffer[1] = 0x1A;
        buffer[2] = self->nbr_uint >> 24;
        buffer[3] = self->nbr_uint >> 16;
        buffer[4] = self->nbr_uint >> 8;
        buffer[5] = self->nbr_uint;
        *next = buffer + 6;
    
    } else {
        
        if (size < 10) goto error;
        buffer[1] = 0x1B;
        buffer[2] = self->nbr_uint >> 56;
        buffer[3] = self->nbr_uint >> 48;
        buffer[4] = self->nbr_uint >> 40;
        buffer[5] = self->nbr_uint >> 32;
        buffer[6] = self->nbr_uint >> 24;
        buffer[7] = self->nbr_uint >> 16;
        buffer[8] = self->nbr_uint >> 8;
        buffer[9] = self->nbr_uint;
        *next = buffer + 10;
    }
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool encode_ubignum(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    if (!self || !buffer || size < 1 || !next) goto error;

    if (self->type != DTN_CBOR_UBIGNUM) goto error;

    uint64_t len = 0;

    buffer[0] = 0xc2;

    if (self->nbr_uint <= 0x0F){
        
        len = 2 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = self->nbr_uint;
        buffer[1] |= 0x40;

        memcpy(buffer + 2, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0x17){
        
        len = 2 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = self->nbr_uint;
        buffer[1] |= 0x50;

        memcpy(buffer + 2, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFF){

        len = 3 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = 0x58;
        buffer[2] = self->nbr_uint;

        memcpy(buffer + 3, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFFFF){

        len = 4 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = 0x59;
        buffer[2] = self->nbr_uint >> 8;
        buffer[3] = self->nbr_uint;

        memcpy(buffer + 4, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFFFFffff){

        len = 6 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = 0x5A;
        buffer[2] = self->nbr_uint >> 24;
        buffer[3] = self->nbr_uint >> 16;
        buffer[4] = self->nbr_uint >> 8;
        buffer[5] = self->nbr_uint;

        memcpy(buffer + 6, self->string, self->nbr_uint);
        *next = buffer + len;
    
    } else {
        
        len = 10 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = 0x5B;
        buffer[2] = self->nbr_uint >> 56;
        buffer[3] = self->nbr_uint >> 48;
        buffer[4] = self->nbr_uint >> 40;
        buffer[5] = self->nbr_uint >> 32;
        buffer[6] = self->nbr_uint >> 24;
        buffer[7] = self->nbr_uint >> 16;
        buffer[8] = self->nbr_uint >> 8;
        buffer[9] = self->nbr_uint;

        memcpy(buffer + 10, self->string, self->nbr_uint);
        *next = buffer + len;
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool encode_ibignum(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    if (!self || !buffer || size < 1 || !next) goto error;

    if (self->type != DTN_CBOR_IBIGNUM) goto error;

    uint64_t len = 0;

    buffer[0] = 0xc3;

    if (self->nbr_uint <= 0x0F){
        
        len = 2 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = self->nbr_uint;
        buffer[1] |= 0x40;

        memcpy(buffer + 2, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0x17){
        
        len = 2 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = self->nbr_uint;
        buffer[1] |= 0x50;

        memcpy(buffer + 2, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFF){

        len = 3 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = 0x58;
        buffer[2] = self->nbr_uint;

        memcpy(buffer + 3, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFFFF){

        len = 4 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = 0x59;
        buffer[2] = self->nbr_uint >> 8;
        buffer[3] = self->nbr_uint;

        memcpy(buffer + 4, self->string, self->nbr_uint);
        *next = buffer + len;

    } else if (self->nbr_uint <= 0xFFFFffff){

        len = 6 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = 0x5A;
        buffer[2] = self->nbr_uint >> 24;
        buffer[3] = self->nbr_uint >> 16;
        buffer[4] = self->nbr_uint >> 8;
        buffer[5] = self->nbr_uint;

        memcpy(buffer + 6, self->string, self->nbr_uint);
        *next = buffer + len;
    
    } else {
        
        len = 10 + self->nbr_uint;
        if (size < len) goto error;

        buffer[1] = 0x5B;
        buffer[2] = self->nbr_uint >> 56;
        buffer[3] = self->nbr_uint >> 48;
        buffer[4] = self->nbr_uint >> 40;
        buffer[5] = self->nbr_uint >> 32;
        buffer[6] = self->nbr_uint >> 24;
        buffer[7] = self->nbr_uint >> 16;
        buffer[8] = self->nbr_uint >> 8;
        buffer[9] = self->nbr_uint;

        memcpy(buffer + 10, self->string, self->nbr_uint);
        *next = buffer + len;
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool encode_fraction(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    if (!self || !buffer || size < 1 || !next) goto error;

    if (self->type != DTN_CBOR_DEC_FRACTION) goto error;

    uint64_t len = 0;
    uint8_t *ptr = NULL;

    if (size < 2) goto error;

    buffer[0] = 0xc4;

    if (dtn_cbor_array_count(self->data) == 0){
        
        if (size < 2) goto error;   
        buffer[1] = 0x80;
        *next = buffer + 2;
        goto done;
    
    }

    uint64_t items = 0;
    for (uint64_t i = 0; i < dtn_cbor_array_count(self->data); i++){

        items += dtn_cbor_encoding_size(
            dtn_cbor_array_get(self->data, i));

    }

    if (size < items) goto error;

    if (items <= 0x0F){

        len = 2 + items;
        if (size < len) goto error;

        buffer[1] = items;
        buffer[1] |= 0x80;

        ptr = buffer + 2;

    } else if (items <= 0x17){

        len = 2 + items;
        if (size < len) goto error;

        buffer[1] = items;
        buffer[1] |= 0x90;

        ptr = buffer + 2;

    } else if (items <= 0xFF){

        len = 3 + items;
        if (size < len) goto error;

        buffer[1] = 0x98;
        buffer[2] = items;

        ptr = buffer + 3;

    } else if (items <= 0xFFFF){
        
        len = 4 + items;
        if (size < len) goto error;

        buffer[1] = 0x99;
        buffer[2] = items >> 8;
        buffer[3] = items;
        
        ptr = buffer + 4;

    } else if (items <= 0xFFFFffff){
        
        len = 6 + items;
        if (size < len) goto error;

        buffer[1] = 0x9A;
        buffer[2] = items >> 24;
        buffer[3] = items >> 16;
        buffer[4] = items >> 8;
        buffer[5] = items;
        
        ptr = buffer + 6;

    } else {
        
        len = 10 + items;
        if (size < len) goto error;

        buffer[1] = 0x9B;
        buffer[2] = items >> 56;
        buffer[3] = items >> 48;
        buffer[4] = items >> 40;
        buffer[5] = items >> 32;
        buffer[6] = items >> 24;
        buffer[7] = items >> 16;
        buffer[8] = items >> 8;
        buffer[9] = items;
        
        ptr = buffer + 10;
    }

    for (uint64_t i = 0; i < dtn_cbor_array_count(self->data); i++){

        const dtn_cbor *item = dtn_cbor_array_get(self->data, i);

        if (!dtn_cbor_encode(
            item, 
            ptr, 
            size - (ptr -buffer),
            &ptr)) goto error;


    }
    
    *next = ptr;
   
done:
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool encode_bigfloat(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    if (!self || !buffer || size < 1 || !next) goto error;

    if (self->type != DTN_CBOR_BIGFLOAT) goto error;

    uint64_t len = 0;
    uint8_t *ptr = NULL;

    if (size < 2) goto error;

    buffer[0] = 0xc5;

    if (dtn_cbor_array_count(self->data) == 0){
        
        if (size < 2) goto error;   
        buffer[1] = 0x80;
        *next = buffer + 2;
        goto done;
    
    }

    uint64_t items = 0;
    for (uint64_t i = 0; i < dtn_cbor_array_count(self->data); i++){

        items += dtn_cbor_encoding_size(
            dtn_cbor_array_get(self->data, i));

    }

    if (size < items) goto error;

    if (items <= 0x0F){

        len = 2 + items;
        if (size < len) goto error;

        buffer[1] = items;
        buffer[1] |= 0x80;

        ptr = buffer + 2;

    } else if (items <= 0x17){

        len = 2 + items;
        if (size < len) goto error;

        buffer[1] = items;
        buffer[1] |= 0x90;

        ptr = buffer + 2;

    } else if (items <= 0xFF){

        len = 3 + items;
        if (size < len) goto error;

        buffer[1] = 0x98;
        buffer[2] = items;

        ptr = buffer + 3;

    } else if (items <= 0xFFFF){
        
        len = 4 + items;
        if (size < len) goto error;

        buffer[1] = 0x99;
        buffer[2] = items >> 8;
        buffer[3] = items;
        
        ptr = buffer + 4;

    } else if (items <= 0xFFFFffff){
        
        len = 6 + items;
        if (size < len) goto error;

        buffer[1] = 0x9A;
        buffer[2] = items >> 24;
        buffer[3] = items >> 16;
        buffer[4] = items >> 8;
        buffer[5] = items;
        
        ptr = buffer + 6;

    } else {
        
        len = 10 + items;
        if (size < len) goto error;

        buffer[1] = 0x9B;
        buffer[2] = items >> 56;
        buffer[3] = items >> 48;
        buffer[4] = items >> 40;
        buffer[5] = items >> 32;
        buffer[6] = items >> 24;
        buffer[7] = items >> 16;
        buffer[8] = items >> 8;
        buffer[9] = items;
        
        ptr = buffer + 10;
    }

    for (uint64_t i = 0; i < dtn_cbor_array_count(self->data); i++){

        const dtn_cbor *item = dtn_cbor_array_get(self->data, i);

        if (!dtn_cbor_encode(
            item, 
            ptr, 
            size - (ptr -buffer),
            &ptr)) goto error;


    }
    
    *next = ptr;
   
done:
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool encode_tag(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    uint64_t len = 0;

    if (!self || !buffer || size < 1 || !next) goto error;

    if (self->type != DTN_CBOR_TAG) goto error;

    switch (self->tag){

        case 0xc6:
        case 0xc7:
        case 0xc8:
        case 0xc9:
        case 0xca:
        case 0xcb:
        case 0xcc:
        case 0xcd:
        case 0xce:
        case 0xcf:
        case 0xD0:
        case 0xD1:
        case 0xD2:
        case 0xD3:
        case 0xD4:

            if (size < 1) goto error;
            buffer[0] = self->tag;
            buffer[0] |= self->nbr_uint;
            *next = (uint8_t*) buffer + 1;
            goto done;
            break;

        case 0xD5:
        case 0xD6:
        case 0xD7:

            len = cbor_encoding_size(self->data);
            if (size < len) goto error;

            buffer[0] = self->tag;

            if (!dtn_cbor_encode(self->data, 
                buffer + 1, size -1, next)) goto error;

            break;

        case 0xd9:

            len = cbor_encoding_size(self->data);
            if (size < len + 3) goto error;

            buffer[0] = 0xd9;
            buffer[1] = self->nbr_uint >> 8;
            buffer[2] = self->nbr_uint;

            if (!dtn_cbor_encode(self->data, 
                buffer + 3, size - 3, next)) goto error;

            break;

    case 0xdA:

            len = cbor_encoding_size(self->data);
            if (size < len + 5) goto error;

            buffer[0] = 0xdA;
            buffer[1] = self->nbr_uint >> 24;
            buffer[2] = self->nbr_uint >> 16;
            buffer[3] = self->nbr_uint >> 8;
            buffer[4] = self->nbr_uint;

            if (!dtn_cbor_encode(self->data, 
                buffer + 5, size - 5, next)) goto error;

            break;

    case 0xdB:

            len = cbor_encoding_size(self->data);
            if (size < len + 9) goto error;

            buffer[0] = 0xdb;
            buffer[1] = self->nbr_uint >> 56;
            buffer[2] = self->nbr_uint >> 48;
            buffer[3] = self->nbr_uint >> 40;
            buffer[4] = self->nbr_uint >> 32;
            buffer[5] = self->nbr_uint >> 24;
            buffer[6] = self->nbr_uint >> 16;
            buffer[7] = self->nbr_uint >> 8;
            buffer[8] = self->nbr_uint;

            if (!dtn_cbor_encode(self->data, 
                buffer + 9, size - 9, next)) goto error;

            break;
        default:
            goto error;
            break;
    }

done:
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool encode_simple(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    if (!self || !buffer || size < 1 || !next) goto error;

    if (self->type != DTN_CBOR_SIMPLE) goto error;

    switch (self->tag){

        case 0xE0:
        case 0xE1:
        case 0xE2:
        case 0xE3:
        case 0xE4:
        case 0xE5:
        case 0xE6:
        case 0xE7:
        case 0xE8:
        case 0xE9:
        case 0xEA:
        case 0xEB:
        case 0xEC:
        case 0xED:
        case 0xEE:
        case 0xEF:
        case 0xF0:
        case 0xF1:
        case 0xF2:
        case 0xF3:

            if (size < 1) goto error;
            buffer[0] = self->tag;
            buffer[0] |= self->nbr_uint;
            *next = (uint8_t*) buffer + 1;
            goto done;
            break;

        case 0xF8:

            if (size < 2) goto error;
            buffer[0] = self->tag;
            buffer[1] = self->nbr_uint;
            *next = (uint8_t*) buffer + 2;
            break;

        case 0xF9:

            if (size < 3) goto error;
            buffer[0] = self->tag;
            buffer[1] = self->nbr_uint >> 8;
            buffer[2] = self->nbr_uint;
            *next = (uint8_t*) buffer + 3;

            break;

    case 0xFA:

            if (size < 5) goto error;
            buffer[0] = self->tag;
            buffer[1] = self->nbr_uint >> 24;
            buffer[2] = self->nbr_uint >> 16;
            buffer[3] = self->nbr_uint >> 8;
            buffer[4] = self->nbr_uint;
            *next  = (uint8_t*) buffer + 5;
            break;

    case 0xfB:

            if (size < 9) goto error;
            buffer[0] = self->tag;
            buffer[1] = self->nbr_uint >> 56;
            buffer[2] = self->nbr_uint >> 48;
            buffer[3] = self->nbr_uint >> 40;
            buffer[4] = self->nbr_uint >> 32;
            buffer[5] = self->nbr_uint >> 24;
            buffer[6] = self->nbr_uint >> 16;
            buffer[7] = self->nbr_uint >> 8;
            buffer[8] = self->nbr_uint;
            *next = (uint8_t*) buffer + 9;
            break;
        default:
            goto error;
            break;
    }

done:
    return true;
error:
    return false;
}


/*----------------------------------------------------------------------------*/

static bool encode_float(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    uint8_t buf[100] = {0};
    if (!self || !buffer || size < 1 || !next) goto error;

    uint64_t len = snprintf((char*)buf, 100, "%f", self->nbr_float);
    if (len + 1 > size) goto error;

    buffer[0] = 0xfa;

    for (uint64_t i = 0; i < len; i++){
        buffer[i + 1] = buf[i];
    }
    *next = buffer + len;
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool encode_double(const dtn_cbor *self, 
    uint8_t *buffer, size_t size, uint8_t **next){

    uint8_t buf[100] = {0};
    if (!self || !buffer || size < 1 || !next) goto error;

    uint64_t len = snprintf((char*)buf, 100, "%g", self->nbr_float);
    if (len + 1 > size) goto error;

    buffer[0] = 0xfb;

    for (uint64_t i = 0; i < len; i++){
        buffer[i + 1] = buf[i];
    }
    *next = buffer + len;
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_encode(
    const dtn_cbor *value,
    uint8_t *buffer, 
    size_t size,
    uint8_t **next){

    if (!buffer || !value || size < 1 || !next) goto error;

    switch(value->type){

        case DTN_CBOR_UNDEF:

            if (size < 1) goto error;
            buffer[0] = 0xf7;
            *next = (uint8_t*) buffer + 1;
            break;

        case DTN_CBOR_FALSE:

            if (size < 1) goto error;
            buffer[0] = 0xf4;
            *next = (uint8_t*) buffer + 1;
            break;

        case DTN_CBOR_TRUE:

            if (size < 1) goto error;
            buffer[0] = 0xf5;
            *next = (uint8_t*) buffer + 1;
            break;

        case DTN_CBOR_NULL:

            if (size < 1) goto error;
            buffer[0] = 0xf6;
            *next = (uint8_t*) buffer + 1;
            break;

        case DTN_CBOR_UINT64:
            return encode_uint(value, buffer, size, next);
            break;

        case DTN_CBOR_INT64:
            return encode_int(value, buffer, size, next);
            break;

        case DTN_CBOR_STRING:
            return encode_string(value, buffer, size, next);
            break;

        case DTN_CBOR_UTF8:
            return encode_uft8(value, buffer, size, next);
            break;

        case DTN_CBOR_ARRAY:
            return encode_array(value, buffer, size, next);
            break;

        case DTN_CBOR_MAP:
            return encode_map(value, buffer, size, next);
            break;

        case DTN_CBOR_DATE_TIME:
            return encode_date_time(value, buffer, size, next);
            break;

        case DTN_CBOR_DATE_TIME_EPOCH:
            return encode_date_time_epoch(value, buffer, size, next);
            break;

        case DTN_CBOR_UBIGNUM:
            return encode_ubignum(value, buffer, size, next);
            break;

        case DTN_CBOR_IBIGNUM:
            return encode_ibignum(value, buffer, size, next);
            break;

        case DTN_CBOR_DEC_FRACTION:
            return encode_fraction(value, buffer, size, next);
            break;

        case DTN_CBOR_BIGFLOAT:
            return encode_bigfloat(value, buffer, size, next);
            break;

        case DTN_CBOR_TAG:
            return encode_tag(value, buffer, size, next);
            break;

        case DTN_CBOR_SIMPLE:
            return encode_simple(value, buffer, size, next);
            break;

        case DTN_CBOR_FLOAT:
            return encode_float(value, buffer, size, next);
            break;

        case DTN_CBOR_DOUBLE:
            return encode_double(value, buffer, size, next);
            break;
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_encode_array_of_indefinite_length(
    const dtn_cbor *value,
    uint8_t *buffer, 
    size_t size,
    uint8_t **next){

    if (!value || !buffer || size < 2 || !next) goto error;

    if (!dtn_cbor_is_array(value)) goto error;

    uint64_t count = dtn_cbor_array_count(value);
    uint8_t *ptr = NULL;

    // encode the array header 
    buffer[0] = 0x9F;
    ptr = buffer + 1;

    for (uint64_t i = 0; i < count; i++){

        if (!dtn_cbor_encode(
            dtn_cbor_array_get(value, i),
            ptr, 
            size - (ptr - buffer),
            &ptr)) goto error;
    
    }

    if (ptr - buffer == (int64_t) size) goto error;
    
    ptr[0] = 0xFF;
    *next = ptr + 1;
    
    return true;
error:
    return false;
}
/*
 *      ------------------------------------------------------------------------
 *
 *      #map
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor *dtn_cbor_map(){

    dtn_cbor *map = dtn_cbor_create(DTN_CBOR_MAP);
    if (!map) goto error;

    map->data = dtn_dict_create(dtn_cbor_dict_config(255));
    if (!map->data) goto error;

    return map;
error:
    cbor_free(map);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_map(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_MAP) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_map_set(dtn_cbor *map, dtn_cbor *key, dtn_cbor *val){

    if (!map || !key || !val) goto error;

    if (map->type != DTN_CBOR_MAP) goto error;

    return dtn_dict_set(map->data, key, val, NULL);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_map_set_string(dtn_cbor *map, const char *key, dtn_cbor *val){

    dtn_cbor *k = NULL;

    if (!map || !key || !val) goto error;

    if (map->type != DTN_CBOR_MAP) goto error;

    k = dtn_cbor_create(DTN_CBOR_STRING);
    if (!k) goto error;

    k->string = dtn_string_dup(key);

    if (!dtn_dict_set(map->data, k, val, NULL)) goto error;
    return true;
error:
    k = cbor_free(k);
    return false;
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_map_get(const dtn_cbor *map, const dtn_cbor *key){

    if (!map || !key) return NULL;
    if (map->type != DTN_CBOR_MAP) return NULL;
    return dtn_dict_get(map->data, key);
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_map_get_string(const dtn_cbor *map, const char *key){

    dtn_cbor *k = NULL;

    if (!map || !key) goto error;

    if (map->type != DTN_CBOR_MAP) goto error;

    k = dtn_cbor_create(DTN_CBOR_STRING);
    if (!k) goto error;

    k->string = dtn_string_dup(key);
    
    dtn_cbor *out = dtn_dict_get(map->data, k);
    k = cbor_free(k);
    
    return out;
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_map_count(const dtn_cbor *map){

    if (map->type != DTN_CBOR_MAP) return 0;

    return dtn_dict_count(map->data);
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_map_for_each(dtn_cbor *map,
    void *data,
    bool (*function)(const void *key, void *val, void *data)){

    if (!map || !function) goto error;

    if (map->type != DTN_CBOR_MAP) goto error;

    return dtn_dict_for_each(map->data, data, function);
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #array
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor *dtn_cbor_array(){

    dtn_cbor *out = dtn_cbor_create(DTN_CBOR_ARRAY);
    if (!out) goto error;

    out->data = dtn_linked_list_create((dtn_list_config){
        .item.copy = cbor_copy,
        .item.clear = cbor_clear,
        .item.free = cbor_free,
        .item.dump = cbor_dump
    });

    if (!out->data)
        goto error;

    return out;
error:
    cbor_free(out);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_array(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_ARRAY) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_array_push(dtn_cbor *self, dtn_cbor *val){

    if (!self || !val) return false;

    if (self->type != DTN_CBOR_ARRAY) return false;

    return dtn_list_push(self->data, val);
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_array_pop_queue(dtn_cbor *self){

    if (self->type != DTN_CBOR_ARRAY) return NULL;

    dtn_cbor *out = dtn_list_remove(self->data, 1);
    return out;
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_array_pop_stack(dtn_cbor *self){

    if (self->type != DTN_CBOR_ARRAY) return NULL;

    return dtn_list_pop(self->data);
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_array_for_each(dtn_cbor *self, 
    void *data,
    bool (*function)(void *item, void *data)){

    if (!self || !function) return false;

    if (self->type != DTN_CBOR_ARRAY) return false;

    return dtn_list_for_each(self->data, data, function);
}

/*----------------------------------------------------------------------------*/

const dtn_cbor *dtn_cbor_array_get(const dtn_cbor *self, uint64_t index){

    if (!self || self->type != DTN_CBOR_ARRAY) return NULL;

    return dtn_list_get(self->data, index + 1);
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_array_count(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_ARRAY) return 0;
    return dtn_list_count(self->data);
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #string
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor *dtn_cbor_string(const char *string){

    dtn_cbor *self = dtn_cbor_create(DTN_CBOR_STRING);
    if (!self) goto error;
    if (string){
        self->string = dtn_string_dup(string);
        self->nbr_uint = strlen(self->string);
    }
    return self;

error:
    cbor_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_string(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_STRING) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

const char *dtn_cbor_get_string(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_STRING) goto error;

    return self->string;
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_string(dtn_cbor *self, const char *string){

    if (!self || self->type != DTN_CBOR_STRING) goto error;

    self->string = dtn_data_pointer_free(self->string);
    self->string = dtn_string_dup(string);
    self->nbr_uint = strlen(self->string);

    return true;
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #utf8
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor *dtn_cbor_utf8(const uint8_t *buffer , size_t size){

    dtn_cbor *self = dtn_cbor_create(DTN_CBOR_UTF8);
    if (!self) goto error;

    if (!buffer || size < 1) goto done;

    if (!dtn_utf8_validate_sequence(buffer, size)) goto error;

    if (buffer){

        self->nbr_uint = size;
        self->bytes = calloc(size, sizeof(uint8_t));
        if (!self->bytes) goto error;
    
        memcpy(self->bytes, buffer, size);
    }

done:
    return self;
error:
    cbor_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_utf8(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_UTF8) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_get_utf8(dtn_cbor *self, uint8_t **buffer, size_t *size){

    if (!self || !buffer || !size) goto error;
    if (self->type != DTN_CBOR_UTF8) goto error;

    *buffer = self->bytes;
    *size = self->nbr_uint;

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_utf8(dtn_cbor *self, const uint8_t *buffer, size_t size){

    if (!self || !buffer || !size) goto error;
    if (self->type != DTN_CBOR_UTF8) goto error;

    if (!dtn_utf8_validate_sequence(buffer, size)) goto error;

    self->bytes = dtn_data_pointer_free(self->bytes);
    self->bytes = calloc(size, sizeof(uint8_t));
    if (!self->bytes) goto error;
    self->nbr_uint = size;
    memcpy(self->bytes, buffer, size);
    return true;
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #literals
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor *dtn_cbor_true(){

    return dtn_cbor_create(DTN_CBOR_TRUE);
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_true(const dtn_cbor *self){

    if (self->type == DTN_CBOR_TRUE) return true;
    return false;

}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_false(){

    return dtn_cbor_create(DTN_CBOR_FALSE);
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_false(const dtn_cbor *self){

    if (self->type == DTN_CBOR_FALSE) return true;
    return false;

}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_null(){

    return dtn_cbor_create(DTN_CBOR_NULL);
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_null(const dtn_cbor *self){

    if (self->type == DTN_CBOR_NULL) return true;
    return false;

}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_undef(){

    return dtn_cbor_create(DTN_CBOR_UNDEF);
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_undef(const dtn_cbor *self){

    if (self->type == DTN_CBOR_UNDEF) return true;
    return false;

}

/*
 *      ------------------------------------------------------------------------
 *
 *      #numbers
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor *dtn_cbor_uint(uint64_t value){

    dtn_cbor *self = dtn_cbor_create(DTN_CBOR_UINT64);
    if (!self) return NULL;

    self->nbr_uint = value;
    return self;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_uint(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_UINT64) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_get_uint(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_UINT64) return 0;
    return self->nbr_uint;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_uint(dtn_cbor *self, uint64_t value){

    if (!self || self->type != DTN_CBOR_UINT64) return false;
    self->nbr_uint = value;
    return true;
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_int(int64_t value){

    if (value > 0) return NULL;

    dtn_cbor *self = dtn_cbor_create(DTN_CBOR_INT64);
    if (!self) return NULL;

    self->nbr_int = value;
    return self;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_int(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_INT64) return false;
    return true;
}


/*----------------------------------------------------------------------------*/

int64_t dtn_cbor_get_int(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_INT64) return 0;
    return self->nbr_int;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_int(dtn_cbor *self, int64_t value){

    if (!self || self->type != DTN_CBOR_INT64) return false;
    if (value > 0) return false;

    self->nbr_int = value;
    return true;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      #time
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor *dtn_cbor_time(const char *timestamp){

    dtn_cbor *self = dtn_cbor_create(DTN_CBOR_DATE_TIME);
    if (!self) return NULL;

    if (timestamp){
        self->string = dtn_string_dup(timestamp);
        self->nbr_uint = strlen(timestamp);
    }

    
    return self;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_time(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_DATE_TIME) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

const char *dtn_cbor_get_time(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_DATE_TIME) return NULL;
    return self->string;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_time(dtn_cbor *self, const char *timestamp){

    if (!self || self->type != DTN_CBOR_DATE_TIME) return false;

    self->string = dtn_data_pointer_free(self->string);

    if (timestamp){
        self->string = dtn_string_dup(timestamp);
        self->nbr_uint = strlen(timestamp);
    }

    return true;

}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_time_epoch(uint64_t value){

    dtn_cbor *self = dtn_cbor_create(DTN_CBOR_DATE_TIME_EPOCH);
    if (!self) return NULL;

    self->nbr_uint = value;
    return self;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_time_epoch(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_DATE_TIME_EPOCH) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_get_time_epoch(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_DATE_TIME_EPOCH) return 0;
    return self->nbr_uint;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_time_epoch(dtn_cbor *self, uint64_t value){

    if (!self || self->type != DTN_CBOR_DATE_TIME_EPOCH) return false;
    self->nbr_uint = value;
    return true;

}

/*
 *      ------------------------------------------------------------------------
 *
 *      #bignum
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor *dtn_cbor_ubignum(const char *string){

    dtn_cbor *self = dtn_cbor_create(DTN_CBOR_UBIGNUM);
    if (!self) goto error;

    if (string){
        self->string = dtn_string_dup(string);
        self->nbr_uint = strlen(self->string);
    }

    
    return self;

error:
    cbor_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_ubignum(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_UBIGNUM) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

const char *dtn_cbor_get_ubignum(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_UBIGNUM) goto error;

    return self->string;
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_ubignum(dtn_cbor *self, const char *string){

    if (!self || self->type != DTN_CBOR_UBIGNUM) goto error;

    self->string = dtn_data_pointer_free(self->string);
    if (string){
        self->string = dtn_string_dup(string);
        self->nbr_uint = strlen(self->string);
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_ibignum(const char *string){

    dtn_cbor *self = dtn_cbor_create(DTN_CBOR_IBIGNUM);
    if (!self) goto error;

    if (string){
        self->string = dtn_string_dup(string);
        self->nbr_uint = strlen(self->string);
    }
    return self;

error:
    cbor_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_ibignum(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_IBIGNUM) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

const char *dtn_cbor_get_ibignum(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_IBIGNUM) goto error;

    return self->string;
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_ibignum(dtn_cbor *self, const char *string){

    if (!self || self->type != DTN_CBOR_IBIGNUM) goto error;

    self->string = dtn_data_pointer_free(self->string);
    if (string){
        self->string = dtn_string_dup(string);
        self->nbr_uint = strlen(self->string);
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_dec_fraction(dtn_cbor *array){

    dtn_cbor *self = dtn_cbor_create(DTN_CBOR_DEC_FRACTION);
    if (!self) return NULL;

    if (array){
        if (array->type != DTN_CBOR_ARRAY) goto error;
        self->data = array;
    }

    return self;
    
error:
    cbor_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_dec_fraction(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_DEC_FRACTION) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

const dtn_cbor *dtn_cbor_get_dec_fraction(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_DEC_FRACTION) return NULL;
    return self->data;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_dec_fraction(dtn_cbor *self, dtn_cbor *array){

    if (!self || !array) goto error;
    if (self->type != DTN_CBOR_DEC_FRACTION) goto error;
    if (array->type != DTN_CBOR_ARRAY) goto error;

    self->data = cbor_free(self->data);
    self->data = array;
    return true;
error:
    return false;

}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_bigfloat(dtn_cbor *array){

    dtn_cbor *self = dtn_cbor_create(DTN_CBOR_BIGFLOAT);
    if (!self) return NULL;

    if (array){
        if (array->type != DTN_CBOR_ARRAY) goto error;
        self->data = array;
    }

    return self;
    
error:
    cbor_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_bigfloat(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_BIGFLOAT) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

const dtn_cbor *dtn_cbor_get_bigfloat(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_BIGFLOAT) return NULL;
    return self->data;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_bigfloat(dtn_cbor *self, dtn_cbor *array){

    if (!self || !array) goto error;
    if (self->type != DTN_CBOR_BIGFLOAT) goto error;
    if (array->type != DTN_CBOR_ARRAY) goto error;

    self->data = cbor_free(self->data);
    self->data = array;
    return true;
error:
    return false;

}

/*
 *      ------------------------------------------------------------------------
 *
 *      #tag
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor *dtn_cbor_tag(uint64_t tag){

    dtn_cbor *self = dtn_cbor_create(DTN_CBOR_TAG);
    if (!self) goto error;

    self->tag = tag;
    return self;
error:
    cbor_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_tag(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_TAG) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_get_tag(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_TAG) return 0;
    return self->tag;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_get_tag_value(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_TAG) return 0;
    return self->nbr_uint;
}

/*----------------------------------------------------------------------------*/

const dtn_cbor *dtn_cbor_get_tag_data(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_TAG) return NULL;
    return self->data;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_tag(dtn_cbor *self, uint64_t tag){

    if (!self || self->type != DTN_CBOR_TAG) return false;
    self->tag = tag;
    return true;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_tag_data(dtn_cbor *self, dtn_cbor *data){

    if (!self || self->type != DTN_CBOR_TAG) return false;
    self->data = cbor_free(self->data);
    self->data = data;
    return true;

}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_tag_value(dtn_cbor *self, uint64_t val){

    if (!self || self->type != DTN_CBOR_TAG) return false;
    self->nbr_uint = val;
    return true;
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_simple(uint64_t nbr){

    dtn_cbor *self = dtn_cbor_create(DTN_CBOR_SIMPLE);
    if (!self) goto error;

    self->tag = nbr;
    return self;
error:
    cbor_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_simple(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_SIMPLE) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_get_simple_value(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_SIMPLE) return 0;
    return self->nbr_uint;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_get_simple(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_SIMPLE) return 0;
    return self->tag;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_simple(dtn_cbor *self, uint64_t nbr){

    if (!self || self->type != DTN_CBOR_SIMPLE) return false;
    self->tag = nbr;
    return true;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_simple_value(dtn_cbor *self, uint64_t nbr){

    if (!self || self->type != DTN_CBOR_SIMPLE) return false;
    self->nbr_uint = nbr;
    return true;
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_float(float nbr){

    dtn_cbor *self = dtn_cbor_create(DTN_CBOR_FLOAT);
    if (!self) return NULL;

    self->nbr_float = nbr;
    return self;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_float(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_FLOAT) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

float dtn_cbor_get_float(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_FLOAT) return 0;
    return self->nbr_float;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_float(dtn_cbor *self, float nbr){

    if (!self || self->type != DTN_CBOR_FLOAT) return false;
    self->nbr_float = nbr;
    return true;
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_cbor_double(double nbr){

    dtn_cbor *self = dtn_cbor_create(DTN_CBOR_DOUBLE);
    if (!self) return NULL;

    self->nbr_double = nbr;
    return self;
}

/*----------------------------------------------------------------------------*/

bool dtn_cbor_is_double(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_DOUBLE) return false;
    return true;
}

/*----------------------------------------------------------------------------*/

double dtn_cbor_get_double(const dtn_cbor *self){

    if (!self || self->type != DTN_CBOR_DOUBLE) return 0;
    return self->nbr_double;
}


/*----------------------------------------------------------------------------*/

bool dtn_cbor_set_double(dtn_cbor *self, double nbr){

    if (!self || self->type != DTN_CBOR_DOUBLE) return false;
    self->nbr_double = nbr;
    return true;
}