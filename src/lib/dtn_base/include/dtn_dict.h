/***
        ------------------------------------------------------------------------

        Copyright (c) 2018 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_dict.h
        @author         Markus Toepfer
        @author         Michael Beer

        @date           2018-08-15

        @ingroup        dtn_base

        @brief          Definition of a standard interface for KEY/VALUE
                        based implementations used for openvocs.


        ------------------------------------------------------------------------
*/
#ifndef dtn_dict_h
#define dtn_dict_h

#include "dtn_data_function.h"
#include "dtn_hash_functions.h"
#include "dtn_list.h"
#include "dtn_match_functions.h"

#include "dtn_log.h"

#define dtn_DICT_MAGIC_BYTE 0xaabb

typedef struct dtn_dict dtn_dict;
typedef struct dtn_dict_config dtn_dict_config;

struct dtn_dict_config {

    /* Buckets to be used */
    uint64_t slots;

    /* Key confguration */
    struct {

        /* Key content configuration */
        dtn_data_function data_function;

        /* Key functions configuration */
        uint64_t (*hash)(const void *key);
        bool (*match)(const void *key, const void *value);

        bool (*validate_input)(const void *data);
    } key;

    struct {

        /* Value content configuration */
        dtn_data_function data_function;
        bool (*validate_input)(const void *data);
    } value;
};

/*---------------------------------------------------------------------------*/

struct dtn_dict {

    uint16_t magic_byte;
    uint16_t type;

    dtn_dict_config config;

    /*      Check if any ANY item is set within the dict */
    bool (*is_empty)(const dtn_dict *self);

    /*
     *      Function pointer to own create.
     */
    dtn_dict *(*create)(dtn_dict_config config);

    /*
     *      Clear MUST delete all key/value pairs,
     *      using the configured configuration.
     */
    bool (*clear)(dtn_dict *self);

    /*
     *      Free MUST delete all key/value pairs,
     *      and free the dict pointer.
     *      @returns NULL on success, self on error!
     */
    dtn_dict *(*free)(dtn_dict *self);

    /*
     *      Check at all keys if the value pointer is contained.
     *      If value is 0, all keys are returned.
     *      return a list with pointers to all keys.
     */
    dtn_list *(*get_keys)(const dtn_dict *self, const void *value);

    /*
     *      Get MUST return the pointer to value used
     *      at key.
     */
    void *(*get)(const dtn_dict *self, const void *key);

    /*
     *      Set MUST set the value of key within the dict.
     *      If replaced is NULL, ANY existing value MUST be
     *      freed using the value configuration, if replaced is
     *      NOT NULL, any old value MUST be returned over replaced.
     */
    bool (*set)(dtn_dict *self, void *key, void *value, void **replaced);

    /*
     *      Del MUST remove the key/value pair and free the pointers
     *      using the dict configuration.
     */
    bool (*del)(dtn_dict *self, const void *key);

    /*
     *      Remove MUST remove the key/value pair and return the value,
     *      without deleting it automatically.
     */
    void *(*remove)(dtn_dict *self, const void *key);

    /*
     *      For_each MUST apply function at each key value pair.
     */
    bool (*for_each)(dtn_dict *self, void *data,
                     bool (*function)(const void *key, void *value,
                                      void *data));
};

/*
 *      ------------------------------------------------------------------------
 *
 *                        DEFAULT STRUCTURE CREATION
 *
 *      ------------------------------------------------------------------------
 */

/**
        Create a dict with a config. The config MUST be valid,
        which means it need to include a hash as well as a match
        function for keys, and a given slotsize > 0
        MOST used config will be @see dtn_dict_string_key_config
*/
dtn_dict *dtn_dict_create(dtn_dict_config config);

dtn_dict *dtn_dict_cast(const void *data);
dtn_dict *dtn_dict_set_magic_bytes(dtn_dict *dict);

/*
 *      ------------------------------------------------------------------------
 *
 *                        GENERAL TESTING
 *
 *      ------------------------------------------------------------------------
 */

/**
        Checks config and if all function pointers are set.
*/
bool dtn_dict_is_valid(const dtn_dict *dict);
bool dtn_dict_config_is_valid(const dtn_dict_config *config);
bool dtn_dict_is_set(const dtn_dict *dict, const void *key);
int64_t dtn_dict_count(const dtn_dict *dict);

/*---------------------------------------------------------------------------*/

/**
    Dump the load at the slots to file,
    NOTE will only work with standard dict yet.
*/
bool dtn_dict_dump_load_factor(FILE *stream, const dtn_dict *dict);

/*
 *      ------------------------------------------------------------------------
 *
 *                        FUNCTIONS TO INTERNAL POINTERS
 *
 *
 *      ... following functions check if the dict has the respective function
 * and execute the linked function.
 *      ------------------------------------------------------------------------
 */

bool dtn_dict_is_empty(const dtn_dict *dict);
dtn_list *dtn_dict_get_keys(const dtn_dict *dict, const void *value);
void *dtn_dict_get(const dtn_dict *dict, const void *key);
bool dtn_dict_set(dtn_dict *dict, void *key, void *value, void **replaced);
bool dtn_dict_del(dtn_dict *dict, const void *key);
void *dtn_dict_remove(dtn_dict *dict, const void *key);
bool dtn_dict_for_each(dtn_dict *dict, void *data,
                       bool (*function)(const void *key, void *value,
                                        void *data));

/*
 *      ------------------------------------------------------------------------
 *
 *                        DEFAULT CONFIGURATIONS
 *
 *      ------------------------------------------------------------------------
 */

/**
        Creates a config, which expects allocated strings as
        keys for a dictionary.

        {
                .slots                          = slots,

                .key.data_function.free         = dtn_data_string_free,
                .key.data_function.clear        = dtn_data_string_clear,
                .key.data_function.copy         = dtn_data_string_copy,
                .key.data_function.dump         = dtn_data_string_dump,

                .key.hash                       = dtn_hash_pearson_c_string,
                .key.match                      = dtn_match_c_string_strict,

                .value.data_function.free       = NULL,
                .value.data_function.clear      = NULL,
                .value.data_function.copy       = NULL,
                .value.data_function.dump       = NULL,

        }
*/
dtn_dict_config dtn_dict_string_key_config(size_t slots);

/*----------------------------------------------------------------------------*/

/**
        Creates a config, which expects intptr_t
        keys for a dictionary.

        {
                .slots                          = slots,

                .key.data_function.free         = NULL,
                .key.data_function.clear        = NULL,
                .key.data_function.copy         = NULL,
                .key.data_function.dump         = dtn_data_intptr_dump,

                .key.hash                       = dtn_hash_intptr,
                .key.match                      = dtn_match_intptr,

                .value.data_function.free       = NULL,
                .value.data_function.clear      = NULL,
                .value.data_function.copy       = NULL,
                .value.data_function.dump       = NULL,

        }
*/
dtn_dict_config dtn_dict_intptr_key_config(size_t slots);

/*----------------------------------------------------------------------------*/

/**
        Creates a config, which expects uint64_t pointer
        keys for a dictionary.

        {
                .slots                          = slots,

                .key.data_function.free         = dtn_data_uint64_free,
                .key.data_function.clear        = dtn_data_uint64_clear,
                .key.data_function.copy         = dtn_data_uint64_copy,
                .key.data_function.dump         = dtn_data_uint64_dump,

                .key.hash                       = dtn_hash_uint64,
                .key.match                      = dtn_match_uint64,

                .value.data_function.free       = NULL,
                .value.data_function.clear      = NULL,
                .value.data_function.copy       = NULL,
                .value.data_function.dump       = NULL,

        }
*/
dtn_dict_config dtn_dict_uint64_key_config(size_t slots);

/*
 *      ------------------------------------------------------------------------
 *
 *                        DATA FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_data_function dtn_dict_data_functions();
bool dtn_dict_clear(void *data);
void *dtn_dict_free(void *data);
void *dtn_dict_copy(void **destination, const void *data);
bool dtn_dict_dump(FILE *stream, const void *data);

/*
 *      ------------------------------------------------------------------------
 *
 *                        GENERIC FUNCTIONS
 *
 *       ... definition of common generic list functions
 *
 *      ------------------------------------------------------------------------
 */

/**
        Remove a value from a dict, if it is included.
        This function checks if the item pointer is contained as list content.
        If so the value will be removed from the list. (first occurance only)
*/
bool dtn_dict_remove_if_included(dtn_dict *dict, const void *value);

#endif /* dtn_dict_h */
