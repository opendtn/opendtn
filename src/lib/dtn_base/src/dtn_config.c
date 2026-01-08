/***
        ------------------------------------------------------------------------

        Copyright (c) 2020 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_config.c
        @author         Markus Töpfer
        @author         Michael Beer

        @date           2020-03-11


        ------------------------------------------------------------------------
*/

#include "../include/dtn_config.h"

#include "assert.h"

#include "../include/dtn_constants.h"
#include "../include/dtn_file.h"
#include "../include/dtn_item_json.h"
#include "../include/dtn_string.h"
#include "../include/dtn_utils.h"
#include <getopt.h>
#include <math.h>
#include <stdint.h>

#if DTN_ARCH == DTN_LINUX
#include <systemd/sd-journal.h>
#endif

const char *VERSION_REQUEST_ONLY = "VERSION_REQUEST_ONLY";

/*----------------------------------------------------------------------------*/

#define CONFIG_FILE_POSTFIX "_config.json"
#define APP_NAME_MAX_LEN 30

#define CONFIG_FILE_MAXLEN 255

char const *dtn_config_default_config_file_for(char const *app_name) {

    static char config_file[CONFIG_FILE_MAXLEN];

    DTN_ASSERT(strlen(DTN_DEFAULT_CONFIG_DIRECTORY) + 1 + APP_NAME_MAX_LEN +
                   strlen(CONFIG_FILE_POSTFIX) <=
               CONFIG_FILE_MAXLEN);

    if (0 == app_name)
        goto error;

    if (APP_NAME_MAX_LEN < strnlen(app_name, APP_NAME_MAX_LEN + 1)) {

        dtn_log_error("app_name exceeds max length of %zu", APP_NAME_MAX_LEN);
        goto error;
    }

    size_t num_printed =
        snprintf(config_file, CONFIG_FILE_MAXLEN, "%s/%s%s",
                 DTN_DEFAULT_CONFIG_DIRECTORY, app_name, CONFIG_FILE_POSTFIX);

    if (1 > num_printed)
        goto error;

    return config_file;

error:

    return 0;
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_config_load(const char *path) {

    if (!path)
        goto error;

    const char *failure = dtn_file_read_check(path);

    if (failure) {

        dtn_log_error("READ, file (%s) "
                      " %s.",
                      path, failure);

        goto error;
    }

    return dtn_item_json_read_file(path);
error:
    return NULL;
}

/*----------------------------------------------------------------------------*/

const char *dtn_config_path_from_command_line(size_t argc, char **argv) {

    /* getopt is not 0-pointer safe */
    if (0 == argv)
        return 0;

    char *optstring = "c:v";

    int c = 0;

    while (-1 != (c = getopt(argc, argv, optstring))) {

        switch (c) {

        case 'c':
            return optarg;
            break;

        case 'v':
            DTN_VERSION_PRINT(stderr);
            return VERSION_REQUEST_ONLY;
            break;

        default:
            break;
        };
    };

    return NULL;
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_config_from_command_line(size_t argc, char *argv[]) {

    const char *path = dtn_config_path_from_command_line(argc, argv);

    if (VERSION_REQUEST_ONLY == path)
        return 0;

    return dtn_config_load(path);
}

/*----------------------------------------------------------------------------*/

double dtn_config_double_or_default(dtn_item const *jval, char const *key,
                                    double default_val) {

    dtn_item *ival = dtn_item_object_get(jval, key);

    if (!dtn_item_is_number(ival)) {
        return default_val;
    } else {
        return dtn_item_get_number(ival);
    }
}

/*----------------------------------------------------------------------------*/

uint32_t dtn_config_u32_or_default(dtn_item const *jval, char const *key,
                                   uint32_t default_val) {

    dtn_item *ival = dtn_item_object_get(jval, key);
    double dval = dtn_item_get_number(ival);
    double uint32max = (double)UINT32_MAX;

    if ((0 > dval) || (uint32max < dval)) {
        dtn_log_error("%s out of range (expect value in [0;%" PRIu32 "]",
                      dtn_string_sanitize(key), UINT32_MAX);
        return default_val;
    } else if (floor(dval) != dval) {
        dtn_log_error("%f should be an integer", dval);
        return default_val;
    } else if (!dtn_item_is_number(ival)) {
        return default_val;
    } else {
        return dval;
    }
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_config_u64_or_default(dtn_item const *jval, char const *key,
                                   uint64_t default_val) {

    dtn_item *ival = dtn_item_object_get(jval, key);
    double dval = dtn_item_get_number(ival);
    double uint64max = (double)UINT64_MAX;

    if ((0 > dval) || (uint64max < dval)) {
        dtn_log_error("%s out of range (expect value in [0;%" PRIu64 "]",
                      dtn_string_sanitize(key), UINT64_MAX);
        return default_val;
    } else if (floor(dval) != dval) {
        dtn_log_error("%f should be an integer", dval);
        return default_val;
    } else if (!dtn_item_is_number(ival)) {
        return default_val;
    } else {
        return dval;
    }
}

/*----------------------------------------------------------------------------*/

bool dtn_config_bool_or_default(dtn_item const *jval, char const *key,
                                bool default_val) {

    dtn_item *boolval = dtn_item_object_get(jval, key);

    if (dtn_item_is_true(boolval)) {
        return true;
    } else if (dtn_item_is_false(boolval)) {
        return false;
    } else {
        return default_val;
    }
}

/*----------------------------------------------------------------------------*/

dtn_socket_configuration
dtn_config_socket_configuration_or_default(dtn_item const *jcfg,
                                           char const *key) {

    dtn_item const *socket_json = dtn_item_object_get(jcfg, key);

    dtn_socket_configuration scfg =
        dtn_socket_configuration_from_item(socket_json);

    return scfg;
}

/*----------------------------------------------------------------------------*/
