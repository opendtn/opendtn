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
        @file           dtn_bundle.h
        @author         Töpfer, Markus

        @date           2025-12-15

        Implementation of RFC 9171 Bundle Protocol Version 7 - bundle 

        NOTE This file does not contain any bundle handling, only decoding,
        enconding and item access.

        ------------------------------------------------------------------------
*/
#ifndef dtn_bundle_h
#define dtn_bundle_h

#define DTN_BUNDLE_CRC16 "crc16"
#define DTN_BUNDLE_CRC32 "crc32"

#include <stdio.h>
#include "dtn_cbor.h"

typedef struct dtn_bundle dtn_bundle;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_bundle *dtn_bundle_create();
dtn_bundle *dtn_bundle_free(dtn_bundle *self);

// data functions
bool dtn_bundle_clear(void *bundle);
bool dtn_bundle_dump(FILE *stream, void *bundle);
void *dtn_bundle_copy(void **destination, const void *source);
void *dtn_bundle_free_void(void *self);

/*
 *      ------------------------------------------------------------------------
 *
 *      DE/ENCODER
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor_match dtn_bundle_decode(
    const uint8_t *buffer, 
    size_t size,
    dtn_bundle **out, 
    uint8_t **next);

/*----------------------------------------------------------------------------*/

bool dtn_bundle_encode(
    dtn_bundle *self,
    uint8_t *buffer, 
    size_t size,
    uint8_t **next);

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_encoding_size(const dtn_bundle *self);

/*
 *      ------------------------------------------------------------------------
 *
 *      VERIFY
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_bundle_verify(dtn_bundle *self);

/*
 *      ------------------------------------------------------------------------
 *
 *      GETTER / SETTER PRIMARY
 *
 *      ------------------------------------------------------------------------
 */

/**     
 *      All in one to set a primary block. 
 * 
 *      When in doubt about the order use this function to create the
 *      primary block and use getter for items later. 
 */
dtn_cbor *dtn_bundle_add_primary_block(
        dtn_bundle *self, 
        uint64_t flags, 
        uint64_t crc_type,
        const char *destination,
        const char *source,
        const char *report_to,
        uint64_t timestamp,
        uint64_t sequence_number,
        uint64_t lifetime,
        uint64_t optional_fragment_offset,
        uint64_t optional_total_data_length);

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_primary_get_version(const dtn_bundle *self);
bool dtn_bundle_primary_set_version(dtn_bundle *self);

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_primary_get_flags(const dtn_bundle *self);
bool dtn_bundle_primary_set_flags(dtn_bundle *self, uint64_t flags);

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_primary_get_crc_type(const dtn_bundle *self);
bool dtn_bundle_primary_set_crc_type(dtn_bundle *self, uint64_t type);

/*----------------------------------------------------------------------------*/

const char *dtn_bundle_primary_get_destination(const dtn_bundle *self);
bool dtn_bundle_primary_set_destination(dtn_bundle *self, const char *value);

/*----------------------------------------------------------------------------*/

const char *dtn_bundle_primary_get_source(const dtn_bundle *self);
bool dtn_bundle_primary_set_source(dtn_bundle *self, const char *value);

/*----------------------------------------------------------------------------*/

const char *dtn_bundle_primary_get_report(const dtn_bundle *self);
bool dtn_bundle_primary_set_report(dtn_bundle *self, const char *value);

/*----------------------------------------------------------------------------*/

bool dtn_bundle_primary_get_timestamp(const dtn_bundle *self, 
        uint64_t *time, uint64_t *sequence_number);
bool dtn_bundle_primary_set_timestamp(dtn_bundle *self, 
        uint64_t time, uint64_t sequence_number);

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_primary_get_lifetime(const dtn_bundle *self);
bool dtn_bundle_primary_set_lifetime(dtn_bundle *self, uint64_t type);

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_primary_get_fragment_offset(const dtn_bundle *self);
bool dtn_bundle_primary_set_fragment_offset(dtn_bundle *self, uint64_t type);

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_primary_get_totel_data_length(const dtn_bundle *self);
bool dtn_bundle_primary_set_total_data_length(dtn_bundle *self, uint64_t type);

/*
 *      ------------------------------------------------------------------------
 *
 *      GETTER / SETTER CANNONICAL
 *
 *      ------------------------------------------------------------------------
 */

/**     
 *      All in one to set a block. 
 * 
 *      When in doubt about the order use this function to create the
 *      block and use getter for items later. 
 */
dtn_cbor *dtn_bundle_add_block(
        dtn_bundle *self, 
        uint64_t code, 
        uint64_t nbr,
        uint64_t flags,
        uint64_t crc_type,
        dtn_cbor *payload);

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_bundle_get_block(dtn_bundle *self, uint64_t nbr);

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_get_code(const dtn_cbor *self);
bool dtn_bundle_set_code(dtn_cbor *self, uint64_t number);

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_get_number(const dtn_cbor *self);
bool dtn_bundle_set_number(dtn_cbor *self, uint64_t number);

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_get_flags(const dtn_cbor *self);
bool dtn_bundle_set_flags(dtn_cbor *self, uint64_t number);

/*----------------------------------------------------------------------------*/

uint64_t dtn_bundle_get_crc_type(const dtn_cbor *self);
bool dtn_bundle_set_crc_type(dtn_cbor *self, uint64_t number);

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_bundle_get_data(const dtn_cbor *self);
bool dtn_bundle_set_data(dtn_cbor *self, dtn_cbor *data);

/*
 *      ------------------------------------------------------------------------
 *
 *      SPECIAL Functionality
 *
 *      ------------------------------------------------------------------------
 */

/**
 *      Get the "RAW" CBOR instantiation of the bundle. 
 */ 
dtn_cbor *dtn_bundle_get_raw(const dtn_bundle *self);

/*----------------------------------------------------------------------------*/

/**
 *      Set a "RAW" CBOR instantiation to the bundle. 
 */
bool dtn_bundle_set_raw(dtn_bundle *self, dtn_cbor *array);

/*
 *      ------------------------------------------------------------------------
 *
 *      SPECIAL BLOCKS
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor *dtn_bundle_add_previous_node(
        dtn_bundle *self, 
        const char *node_id);

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_bundle_add_bundle_age(
        dtn_bundle *self, 
        uint64_t age);

/*----------------------------------------------------------------------------*/

dtn_cbor *dtn_bundle_add_hop_count(
        dtn_bundle *self, 
        uint64_t count,
        uint64_t limit);

/*----------------------------------------------------------------------------*/


#endif /* dtn_bundle_h */
