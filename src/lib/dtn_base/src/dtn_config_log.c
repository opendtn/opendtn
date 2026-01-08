/***
        ------------------------------------------------------------------------

        Copyright (c) 2021 German Aerospace Center DLR e.V. (GSOC)

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

#include "../include/dtn_config_log.h"

#include "../include/dtn_log.h"
#include "../include/dtn_utils.h"
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/dtn_item.h"

/*----------------------------------------------------------------------------*/

static dtn_log_level dtn_log_level_from_json(dtn_item const *jval) {

    if (0 == jval)
        goto error;

    char const *level = dtn_item_get_string(dtn_item_get(jval, "/level"));

    return dtn_log_level_from_string(level);

error:

    return DTN_LOG_INVALID;
}

/*----------------------------------------------------------------------------*/

static int create_log_file(char const *path) {

    if (0 == path)
        return -1;

    int fh = open(path, O_RDWR | O_CREAT | O_APPEND, S_IRWXU);

    if (0 >= fh) {
        dtn_log_warning("Could not open log file %s", path);
    }

    return fh;
}

/*----------------------------------------------------------------------------*/

static dtn_log_format format_from_json(dtn_item const *jval) {

    dtn_log_format fmt = DTN_LOG_TEXT;

    jval = dtn_item_get(jval, "/format");

    if (0 == jval) {
        return fmt;
    }

    char const *fmt_str = dtn_item_get_string(jval);

    if (0 == fmt_str) {
        dtn_log_error("Invalid config: format is not a string - "
                      "falling back to default "
                      "format");
        return fmt;
    }

    if (0 == strncmp(fmt_str, "json", 4)) {
        fmt = DTN_LOG_JSON;
        goto finish;
    }

    if (0 != strncmp(fmt_str, "text", 4)) {
        dtn_log_error("Invalid config: Unkown format '%s' - falling back to "
                      "default format",
                      fmt_str);
    }

finish:

    return fmt;
}

/*----------------------------------------------------------------------------*/

static bool configure_log_rotation(dtn_log_output *out, dtn_item const *jval) {

    dtn_item const *max_num_messages_jval =
        dtn_item_get(jval, "/messages_per_file");

    dtn_item const *max_num_files_jval = dtn_item_get(jval, "/num_files");

    if ((0 == max_num_messages_jval) && (0 == max_num_files_jval)) {
        goto finish;
    }

    if ((0 != max_num_messages_jval) &&
        (!dtn_item_is_number(max_num_messages_jval))) {

        dtn_log_error("Log config invalid: messages_per_file is not a number");
        goto error;
    }

    if ((0 != max_num_files_jval) &&
        (!dtn_item_is_number(max_num_files_jval))) {

        dtn_log_error("Log config invalid: num_files is not a number");
        goto error;
    }

    size_t messages_per_file = dtn_item_get_number(max_num_messages_jval);
    size_t max_num_files = dtn_item_get_number(max_num_files_jval);

    if (0 == messages_per_file) {
        messages_per_file = DTN_DEFAULT_LOG_MESSAGES_PER_FILE;
    }

    if (0 == max_num_files) {
        max_num_files = DTN_DEFAULT_LOG_MAX_NUM_FILES;
    }

    out->log_rotation.use = true;
    out->log_rotation.max_num_files = max_num_files;
    out->log_rotation.messages_per_file = messages_per_file;

finish:

    return true;

error:

    return false;
}

/*----------------------------------------------------------------------------*/

dtn_log_output dtn_log_output_from_json(dtn_item const *jval) {

    dtn_log_output out = {0};

    if (0 == jval) {
        dtn_log_error("No JSON - 0 pointer");
        goto error;
    }

    if (dtn_item_is_true(dtn_item_get(jval, "/systemd"))) {
        out.use.systemd = true;
    }

    dtn_item const *fjval = dtn_item_get(jval, "/file");

    if (0 == fjval) {
        goto finish;
    }

    dtn_log_format format = format_from_json(jval);

    if (dtn_item_is_object(fjval)) {
        jval = fjval;
    }

    char const *path = dtn_item_get_string(dtn_item_get(jval, "/file"));

    if (0 == path) {
        goto finish;
    }

    if (0 == strcmp("stdout", path)) {
        out.filehandle = 1;
        goto finish;
    }

    if (0 == strcmp("stderr", path)) {
        out.filehandle = 2;
        goto finish;
    }

    out.filehandle = create_log_file(path);
    out.format = format;

    configure_log_rotation(&out, jval);

    if (out.log_rotation.use) {
        out.log_rotation.path = strdup(path);
    }

finish:
error:

    return out;
}

/*----------------------------------------------------------------------------*/

static int set_output_from_json(char const *module, char const *function,
                                dtn_item const *jval) {

    dtn_log_level level = dtn_log_level_from_json(jval);
    dtn_log_output out = dtn_log_output_from_json(jval);

    int old_fh = dtn_log_set_output(module, function, level, out);

    if (0 != out.log_rotation.path) {
        free(out.log_rotation.path);
        out.log_rotation.path = 0;
    }

    if (2 < old_fh) {
        dtn_log_error("Overwriting open file handle ...");
        close(old_fh);
        return old_fh;
    }

    return old_fh;
}

/*----------------------------------------------------------------------------*/

bool configure_function(const void *key, void *value, void *module_name) {

    dtn_log_info("Log: Configuring for module %s function %s", module_name,
                 key);

    dtn_item *jval = dtn_item_cast(value);

    if (0 == jval) {
        dtn_log_critical("Internal error: Expected json pointer, got something "
                         "else");
        return false;
    }

    set_output_from_json(module_name, key, value);

    return true;
}

/*----------------------------------------------------------------------------*/

bool configure_module(const void *key, void *value, void *data) {

    UNUSED(data);

    dtn_log_info("Log: Configuring for module %s", key);

    dtn_item *jval = dtn_item_cast(value);

    if (0 == jval) {
        dtn_log_critical("Internal error: Expected json pointer, got something "
                         "else");
        return false;
    }

    set_output_from_json(key, 0, value);

    dtn_item const *functions = dtn_item_get(jval, "/functions");
    dtn_item_object_for_each((dtn_item *)functions, (void *)key,
                             configure_function);

    return true;
}

/*----------------------------------------------------------------------------*/

bool dtn_config_log_from_json(dtn_item const *jval) {

    const dtn_item *conf = dtn_item_get(jval, "/log");

    if (0 == conf) {
        conf = jval;
    }

    set_output_from_json(0, 0, conf);

    dtn_item const *modules = dtn_item_get(conf, "/modules");

    dtn_item_object_for_each((dtn_item *)modules, 0, configure_module);

    return true;
}

/*----------------------------------------------------------------------------*/
