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
        @file           dtn_cbor.h
        @author         Töpfer, Markus

        @date           2025-12-12

        Implementation of RFC 8949 Concise Binary Object Representation (CBOR)

        ------------------------------------------------------------------------
*/
#ifndef dtn_cbor_h
#define dtn_cbor_h

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

/*----------------------------------------------------------------------------*/

typedef enum dtn_cbor_type {

    DTN_CBOR_UNDEF = 0,
    DTN_CBOR_FALSE,
    DTN_CBOR_TRUE,
    DTN_CBOR_NULL,
    DTN_CBOR_UINT64,
    DTN_CBOR_INT64,
    DTN_CBOR_STRING,
    DTN_CBOR_UTF8,
    DTN_CBOR_ARRAY,
    DTN_CBOR_MAP,
    DTN_CBOR_DATE_TIME,
    DTN_CBOR_DATE_TIME_EPOCH,
    DTN_CBOR_UBIGNUM,
    DTN_CBOR_IBIGNUM,
    DTN_CBOR_DEC_FRACTION,
    DTN_CBOR_BIGFLOAT,
    DTN_CBOR_TAG,
    DTN_CBOR_SIMPLE,
    DTN_CBOR_FLOAT,
    DTN_CBOR_DOUBLE

} dtn_cbor_type;

/*----------------------------------------------------------------------------*/

typedef struct dtn_cbor dtn_cbor;

/*----------------------------------------------------------------------------*/

typedef struct dtn_cbor_config {

    struct {

        uint64_t string_size;
        uint64_t utf8_string_size;
        uint64_t array_size;
        uint64_t undef_length_array;
        uint64_t map_size;
        uint64_t undef_length_map;

    } limits;

} dtn_cbor_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_cbor *dtn_cbor_map();
dtn_cbor *dtn_cbor_array();
dtn_cbor *dtn_cbor_string(const char *string);
dtn_cbor *dtn_cbor_utf8(const uint8_t *buffer , size_t size);
dtn_cbor *dtn_cbor_true();
dtn_cbor *dtn_cbor_false();
dtn_cbor *dtn_cbor_null();
dtn_cbor *dtn_cbor_undef();
dtn_cbor *dtn_cbor_uint(uint64_t value);
dtn_cbor *dtn_cbor_int(int64_t value);
dtn_cbor *dtn_cbor_time(const char *timestamp);
dtn_cbor *dtn_cbor_time_epoch(uint64_t value);
dtn_cbor *dtn_cbor_ubignum(const char *num);
dtn_cbor *dtn_cbor_ibignum(const char *num);
dtn_cbor *dtn_cbor_dec_fraction(dtn_cbor *array);
dtn_cbor *dtn_cbor_bigfloat(dtn_cbor *array);
dtn_cbor *dtn_cbor_tag(uint64_t tag);
dtn_cbor *dtn_cbor_simple(uint64_t nbr);
dtn_cbor *dtn_cbor_float(float nbr);
dtn_cbor *dtn_cbor_double(double nbr);

dtn_cbor *dtn_cbor_free(dtn_cbor *self);
dtn_cbor_type dtn_cbor_get_type(const dtn_cbor *self);

/*----------------------------------------------------------------------------*/

/**
 *  Configure MAX item values for the parser.
 * 
 *  SHOULD be used for every usage szenario due to potentially very large 
 *  allocation of memory.
 */
bool dtn_cbor_configure(dtn_cbor_config config);

/*
 *      ------------------------------------------------------------------------
 *
 *      DE/ENCODER
 *
 *      ------------------------------------------------------------------------
 */

typedef enum dtn_cbor_match {

    DTN_CBOR_NO_MATCH = 0,
    DTN_CBOR_MATCH_PARTIAL = 1,
    DTN_CBOR_MATCH_FULL = 2

} dtn_cbor_match;

/*----------------------------------------------------------------------------*/

/**
 *  Decode a CBOR buffer to some value.
 * 
 *  @param buffer   pointer to buffer to decode
 *  @param size     size of buffer
 *  @param out      pointer to decoded value
 *  @param next     pointer to next byte after decoded value
 */
dtn_cbor_match dtn_cbor_decode(
    const uint8_t *buffer, 
    size_t size,
    dtn_cbor **out, 
    uint8_t **next);

/*----------------------------------------------------------------------------*/

/**
 *  Encode a CBOR value to some buffer.
 *  
 *  @param value    value to encode
 *  @param buffer   pointer to buffer
 *  @param size     size of buffer
 *  @param next     pointer to next byte after encoded value
 */
bool dtn_cbor_encode(
    const dtn_cbor *value,
    uint8_t *buffer, 
    size_t size,
    uint8_t **next);

/*----------------------------------------------------------------------------*/

/**
 *  Encode a CBOR array as a indefinite_length value to some buffer.
 *  This is the implemetation required for DTN bundle protocol RFC 9171
 *  
 *  @param value    value to encode
 *  @param buffer   pointer to buffer
 *  @param size     size of buffer
 *  @param next     pointer to next byte after encoded value
 */
bool dtn_cbor_encode_array_of_indefinite_length(
    const dtn_cbor *value,
    uint8_t *buffer, 
    size_t size,
    uint8_t **next);

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_encoding_size(const dtn_cbor *value);

/*
 *      ------------------------------------------------------------------------
 *
 *      ITEM CHECKS
 *
 *      ------------------------------------------------------------------------
 */

bool dtn_cbor_is_map(const dtn_cbor *self);
bool dtn_cbor_is_array(const dtn_cbor *self);
bool dtn_cbor_is_string(const dtn_cbor *self);
bool dtn_cbor_is_uft8(const dtn_cbor *self);
bool dtn_cbor_is_true(const dtn_cbor *self);
bool dtn_cbor_is_false(const dtn_cbor *self);
bool dtn_cbor_is_null(const dtn_cbor *self);
bool dtn_cbor_is_undef(const dtn_cbor *self);
bool dtn_cbor_is_uint(const dtn_cbor *self);
bool dtn_cbor_is_int(const dtn_cbor *self);
bool dtn_cbor_is_time(const dtn_cbor *self);
bool dtn_cbor_is_time_epoch(const dtn_cbor *self);
bool dtn_cbor_is_ubignum(const dtn_cbor *self);
bool dtn_cbor_is_ibignum(const dtn_cbor *self);
bool dtn_cbor_is_dec_fraction(const dtn_cbor *self);
bool dtn_cbor_is_bigfloat(const dtn_cbor *self);
bool dtn_cbor_is_tag(const dtn_cbor *self);
bool dtn_cbor_is_simple(const dtn_cbor *self);
bool dtn_cbor_is_float(const dtn_cbor *self);
bool dtn_cbor_is_double(const dtn_cbor *self);

/*
 *      ------------------------------------------------------------------------
 *
 *      MAP GETTER / SETTER
 *
 *      ------------------------------------------------------------------------
 */

/**
 *  Set some key / value pair, 
 *  on success key and value become part of the map 
 *  @param map  map instance
 *  @param key  key to set in map
 *  @param val  value to set at key.
 */
bool dtn_cbor_map_set(dtn_cbor *map, dtn_cbor *key, dtn_cbor *val);

/*----------------------------------------------------------------------------*/

/**
 *  Get some map value based on a dtn_cbor key.
 *  @param map  map instance
 *  @param key  key to get
 */
dtn_cbor *dtn_cbor_map_get(const dtn_cbor *map, const dtn_cbor *key);

/*----------------------------------------------------------------------------*/

/**
 *  Set some value at a string key, the dtn_cbor_string for the key
 *  will be autgenerated. 
 *  This is a convinience function for usage of string key based maps. 
 * 
 *  @param map  map instance
 *  @param key  string to set 
 *  @param val  value to be set at key, value will become part of the map
 */
bool dtn_cbor_map_set_string(dtn_cbor *map, const char *key, dtn_cbor *val);

/*----------------------------------------------------------------------------*/

/**
 *  Get some string based key using the string instead of a dtn_cbor string.
 *  This is a convinience function for usage of string key based maps.
 * 
 *  @param map  map instance
 *  @param key  keystring to search
 */
dtn_cbor *dtn_cbor_map_get_string(const dtn_cbor *map, const char *key);

/*----------------------------------------------------------------------------*/

/**
 *  This function will apply the function input on any item of the map. 
 *  Use with care and don't delete keys or values using this function. 
 *  
 *  @param map      map instance
 *  @param data     custom userdata to be used as input to the function
 *  @param function function to be applied to any key/value pair 
 */
bool dtn_cbor_map_for_each(dtn_cbor *map,
    void *data,
    bool (*function)(const void *key, void *val, void *data));

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_map_count(const dtn_cbor *map);

/*
 *      ------------------------------------------------------------------------
 *
 *      ARRAY GETTER / SETTER
 *
 *      ------------------------------------------------------------------------
 */

/**
 *  Get some item out of the array. 
 *  
 *  @param self     array instance
 *  @param index    index 0 ... max
 */
const dtn_cbor *dtn_cbor_array_get(const dtn_cbor *self, uint64_t index);

/*----------------------------------------------------------------------------*/

/**
 *  Push some item to the end of the array. 
 *  
 *  @param self     array instance
 *  @param val      value to be set within the array.
 */
bool dtn_cbor_array_push(dtn_cbor *self, dtn_cbor *val);

/*----------------------------------------------------------------------------*/

/**
 *  Pop some item from the front of the array FIFO. 
 */
dtn_cbor *dtn_cbor_array_pop_queue(dtn_cbor *self);

/*----------------------------------------------------------------------------*/

/**
 *  Pop some item from the back of the array LIFO. 
 */
dtn_cbor *dtn_cbor_array_pop_stack(dtn_cbor *self);

/*----------------------------------------------------------------------------*/

/**
 *  This function will apply the function input on any item of the array. 
 *  Use with care and don't delete values using this function. 
 *  
 *  @param self     array instance
 *  @param data     custom userdata to be used as input to the function
 *  @param function function to be applied to any key/value pair 
 */
bool dtn_cbor_array_for_each(dtn_cbor *self, 
    void *data,
    bool (*function)(void *item, void *data));

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_array_count(const dtn_cbor *array);

/*
 *      ------------------------------------------------------------------------
 *
 *      GETTER / SETTER
 *
 *      ------------------------------------------------------------------------
 */

const char *dtn_cbor_get_string(const dtn_cbor *self);
bool dtn_cbor_set_string(dtn_cbor *self, const char *string);

/*----------------------------------------------------------------------------*/

bool dtn_cbor_get_utf8(dtn_cbor *self, uint8_t **buffer, size_t *size);
bool dtn_cbor_set_utf8(dtn_cbor *self, const uint8_t *buffer, size_t size);

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_get_uint(const dtn_cbor *self);
bool dtn_cbor_set_uint(dtn_cbor *self, uint64_t value);

/*----------------------------------------------------------------------------*/

int64_t dtn_cbor_get_int(const dtn_cbor *self);
bool dtn_cbor_set_int(dtn_cbor *self, int64_t value);

/*----------------------------------------------------------------------------*/

const char *dtn_cbor_get_time(const dtn_cbor *self);
bool dtn_cbor_set_time(dtn_cbor *self, const char *timestamp);

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_get_time_epoch(const dtn_cbor *self);
bool dtn_cbor_set_time_epoch(dtn_cbor *self, uint64_t value);

/*----------------------------------------------------------------------------*/

const char *dtn_cbor_get_ubignum(const dtn_cbor *self);
bool dtn_cbor_set_ubignum(dtn_cbor *self, const char *num);

/*----------------------------------------------------------------------------*/

const char *dtn_cbor_get_ibignum(const dtn_cbor *self);
bool dtn_cbor_set_ibignum(dtn_cbor *self, const char *num);

/*----------------------------------------------------------------------------*/

const dtn_cbor *dtn_cbor_get_dec_fraction(const dtn_cbor *self);
bool dtn_cbor_set_dec_fraction(dtn_cbor *self, dtn_cbor *array);

/*----------------------------------------------------------------------------*/

const dtn_cbor *dtn_cbor_get_bigfloat(const dtn_cbor *self);
bool dtn_cbor_set_bigfloat(dtn_cbor *self, dtn_cbor *array);

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_get_tag(const dtn_cbor *self);
uint64_t dtn_cbor_get_tag_value(const dtn_cbor *self);
const dtn_cbor *dtn_cbor_get_tag_data(const dtn_cbor *self);
bool dtn_cbor_set_tag(dtn_cbor *self, uint64_t tag);
bool dtn_cbor_set_tag_data(dtn_cbor *self, dtn_cbor *data);
bool dtn_cbor_set_tag_value(dtn_cbor *self, uint64_t val);

/*----------------------------------------------------------------------------*/

uint64_t dtn_cbor_get_simple_value(const dtn_cbor *self);
uint64_t dtn_cbor_get_simple(const dtn_cbor *self);
bool dtn_cbor_set_simple(dtn_cbor *self, uint64_t nbr);
bool dtn_cbor_set_simple_value(dtn_cbor *self, uint64_t nbr);

/*----------------------------------------------------------------------------*/

float dtn_cbor_get_float(const dtn_cbor *self);
bool dtn_cbor_set_float(dtn_cbor *self, float nbr);

/*----------------------------------------------------------------------------*/

double dtn_cbor_get_double(const dtn_cbor *self);
bool dtn_cbor_set_double(dtn_cbor *self, double nbr);

#endif /* dtn_cbor_h */