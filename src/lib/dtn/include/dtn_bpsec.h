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
        @file           dtn_bpsec.h
        @author         Töpfer, Markus

        @date           2026-01-04


        ------------------------------------------------------------------------
*/
#ifndef dtn_bpsec_h
#define dtn_bpsec_h

#include "dtn_cbor.h"
#include "dtn_dtn_uri.h"
#include "dtn_ipn.h"

/*----------------------------------------------------------------------------*/

typedef enum dtn_bpsec_sha_variant {

    HMAC256 = 5,
    HMAC384 = 6,
    HMAC512 = 7

} dtn_bpsec_sha_variant;

/*----------------------------------------------------------------------------*/

typedef enum dtn_bpsec_aes_variant {

    A128GCM = 1,
    A256GCM = 2

} dtn_bpsec_aes_variant;

/*----------------------------------------------------------------------------*/

typedef struct dtn_bpsec_asb {

    dtn_cbor *target;            // array
    dtn_cbor *context_id;        // uint
    dtn_cbor *context_flags;     // uint
    dtn_cbor *source;            // array EID
    dtn_cbor *context_parameter; // array
    dtn_cbor *results;           // array

} dtn_bpsec_asb;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_bpsec_asb *dtn_bpsec_asb_create();
dtn_bpsec_asb *dtn_bpsec_asb_free(dtn_bpsec_asb *asb);

/*
 *      ------------------------------------------------------------------------
 *
 *      GETTER / SETTER RFC 9173
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_bpsec_add_aes_variant(dtn_bpsec_asb *asb, dtn_bpsec_aes_variant aes);
dtn_bpsec_aes_variant dtn_bpsec_get_aes_variant(const dtn_bpsec_asb *asb);

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_add_sha_variant(dtn_bpsec_asb *asb, dtn_bpsec_sha_variant sha);
dtn_bpsec_sha_variant dtn_bpsec_get_sha_variant(const dtn_bpsec_asb *asb);

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_add_iv(dtn_bpsec_asb *asb, const uint8_t *key, size_t size);
bool dtn_bpsec_get_iv(dtn_bpsec_asb *asb, uint8_t **out, size_t *size);

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_add_wrapped_key(dtn_bpsec_asb *asb, const uint8_t *key,
                               size_t size);
bool dtn_bpsec_get_wrapped_key(dtn_bpsec_asb *asb, uint8_t **out, size_t *size);

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_add_integrity_flags_bib(dtn_bpsec_asb *asb, uint8_t flags);
bool dtn_bpsec_add_integrity_flags_bcb(dtn_bpsec_asb *asb, uint8_t flags);
bool dtn_bpsec_get_integrity_flags_bib(dtn_bpsec_asb *asb, uint64_t *flags);
bool dtn_bpsec_get_integrity_flags_bcb(dtn_bpsec_asb *asb, uint64_t *flags);

/*----------------------------------------------------------------------------*/

bool dtn_bpsec_add_result(dtn_bpsec_asb *asb, uint8_t *data, size_t size);
bool dtn_bpsec_get_result(dtn_bpsec_asb *asb, uint64_t id, uint8_t **data,
                          size_t *size);

/*
 *      ------------------------------------------------------------------------
 *
 *      GETTER / SETTER
 *
 *      ------------------------------------------------------------------------
 */

uint64_t dtn_bpsec_count_targets(const dtn_bpsec_asb *asb);
bool dtn_bpsec_add_target(dtn_bpsec_asb *asb, uint64_t nbr);

bool dtn_bpsec_set_context_id(dtn_bpsec_asb *asb, uint64_t nbr);
uint64_t dtn_bpsec_get_context_id(dtn_bpsec_asb *asb);

bool dtn_bpsec_set_context_flags(dtn_bpsec_asb *asb, uint64_t nbr);
uint64_t dtn_bpsec_get_context_flags(dtn_bpsec_asb *asb);

bool dtn_bpsec_set_source(dtn_bpsec_asb *asb, dtn_dtn_uri *uri);
bool dtn_bpsec_get_source(dtn_bpsec_asb *asb, uint64_t id, dtn_dtn_uri **uri,
                          dtn_ipn **ipn);

bool dtn_bpsec_add_context_parameter(dtn_bpsec_asb *asb, uint64_t id,
                                     dtn_cbor *value);

/*
 *      ------------------------------------------------------------------------
 *
 *      DE / ENCODER
 *
 *      ------------------------------------------------------------------------
 */

/**
 *  Decode a byte_string to ASB
 */
dtn_bpsec_asb *dtn_bpsec_asb_decode(const dtn_cbor *byte_string);

/*----------------------------------------------------------------------------*/

/**
 *  Encode an ASB to cbor byte_string
 */
dtn_cbor *dtn_bpsec_asb_encode(const dtn_bpsec_asb *asb);

#endif /* dtn_bpsec_h */
