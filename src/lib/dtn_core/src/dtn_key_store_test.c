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
        @file           dtn_key_store_test.c
        @author         Töpfer, Markus

        @date           2026-01-03


        ------------------------------------------------------------------------
*/
#include "dtn_key_store.c"
#include <dtn_base/testrun.h>

#ifndef DTN_TEST_RESOURCE_DIR
#error "Must provide -D DTN_TEST_RESOURCE_DIR=value while compiling this file."
#endif

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_key_store_create() {

    dtn_key_store_config config =
        (dtn_key_store_config){.path = DTN_TEST_RESOURCE_DIR "/keys"};

    dtn_key_store *store = dtn_key_store_create(config);
    testrun(store);
    testrun(store->data);

    testrun(NULL == dtn_key_store_free(store));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_key_store_free() {

    dtn_key_store_config config =
        (dtn_key_store_config){.path = DTN_TEST_RESOURCE_DIR "keys/"};

    dtn_key_store *store = dtn_key_store_create(config);
    testrun(store);
    testrun(store->data);

    dtn_log_debug("%s", store->config.path);

    testrun(dtn_key_store_load(store, NULL));
    testrun(3 == dtn_dict_count(store->data));

    testrun(NULL == dtn_key_store_free(store));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_key_store_load() {

    dtn_key_store_config config =
        (dtn_key_store_config){.path = DTN_TEST_RESOURCE_DIR "keys"};

    dtn_key_store *store = dtn_key_store_create(config);
    testrun(store);
    testrun(store->data);

    testrun(dtn_key_store_load(store, NULL));
    testrun(3 == dtn_dict_count(store->data));
    testrun(dtn_dict_get(store->data, "aes128"));
    testrun(dtn_dict_get(store->data, "aes192"));
    testrun(dtn_dict_get(store->data, "aes256"));

    testrun(NULL == dtn_key_store_free(store));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_key_store_save() {

    dtn_key_store_config config =
        (dtn_key_store_config){.path = DTN_TEST_RESOURCE_DIR "keys"};

    dtn_key_store *store = dtn_key_store_create(config);
    testrun(store);
    testrun(store->data);

    testrun(dtn_key_store_load(store, NULL));
    testrun(3 == dtn_dict_count(store->data));
    testrun(dtn_dict_get(store->data, "aes128"));
    testrun(dtn_dict_get(store->data, "aes192"));
    testrun(dtn_dict_get(store->data, "aes256"));

    testrun(dtn_key_store_save(store, NULL));

    testrun(NULL == dtn_key_store_free(store));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_key_store_get() {

    dtn_key_store_config config =
        (dtn_key_store_config){.path = DTN_TEST_RESOURCE_DIR "keys"};

    dtn_key_store *store = dtn_key_store_create(config);
    testrun(store);
    testrun(store->data);

    testrun(dtn_key_store_load(store, NULL));
    testrun(3 == dtn_dict_count(store->data));
    testrun(dtn_dict_get(store->data, "aes128"));
    testrun(dtn_dict_get(store->data, "aes192"));
    testrun(dtn_dict_get(store->data, "aes256"));

    dtn_buffer *buffer = dtn_key_store_get(store, "aes128");
    testrun(buffer);
    testrun(buffer->length == 128);
    buffer = dtn_buffer_free(buffer);

    testrun(NULL == dtn_key_store_free(store));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_key_store_set() {

    dtn_key_store_config config =
        (dtn_key_store_config){.path = DTN_TEST_RESOURCE_DIR "keys"};

    dtn_key_store *store = dtn_key_store_create(config);
    testrun(store);
    testrun(store->data);

    testrun(dtn_key_store_load(store, NULL));
    testrun(3 == dtn_dict_count(store->data));
    testrun(dtn_dict_get(store->data, "aes128"));
    testrun(dtn_dict_get(store->data, "aes192"));
    testrun(dtn_dict_get(store->data, "aes256"));

    dtn_buffer *buffer = dtn_key_store_get(store, "aes128");
    testrun(buffer);
    testrun(buffer->length == 128);

    testrun(dtn_key_store_set(store, "test", buffer));
    testrun(4 == dtn_dict_count(store->data));
    testrun(dtn_dict_get(store->data, "aes128"));
    testrun(dtn_dict_get(store->data, "aes192"));
    testrun(dtn_dict_get(store->data, "aes256"));
    testrun(dtn_dict_get(store->data, "test"));

    testrun(NULL == dtn_key_store_free(store));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CLUSTER                                                    #CLUSTER
 *
 *      ------------------------------------------------------------------------
 */

int all_tests() {

    testrun_init();
    testrun_test(test_dtn_key_store_create);
    testrun_test(test_dtn_key_store_free);
    testrun_test(test_dtn_key_store_load);
    testrun_test(test_dtn_key_store_save);
    testrun_test(test_dtn_key_store_get);
    testrun_test(test_dtn_key_store_set);

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
