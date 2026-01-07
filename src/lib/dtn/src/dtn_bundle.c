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
        @file           dtn_bundle.c
        @author         Töpfer, Markus

        @date           2025-12-15


        ------------------------------------------------------------------------
*/
#include "../include/dtn_bundle.h"

#include <stdlib.h>
#include <dtn_base/dtn_utils.h>
#include <dtn_base/dtn_data_function.h>
#include <dtn_base/dtn_buffer.h>
#include <dtn_base/dtn_crc16.h>
#include <dtn_base/dtn_crc32.h>
#include <dtn_base/dtn_dump.h>

#include <dtn_core/dtn_aes_key_wrap.h>
#include <dtn_core/dtn_hmac.h>

#include <openssl/rand.h>
#include <openssl/evp.h>


/*----------------------------------------------------------------------------*/

struct dtn_bundle {

    dtn_cbor *data;   

};

/*----------------------------------------------------------------------------*/

dtn_bundle *dtn_bundle_create(){

    dtn_bundle *self = calloc(1, sizeof(dtn_bundle));
    if (!self) goto error;

    return self;
error:
    dtn_bundle_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_bundle *dtn_bundle_free(dtn_bundle *self){

    if (!self) return self;

    self->data = dtn_cbor_free(self->data);

    self = dtn_data_pointer_free(self);
    return NULL;

}

/*
 *      ------------------------------------------------------------------------
 *
 *      DE/ENCODER
 *
 *      ------------------------------------------------------------------------
 */

static bool check_crc(const dtn_cbor *block, 
    const char *alg, const dtn_cbor *crc){

    bool crc_check = false;
    dtn_buffer *buffer = NULL;

    if (!block || !alg || !crc) goto error;

    uint8_t *next = NULL;
    uint64_t length = dtn_cbor_encoding_size(block);
    buffer = dtn_buffer_create(length);

    if (!dtn_cbor_encode(block, buffer->start, buffer->capacity, &next))
        goto error;

    buffer->length = next - buffer->start;

    uint8_t *crc_num = NULL;
    size_t size = 0;

    if (!dtn_cbor_get_byte_string(crc, &crc_num, &size))
        goto error;

    if (!crc_num) goto error;
    if (size < 2) goto error;

    if (0 == strcmp(alg, DTN_BUNDLE_CRC16)){

        buffer->start[buffer->length] = 0x00;
        buffer->start[buffer->length - 1] = 0x00;
        buffer->start[buffer->length - 2] = 0x00;;

        uint16_t crc_sum = crc16x25(buffer->start, buffer->length);
        uint16_t expect = crc_num[0] << 8;
        expect += crc_num[1];

        if (crc_sum == expect){

            crc_check = true;
        } 

    } else if (0 == strcmp(alg, DTN_BUNDLE_CRC32)){

        if (size < 4) goto error;

        buffer->start[buffer->length] = 0x00;
        buffer->start[buffer->length - 1] = 0x00;
        buffer->start[buffer->length - 2] = 0x00;
        buffer->start[buffer->length - 3] = 0x00;
        buffer->start[buffer->length - 4] = 0x00;

        uint32_t expect = crc_num[0] << 24;
        expect += crc_num[1] << 16;
        expect += crc_num[2] << 8;
        expect += crc_num[3];

        uint32_t crc_sum = dtn_crc32c(buffer->start, buffer->length);

        if (crc_sum == expect){

            crc_check = true;
        } 

    } else {

        goto error;
    }

    if (!crc_check) dtn_log_error("CRC check failed.");

    buffer = dtn_buffer_free(buffer);
    return crc_check;
error:
    dtn_buffer_free(buffer);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool check_primary_block(dtn_bundle *self){

    if (!self) goto error;

    const dtn_cbor *data = dtn_cbor_array_get(self->data, 0);
    if (!dtn_cbor_is_array(data)) goto error;

    uint64_t count = dtn_cbor_array_count(data);
    if (count < 8) goto error;

    const dtn_cbor *version = dtn_cbor_array_get(data, 0);
    if (!dtn_cbor_is_uint(version)) goto error;

    if (0x07 != dtn_cbor_get_uint(version)) goto error;

    const dtn_cbor *flags = dtn_cbor_array_get(data, 1);
    if (!dtn_cbor_is_uint(flags)) goto error;

    uint64_t flag = dtn_cbor_get_uint(flags);

    const dtn_cbor *crc_type = dtn_cbor_array_get(data, 2);
    if (!dtn_cbor_is_uint(crc_type)) goto error;

    uint64_t crc_flags = dtn_cbor_get_uint(crc_type);
    const char *crc_alg = NULL;

    switch (crc_flags){

        case 0x00:
            crc_alg = NULL;
            break;
        case 0x01:
            crc_alg = DTN_BUNDLE_CRC16;
            break;
        case 0x02:
            crc_alg = DTN_BUNDLE_CRC32;
            break;
        default:
            goto error;

    }

    const dtn_cbor *dest = dtn_cbor_array_get(data, 3);
    if (!dtn_cbor_is_string(dest)) goto error;

    const dtn_cbor *source = dtn_cbor_array_get(data, 4);
    if (!dtn_cbor_is_string(source)) goto error;

    const dtn_cbor *report = dtn_cbor_array_get(data, 5);
    if (!dtn_cbor_is_string(report)) goto error;
    
    const dtn_cbor *timestamp = dtn_cbor_array_get(data, 6);
    if (!dtn_cbor_is_array(timestamp)) goto error;
    if (2 != dtn_cbor_array_count(timestamp)) goto error;

    const dtn_cbor *lifetime = dtn_cbor_array_get(data, 7);
    if (!dtn_cbor_is_uint(lifetime)) goto error;

    const dtn_cbor *fragment = NULL;
    const dtn_cbor *total = NULL;
    const dtn_cbor *crc = NULL;

    if (flag & 0x000001){

        if (count < 10) goto error;

        fragment = dtn_cbor_array_get(data, 8);
        if (!dtn_cbor_is_uint(fragment)) goto error;

        total = dtn_cbor_array_get(data, 9);
        if (!dtn_cbor_is_uint(total)) goto error;

        if (crc_alg){

            crc = dtn_cbor_array_get(data, 10);
            if (!check_crc(data, crc_alg, crc)) goto error;
        }

    } else {

        if (crc_alg){

            crc = dtn_cbor_array_get(data, 8);
            if (!check_crc(data, crc_alg, crc)) goto error;
        }
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool check_canonical_block(const dtn_cbor *array){

    if (!dtn_cbor_is_array(array)) goto error;

    const dtn_cbor *code = NULL;
    const dtn_cbor *num = NULL;
    const dtn_cbor *flags = NULL;
    const dtn_cbor *crc_type = NULL;
    const dtn_cbor *data = NULL;
    const dtn_cbor *crc = NULL;

    uint64_t count = dtn_cbor_array_count(array);
    if (count < 5) goto error;

    code = dtn_cbor_array_get(array, 0);
    if (!dtn_cbor_is_uint(code)) goto error;

    num = dtn_cbor_array_get(array, 1);
    if (!dtn_cbor_is_uint(num)) goto error;

    flags = dtn_cbor_array_get(array, 2);
    if (!dtn_cbor_is_uint(flags)) goto error;

    crc_type = dtn_cbor_array_get(array, 3);
    if (!dtn_cbor_is_uint(crc_type)) goto error;

    uint64_t crc_flags = dtn_cbor_get_uint(crc_type);
    const char *crc_alg = NULL;

    switch (crc_flags){

        case 0x00:
            crc_alg = NULL;
            break;
        case 0x01:
            crc_alg = DTN_BUNDLE_CRC16;
            break;
        case 0x02:
            crc_alg = DTN_BUNDLE_CRC32;
            break;
        default:
            goto error;

    }

    data = dtn_cbor_array_get(array, 4);
    if (!dtn_cbor_is_string(data)) goto error;

    if (crc_alg){

        crc = dtn_cbor_array_get(array, 5);
        if (!check_crc(array, crc_alg, crc)) goto error;
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool check_canonical_blocks(const dtn_bundle *self){

    if (!self) goto error;

    uint64_t count = dtn_cbor_array_count(self->data);

    for (uint64_t i = 1; i < count; i++){

        dtn_cbor *item = dtn_cbor_array_get(self->data, i);

        if (!check_canonical_block(item))
            goto error;

    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool check_payload_block(dtn_bundle *self){

    if (!self) goto error;

    // NOTE CRC checks and item count already done in check_canonical_blocks

    const dtn_cbor *payload = NULL;
    uint64_t count = dtn_cbor_array_count(self->data);

    payload = dtn_cbor_array_get(self->data, count - 1);
    if (!dtn_cbor_is_array(payload)) goto error;

    const dtn_cbor *code = dtn_cbor_array_get(payload, 0);
    if (!dtn_cbor_is_uint(code)) goto error;
    if (1 != dtn_cbor_get_uint(code)) goto error;

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool check_primary_bytes(uint8_t *start , size_t size){

    uint8_t *ptr = start;
    dtn_cbor *item = NULL;

    uint64_t flags = 0;
    uint64_t crc = 0;

    switch(ptr[0]){
        case 0x88:
        case 0x89:
        case 0x8A:
        case 0x8B:
            break;
        default:
            goto error;
    }

    ptr++;

    if ((int64_t) size - (ptr - start) <= 0) return true;
    
    dtn_cbor_match match = dtn_cbor_decode(ptr, size - (ptr - start), &item, &ptr);
    if (match == DTN_CBOR_MATCH_PARTIAL) return true;
    if (match != DTN_CBOR_MATCH_FULL) goto error;
    if (!dtn_cbor_is_uint(item)) goto error;
    if (0x07 != dtn_cbor_get_uint(item)) goto error;
    item = dtn_cbor_free(item);

    if ((int64_t) size - (ptr - start) <= 0) return true;

    match = dtn_cbor_decode(ptr, size - (ptr - start), &item, &ptr);
    if (match == DTN_CBOR_MATCH_PARTIAL) return true;
    if (match != DTN_CBOR_MATCH_FULL) goto error;
    if (!dtn_cbor_is_uint(item)) goto error;
    flags = dtn_cbor_get_uint(item);
    item = dtn_cbor_free(item);

    if ((int64_t) size - (ptr - start) <= 0) return true;

    match = dtn_cbor_decode(ptr, size - (ptr - start), &item, &ptr);
    if (match == DTN_CBOR_MATCH_PARTIAL) return true;
    if (match != DTN_CBOR_MATCH_FULL) goto error;
    if (!dtn_cbor_is_uint(item)) goto error;
    crc = dtn_cbor_get_uint(item);
    item = dtn_cbor_free(item);

    if ((int64_t) size - (ptr - start) <= 0) return true;

    match = dtn_cbor_decode(ptr, size - (ptr - start), &item, &ptr);
    if (match == DTN_CBOR_MATCH_PARTIAL) return true;
    if (match != DTN_CBOR_MATCH_FULL) goto error;
    if (!dtn_cbor_is_string(item)) goto error;
    item = dtn_cbor_free(item);

    if ((int64_t) size - (ptr - start) <= 0) return true;

    match = dtn_cbor_decode(ptr, size - (ptr - start), &item, &ptr);
    if (match == DTN_CBOR_MATCH_PARTIAL) return true;
    if (match != DTN_CBOR_MATCH_FULL) goto error;
    if (!dtn_cbor_is_string(item)) goto error;
    item = dtn_cbor_free(item);

    if ((int64_t) size - (ptr - start) <= 0) return true;

    match = dtn_cbor_decode(ptr, size - (ptr - start), &item, &ptr);
    if (match == DTN_CBOR_MATCH_PARTIAL) return true;
    if (match != DTN_CBOR_MATCH_FULL) goto error;
    if (!dtn_cbor_is_string(item)) goto error;
    item = dtn_cbor_free(item);

    if ((int64_t) size - (ptr - start) <= 0) return true;

    match = dtn_cbor_decode(ptr, size - (ptr - start), &item, &ptr);
    if (match == DTN_CBOR_MATCH_PARTIAL) return true;
    if (match != DTN_CBOR_MATCH_FULL) goto error;
    if (!dtn_cbor_is_array(item)) goto error;
    if (dtn_cbor_array_count(item) != 2) goto error;
    item = dtn_cbor_free(item);

    if ((int64_t) size - (ptr - start) <= 0) return true;

    match = dtn_cbor_decode(ptr, size - (ptr - start), &item, &ptr);
    if (match == DTN_CBOR_MATCH_PARTIAL) return true;
    if (match != DTN_CBOR_MATCH_FULL) goto error;
    if (!dtn_cbor_is_uint(item)) goto error;
    item = dtn_cbor_free(item);

    if ((int64_t) size - (ptr - start) <= 0) return true;

    if (flags &0x01){

        match = dtn_cbor_decode(ptr, size - (ptr - start), &item, &ptr);
        if (match == DTN_CBOR_MATCH_PARTIAL) return true;
        if (match != DTN_CBOR_MATCH_FULL) goto error;
        if (!dtn_cbor_is_uint(item)) goto error;
        item = dtn_cbor_free(item);

        if ((int64_t) size - (ptr - start) <= 0) return true;

        match = dtn_cbor_decode(ptr, size - (ptr - start), &item, &ptr);
        if (match == DTN_CBOR_MATCH_PARTIAL) return true;
        if (match != DTN_CBOR_MATCH_FULL) goto error;
        if (!dtn_cbor_is_uint(item)) goto error;
        item = dtn_cbor_free(item);

        if ((int64_t) size - (ptr - start) <= 0) return true;

        if (crc > 0){

            match = dtn_cbor_decode(ptr, size - (ptr - start), &item, &ptr);
            if (match == DTN_CBOR_MATCH_PARTIAL) return true;
            if (match != DTN_CBOR_MATCH_FULL) goto error;
            if (!dtn_cbor_is_array(item)) goto error;
            if (2 != dtn_cbor_array_count(item)) goto error;
            item = dtn_cbor_free(item);

        }
   
    } else if (crc > 0){

        match = dtn_cbor_decode(ptr, size - (ptr - start), &item, &ptr);
        if (match == DTN_CBOR_MATCH_PARTIAL) return true;
        if (match != DTN_CBOR_MATCH_FULL) goto error;
        if (!dtn_cbor_is_array(item)) goto error;
        if (2 != dtn_cbor_array_count(item)) goto error;
        item = dtn_cbor_free(item);

    }

    dtn_cbor_free(item);
    return true;
error:
    dtn_cbor_free(item);
    return false;
}

/*----------------------------------------------------------------------------*/

dtn_cbor_match dtn_bundle_decode(
    const uint8_t *buffer, 
    size_t size,
    dtn_bundle **out, 
    uint8_t **next){

    dtn_bundle *bundle = NULL;

    if (!buffer || !out || !next) goto error;

    if (size < 1) goto error;
    if (buffer[0]!=0x9f) goto error;
    if (size == 1) return DTN_CBOR_MATCH_PARTIAL;

    bundle = dtn_bundle_create();
    bundle->data = dtn_cbor_array();

    uint8_t *ptr = (uint8_t*) buffer + 1;
    dtn_cbor_match match = DTN_CBOR_NO_MATCH;

    uint64_t counter = 0;
    uint8_t *start = ptr;

    while ((int64_t) size > ptr - buffer){

        dtn_cbor *item = NULL;
        start = ptr;
        if (counter == 0){

            // expect primary block
            switch(ptr[0]){

                case 0x88:
                case 0x89:
                case 0x8A:
                case 0x8B:
                    
                    // primary array detected

                    match = dtn_cbor_decode(ptr, 
                        size - (ptr - buffer), &item, &ptr);
                    
                    switch(match){

                        case DTN_CBOR_NO_MATCH:
                            goto error;

                        case DTN_CBOR_MATCH_PARTIAL:
                            *next = ptr;
                            if (!check_primary_bytes(start, (ptr - start)))
                                goto error;
                            bundle = dtn_bundle_free(bundle);
                            return match;

                        case DTN_CBOR_MATCH_FULL:

                            dtn_cbor_array_push(bundle->data, item);

                            if (!check_primary_block(bundle)){
                                fprintf(stdout, "\nprimary failed\n");
                                bundle = dtn_bundle_free(bundle);
                                goto error;
                            }

                            break;
                    }
                    break;

                default:
                    *next = ptr;
                    goto error;
            }
        
        } else {

            // expect canonical block
            switch (ptr[0]){

                case 0x85:
                case 0x86:
                    
                    // canonical block detected
                    
                    match = dtn_cbor_decode(
                        ptr, size- (ptr -buffer), &item, &ptr);

                    switch(match){

                        case DTN_CBOR_NO_MATCH:
                            goto error;

                        case DTN_CBOR_MATCH_PARTIAL:
                            *next = ptr;
                            bundle = dtn_bundle_free(bundle);
                            return match;

                        case DTN_CBOR_MATCH_FULL:

                            if (!check_canonical_block(item)){
                                fprintf(stdout, "\ncheck_canonical_block failed\n");
                                bundle = dtn_bundle_free(bundle);
                                item = dtn_cbor_free(item);
                                goto error;
                            }

                            dtn_cbor_array_push(bundle->data, item);
                            break;
                    }

                    break;

                default:
                    *next = ptr;
                    goto error;
            }
        }

        if ((int64_t) size == ptr - buffer){
            *next = ptr;
            bundle = dtn_bundle_free(bundle);
            return DTN_CBOR_MATCH_PARTIAL;
        }

        if (ptr[0] == 0xff){
            // bundle_close
            *next = ptr + 1;
            goto done;

        }

        counter++;
    }

done:
    counter = dtn_cbor_array_count(bundle->data);
    if (counter < 2) goto error;

    // we verify again to check for payload block
    if (!dtn_bundle_verify(bundle)) goto error;
    
    *out = bundle;
    return DTN_CBOR_MATCH_FULL;
error:
    if (next) *next = (uint8_t*) buffer;
    dtn_bundle_free(bundle);
    return DTN_CBOR_NO_MATCH;
}

/*----------------------------------------------------------------------------*/

static bool set_crc_primary(dtn_cbor *block){

    dtn_buffer *buffer = NULL;

    if (!block) goto error;
    
    uint64_t count = dtn_cbor_array_count(block);

    const dtn_cbor *crc_type = dtn_cbor_array_get(block, 2);
    if (!dtn_cbor_is_uint(crc_type)) goto error;

    uint64_t crc_flags = dtn_cbor_get_uint(crc_type);

    dtn_cbor *crc = NULL;

    switch (crc_flags){

        case 0x00:
            crc = NULL;
            goto done;
            break;
        case 0x01:

            if (count == 9){

                crc = dtn_cbor_array_get(block, 8);

            } else if (count == 11){

                crc = dtn_cbor_array_get(block, 10);
           
            } else {

                crc = dtn_cbor_string("12");
                if (!dtn_cbor_array_push(block, crc))
                    goto error;

            }

            break;

        case 0x02:

            if (count == 9){

                crc = dtn_cbor_array_get(block, 8);

            } else if (count == 11){

                crc = dtn_cbor_array_get(block, 10);
           
            } else {

                crc = dtn_cbor_string("1234");
                if (!dtn_cbor_array_push(block, crc))
                    goto error;

            }
            
            break;

        default:
            goto error;

    }

    uint8_t *next = NULL;
    uint64_t length = dtn_cbor_encoding_size(block);
    buffer = dtn_buffer_create(length);

    if (!dtn_cbor_encode(block, buffer->start, buffer->capacity, &next))
        goto error;

    buffer->length = next - buffer->start;

    uint32_t crc_sum32 = 0;
    uint16_t crc_sum16 = 0;
    char *bytes = NULL;

    switch (crc_flags){

        case 0x01:

            buffer->start[buffer->length] = 0x00;
            buffer->start[buffer->length - 1] = 0x00;
            buffer->start[buffer->length - 2] = 0x00;
            
            crc_sum16 = crc16x25(buffer->start, buffer->length);

            bytes = (char*) dtn_cbor_get_string(crc);
            if (!bytes) goto error;

            bytes[0] = crc_sum16 >> 8;
            bytes[1] = crc_sum16;

            break;
        case 0x02:

            buffer->start[buffer->length] = 0x00;
            buffer->start[buffer->length - 1] = 0x00;
            buffer->start[buffer->length - 2] = 0x00;
            buffer->start[buffer->length - 3] = 0x00;
            buffer->start[buffer->length - 4] = 0x00;
            
            crc_sum32 = dtn_crc32c(buffer->start, buffer->length);
            bytes = (char*) dtn_cbor_get_string(crc);
            if (!bytes) goto error;

            bytes[0] = crc_sum32 >> 24;
            bytes[1] = crc_sum32 >> 16;
            bytes[2] = crc_sum32 >> 8;
            bytes[3] = crc_sum32;
            break;
        default:
            goto error;

    }

done:
    dtn_buffer_free(buffer);
    return true;
error:
    dtn_buffer_free(buffer);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool set_crc_block(dtn_cbor *block){

    dtn_buffer *buffer = NULL;

    if (!block) goto error;
    
    uint64_t count = dtn_cbor_array_count(block);

    const dtn_cbor *crc_type = dtn_cbor_array_get(block, 3);
    if (!dtn_cbor_is_uint(crc_type)) goto error;

    uint64_t crc_flags = dtn_cbor_get_uint(crc_type);

    dtn_cbor *crc = NULL;

    switch (crc_flags){

        case 0x00:
            goto done;
            break;
        case 0x01:

            if (count == 6){

                crc = dtn_cbor_array_get(block, 5);
            
            } else {

                crc = dtn_cbor_string("12");
                if (!dtn_cbor_array_push(block, crc))
                    goto error;
            }
            
            break;
        case 0x02:


            if (count == 6){

                crc = dtn_cbor_array_get(block, 5);
            
            } else {

                crc = dtn_cbor_string("1234");
                if (!dtn_cbor_array_push(block, crc))
                    goto error;
            }
            
            break;
        default:
            goto error;

    }

    uint8_t *next = NULL;
    uint64_t length = dtn_cbor_encoding_size(block);
    buffer = dtn_buffer_create(length);

    if (!dtn_cbor_encode(block, buffer->start, buffer->capacity, &next))
        goto error;

    buffer->length = next - buffer->start;

    uint32_t crc_sum32 = 0;
    uint16_t crc_sum16 = 0;
    char *bytes = NULL;

    switch (crc_flags){

        case 0x01:

            buffer->start[buffer->length] = 0x00;
            buffer->start[buffer->length - 1] = 0x00;
            buffer->start[buffer->length - 2] = 0x00;
            
            crc_sum16 = crc16x25(buffer->start, buffer->length);

            bytes = (char*) dtn_cbor_get_string(crc);
            if (!bytes) goto error;

            bytes[0] = crc_sum16 >> 8;
            bytes[1] = crc_sum16;

            break;
        case 0x02:

            buffer->start[buffer->length] = 0x00;
            buffer->start[buffer->length - 1] = 0x00;
            buffer->start[buffer->length - 2] = 0x00;
            buffer->start[buffer->length - 3] = 0x00;
            buffer->start[buffer->length - 4] = 0x00;
            
            crc_sum32 = dtn_crc32c(buffer->start, buffer->length);
            bytes = (char*) dtn_cbor_get_string(crc);
            if (!bytes) goto error;

            bytes[0] = crc_sum32 >> 24;
            bytes[1] = crc_sum32 >> 16;
            bytes[2] = crc_sum32 >> 8;
            bytes[3] = crc_sum32;
            
            break;
        default:
            goto error;

    }

done:
    dtn_buffer_free(buffer);
    return true;
error:
    dtn_buffer_free(buffer);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool set_crcs(dtn_bundle *self){

    if (!self) goto error;

    uint64_t count = dtn_cbor_array_count(self->data);

    for (uint64_t i = 0; i < count; i++){

        dtn_cbor *block = dtn_cbor_array_get(self->data, i);

        if (i == 0){
            if (!set_crc_primary(block))
                goto error;
        } else {
            if (!set_crc_block(block))
                goto error;
        }
    }

    return true;
error:
    return false;

}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_encode(
    dtn_bundle *self,
    uint8_t *buffer, 
    size_t size,
    uint8_t **next){

    if (!self || !buffer || size < 1 || !next) goto error;

    if (!set_crcs(self)) goto error;
    if (!dtn_bundle_verify(self)) goto error;

    return dtn_cbor_encode_array_of_indefinite_length(self->data, buffer, size, next);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_encoding_size(const dtn_bundle *self){

    if (!self) return 0;
    // NOTE this may return a larger buffer due to the 
    // bundle array infinitie length encoding. 

    return dtn_cbor_encoding_size(self->data);
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_verify(dtn_bundle *self){

    if (!self) goto error;

    if (!check_primary_block(self)) goto error;
    if (!check_canonical_blocks(self)) goto error;
    if (!check_payload_block(self)) goto error;

    return true;
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      GETTER / SETTER PRIMARY
 *
 *      ------------------------------------------------------------------------
 */


dtn_cbor *dtn_bundle_add_primary_block(
        dtn_bundle *self, 
        uint64_t flags, 
        uint64_t crc_type,
        const char *destination,
        const char *source,
        const char *report_to,
        uint64_t time,
        uint64_t seq,
        uint64_t lifetime,
        uint64_t offset,
        uint64_t length){

    if (!self) goto error;
    if (!self->data) self->data = dtn_cbor_array();

    if (dtn_cbor_array_get(self->data, 0)){
        dtn_log_error("Primary bundle already set - abort.");
        goto error;
    }

    if (!dtn_bundle_primary_set_version(self)) goto error;
    if (!dtn_bundle_primary_set_flags(self, flags)) goto error;
    if (!dtn_bundle_primary_set_crc_type(self, crc_type)) goto error;
    if (!dtn_bundle_primary_set_destination(self, destination)) goto error;
    if (!dtn_bundle_primary_set_source(self, source)) goto error;
    if (!dtn_bundle_primary_set_report(self, report_to)) goto error;
    if (!dtn_bundle_primary_set_timestamp(self, time, seq)) goto error;
    if (!dtn_bundle_primary_set_lifetime(self, lifetime)) goto error;

    if (flags &0x01){
        if (!dtn_bundle_primary_set_fragment_offset(self, offset)) goto error;
        if (!dtn_bundle_primary_set_total_data_length(self, length))goto error;
    }
        
    return dtn_cbor_array_get(self->data, 0);
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_primary_get_version(const dtn_bundle *self){

    if (!self || !self->data) goto error;

    const dtn_cbor *block = dtn_cbor_array_get(self->data, 0);
    const dtn_cbor *item = dtn_cbor_array_get(block, 0);
    return dtn_cbor_get_uint(item);

error:
    return 0;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_primary_set_version(dtn_bundle *self){

    if (!self) goto error;

    if (!self->data) self->data = dtn_cbor_array();

    dtn_cbor *block = (dtn_cbor*)dtn_cbor_array_get(self->data, 0);
    if (!block){
        block = dtn_cbor_array();
        if (!dtn_cbor_array_push((dtn_cbor*)self->data, block)) {
            block = dtn_cbor_free(block);
            goto error;
        }
    }
    dtn_cbor *item =(dtn_cbor*) dtn_cbor_array_get(block, 0);
    if (!item){
        item = dtn_cbor_uint(0x07);
        if (!dtn_cbor_array_push(block, item)){
            item = dtn_cbor_free(item);
            goto error;
        }
    }
    return dtn_cbor_set_uint(item, 0x07);

error:
    return false;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_primary_get_flags(const dtn_bundle *self){

    if (!self || !self->data) goto error;

    const dtn_cbor *block = dtn_cbor_array_get(self->data, 0);
    const dtn_cbor *item = dtn_cbor_array_get(block, 1);
    return dtn_cbor_get_uint(item);

error:
    return 0;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_primary_set_flags(dtn_bundle *self, uint64_t flags){

    if (!self || !self->data) goto error;

    dtn_cbor *block = (dtn_cbor*)dtn_cbor_array_get(self->data, 0);
    if (!block) goto error;

    dtn_cbor *item = (dtn_cbor*)dtn_cbor_array_get(block, 1);
    if (!item){
        item = dtn_cbor_uint(flags);
        if (!dtn_cbor_array_push((dtn_cbor*)block, item)){
            item = dtn_cbor_free(item);
            goto error;
        }
    }
    return dtn_cbor_set_uint(item, flags);

error:
    return false;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_primary_get_crc_type(const dtn_bundle *self){
    
    if (!self || !self->data) goto error;

    const dtn_cbor *block = dtn_cbor_array_get(self->data, 0);
    const dtn_cbor *item = dtn_cbor_array_get(block, 2);
    return dtn_cbor_get_uint(item);

error:
    return 0;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_primary_set_crc_type(dtn_bundle *self, uint64_t type){

    if (!self || !self->data) goto error;

    dtn_cbor *block =(dtn_cbor*) dtn_cbor_array_get(self->data, 0);
    if (!block) goto error;

    dtn_cbor *item = (dtn_cbor*) dtn_cbor_array_get(block, 2);
    if (!item){
        item = dtn_cbor_uint(type);
        if (!dtn_cbor_array_push((dtn_cbor*)block, item)){
            item = dtn_cbor_free(item);
            goto error;
        }
    }
    return dtn_cbor_set_uint(item, type);

error:
    return false;
}

/*----------------------------------------------------------------------------*/

const char *dtn_bundle_primary_get_destination(const dtn_bundle *self){

    if (!self || !self->data) goto error;

    const dtn_cbor *block = dtn_cbor_array_get(self->data, 0);
    const dtn_cbor *item = dtn_cbor_array_get(block, 3);
    if (!dtn_cbor_is_string(item)) goto error;

    return dtn_cbor_get_string(item);

error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_primary_set_destination(
        dtn_bundle *self, const char *value){

    if (!self || !self->data) goto error;

    dtn_cbor *block = (dtn_cbor*)dtn_cbor_array_get(self->data, 0);
    if (!block) goto error;

    dtn_cbor *item = (dtn_cbor*)dtn_cbor_array_get(block, 3);
    if (!item){

        item = dtn_cbor_string(value);
        if (!dtn_cbor_array_push((dtn_cbor*)block, item)){
            item = dtn_cbor_free(item);
            goto error;
        }
    
    } else {

        if (!dtn_cbor_is_string(item)) goto error;
    }

    return dtn_cbor_set_string(item, value);

error:
    return false;
}

/*----------------------------------------------------------------------------*/

const char *dtn_bundle_primary_get_source(const dtn_bundle *self){

    if (!self || !self->data) goto error;

    const dtn_cbor *block = dtn_cbor_array_get(self->data, 0);
    const dtn_cbor *item = dtn_cbor_array_get(block, 4);
    if (!dtn_cbor_is_string(item)) goto error;
    return dtn_cbor_get_string(item);

error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_primary_set_source(
        dtn_bundle *self, const char *value){

    if (!self || !self->data) goto error;

    dtn_cbor *block = (dtn_cbor*)dtn_cbor_array_get(self->data, 0);
    if (!block) goto error;

    dtn_cbor *item = (dtn_cbor*)dtn_cbor_array_get(block, 4);
    if (!item){

        item = dtn_cbor_string(value);
        if (!dtn_cbor_array_push((dtn_cbor*)block, item)){
            item = dtn_cbor_free(item);
            goto error;
        }
    
    } else {

        if (!dtn_cbor_is_string(item)) goto error;
    }
   
    return dtn_cbor_set_string(item, value);

error:
    return false;
}

/*----------------------------------------------------------------------------*/

const char *dtn_bundle_primary_get_report(const dtn_bundle *self){

    if (!self || !self->data) goto error;

    const dtn_cbor *block = dtn_cbor_array_get(self->data, 0);
    const dtn_cbor *item = dtn_cbor_array_get(block, 5);
    if (!dtn_cbor_is_string(item)) goto error;
    return dtn_cbor_get_string(item);

error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_primary_set_report(
        dtn_bundle *self, const char *value){

    if (!self || !self->data) goto error;

    dtn_cbor *block = (dtn_cbor*)dtn_cbor_array_get(self->data, 0);
    if (!block) goto error;

    dtn_cbor *item = (dtn_cbor*)dtn_cbor_array_get(block, 5);
    if (!item){

        item = dtn_cbor_string(value);
        if (!dtn_cbor_array_push((dtn_cbor*)block, item)){
            item = dtn_cbor_free(item);
            goto error;
        }
    
    } else {

        if (!dtn_cbor_is_string(item)) goto error;
    }
   
    return dtn_cbor_set_string(item, value);

error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_primary_get_timestamp(const dtn_bundle *self, 
        uint64_t *time, uint64_t *sequence_number){

    if (!self || !self->data || !time || !sequence_number) goto error;

    const dtn_cbor *block = dtn_cbor_array_get(self->data, 0);
    const dtn_cbor *item = dtn_cbor_array_get(block, 6);
    if (!dtn_cbor_is_array(item)) goto error;

    const dtn_cbor *timestamp = dtn_cbor_array_get(item, 0);
    if (timestamp){
        *time = dtn_cbor_get_uint(timestamp);
    } else {
        goto error;
    }

    const dtn_cbor *sequence = dtn_cbor_array_get(item, 1);
    if (sequence){
        *sequence_number = dtn_cbor_get_uint(sequence);
    } else {
        goto error;
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_primary_set_timestamp(dtn_bundle *self, 
        uint64_t time, uint64_t sequence_number){

    if (!self || !self->data) goto error;

    dtn_cbor *block = (dtn_cbor*)dtn_cbor_array_get(self->data, 0);
    if (!block) goto error;

    dtn_cbor *item = (dtn_cbor*)dtn_cbor_array_get(block, 6);
    if (!item){

        item = dtn_cbor_array();
        if (!dtn_cbor_array_push((dtn_cbor*)block, item)){
            item = dtn_cbor_free(item);
            goto error;
        }
    
    } else {

        if (!dtn_cbor_is_array(item)) goto error;
    }

    dtn_cbor *timestamp = (dtn_cbor*)dtn_cbor_array_get(item, 0);
    if (timestamp){
        dtn_cbor_set_uint(timestamp, time);
    } else {
        timestamp = dtn_cbor_uint(time);
        if (!dtn_cbor_array_push((dtn_cbor*)item, timestamp)){
            timestamp = dtn_cbor_free(timestamp);
            goto error;
        }
    }

    dtn_cbor *sequence = (dtn_cbor*)dtn_cbor_array_get(item, 1);
    if (sequence){
        dtn_cbor_set_uint(sequence, sequence_number);
    } else {
        sequence = dtn_cbor_uint(sequence_number);
        if (!dtn_cbor_array_push((dtn_cbor*)item, sequence)){
            sequence = dtn_cbor_free(sequence);
            goto error;
        }
    }
   
    return true;

error:
    return false;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_primary_get_lifetime(const dtn_bundle *self){

    if (!self || !self->data) goto error;

    const dtn_cbor *block = dtn_cbor_array_get(self->data, 0);
    const dtn_cbor *item = dtn_cbor_array_get(block, 7);
    if (!dtn_cbor_is_uint(item)) goto error;
    return dtn_cbor_get_uint(item);

error:
    return 0;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_primary_set_lifetime(dtn_bundle *self, uint64_t type){

    if (!self || !self->data) goto error;

    dtn_cbor *block = (dtn_cbor*)dtn_cbor_array_get(self->data, 0);
    if (!block) goto error;

    dtn_cbor *item = (dtn_cbor*)dtn_cbor_array_get(block, 7);
    if (!item){

        item = dtn_cbor_uint(type);
        if (!dtn_cbor_array_push((dtn_cbor*)block, item)){
            item = dtn_cbor_free(item);
            goto error;
        }
    
    } else {

        if (!dtn_cbor_is_uint(item)) goto error;
    }
   
    return dtn_cbor_set_uint(item, type);

error:
    return false;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_primary_get_fragment_offset(const dtn_bundle *self){

    if (!self || !self->data) goto error;

    const dtn_cbor *block = dtn_cbor_array_get(self->data, 0);
    const dtn_cbor *item = dtn_cbor_array_get(block, 8);
    if (!dtn_cbor_is_uint(item)) goto error;
    return dtn_cbor_get_uint(item);

error:
    return 0;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_primary_set_fragment_offset(dtn_bundle *self, uint64_t type){

    if (!self || !self->data) goto error;

    dtn_cbor *block = (dtn_cbor*)dtn_cbor_array_get(self->data, 0);
    if (!block) goto error;

    dtn_cbor *item = (dtn_cbor*)dtn_cbor_array_get(block, 8);
    if (!item){

        item = dtn_cbor_uint(type);
        if (!dtn_cbor_array_push((dtn_cbor*)block, item)){
            item = dtn_cbor_free(item);
            goto error;
        }
    
    } else {

        if (!dtn_cbor_is_uint(item)) goto error;
    }
   
    return dtn_cbor_set_uint(item, type);

error:
    return false;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_primary_get_totel_data_length(const dtn_bundle *self){

    if (!self || !self->data) goto error;

    const dtn_cbor *block = dtn_cbor_array_get(self->data, 0);
    const dtn_cbor *item = dtn_cbor_array_get(block, 9);
    if (!dtn_cbor_is_uint(item)) goto error;
    return dtn_cbor_get_uint(item);

error:
    return 0;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_primary_set_total_data_length(dtn_bundle *self, uint64_t type){

    if (!self || !self->data) goto error;

    dtn_cbor *block = (dtn_cbor*)dtn_cbor_array_get(self->data, 0);
    if (!block) goto error;

    dtn_cbor *item = (dtn_cbor*)dtn_cbor_array_get(block, 9);
    if (!item){

        item = dtn_cbor_uint(type);
        if (!dtn_cbor_array_push((dtn_cbor*)block, item)){
            item = dtn_cbor_free(item);
            goto error;
        }
    
    } else {

        if (!dtn_cbor_is_uint(item)) goto error;
    }
   
    return dtn_cbor_set_uint(item, type);

error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      GETTER / SETTER CANNONICAL
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor *dtn_bundle_add_block(
    dtn_bundle *self, 
    uint64_t code,
    uint64_t nbr,
    uint64_t flags,
    uint64_t crc_type,
    dtn_cbor *payload){

    dtn_cbor *array = NULL;
    dtn_cbor *item = NULL;

    if (!self) goto error;
    if (!self->data) goto error;

    array = dtn_cbor_array();
    if (!array) goto error;

    item = dtn_cbor_uint(code);
    if (!dtn_cbor_array_push(array, item)) goto error;

    item = dtn_cbor_uint(nbr);
    if (!dtn_cbor_array_push(array, item)) goto error;

    item = dtn_cbor_uint(flags);
    if (!dtn_cbor_array_push(array, item)) goto error;

    item = dtn_cbor_uint(crc_type);
    if (!dtn_cbor_array_push(array, item)) goto error;

    item = NULL;

    if (payload)
        if (!dtn_cbor_array_push(array, payload)) goto error;

    if (!dtn_cbor_array_push(self->data, array)) goto error;

    return array;
error:
    dtn_cbor_free(array);
    dtn_cbor_free(item);
    return NULL;
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_bundle_get_block(dtn_bundle *self, 
        uint64_t nbr){

    if (!self || !self->data) return NULL;

    uint64_t count = dtn_cbor_array_count(self->data);

    for (uint64_t i = 0; i < count; i++){

        dtn_cbor *item = dtn_cbor_array_get(self->data, i);

        if (dtn_bundle_get_number(item) == nbr)
            return item;
    }

    return NULL;

}

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_get_number(const dtn_cbor *self){

    if (!self) return 0;
    if (!dtn_cbor_is_array(self)) goto error;

    dtn_cbor *item = dtn_cbor_array_get(self, 1);
    return dtn_cbor_get_uint(item);
error:
    return 0;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_set_number(dtn_cbor *self, uint64_t number){

    if (!self) goto error;

    dtn_cbor *item = dtn_cbor_array_get(self, 1);
    if (!item) goto error;

    return dtn_cbor_set_uint(item, number);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_get_code(const dtn_cbor *self){

    if (!self) return 0;
    if (!dtn_cbor_is_array(self)) goto error;

    dtn_cbor *item = dtn_cbor_array_get(self, 0);
    return dtn_cbor_get_uint(item);
error:
    return 0;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_set_code(dtn_cbor *self, uint64_t number){

    if (!self) goto error;

    dtn_cbor *item = dtn_cbor_array_get(self, 0);
    if (!item) goto error;

    return dtn_cbor_set_uint(item, number);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_get_flags(const dtn_cbor *self){

    if (!self) return 0;
    if (!dtn_cbor_is_array(self)) goto error;

    dtn_cbor *item = dtn_cbor_array_get(self, 2);
    return dtn_cbor_get_uint(item);
error:
    return 0;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_set_flags(dtn_cbor *self, uint64_t number){

    if (!self) goto error;

    dtn_cbor *item = dtn_cbor_array_get(self, 2);
    if (!item) goto error;

    return dtn_cbor_set_uint(item, number);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_get_crc_type(const dtn_cbor *self){

    if (!self) return 0;
    if (!dtn_cbor_is_array(self)) goto error;

    dtn_cbor *item = dtn_cbor_array_get(self, 3);
    return dtn_cbor_get_uint(item);
error:
    return 0;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_set_crc_type(dtn_cbor *self, uint64_t number){

    if (!self) goto error;

    dtn_cbor *item = dtn_cbor_array_get(self, 3);
    if (!item) goto error;

    return dtn_cbor_set_uint(item, number);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_bundle_get_data(const dtn_cbor *self){

    if (!self) return 0;
    if (!dtn_cbor_is_array(self)) goto error;

    dtn_cbor *item = dtn_cbor_array_get(self, 4);
    return item;
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_set_data(dtn_cbor *self, dtn_cbor *data){

    if (!self) return 0;
    if (!dtn_cbor_is_array(self)) goto error;

    return dtn_cbor_array_set(self, 4, data);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_clear(void *self){

    if (!self) goto error;
    dtn_bundle *bundle = (dtn_bundle*) self;

    bundle->data = dtn_cbor_free(bundle->data);
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_dump(FILE *stream, void *bundle){

    dtn_buffer *buffer = NULL;
    uint8_t *next = NULL;

    if (!stream || !bundle) goto error;

    dtn_bundle *self = (dtn_bundle*) bundle;

    size_t size = dtn_bundle_encoding_size(self);

    buffer = dtn_buffer_create(size);
    if (!buffer) goto error;

    if (!dtn_bundle_encode(
        self, 
        buffer->start, buffer->capacity,
        &next)) goto error;

    buffer->length = next - buffer->start;
    
    dtn_dump_binary_as_hex(stream, buffer->start, buffer->length);

    buffer = dtn_buffer_free(buffer);
    return true;
error:
    dtn_buffer_free(buffer);
    return false;
}

/*----------------------------------------------------------------------------*/

void *dtn_bundle_copy(void **destination, const void *source){

    dtn_bundle *copy = NULL;

    if (!destination || !source) goto error;
    if (*destination) goto error;

    dtn_bundle *self = (dtn_bundle*) source;

    copy = dtn_bundle_create();
    if (!copy) goto error;

    if (self->data)
        if (!dtn_cbor_copy((void**)&copy->data, self->data))
            goto error;

    *destination = copy;
    return copy;
error:
    dtn_bundle_free(copy);
    return NULL;
}

/*----------------------------------------------------------------------------*/

void *dtn_bundle_free_void(void *self){

    if (!self) return self;
    dtn_bundle *bundle = (dtn_bundle*) self;
    return dtn_bundle_free(bundle);
}

/*
 *      ------------------------------------------------------------------------
 *
 *      SPECIAL Functionality
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor *dtn_bundle_get_raw(const dtn_bundle *self){

    if (!self) return NULL;
    return self->data;

}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_set_raw(dtn_bundle *self, dtn_cbor *array){

    if (!self || !dtn_cbor_is_array(array)) goto error;

    self->data = dtn_cbor_free(self->data);
    self->data = array;
    return true;
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      SPECIAL BLOCKS
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor *dtn_bundle_add_previous_node(
        dtn_bundle *self, 
        const char *node_id){

    if (!self || !node_id) return NULL;

    return dtn_bundle_add_block(
        self, 
        6, 
        6,
        0,
        0,
        dtn_cbor_string(node_id));
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_bundle_add_bundle_age(
        dtn_bundle *self, 
        uint64_t age){

    if (!self) return NULL;

    return dtn_bundle_add_block(
        self, 
        7, 
        7,
        0,
        0,
        dtn_cbor_uint(age));
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_bundle_add_hop_count(
        dtn_bundle *self, 
        uint64_t count,
        uint64_t limit){

    dtn_cbor *arr = NULL;

    if (!self) goto error;

    arr = dtn_cbor_array();
    if (!arr) goto error;

    if (!dtn_cbor_array_push(arr, dtn_cbor_uint(limit))) goto error;
    if (!dtn_cbor_array_push(arr, dtn_cbor_uint(count))) goto error;

    return dtn_bundle_add_block(
        self, 
        10, 
        10,
        0,
        0,
        arr);
error:
    dtn_cbor_free(arr);
    return NULL;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      SECURITY BLOCKS
 *
 *      ------------------------------------------------------------------------
 */
/*
static bool set_crc_flags_to_null(void *item, void *data){

    uint64_t nbr = dtn_cbor_get_uint(item);
    dtn_bundle *bundle = (dtn_bundle*) data;

    dtn_cbor *block = dtn_bundle_get_block(bundle, nbr);
    return dtn_bundle_set_crc_type(block, 0);
}
*/

/*----------------------------------------------------------------------------*/

static dtn_buffer *generate_new_key(dtn_bpsec_sha_variant sha){

    dtn_buffer *buffer = NULL;

    size_t len = 256;

    switch(sha){
        case HMAC256:
            len = 256;
            break;
        case HMAC384:
            len = 384;
            break;
        case HMAC512:
            len = 512;
            break;
    }

    buffer = dtn_buffer_create(len);
    if (!buffer) goto error;

    if (1 != RAND_bytes(buffer->start, len))
        goto error;

    buffer->length = len;
    return buffer;
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

static dtn_buffer *generate_iv(size_t len){

    dtn_buffer *buffer = NULL;

    buffer = dtn_buffer_create(len);
    if (!buffer) goto error;

    if (1 != RAND_bytes(buffer->start, len))
        goto error;

    buffer->length = len;
    return buffer;
error:
    return NULL;
}
/*----------------------------------------------------------------------------*/

static bool generate_hash_input(
    dtn_bundle *self,
    dtn_cbor *bib,
    dtn_cbor *target,
    uint8_t aad_flags,
    uint8_t *buffer,
    size_t *len,
    bool target_payload){

    if (!self || !bib|| !target || !buffer) goto error;

    dtn_cbor *block = NULL;

    buffer[0] = aad_flags;
    uint8_t *ptr = buffer + 1;

    size_t size = *len;

    if (target == dtn_cbor_array_get(self->data, 0)){

        // primary block target
        if (!dtn_bundle_primary_set_crc_type(self, 0)) goto error;

        if (aad_flags & 0x03){

            block = dtn_cbor_array_get(bib, 0);
            if (!dtn_cbor_encode(block, ptr, size - (ptr-buffer), &ptr))
                goto error;

            block = dtn_cbor_array_get(bib, 1);
            if (!dtn_cbor_encode(block, ptr, size - (ptr-buffer), &ptr))
                goto error;

            block = dtn_cbor_array_get(bib, 2);
            if (!dtn_cbor_encode(block, ptr, size - (ptr-buffer), &ptr))
                goto error;
        
        }

        block = dtn_cbor_array_get(self->data, 0);
        if (!dtn_cbor_encode(block, ptr, size - (ptr-buffer), &ptr))
            goto error;

    } else {

        if (aad_flags & 0x01)
            if (!dtn_bundle_primary_set_crc_type(self, 0)) goto error;

        if (!dtn_bundle_set_crc_type(target, 0)) goto error;

        if (aad_flags & 0x01){
            block = dtn_cbor_array_get(self->data, 0);
            if (!dtn_cbor_encode(block, ptr, size - (ptr-buffer), &ptr))
                goto error;
        }

        if (aad_flags & 0x02){

            block = dtn_cbor_array_get(target, 0);
            if (!dtn_cbor_encode(block, ptr, size - (ptr-buffer), &ptr))
                goto error;

            block = dtn_cbor_array_get(target, 1);
            if (!dtn_cbor_encode(block, ptr, size - (ptr-buffer), &ptr))
                goto error;

            block = dtn_cbor_array_get(target, 2);
            if (!dtn_cbor_encode(block, ptr, size - (ptr-buffer), &ptr))
                goto error;

        }

        if (aad_flags & 0x03){

            block = dtn_cbor_array_get(bib, 0);
            if (!dtn_cbor_encode(block, ptr, size - (ptr-buffer), &ptr))
                goto error;

            block = dtn_cbor_array_get(bib, 1);
            if (!dtn_cbor_encode(block, ptr, size - (ptr-buffer), &ptr))
                goto error;

            block = dtn_cbor_array_get(bib, 2);
            if (!dtn_cbor_encode(block, ptr, size - (ptr-buffer), &ptr))
                goto error;
        
        }

        if (target_payload){
            block = dtn_bundle_get_data(target);
            if (!dtn_cbor_encode(block, ptr, size - (ptr-buffer), &ptr))
                goto error;
        }
    }

    *len = ptr - buffer;
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_bib_protect(
        dtn_bundle *self,
        dtn_cbor *bib,
        dtn_cbor *target,
        const dtn_buffer *key,
        uint8_t aad_flags,
        dtn_bpsec_sha_variant sha,
        dtn_dtn_uri *source,
        bool add_new_key){

    dtn_bpsec_asb *asb = NULL;
    dtn_buffer *new_key = NULL;

    uint8_t buffer[4096] = { 0 };
    size_t size = 4096;

    uint8_t hash[4096] = { 0 };
    size_t hash_size = 4096;

    uint8_t wrapped[4096] = { 0 };
    size_t wrapped_size = 4096;

    if (!generate_hash_input(
        self,
        bib,
        target,
        aad_flags,
        buffer,
        &size,
        true)) goto error;

    if (add_new_key){

        new_key = generate_new_key(sha);

        if (!dtn_aes_key_wrap(new_key->start,   
                          new_key->length,
                          wrapped,
                          &wrapped_size,
                          key->start,
                          key->length / 8)) goto error;
    } else {

        new_key = (dtn_buffer*) key;
    }

    dtn_hash_function hash_function = DTN_HASH_SHA384;

    switch(sha){
        case HMAC256:
            hash_function = DTN_HASH_SHA256;
            break;
        case HMAC384:
            hash_function = DTN_HASH_SHA384;
            break;
        case HMAC512:
            hash_function = DTN_HASH_SHA512;
            break;
    }

    if (!dtn_hmac(hash_function,
        buffer, 
        size, 
        new_key->start,
        new_key->length,
        hash,
        &hash_size)) goto error;
    
    asb = dtn_bpsec_asb_create();
    if (!dtn_bpsec_add_target(asb, dtn_bundle_get_number(target))) goto error;
    if (!dtn_bpsec_set_context_id(asb, 1)) goto error;
    if (!dtn_bpsec_set_context_flags(asb, 1)) goto error;
    if (!dtn_bpsec_set_source(asb, source)) goto error;
    if (!dtn_bpsec_add_sha_variant(asb, sha)) goto error;
    if (add_new_key){
        if (!dtn_bpsec_add_wrapped_key(asb, wrapped, wrapped_size)) goto error;
    }
    if (!dtn_bpsec_add_integrity_flags(asb, aad_flags)) goto error;
    if (!dtn_bpsec_add_result(asb, 1, hash, hash_size)) goto error;

    dtn_cbor *result = dtn_bpsec_asb_encode(asb);
    asb = dtn_bpsec_asb_free(asb);

    if (!dtn_bundle_set_data(bib, result)) goto error;

    if (add_new_key){
        new_key = dtn_buffer_free(new_key);
    }
    
    return true;

error:
    asb = dtn_bpsec_asb_free(asb);
    if (add_new_key){
        new_key = dtn_buffer_free(new_key);
    }
    return false;
}

/*----------------------------------------------------------------------------*/

static bool check_bib_contained(void *value, void *data){

    dtn_cbor *block = (dtn_cbor*) value;

    if (11 == dtn_bundle_get_code(block)){

        uint64_t *number = (uint64_t*) data;
        *number = dtn_bundle_get_number(block);

    }
    
    return true;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_is_bib_protected(const dtn_bundle *self){

    uint64_t nbr = 0;

    if (!dtn_cbor_array_for_each(self->data, 
        &nbr, 
        check_bib_contained)) goto error;

    if (0 != nbr)
        return true;

error:
    return false;
}

/*----------------------------------------------------------------------------*/

struct container {

    dtn_bundle *self;
    dtn_key_store *store;
    dtn_cbor *bib;
    dtn_bpsec_asb *asb;
    uint8_t *wrapped_key;
    size_t wrapped_key_size;
    uint8_t *iv;
    size_t iv_size;
    uint64_t context_id;
    uint64_t context_flags;
    uint64_t integrity_flags;
    dtn_bpsec_sha_variant sha;
    dtn_bpsec_aes_variant aes;
    uint64_t count;

};

/*----------------------------------------------------------------------------*/

static bool check_integrity_for_target(void *item, void *data){

    dtn_dtn_uri *uri = NULL;
    dtn_ipn *ipn = NULL;

    dtn_buffer *master_key = NULL;

    uint8_t *result = NULL;
    size_t result_size = 0;

    uint8_t key_buffer[1024] = {0};
    size_t key_size = 1024;

    uint8_t *key = key_buffer;

    char source[4096] = {0};
    size_t source_size = 4096;

    uint8_t hash_input[4096] = {0};
    size_t hash_input_size = 4096;

    uint8_t hash[4096] = { 0 };
    size_t hash_size = 4096;

    dtn_cbor *target_id = (dtn_cbor*) item;
    uint64_t id = dtn_cbor_get_uint(target_id);
    struct container *container = (struct container*) data;

    container->count++;

    dtn_cbor *target = dtn_bundle_get_block(container->self, id);
    if (!target) goto error;

    if (!dtn_bpsec_get_result(
        container->asb,
        container->count,
        &result,
        &result_size)) goto error;

    if (!dtn_bpsec_get_source(container->asb, 
        container->count, &uri, &ipn))
        goto error;

    if (uri) {

        snprintf(source, source_size, "%s/%s", 
            uri->name, uri->demux);

    } else if (ipn){

        snprintf(source, source_size, "%s.%s", 
            ipn->node, ipn->service);

    } else {

        goto error;
    }

    master_key = dtn_key_store_get(container->store, source);
    if (!master_key) {
        dtn_log_error("no masterkey found for %s", source);
        goto error;
    }

    if (!dtn_bpsec_get_result(
        container->asb,
        container->count, 
        &result,
        &result_size)) goto error;

    if (container->wrapped_key){

        if (!dtn_aes_key_unwrap(
            container->wrapped_key,
            container->wrapped_key_size,
            key, &key_size,
            master_key->start,
            master_key->length / 8)) goto error;

    } else {

        key = master_key->start,
        key_size = master_key->length;
    
    }

    if (!generate_hash_input(
        container->self,
        container->bib,
        target,
        container->integrity_flags,
        hash_input, 
        &hash_input_size,
        true)) goto error;

    dtn_hash_function hash_function = DTN_HASH_SHA384;

    switch(container->sha){
        case HMAC256:
            hash_function = DTN_HASH_SHA256;
            break;
        case HMAC384:
            hash_function = DTN_HASH_SHA384;
            break;
        case HMAC512:
            hash_function = DTN_HASH_SHA512;
            break;
    }

    if (!dtn_hmac(hash_function,
        hash_input, 
        hash_input_size, 
        key,
        key_size,
        hash,
        &hash_size)) goto error;

    if (hash_size != result_size) goto error;

    if (0 != memcmp(hash, result, hash_size)) goto error;
    
    uri = dtn_dtn_uri_free(uri);
    ipn = dtn_ipn_free(ipn);
    master_key = dtn_buffer_free(master_key);
    return true;
error:
    uri = dtn_dtn_uri_free(uri);
    ipn = dtn_ipn_free(ipn);
    master_key = dtn_buffer_free(master_key);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool bib_verify(void *value, void *data){


    dtn_cbor *block = (dtn_cbor*) value;
    struct container *container = (struct container*)data;

    if (11 != dtn_bundle_get_code(block))
        return true;

    container->bib = block;
    if (!container->bib) goto error;

    container->asb = dtn_bpsec_asb_decode(dtn_bundle_get_data(block));
    if (!container->asb) goto error;

    container->context_id = dtn_bpsec_get_context_id(container->asb);
    container->context_flags = dtn_bpsec_get_context_flags(container->asb);
    container->sha = dtn_bpsec_get_sha_variant(container->asb);
    dtn_bpsec_get_wrapped_key(container->asb, 
        &container->wrapped_key, &container->wrapped_key_size);
    dtn_bpsec_get_integrity_flags(container->asb, &container->integrity_flags);

    if (1 != container->context_id) goto error;
    
    if (0 == container->integrity_flags)
        container->integrity_flags = 0x07;

    if (!dtn_cbor_array_for_each(container->asb->target, 
        container,
        check_integrity_for_target)) goto error;

    container->asb = dtn_bpsec_asb_free(container->asb);
    return true;
error:
    if (container)
        container->asb = dtn_bpsec_asb_free(container->asb);
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_bib_verify(
        dtn_bundle *self,
        dtn_key_store *store){

    if (!self || !store) goto error;

    // there may be multiple verify blocks

    struct container container = (struct container){
        .self = self,
        .store = store
    }; 

    return dtn_cbor_array_for_each(
        self->data, 
        &container,
        bib_verify);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool gcm_encrypt(
    dtn_bpsec_aes_variant aes,
    uint8_t *plaintext,
    size_t plaintext_size,
    uint8_t *aad,
    size_t aad_size,
    uint8_t *key,
    size_t key_size,
    uint8_t *iv,
    size_t iv_len,
    uint8_t *ciphertext,
    size_t *ciphertext_size,
    uint8_t *tag,
    size_t tag_size){

    EVP_CIPHER_CTX *ctx;

    UNUSED(key_size);

    int len = 0;
    int ciphertext_len = 0;

    if(!(ctx = EVP_CIPHER_CTX_new()))
        goto error;

    switch (aes){

        case A128GCM:

            if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL))
                goto error;

            break;

        case A256GCM:

            if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
                goto error;

            break;
    };

    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, iv_len, NULL))
        goto error;

    if(1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv))
        goto error;

    if(1 != EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_size))
        goto error;

    if(1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_size))
        goto error;

    ciphertext_len = len;

    if(1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len))
        goto error;

    ciphertext_len += len;

    if(1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_GET_TAG, tag_size, tag))
        goto error;

    *ciphertext_size = ciphertext_len;

    EVP_CIPHER_CTX_free(ctx);
    return true;
error:
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    return false;
} 


/*----------------------------------------------------------------------------*/

static bool gcm_decrypt(
    dtn_bpsec_aes_variant aes,
    uint8_t *ciphertext, size_t ciphertext_size,
    uint8_t *aad, size_t aad_size,
    uint8_t *tag, size_t tag_size,
    uint8_t *key,
    uint8_t *iv, size_t iv_size,
    uint8_t *plaintext, size_t *plaintext_size){

    EVP_CIPHER_CTX *ctx = NULL;
    int len = 0;
    int plaintext_len = 0;
    int ret = 0;

    if(!(ctx = EVP_CIPHER_CTX_new()))
        goto error;

    switch (aes){

        case A128GCM:
            if(!EVP_DecryptInit_ex(ctx, EVP_aes_128_gcm(), NULL, NULL, NULL))
                goto error;
            break;
        case A256GCM:
            if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
                goto error;

    }

    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_IVLEN, iv_size, NULL))
        goto error;

    if(!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv))
        goto error;
    
    if(!EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_size))
        goto error;

    if(!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_size))
        goto error;

    plaintext_len = len;

    if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_AEAD_SET_TAG, tag_size, tag))
        goto error;

    ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

    if(ret > 0) {

        EVP_CIPHER_CTX_free(ctx);

        plaintext_len += len;
        *plaintext_size = plaintext_len;
        
        return true;
    }

error:
    if (ctx) EVP_CIPHER_CTX_free(ctx);
    return false;
}


/*----------------------------------------------------------------------------*/

bool dtn_bundle_bcb_protect(
        dtn_bundle *self,
        dtn_cbor *bcb,
        dtn_cbor *target,
        const dtn_buffer *key,
        dtn_dtn_uri *source,
        uint8_t aad_flags,
        dtn_bpsec_aes_variant aes,
        bool add_new_key){

    dtn_buffer *iv = NULL;
    dtn_buffer *new_key = NULL;
    dtn_bpsec_asb *asb = NULL;

    if (!self || !key || !bcb || !target) goto error;

    uint8_t aad_input[4096] = {0};
    size_t aad_input_size = 4096;

    uint8_t ciphertext[4096] = {0};
    size_t ciphertext_size = 4096;

    uint8_t tag[16] = {0};
    size_t tag_size = 16;

    uint8_t *plaintext = NULL;
    size_t plaintext_size = 0;

    uint8_t wrapped[4096] = { 0 };
    size_t wrapped_size = 4096;

    if (add_new_key){

        new_key = generate_new_key(HMAC256);

        if (!dtn_aes_key_wrap(new_key->start,   
                          new_key->length,
                          wrapped,
                          &wrapped_size,
                          key->start,
                          key->length / 8)) goto error;
    }

    dtn_cbor *data = dtn_bundle_get_data(target);

    if (!dtn_cbor_get_byte_string(data, &plaintext, &plaintext_size))
        goto error;

    if (!generate_hash_input(
        self, 
        bcb, 
        target, 
        aad_flags,
        aad_input,
        &aad_input_size,
        false)) goto error;
    
    iv = generate_iv(12);
    if (!iv) goto error;

    if (new_key){

        if (!gcm_encrypt(aes,
            plaintext,
            plaintext_size,
            aad_input,
            aad_input_size,
            new_key->start,
            new_key->length,
            iv->start,
            iv->length,
            ciphertext,
            &ciphertext_size,
            tag,
            tag_size)) goto error;

    } else {

        if (!gcm_encrypt(aes,
            plaintext,
            plaintext_size,
            aad_input,
            aad_input_size,
            key->start,
            key->length,
            iv->start,
            iv->length,
            ciphertext,
            &ciphertext_size,
            tag,
            tag_size)) goto error;
    }

    uint8_t p[4096] = {0};
    size_t p_len = 4096;

    if (!gcm_decrypt(
        aes,
        ciphertext, ciphertext_size,
        aad_input, aad_input_size,
        tag, tag_size,
        key->start,
        iv->start, iv->length,
        p, &p_len
        )) goto error;
    
    asb = dtn_bpsec_asb_create();
    if (!dtn_bpsec_add_target(asb, dtn_bundle_get_number(target))) goto error;
    if (!dtn_bpsec_set_context_id(asb, 2)) goto error;
    if (!dtn_bpsec_set_context_flags(asb, 1)) goto error;
    if (!dtn_bpsec_set_source(asb, source)) goto error;
    if (!dtn_bpsec_add_iv(asb, iv->start, iv->length)) goto error;
    if (!dtn_bpsec_add_aes_variant(asb, aes)) goto error;
    if (add_new_key){
        if (!dtn_bpsec_add_wrapped_key(asb, wrapped, wrapped_size)) goto error;
    }
    if (!dtn_bpsec_add_integrity_flags(asb, aad_flags)) goto error;
    if (!dtn_bpsec_add_result(asb, 1, tag, tag_size)) goto error;

    dtn_cbor *result = dtn_bpsec_asb_encode(asb);
    asb = dtn_bpsec_asb_free(asb);

    if (!dtn_bundle_set_data(bcb, result)) goto error;
    
    if (!dtn_cbor_set_byte_string(data, ciphertext, ciphertext_size))
        goto error;

    new_key = dtn_buffer_free(new_key);
    iv = dtn_buffer_free(iv);
    return true;
error:
    new_key = dtn_buffer_free(new_key);
    iv = dtn_buffer_free(iv);
    asb = dtn_bpsec_asb_free(asb);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool check_bcb_contained(void *value, void *data){

    dtn_cbor *block = (dtn_cbor*) value;

    if (12 == dtn_bundle_get_code(block)){

        uint64_t *number = (uint64_t*) data;
        *number = dtn_bundle_get_number(block);

    }
    
    return true;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_is_bcb_protected(const dtn_bundle *self){

    uint64_t nbr = 0;

    if (!dtn_cbor_array_for_each(self->data, 
        &nbr, 
        check_bcb_contained)) goto error;

    if (0 != nbr)
        return true;

error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool bcb_unprotect_target(void *item, void *data){

    dtn_dtn_uri *uri = NULL;
    dtn_ipn *ipn = NULL;

    uint8_t key_buffer[1024] = {0};
    size_t key_size = 1024;

    uint8_t aad_input[4096] = {0};
    size_t aad_input_size = 4096;

    uint8_t *key = key_buffer;

    dtn_buffer *master_key = NULL;

    uint8_t *result = NULL;
    size_t result_size = 0;

    char source[4096] = {0};
    size_t source_size = 4096;

    uint8_t *ciphertext = NULL;
    size_t ciphertext_size = 0;

    uint8_t plaintext[4096] = {0};
    size_t plaintext_size = 4096;

    dtn_cbor *target_id = (dtn_cbor*) item;
    struct container *container = (struct container*) data;

    uint64_t id = dtn_cbor_get_uint(target_id);
    dtn_cbor *target = dtn_bundle_get_block(container->self, id);
    if (!target) goto error;

    dtn_cbor *target_data = dtn_bundle_get_data(target);
    if (!dtn_cbor_get_byte_string(target_data, &ciphertext, &ciphertext_size))
        goto error;

    container->count++;

    if (!dtn_bpsec_get_source(container->asb, 
        container->count, &uri, &ipn))
        goto error;

    if (uri) {

        snprintf(source, source_size, "%s/%s", 
            uri->name, uri->demux);

    } else if (ipn){

        snprintf(source, source_size, "%s.%s", 
            ipn->node, ipn->service);

    } else {

        goto error;
    }

    master_key = dtn_key_store_get(container->store, source);
    if (!master_key) {
        dtn_log_error("no masterkey found for %s", source);
        goto error;
    }

    if (!dtn_bpsec_get_result(
        container->asb,
        container->count, 
        &result,
        &result_size)) goto error;

    if (container->wrapped_key){

        if (!dtn_aes_key_unwrap(
            container->wrapped_key,
            container->wrapped_key_size,
            key, &key_size,
            master_key->start,
            master_key->length / 8)) goto error;

    } else {

        key = master_key->start,
        key_size = master_key->length;
    
    }

    if (!generate_hash_input(
        container->self, 
        container->bib, 
        target, 
        container->integrity_flags,
        aad_input,
        &aad_input_size,
        false)) goto error;

    if (!gcm_decrypt(
        container->aes,
        ciphertext, ciphertext_size,
        aad_input, aad_input_size,
        result, result_size,
        key,
        container->iv, container->iv_size,
        plaintext, &plaintext_size)) goto error;

    if (!dtn_cbor_set_byte_string(target_data,  plaintext, plaintext_size))
        goto error;

    uri = dtn_dtn_uri_free(uri);
    ipn = dtn_ipn_free(ipn);
    master_key = dtn_buffer_free(master_key);

    return true;
error:
    uri = dtn_dtn_uri_free(uri);
    ipn = dtn_ipn_free(ipn);
    master_key = dtn_buffer_free(master_key);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool bcb_unprotect(void *value, void *data){

    dtn_cbor *block = (dtn_cbor*) value;

    struct container *container = (struct container*)data;

    if (12 != dtn_bundle_get_code(block))
        return true;

    container->bib = block;
    container->asb = dtn_bpsec_asb_decode(dtn_bundle_get_data(block));
    if (!container->asb) goto error;

    container->context_id = dtn_bpsec_get_context_id(container->asb);
    container->context_flags = dtn_bpsec_get_context_flags(container->asb);
    container->aes = dtn_bpsec_get_aes_variant(container->asb);
    dtn_bpsec_get_wrapped_key(container->asb, 
        &container->wrapped_key, &container->wrapped_key_size);
    dtn_bpsec_get_iv(container->asb, 
        &container->iv, &container->iv_size);
    dtn_bpsec_get_integrity_flags(container->asb, &container->integrity_flags);

    if (2 != container->context_id) goto error;
    
    if (0 == container->integrity_flags)
        container->integrity_flags = 0x07;

    if (!dtn_cbor_array_for_each(container->asb->target, 
        container,
        bcb_unprotect_target)) goto error;

    container->asb = dtn_bpsec_asb_free(container->asb);
    return true;
error:
    if (container)
        container->asb = dtn_bpsec_asb_free(container->asb);
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bundle_bcb_unprotect(
        dtn_bundle *self,
        dtn_key_store *store){

    if (!self || !store) goto error;

    // there may be multiple bcb blocks

    struct container container = (struct container){
        .self = self,
        .store = store
    }; 

    return dtn_cbor_array_for_each(
        self->data, 
        &container,
        bcb_unprotect);
error:
    return false;
}