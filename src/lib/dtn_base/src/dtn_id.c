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

        @author         Michael J. Beer, DLR/GSOC

        ------------------------------------------------------------------------
*/
#include "../include/dtn_id.h"
#include "../include/dtn_random.h"
#include "../include/dtn_utils.h"

#include <string.h>

/*----------------------------------------------------------------------------*/

bool dtn_id_valid(char const *id) {

  if ((0 == id) || (0 == id[0])) {
    return false;
  } else {
    return 37 > strnlen(id, 37);
  }
}

/*----------------------------------------------------------------------------*/

bool dtn_id_clear(dtn_id id) {

  if (0 != id) {
    memset(id, 0, sizeof(dtn_id));
    return true;
  } else {
    return false;
  }
}

/*----------------------------------------------------------------------------*/

bool dtn_id_set(dtn_id id, char const *str) {

  if ((0 == id) || (!dtn_id_valid(str))) {
    return false;
  } else {
    memset(id, 0, sizeof(dtn_id));
    strncpy(id, str, sizeof(dtn_id) - 1);
    return true;
  }
}

/*----------------------------------------------------------------------------*/

char *dtn_id_dup(char const *id) {

  if (!dtn_id_valid(id)) {
    return 0;
  } else {
    return strndup(id, sizeof(dtn_id));
  }
}

/*----------------------------------------------------------------------------*/

static char *add_dash_nocheck(char *wptr) {

  *wptr = '-';
  return wptr + 1;
}

/*----------------------------------------------------------------------------*/

static char *add_random_digits(char *wptr, size_t num_digits_to_add) {

  // dtn_random_string creates terminal 0, thus we produce one more
  // and ignore it afterwards
  dtn_random_string(&wptr, num_digits_to_add + 1, "01234567890abcdef");
  return wptr + num_digits_to_add;
}

/*----------------------------------------------------------------------------*/

bool dtn_id_fill_with_uuid(dtn_id target) {

  if (dtn_ptr_valid(target, "Cannot fill ID: No ID to write to")) {

    //  8       -  4 - 4  - 4  -      12
    // "cdfe55fb-2ade-44dd-8228-96554aaeaf6c"

    char *wptr = add_random_digits(target, 8);
    wptr = add_dash_nocheck(wptr);

    wptr = add_random_digits(wptr, 4);
    wptr = add_dash_nocheck(wptr);

    wptr = add_random_digits(wptr, 4);
    wptr = add_dash_nocheck(wptr);

    wptr = add_random_digits(wptr, 4);
    wptr = add_dash_nocheck(wptr);

    wptr = add_random_digits(wptr, 12);
    *wptr = 0;

    return true;

  } else {
    return false;
  }
}

/*----------------------------------------------------------------------------*/

bool dtn_id_match(char const *restrict id1, char const *restrict id2) {

  if ((0 == id1) || (0 == id2)) {
    return id1 == id2;
  } else {
    for (size_t i = 0; i < sizeof(dtn_id) - 1; ++i) {
      if ((0 == id1[i]) && (0 == id2[i])) {
        return true;
      } else if (id1[i] != id2[i]) {
        return false;
      }
    }

    char trailer1 = id1[sizeof(dtn_id) - 1];
    return (0 == trailer1) && (trailer1 == id2[sizeof(dtn_id) - 1]);
  }
}

/*----------------------------------------------------------------------------*/

bool dtn_id_array_reset(dtn_id ids[], size_t capacity) {

  if (!dtn_ptr_valid(ids, "Cannot reset ID array: No array")) {

    return false;

  } else {

    for (size_t i = 0; i < capacity; ++i) {
      dtn_id_clear(ids[i]);
    }

    return true;
  }
}

/*----------------------------------------------------------------------------*/

bool dtn_id_array_add(dtn_id ids[], size_t capacity, char const *id) {

  if ((!dtn_ptr_valid(ids, "Cannot add ID to array: No array")) ||
      (!dtn_ptr_valid(id, "Cannot add ID to array: No ID"))) {

    return false;

  } else {

    for (size_t i = 0; i < capacity; ++i) {
      if (!dtn_id_valid(ids[i])) {
        return dtn_id_set(ids[i], id);
      }
    }

    return false;
    ;
  }
}

/*----------------------------------------------------------------------------*/

bool dtn_id_array_del(dtn_id ids[], size_t capacity, char const *id) {

  if ((!dtn_ptr_valid(ids, "Cannot add ID to array: No array")) ||
      (!dtn_ptr_valid(id, "Cannot add ID to array: No ID"))) {

    return false;

  } else {

    for (size_t i = 0; i < capacity; ++i) {
      if (dtn_id_match(ids[i], id)) {
        return dtn_id_clear(ids[i]);
      }
    }

    return true;
  }
}

/*----------------------------------------------------------------------------*/

ssize_t dtn_id_array_get_index(dtn_id const *ids, size_t capacity,
                              char const *id) {

  if ((!dtn_ptr_valid(ids, "Cannot add ID to array: No array")) ||
      (!dtn_ptr_valid(id, "Cannot add ID to array: No ID"))) {

    return -1;

  } else {

    for (size_t i = 0; i < capacity; ++i) {
      if (dtn_id_match(ids[i], id)) {
        return i;
      }
    }

    return -1;
  }
}

/*----------------------------------------------------------------------------*/

ssize_t dtn_id_array_next(dtn_id const *ids, size_t capacity,
                         ssize_t last_index) {

  if (-1 > last_index) {
    last_index = -1;
  }

  if (!dtn_ptr_valid(ids, "Cannot iterate over ID array - no array")) {
    return -1;
  } else {

    for (size_t i = 1 + last_index; i < capacity; ++i) {

      if (dtn_id_valid(ids[i])) {
        return i;
      }
    }

    return -1;
  }
}

/*----------------------------------------------------------------------------*/
