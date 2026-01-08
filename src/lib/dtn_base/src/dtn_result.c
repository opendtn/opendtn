/***
        ------------------------------------------------------------------------

        Copyright (c) 2019 German Aerospace Center DLR e.V. (GSOC)

        Licensed under the Apache License, Version 2.0 (the "License");
        you may not use this file except in compliance with the License.
        You may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

        Unless required by applicable law or agreed to in writing, software
        distributed under the License is distributed on an "AS IS" BASIS,
        WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
        See the License for the specific language gdtnerning permissions and
        limitations under the License.

        This file is part of the openvocs project. https://openvocs.org

        ------------------------------------------------------------------------
*//**
        @file           dtn_result.c
        @author         Michael J. Beer

        @date           2019-10-29

        ------------------------------------------------------------------------
*/

#include "../include/dtn_result.h"
#include "../include/dtn_utils.h"
#include <stdlib.h>
#include <string.h>

/*----------------------------------------------------------------------------*/

bool dtn_result_set(dtn_result *result, int error_code, char const *message) {

    if (0 == result)
        goto error;

    if ((DTN_ERROR_NO_ERROR == error_code) && (0 != message)) {
        goto error;
    }

    if ((0 == message) && (DTN_ERROR_NO_ERROR != error_code)) {
        goto error;
    }

    if (result->message) {
        free(result->message);
    }

    result->error_code = error_code;
    result->message = 0;

    if (0 != message) {
        result->message = strdup(message);
    }

    return true;

error:

    return false;
}

/*----------------------------------------------------------------------------*/

char const *dtn_result_get_message(dtn_result const result) {

    if (0 == result.message) {
        return "";
    }

    return result.message;
}

/*----------------------------------------------------------------------------*/

bool dtn_result_clear(dtn_result *result) {

    if (0 == result)
        goto error;

    if (0 != result->message) {

        free(result->message);
    }

    memset(result, 0, sizeof(*result));

    return true;

error:

    return false;
}

/*----------------------------------------------------------------------------*/
