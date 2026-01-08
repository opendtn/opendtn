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
        @file           dtn_bpsec.c
        @author         Töpfer, Markus

        @date           2026-01-04


        ------------------------------------------------------------------------
*/
#include "../include/dtn_bpsec.h"

#include <dtn_base/dtn_string.h>
#include <dtn_base/dtn_dump.h>
#include <dtn_base/dtn_data_function.h>
#include <stdlib.h>


/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_bpsec_asb *dtn_bpsec_asb_create(){

    dtn_bpsec_asb *self = calloc(1, sizeof(dtn_bpsec_asb));
    if (!self) goto error;

    self->target = dtn_cbor_array();
    self->context_id = dtn_cbor_uint(0);
    self->context_flags = dtn_cbor_uint(0);
    self->source = dtn_cbor_array();
    self->context_parameter = dtn_cbor_array();
    self->results = dtn_cbor_array();

    return self;
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_bpsec_asb *dtn_bpsec_asb_free(dtn_bpsec_asb *self){

    if (!self) return NULL;

    self->target = dtn_cbor_free(self->target);
    self->context_id = dtn_cbor_free(self->context_id);
    self->context_flags = dtn_cbor_free(self->context_flags);
    self->source = dtn_cbor_free(self->source);
    self->context_parameter = dtn_cbor_free(self->context_parameter);
    self->results = dtn_cbor_free(self->results);
    self = dtn_data_pointer_free(self);
    return NULL;

}

/*
 *      ------------------------------------------------------------------------
 *
 *      GETTER / SETTER
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_bpsec_add_target(dtn_bpsec_asb *asb, uint64_t nbr){

    if (!asb) return false;
    return dtn_cbor_array_push(asb->target, dtn_cbor_uint(nbr));

}

/*----------------------------------------------------------------------------*/

uint64_t dtn_bpsec_count_targets(const dtn_bpsec_asb *asb){

    return dtn_cbor_array_count(asb->target);

}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_set_context_id(dtn_bpsec_asb *asb, uint64_t nbr){

    if (!asb) return false;
    return dtn_cbor_set_uint(asb->context_id, nbr);
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_bpsec_get_context_id(dtn_bpsec_asb *asb){

    if (!asb) return false;
    return dtn_cbor_get_uint(asb->context_id);
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_set_context_flags(dtn_bpsec_asb *asb, uint64_t nbr){

    if (!asb) return false;
    return dtn_cbor_set_uint(asb->context_flags, nbr);
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_bpsec_get_context_flags(dtn_bpsec_asb *asb){

    if (!asb) return false;
    return dtn_cbor_get_uint(asb->context_flags);
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_set_source(dtn_bpsec_asb *asb, dtn_dtn_uri *uri){

    if (!asb || !uri) return false;
    
    dtn_cbor *entry = dtn_cbor_array();
    if (!dtn_cbor_array_push(asb->source, entry)) goto error;

    if (0 == dtn_string_compare(uri->scheme, "dtn")){

        dtn_cbor_array_push(entry, dtn_cbor_uint(1));

        dtn_cbor *item = dtn_cbor_array();
        if (!dtn_cbor_array_push(entry, item)) goto error;
    
        if (!dtn_cbor_array_push(item, dtn_cbor_string(uri->name)))
            goto error;

        if (!dtn_cbor_array_push(item, dtn_cbor_string(uri->demux)))
            goto error;

    } else if (0 == dtn_string_compare(uri->scheme, "ipn")){

        dtn_cbor_array_push(entry, dtn_cbor_uint(2));

        dtn_cbor *item = dtn_cbor_array();
        if (!dtn_cbor_array_push(entry, item)) goto error;
    
        if (!dtn_cbor_array_push(item, dtn_cbor_string(uri->name)))
            goto error;

        if (!dtn_cbor_array_push(item, dtn_cbor_string(uri->demux)))
            goto error;

    } else {
        goto error;
    }

    

    return true;
error:
    return false;

}

/*----------------------------------------------------------------------------*/

struct container1 {

    uint64_t id;
    dtn_dtn_uri *uri;
    dtn_ipn *ipn;
};

/*----------------------------------------------------------------------------*/

static bool find_source_id(void *item, void *data){

    dtn_cbor *entry = (dtn_cbor*) item;
    struct container1 *container = (struct container1*) data;

    uint64_t id = dtn_cbor_get_uint(dtn_cbor_array_get(entry, 0));
    if (id != container->id) return true;

    dtn_cbor *uri = dtn_cbor_array_get(entry, 1);

    if (container->uri || container->ipn) goto error;
    
    switch (id) {

        case 1:

            container->uri = dtn_dtn_uri_create();
            container->uri->scheme = dtn_string_dup("dtn");
            container->uri->name = dtn_string_dup(
                dtn_cbor_get_string(dtn_cbor_array_get(uri,0)));
            container->uri->demux = dtn_string_dup(
                dtn_cbor_get_string(dtn_cbor_array_get(uri,1)));
            break;

        case 2:

            container->ipn = dtn_ipn_create();
            container->ipn->scheme = dtn_string_dup("ipn");
            container->ipn->node = dtn_string_dup(
                dtn_cbor_get_string(dtn_cbor_array_get(uri,0)));
            container->ipn->service = dtn_string_dup(
                dtn_cbor_get_string(dtn_cbor_array_get(uri,1)));
            break;

        default:
            goto error;
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_get_source(
    dtn_bpsec_asb *asb, 
    uint64_t id,
    dtn_dtn_uri **uri,
    dtn_ipn **ipn){

    struct container1 container = (struct container1){
        .id = id,
        .uri = NULL,
        .ipn = NULL
    };

    if (!dtn_cbor_array_for_each(
        asb->source, 
        &container,
        find_source_id)) goto error;

    *uri = container.uri;
    *ipn = container.ipn;

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_add_context_parameter(dtn_bpsec_asb *asb, uint64_t id, 
    dtn_cbor *value){

    if (!asb || !value) goto error;

    dtn_cbor *array = dtn_cbor_array();
    if (!dtn_cbor_array_push(asb->context_parameter, array)) goto error;

    if (!dtn_cbor_array_push(array, dtn_cbor_uint(id)))
        goto error;

    if (!dtn_cbor_array_push(array, value))
        goto error;

    return true;
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      DE / ENCODER
 *
 *      ------------------------------------------------------------------------
 */

dtn_bpsec_asb *dtn_bpsec_asb_decode(const dtn_cbor *byte_string){

    dtn_bpsec_asb *asb = calloc(1, sizeof(dtn_bpsec_asb));
    if (!asb) goto error;

    if (!byte_string) goto error;

    uint8_t *buffer = NULL;
    size_t size = 0;

    if (!dtn_cbor_get_byte_string(byte_string, &buffer, &size))
        goto error;

    uint8_t *ptr = buffer;

    if (DTN_CBOR_MATCH_FULL != dtn_cbor_decode(
        ptr, size, &asb->target, &ptr)) goto error;

    if (DTN_CBOR_MATCH_FULL != dtn_cbor_decode(
        ptr, size - (ptr - buffer), &asb->context_id, &ptr)) goto error;

    if (DTN_CBOR_MATCH_FULL != dtn_cbor_decode(
        ptr, size - (ptr - buffer), &asb->context_flags, &ptr)) goto error;

    if (DTN_CBOR_MATCH_FULL != dtn_cbor_decode(
        ptr, size - (ptr - buffer), &asb->source, &ptr)) goto error;

    if (DTN_CBOR_MATCH_FULL != dtn_cbor_decode(
        ptr, size - (ptr - buffer), &asb->context_parameter, &ptr)) goto error;

    if (ptr - buffer == (int64_t) size){

        asb->results = asb->context_parameter;
        asb->context_parameter = NULL;

    } else {

        if (DTN_CBOR_MATCH_FULL != dtn_cbor_decode(
            ptr, size - (ptr - buffer), &asb->results, &ptr)) goto error;

    }

    if (ptr - buffer != (int64_t)size) goto error;

    return asb;
error:
    dtn_bpsec_asb_free(asb);
    return NULL;
}

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_bpsec_asb_encode(const dtn_bpsec_asb *asb){

    dtn_cbor *cbor = NULL;

    uint8_t buffer[4096] = {0};
    size_t size = 4096;
    uint8_t *ptr = buffer;

    if (!asb) goto error;
    if (!asb->target) goto error;
    if (!asb->context_id) goto error;
    if (!asb->context_flags) goto error;
    if (!asb->source) goto error;
    if (!asb->results) goto error;

    if (!dtn_cbor_encode(asb->target, ptr, size, &ptr)) 
        goto error;

    if (!dtn_cbor_encode(asb->context_id, 
        ptr, size - (ptr - buffer), &ptr)) goto error;

    if (!dtn_cbor_encode(asb->context_flags, 
        ptr, size - (ptr - buffer), &ptr)) goto error;

    if (!dtn_cbor_encode(asb->source, 
        ptr, size - (ptr - buffer), &ptr)) goto error;

    if (asb->context_parameter)
        if (!dtn_cbor_encode(asb->context_parameter, 
            ptr, size - (ptr - buffer), &ptr)) goto error;

    if (!dtn_cbor_encode(asb->results, 
        ptr, size - (ptr - buffer), &ptr)) goto error;

    size = ptr - buffer;
    cbor = dtn_cbor_string(NULL);
    if (!dtn_cbor_set_byte_string(cbor, buffer, size))
        goto error;

    return cbor;

error:
    cbor = dtn_cbor_free(cbor);
    return NULL;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      GETTER / SETTER RFC 9173
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_bpsec_add_aes_variant(dtn_bpsec_asb *asb, dtn_bpsec_aes_variant aes){

    if (!asb) goto error;
    
    if (!asb->context_parameter)
        asb->context_parameter = dtn_cbor_array();

    dtn_cbor *param = dtn_cbor_uint(0);
    switch (aes) {

        case A128GCM:
            dtn_cbor_set_uint(param, 1);
            break;

        case A256GCM:
            dtn_cbor_set_uint(param, 3);
            break;
    }

    return dtn_bpsec_add_context_parameter(asb, 2, param);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

dtn_bpsec_aes_variant dtn_bpsec_get_aes_variant(const dtn_bpsec_asb *asb){

    if (!asb || !asb->context_parameter) return A256GCM;

    dtn_cbor *parameter = dtn_cbor_array_get(asb->context_parameter, 1);
    if (!dtn_cbor_is_array(parameter)) goto error;

    if (2 != dtn_cbor_get_uint(dtn_cbor_array_get(parameter, 0)))
        goto error;

    dtn_cbor *value = dtn_cbor_array_get(parameter, 1);

    uint64_t variant = dtn_cbor_get_uint(value);

    switch (variant){

        case 1: 
            return A128GCM;
        case 3:
            return A256GCM;

        default:
            goto error;
    }
error:
    return A256GCM;
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_add_sha_variant(dtn_bpsec_asb *asb, dtn_bpsec_sha_variant sha){

    if (!asb) goto error;
    
    if (!asb->context_parameter)
        asb->context_parameter = dtn_cbor_array();

    dtn_cbor *param = dtn_cbor_uint(0);
    switch (sha) {

        case HMAC256:
            dtn_cbor_set_uint(param, 5);
            break;

        case HMAC384:
            dtn_cbor_set_uint(param, 6);
            break;

        case HMAC512:
            dtn_cbor_set_uint(param, 7);
            break;
    }

    return dtn_bpsec_add_context_parameter(asb, 1, param);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

dtn_bpsec_sha_variant dtn_bpsec_get_sha_variant(const dtn_bpsec_asb *asb){

    if (!asb || !asb->context_parameter) return HMAC384;

    dtn_cbor *parameter = dtn_cbor_array_get(asb->context_parameter, 0);
    if (!dtn_cbor_is_array(parameter)) goto error;

    if (1 != dtn_cbor_get_uint(dtn_cbor_array_get(parameter, 0)))
        goto error;

    dtn_cbor *value = dtn_cbor_array_get(parameter, 1);

    uint64_t variant = dtn_cbor_get_uint(value);

    switch (variant){

        case 5: 
            return HMAC256;
        case 6:
            return HMAC384;
        case 7:
            return HMAC512;

        default:
            goto error;
    }
error:
    return HMAC384;
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_add_wrapped_key(dtn_bpsec_asb *asb, const uint8_t* key, size_t size){

    if (!asb || !asb->context_parameter) goto error;

    dtn_cbor *param = dtn_cbor_string(NULL);
    dtn_cbor_set_byte_string(param, key, size);
    return dtn_bpsec_add_context_parameter(asb, 2, param);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_get_wrapped_key(dtn_bpsec_asb *asb, uint8_t **out, size_t *size){

    if (!asb || !asb->context_parameter) goto error;

    dtn_cbor *parameter = dtn_cbor_array_get(asb->context_parameter, 1);
    if (!dtn_cbor_is_array(parameter)) goto error;

    if (2 != dtn_cbor_get_uint(dtn_cbor_array_get(parameter, 0)))
        goto error;

    dtn_cbor *value = dtn_cbor_array_get(parameter, 1);
    dtn_cbor_get_byte_string(value, out, size);
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_add_iv(dtn_bpsec_asb *asb, const uint8_t* key, size_t size){

    if (!asb || !asb->context_parameter) goto error;

    dtn_cbor *param = dtn_cbor_string(NULL);
    dtn_cbor_set_byte_string(param, key, size);
    return dtn_bpsec_add_context_parameter(asb, 1, param);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_get_iv(dtn_bpsec_asb *asb, uint8_t **out, size_t *size){

    if (!asb || !asb->context_parameter) goto error;

    dtn_cbor *parameter = dtn_cbor_array_get(asb->context_parameter, 0);
    if (!dtn_cbor_is_array(parameter)) goto error;

    if (1 != dtn_cbor_get_uint(dtn_cbor_array_get(parameter, 0)))
        goto error;

    dtn_cbor *value = dtn_cbor_array_get(parameter, 1);
    dtn_cbor_get_byte_string(value, out, size);
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_add_integrity_flags_bib(dtn_bpsec_asb *asb, uint8_t flags){

     if (!asb || !asb->context_parameter) goto error;

    dtn_cbor *param = dtn_cbor_uint(flags);
    return dtn_bpsec_add_context_parameter(asb, 3, param);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_add_integrity_flags_bcb(dtn_bpsec_asb *asb, uint8_t flags){

     if (!asb || !asb->context_parameter) goto error;

    dtn_cbor *param = dtn_cbor_uint(flags);
    return dtn_bpsec_add_context_parameter(asb, 4, param);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_get_integrity_flags_bib(dtn_bpsec_asb *asb, uint64_t *flags){

    if (!asb || !asb->context_parameter) goto error;

    uint64_t count = dtn_cbor_array_count(asb->context_parameter);

    dtn_cbor *parameter = dtn_cbor_array_get(asb->context_parameter, count - 1);
    if (!dtn_cbor_is_array(parameter)) goto error;

    if (3 != dtn_cbor_get_uint(dtn_cbor_array_get(parameter, 0)))
        goto error;

    dtn_cbor *value = dtn_cbor_array_get(parameter, 1);
    uint64_t data = dtn_cbor_get_uint(value);
    *flags = data;
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_get_integrity_flags_bcb(dtn_bpsec_asb *asb, uint64_t *flags){

    if (!asb || !asb->context_parameter) goto error;

    uint64_t count = dtn_cbor_array_count(asb->context_parameter);

    dtn_cbor *parameter = dtn_cbor_array_get(asb->context_parameter, count - 1);
    if (!dtn_cbor_is_array(parameter)) goto error;

    if (4 != dtn_cbor_get_uint(dtn_cbor_array_get(parameter, 0)))
        goto error;

    dtn_cbor *value = dtn_cbor_array_get(parameter, 1);
    uint64_t data = dtn_cbor_get_uint(value);
    *flags = data;
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_add_result(dtn_bpsec_asb *asb, uint8_t *data, size_t size){

    if (!asb) goto error;
    if (!asb->results) asb->results = dtn_cbor_array();

    uint64_t id = dtn_cbor_array_count(asb->results) + 1;

    dtn_cbor *result = dtn_cbor_array();
    dtn_cbor_array_push(result, dtn_cbor_uint(id));
    dtn_cbor *string = dtn_cbor_string(NULL);
    dtn_cbor_set_byte_string(string, data, size);
    dtn_cbor_array_push(result, string);
    return dtn_cbor_array_push(asb->results, result);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

struct container {

    dtn_bpsec_asb *asb;
    uint64_t id;
    dtn_cbor *result;
};

/*----------------------------------------------------------------------------*/

static bool find_result_by_id(void *item, void *data){

    struct container *container = (struct container*) data;
    dtn_cbor *array = (dtn_cbor*) item;
    dtn_cbor *value = dtn_cbor_array_get(array, 0);
    if (container->id == dtn_cbor_get_uint(value))
        container->result = array;

    return true;
}

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_get_result(dtn_bpsec_asb *asb, 
    uint64_t id, uint8_t **data, size_t *size){

    if (!asb || !asb->results) goto error;

    struct container container = (struct container){
        .asb = asb,
        .id = id,
        .result = NULL
    };

    dtn_cbor_array_for_each(asb->results, &container, find_result_by_id);

    if (!container.result) goto error;

    dtn_cbor *item = dtn_cbor_array_get(container.result, 1);
    if (!dtn_cbor_get_byte_string(item, data, size)) goto error;

    return true;

error:
    return false;
}
