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
        @file           dtn_config_test.c
        @author         Markus Töpfer
        @author         Michael Beer

        @date           2020-03-11


        ------------------------------------------------------------------------
*/

#include "dtn_config.h"
#ifndef DTN_TEST_RESOURCE_DIR
#error "Must provide -D DTN_TEST_RESOURCE_DIR=value while compiling this file."
#endif

#include "../include/dtn_getopt.h"
#include "../include/dtn_item_json.h"
#include "../include/dtn_utils.h"
#include "../include/testrun.h"
#include "sys/stat.h"

#include "dtn_config.c"

/******************************************************************************
 *                                  HELPERS
 ******************************************************************************/

bool create_dummy_config_file(char const *filename) {

    dtn_item *dummy_config = dtn_item_object();

    struct stat statbuf = {0};
    if (0 == stat(filename, &statbuf)) {
        goto error;
    }

    if (0 == dummy_config) {
        goto error;
    }

    if (!dtn_item_json_write_file(filename, dummy_config)) {
        goto error;
    }

    dummy_config = dtn_item_free(dummy_config);

    return true;

error:

    dummy_config = dtn_item_free(dummy_config);

    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_config_default_config_file_for() {

    testrun(0 == dtn_config_default_config_file_for(0));

    char const *file = dtn_config_default_config_file_for("me_test");

    testrun(0 != file);

    const size_t len = strlen(file);
    testrun(0 < len);

    /* File ends in ".json" - Rest cannot be assumed */
    testrun('n' == file[len - 1]);
    testrun('o' == file[len - 2]);
    testrun('s' == file[len - 3]);
    testrun('j' == file[len - 4]);
    testrun('.' == file[len - 5]);

    file = 0;

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_config_load() {

    const char *path = DTN_TEST_RESOURCE_DIR "JSON/json.ok1";

    testrun(!dtn_config_load(NULL));

    dtn_item *value = dtn_config_load(path);
    testrun(value);

    testrun(NULL == dtn_item_free(value));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_config_path_from_command_line() {

    char *path = DTN_TEST_RESOURCE_DIR "JSON/json.ok1";

    char *argv[] = {"filename", "-v", "-y", "-x"};
    dtn_reset_getopt();
    testrun(NULL == dtn_config_path_from_command_line(1, argv));

    dtn_reset_getopt();
    testrun(VERSION_REQUEST_ONLY == dtn_config_path_from_command_line(2, argv));
    dtn_reset_getopt();
    testrun(VERSION_REQUEST_ONLY == dtn_config_path_from_command_line(3, argv));
    dtn_reset_getopt();
    testrun(VERSION_REQUEST_ONLY == dtn_config_path_from_command_line(4, argv));

    argv[1] = "-c";
    argv[2] = path;

    dtn_reset_getopt();
    const char *p = dtn_config_path_from_command_line(3, argv);
    testrun(p);
    fprintf(stderr, "%s\n", p);

    testrun(NULL == dtn_config_path_from_command_line(4, argv));
    /* expect log invalid option -x */

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_config_from_command_line() {

    /* Only patholocial cases are checked here,
     * see dtn_config_load_test for remainder of tests */

    /* Critical string with special meaning - must NOT be loaded */

    testrun(create_dummy_config_file(VERSION_REQUEST_ONLY));

    char *argv[] = {"filename", "-v"};

    optind = 0;
    optarg = 0;

    /* this sequence of commands grants removal of test file */

    dtn_item *loaded = dtn_config_from_command_line(2, argv);

    if (0 != unlink(VERSION_REQUEST_ONLY)) {
        testrun_log_error("Could not remove file %s", VERSION_REQUEST_ONLY);
    }

    testrun(0 == loaded);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static dtn_item *json_from_str(char const *str) {
    return dtn_item_from_json(str);
}

/*----------------------------------------------------------------------------*/

static int test_dtn_config_double_or_default() {

    testrun(3 == dtn_config_double_or_default(0, "/o", 3));

    dtn_item *jval = json_from_str("{\"o\": 1, \"p\": -1, \"q\":1.12}");

    testrun(1.0 == dtn_config_double_or_default(jval, "o", 3));
    testrun(-1.0 == dtn_config_double_or_default(jval, "p", 3));
    testrun(1.12 == dtn_config_double_or_default(jval, "q", 3));
    testrun(3 == dtn_config_double_or_default(jval, "r", 3));

    jval = dtn_item_free(jval);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int test_dtn_config_u32_or_default() {

    testrun(3 == dtn_config_u32_or_default(0, "/o", 3));

    dtn_item *jval = json_from_str("{\"o\": 1, \"p\": -1, \"q\":1.12}");

    testrun(1 == dtn_config_u32_or_default(jval, "o", 3));
    testrun(3 == dtn_config_u32_or_default(jval, "p", 3));
    testrun(3 == dtn_config_u32_or_default(jval, "q", 3));
    testrun(3 == dtn_config_u32_or_default(jval, "r", 3));

    jval = dtn_item_free(jval);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int test_dtn_config_u64_or_default() {

    testrun(3 == dtn_config_u64_or_default(0, "/o", 3));

    dtn_item *jval = json_from_str("{\"o\": 1, \"p\": -1, \"q\":1.12}");

    testrun(1 == dtn_config_u64_or_default(jval, "o", 3));
    testrun(3 == dtn_config_u64_or_default(jval, "p", 3));
    testrun(3 == dtn_config_u64_or_default(jval, "q", 3));
    testrun(3 == dtn_config_u64_or_default(jval, "r", 3));

    jval = dtn_item_free(jval);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int test_dtn_config_bool_or_default() {

    testrun(false == dtn_config_bool_or_default(0, 0, false));
    testrun(true == dtn_config_bool_or_default(0, 0, true));

    testrun(false == dtn_config_bool_or_default(
                         0, "tralladi trallada - nicht da", false));
    testrun(true == dtn_config_bool_or_default(
                        0, "tralladi trallada - nicht da", true));

    dtn_item *jval = dtn_item_object();
    testrun(0 != jval);

    testrun(false == dtn_config_bool_or_default(jval, 0, false));
    testrun(true == dtn_config_bool_or_default(jval, 0, true));

    testrun(false == dtn_config_bool_or_default(
                         jval, "tralladi trallada - nicht da", false));
    testrun(true == dtn_config_bool_or_default(
                        jval, "tralladi trallada - nicht da", true));

    testrun(dtn_item_object_set(jval, "wahr", dtn_item_true()));
    testrun(dtn_item_object_set(jval, "falsch", dtn_item_false()));
    testrun(dtn_item_object_set(jval, "muell", dtn_item_string("muellme")));

    testrun(false == dtn_config_bool_or_default(jval, 0, false));
    testrun(true == dtn_config_bool_or_default(jval, 0, true));

    testrun(false == dtn_config_bool_or_default(
                         jval, "tralladi trallada - nicht da", false));
    testrun(true == dtn_config_bool_or_default(
                        jval, "tralladi trallada - nicht da", true));

    testrun(true == dtn_config_bool_or_default(jval, "wahr", false));
    testrun(true == dtn_config_bool_or_default(jval, "wahr", true));

    testrun(false == dtn_config_bool_or_default(jval, "falsch", false));
    testrun(false == dtn_config_bool_or_default(jval, "falsch", true));

    testrun(false == dtn_config_bool_or_default(jval, "muell", false));
    testrun(true == dtn_config_bool_or_default(jval, "muell", true));

    jval = dtn_item_free(jval);
    testrun(0 == jval);

    return testrun_log_success();
}
/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CLUSTER                                                    #CLUSTER
 *
 *      ------------------------------------------------------------------------
 */

int all_tests() {

    testrun_init();

    testrun_test(test_dtn_config_default_config_file_for);
    testrun_test(test_dtn_config_load);
    testrun_test(test_dtn_config_path_from_command_line);
    testrun_test(test_dtn_config_from_command_line);
    testrun_test(test_dtn_config_double_or_default);
    testrun_test(test_dtn_config_u32_or_default);
    testrun_test(test_dtn_config_u64_or_default);
    testrun_test(test_dtn_config_bool_or_default);

    return testrun_counter;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST EXECUTION                                                  #EXEC
 *
 *      ------------------------------------------------------------------------
 */

testrun_run(all_tests);