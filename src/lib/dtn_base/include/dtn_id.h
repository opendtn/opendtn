/***
        ------------------------------------------------------------------------

        Copyright (c) 2022 German Aerospace Center DLR e.V. (GSOC)

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

    An openvocs ID is a string of at least one char.
    Any non-empty string that is AT MOST 36 chars long (excluding terminal 0)
    is a valid ID.

    "1" is as valid as "TRAGxvar1231231231244ta3"

    "" is NOT a valid ID.

        @author Michael J. Beer, DLR/GSOC
        @copyright (c) 2022 German Aerospace Center DLR e.V. (GSOC)

        ------------------------------------------------------------------------
*/
#ifndef dtn_ID_H
#define dtn_ID_H

#include <stdbool.h>
#include <unistd.h>

/*----------------------------------------------------------------------------*/

#define DTN_ID_INVALID ((dtn_id){0})

/**
 * An OV ID is 36 chars plus 1 byte for the terminal zero -
 * the last byte MUST always be 0
 */
typedef char dtn_id[36 + 1];

bool dtn_id_valid(char const *id);

bool dtn_id_clear(dtn_id id);

bool dtn_id_set(dtn_id id, char const *str);

char *dtn_id_dup(char const *id);

/**
 * Fill ID with random string.
 * Effectively, creates a UUID string
 */
bool dtn_id_fill_with_uuid(dtn_id target);

bool dtn_id_match(char const *restrict id1, char const *restrict id2);

/*----------------------------------------------------------------------------*/

bool dtn_id_array_reset(dtn_id ids[], size_t capacity);

bool dtn_id_array_add(dtn_id ids[], size_t capacity, char const *id);

bool dtn_id_array_del(dtn_id ids[], size_t capacity, char const *id);

ssize_t dtn_id_array_get_index(dtn_id const *uuids, size_t capacity,
                              char const *id);

/**
 * Returns the index of the first used ID entry in array AFTER `index`
 * To start from the beginning, use `-1` as `index`.
 * call `dtn_uuid_array_next(uuids, c, -1);
 */
ssize_t dtn_id_array_next(dtn_id const *uuids, size_t capacity,
                         ssize_t last_index);

/*----------------------------------------------------------------------------*/
#endif
