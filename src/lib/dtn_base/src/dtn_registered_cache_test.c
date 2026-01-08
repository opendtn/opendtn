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
        @file           dtn_registered_cache_test.c
        @author         Michael Beer
        @author         Markus Toepfer

        ------------------------------------------------------------------------
*/
#include "../include/testrun.h"
#include "dtn_registered_cache.c"

#ifndef DTN_DISABLE_CACHING

#include <pthread.h>

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST HELPER
 *
 *      ------------------------------------------------------------------------
 */

/* To be used to concurrently modify a cache */
static void *cache_worker(void *arg) {

    DTN_ASSERT(0 == pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0));
    DTN_ASSERT(0 == pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0));

    dtn_registered_cache *cache = arg;

    struct timespec time_to_wait = {.tv_sec = 0, .tv_nsec = 1000};

    while (true) {

        size_t *value = dtn_registered_cache_get(cache);
        if (0 != value)
            dtn_registered_cache_put(cache, value);

        nanosleep(&time_to_wait, 0);
    }

    return 0;
}

/*----------------------------------------------------------------------------*/

static int test_dtn_registered_cache_get() {

    dtn_registered_cache_config cfg = {0};

    /* Content does not matter - all we want is addresses */
    size_t testvalues[10] = {0};

    const size_t num_testvalues = sizeof(testvalues) / sizeof(testvalues[0]);

    testrun(0 == dtn_registered_cache_get(0));

    dtn_registered_cache *cache = dtn_registered_cache_extend("get", cfg);

    testrun(0 == dtn_registered_cache_get(cache));

    testrun(0 == dtn_registered_cache_put(cache, &testvalues[0]));
    testrun(&testvalues[0] == dtn_registered_cache_get(cache));

    for (size_t i = 0; num_testvalues > i; ++i) {

        testrun(0 == dtn_registered_cache_put(cache, testvalues + i));
    }

    for (size_t i = 0; num_testvalues > i; ++i) {

        void *object = dtn_registered_cache_get(cache);
        testrun((void *)testvalues <= object);
        testrun((void *)(testvalues + num_testvalues) > object);
    }

    testrun(0 == dtn_registered_cache_get(cache));

    /**********************************************************************
                The same gain, but with finite capacity
     **********************************************************************/

    cfg.capacity = num_testvalues - 1;

    cache = dtn_registered_cache_extend("get_2", cfg);

    testrun(0 == dtn_registered_cache_get(cache));

    testrun(0 == dtn_registered_cache_put(cache, &testvalues[0]));
    testrun(&testvalues[0] == dtn_registered_cache_get(cache));

    for (size_t i = 0; num_testvalues - 1 > i; ++i) {

        testrun(0 == dtn_registered_cache_put(cache, testvalues + i));
    }

    for (size_t i = 0; num_testvalues - 1 > i; ++i) {

        void *object = dtn_registered_cache_get(cache);
        testrun((void *)testvalues <= object);
        testrun((void *)(testvalues + num_testvalues - 1) > object);
    }

    testrun(0 == dtn_registered_cache_get(cache));

    for (size_t i = 0; num_testvalues - 1 > i; ++i) {

        testrun(0 == dtn_registered_cache_put(cache, testvalues + i));
    }

    dtn_registered_cache_free_all();

    cache = 0;

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int test_dtn_registered_cache_put() {

    /* Tests done by test_dtn_registered_cache_get */
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int check_concurrent_access() {

    dtn_registered_cache_config cfg = {0};

    /* Content does not matter - all we want is addresses */
    size_t testvalues[10] = {0};

    const size_t num_testvalues = sizeof(testvalues) / sizeof(testvalues[0]);

    testrun(0 == dtn_registered_cache_get(0));

    dtn_registered_cache *cache =
        dtn_registered_cache_extend("multithreaded", cfg);

    for (size_t i = 0; num_testvalues > i; ++i) {

        testrun(0 == dtn_registered_cache_put(cache, testvalues + i));
    }

    pthread_t threads[5] = {0};

    const size_t num_threads = sizeof(threads) / sizeof(threads[0]);

    for (size_t i = 0; num_threads > i; ++i) {
        testrun(0 == pthread_create(threads + i, 0, cache_worker, cache));
    }

    struct timespec time_to_wait = {
        .tv_sec = 1,
    };

    testrun(0 == nanosleep(&time_to_wait, 0));

    for (size_t i = 0; num_threads > i; ++i) {

        testrun(0 == pthread_cancel(threads[i]));
    }

    void *thread_retval;

    for (size_t i = 0; num_threads > i; ++i) {

        testrun(0 == pthread_join(threads[i], &thread_retval));
    }

    dtn_registered_cache_free_all();
    cache = 0;

    return testrun_log_success();
}

/******************************************************************************
 *                        dtn_registered_cache_set_element_checker
 ******************************************************************************/

static bool is_23(void *element) {

    DTN_ASSERT(0 != element);

    int *ipointer = element;

    return 23 == *ipointer;
}

/*----------------------------------------------------------------------------*/

static int test_dtn_registered_cache_set_element_checker() {

    dtn_registered_cache_config cfg = {0};

    dtn_registered_cache *cache = dtn_registered_cache_extend("checker", cfg);
    testrun(0 != cache);

    /* Verify it works without element checker */
    int not_correct = 17;
    int correct = 23;

    testrun(0 == dtn_registered_cache_put(cache, &not_correct));
    testrun(0 == dtn_registered_cache_put(cache, &correct));

    cache = dtn_registered_cache_extend("checker_2", cfg);
    testrun(0 != cache);

    dtn_registered_cache_set_element_checker(cache, is_23);

    // this test triggers DTN_ASSERT
    // testrun(&not_correct == dtn_registered_cache_put(cache, &not_correct));
    testrun(0 == dtn_registered_cache_put(cache, &correct));

    // this test triggers DTN_ASSERT
    // testrun(dtn_registered_cache_put(cache, &not_correct));

    dtn_registered_cache_free_all();

    cache = 0;

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static void *string_free(void *str) {

    if (0 != str) {

        free(str);
        str = 0;
    }

    return str;
}

/*----------------------------------------------------------------------------*/

static int test_dtn_registered_cache_register_extend() {

#define TEST_NAME "test_cache"

    dtn_registered_cache_config cfg = {0};

    /* Should create a non-registered cache with default cache size */
    dtn_registered_cache *cache = dtn_registered_cache_extend(0, cfg);
    testrun(0 == cache);

    /* Create a registered cache with default capacity */
    cache = dtn_registered_cache_extend(TEST_NAME, cfg);
    testrun(0 != cache);

    dtn_registered_cache_free_all();

    /* Create a registered cache with capacity 1 */
    cfg.capacity = 1;

    cache = dtn_registered_cache_extend(TEST_NAME, cfg);
    testrun(0 != cache);

    /* Extending cache with different item_free should fail */
    void *(*old_free)(void *) = cfg.item_free;

    cfg.item_free = string_free;
    testrun(0 == dtn_registered_cache_extend(TEST_NAME, cfg));

    cfg.item_free = old_free;

    int testvals[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};

    const size_t num_testvals = sizeof(testvals) / sizeof(testvals[0]);

    testrun(0 == dtn_registered_cache_put(cache, testvals + 0));

    for (size_t i = 1; i < num_testvals; ++i) {
        testrun(testvals + i == dtn_registered_cache_put(cache, testvals + i));
    }

    cfg.capacity = 1;
    testrun(cache == dtn_registered_cache_extend(TEST_NAME, cfg));

    /* Total capacity: 2 */

    testrun(0 == dtn_registered_cache_put(cache, testvals + 1));

    for (size_t i = 0; i < num_testvals; ++i) {
        testrun(testvals + i == dtn_registered_cache_put(cache, testvals + i));
    }

    testrun(0 != dtn_registered_cache_get(cache));
    testrun(0 != dtn_registered_cache_get(cache));

    testrun(0 == dtn_registered_cache_get(cache));

    cfg.capacity = num_testvals;
    cache = dtn_registered_cache_extend(TEST_NAME, cfg);
    testrun(0 != cache);

    for (size_t i = 0; i < num_testvals; ++i) {
        testrun(0 == dtn_registered_cache_put(cache, testvals + i));
    }

    for (size_t i = 0; i < num_testvals; ++i) {
        testrun(0 != dtn_registered_cache_get(cache));
    }

    testrun(0 == dtn_registered_cache_get(cache));

    /* Lastly: Check whether elements are moved into new cache upon extension */

    for (size_t i = 0; i < num_testvals; ++i) {
        testrun(0 == dtn_registered_cache_put(cache, testvals + i));
    }

    cfg.capacity = 12;
    cache = dtn_registered_cache_extend(TEST_NAME, cfg);

    for (size_t i = 0; i < num_testvals; ++i) {
        testrun(0 != dtn_registered_cache_get(cache));
    }

    testrun(0 == dtn_registered_cache_get(cache));

    dtn_registered_cache_free_all();
    cache = 0;

    /* Check excess elements properly freed */
    cfg.item_free = string_free;
    cfg.capacity = 12;

    cache = dtn_registered_cache_extend(TEST_NAME "_string", cfg);

    for (size_t i = 0; i < 12; ++i) {

        char entry[] = "XXXXXXXXXXXXXXXXXXXX";
        snprintf(entry, sizeof(entry), "entry-%zu", i);
        testrun(0 == dtn_registered_cache_put(cache, strdup(entry)));
    }

    dtn_registered_cache_free_all();

    cache = 0;

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int test_dtn_registered_cache_free_all() {

    /* Dummy, there is not really much to test */

    dtn_registered_cache_free_all();

    return testrun_log_success();
}

/*****************************************************************************
                                 CONFIGURATION
 ****************************************************************************/

static dtn_item *json_from_string(char const *str) {

    DTN_ASSERT(0 != str);
    return dtn_item_from_json(str);
}

/*----------------------------------------------------------------------------*/

static dtn_registered_cache_sizes *sizes_from_string(char const *str) {

    dtn_item *jval = json_from_string(str);
    DTN_ASSERT(0 != jval);
    dtn_registered_cache_sizes *sizes =
        dtn_registered_cache_sizes_from_json(jval);

    jval = dtn_item_free(jval);
    DTN_ASSERT(0 == jval);

    return sizes;
}

/*----------------------------------------------------------------------------*/

static int test_dtn_registered_cache_sizes_free() {

    testrun(0 == dtn_registered_cache_sizes_free(0));

    dtn_registered_cache_sizes *cfg = sizes_from_string("{}");
    testrun(0 == dtn_registered_cache_sizes_free(cfg));

    // pointer is freed above
    // cfg = dtn_registered_cache_sizes_free(cfg);
    // testrun(0 == cfg);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int test_dtn_registered_cache_sizes_from_json() {

#define CACHE_NAME_1 "draupnir"
#define CACHE_NAME_2 "gullinborsti"

    testrun(!dtn_registered_cache_sizes_from_json(0));

    dtn_item *jval = json_from_string("{}");
    testrun(0 != jval);

    dtn_registered_cache_sizes *sizes =
        dtn_registered_cache_sizes_from_json(jval);
    testrun(0 != sizes);

    jval = dtn_item_free(jval);
    testrun(0 == jval);

    testrun(DTN_DEFAULT_CACHE_SIZE ==
            dtn_registered_cache_size_for(sizes, CACHE_NAME_1));

    testrun(DTN_DEFAULT_CACHE_SIZE ==
            dtn_registered_cache_size_for(sizes, CACHE_NAME_2));

    sizes = dtn_registered_cache_sizes_free(sizes);
    testrun(0 == sizes);

    // CACHING DISABLED

    jval = json_from_string("{\""
                            "enabled"
                            "\": false}");
    testrun(0 != jval);

    sizes = dtn_registered_cache_sizes_from_json(jval);
    testrun(0 != sizes);

    jval = dtn_item_free(jval);
    testrun(0 == jval);

    testrun(0 == dtn_registered_cache_size_for(sizes, CACHE_NAME_1));
    testrun(0 == dtn_registered_cache_size_for(sizes, CACHE_NAME_2));

    sizes = dtn_registered_cache_sizes_free(sizes);
    testrun(0 == sizes);

    // CACHE ENABLED - No explicit sizes

    jval = json_from_string("{\"enabled\": true}");
    testrun(0 != jval);

    sizes = dtn_registered_cache_sizes_from_json(jval);
    testrun(0 != sizes);

    jval = dtn_item_free(jval);
    testrun(0 == jval);

    testrun(DTN_DEFAULT_CACHE_SIZE ==
            dtn_registered_cache_size_for(sizes, CACHE_NAME_1));

    testrun(DTN_DEFAULT_CACHE_SIZE ==
            dtn_registered_cache_size_for(sizes, CACHE_NAME_2));

    sizes = dtn_registered_cache_sizes_free(sizes);
    testrun(0 == sizes);

    /* Case 2: Empty sizes section */

    jval = json_from_string("{\"enabled\": true,"
                            "\"sizes\": {}}");
    testrun(0 != jval);

    sizes = dtn_registered_cache_sizes_from_json(jval);
    testrun(0 != sizes);

    jval = dtn_item_free(jval);
    testrun(0 == jval);

    testrun(DTN_DEFAULT_CACHE_SIZE ==
            dtn_registered_cache_size_for(sizes, CACHE_NAME_1));

    testrun(DTN_DEFAULT_CACHE_SIZE ==
            dtn_registered_cache_size_for(sizes, CACHE_NAME_2));

    sizes = dtn_registered_cache_sizes_free(sizes);
    testrun(0 == sizes);

    /* Enabled with cache sizes section */

    jval = json_from_string("{\"enabled\": true,"
                            "\"sizes\": {"
                            "\"" CACHE_NAME_1 "\":1,"
                            "\"" CACHE_NAME_2 "\":3"
                            "}}");
    testrun(0 != jval);

    sizes = dtn_registered_cache_sizes_from_json(jval);
    testrun(0 != sizes);

    jval = dtn_item_free(jval);
    testrun(0 == jval);

    testrun(1 == dtn_registered_cache_size_for(sizes, CACHE_NAME_1));
    testrun(3 == dtn_registered_cache_size_for(sizes, CACHE_NAME_2));

    sizes = dtn_registered_cache_sizes_free(sizes);
    testrun(0 == sizes);

    return testrun_log_success();

#undef CACHE_NAME_1
#undef CACHE_NAME_2
}

/*----------------------------------------------------------------------------*/

static int test_dtn_registered_cache_size_for() {

#define CACHE_NAME_1 "draupnir"
#define CACHE_NAME_2 "gullinborsti"

    testrun(0 == dtn_registered_cache_size_for(0, 0));
    testrun(0 == dtn_registered_cache_size_for(0, CACHE_NAME_1));
    testrun(0 == dtn_registered_cache_size_for(0, CACHE_NAME_2));

    // Remainder checked in dtn_registered_cache_size_from_json

    return testrun_log_success();

#undef CACHE_NAME_1
#undef CACHE_NAME_2
}

/*----------------------------------------------------------------------------*/

static bool check_caches_enabled(dtn_hashtable *sizes, char const **cache_names,
                                 size_t num_names) {

    UNUSED(sizes);

    for (size_t i = 0; i < num_names; ++i) {

        dtn_registered_cache *cache =
            dtn_hashtable_get(g_registry, cache_names[i]);

        if (0 == cache) {
            testrun_log_error("Expected cache %s not found", cache_names[i]);
            return false;
        }
    }

    return true;
}

/*----------------------------------------------------------------------------*/

static bool check_caches(dtn_registered_cache_sizes *sizes) {

    // Tricky: those need to be the names the corr. units use internally to
    // identify their cache
    char const *cache_names[] = {"buffer", "linked_list"};

    size_t num_cache_names = sizeof(cache_names) / sizeof(cache_names[0]);

    if (sizes->enabled) {
        return check_caches_enabled(sizes->sizes, cache_names, num_cache_names);
    }

    DTN_ASSERT(0 == sizes->sizes);

    for (size_t i = 0; i < num_cache_names; ++i) {
        dtn_registered_cache *cache =
            dtn_hashtable_get(g_registry, cache_names[i]);

        if (0 != cache) {
            return false;
        }
    }

    return true;
}

/*----------------------------------------------------------------------------*/

static int test_dtn_registered_cache_sizes_configure() {

    dtn_registered_cache_free_all();

    /* All caches enabled - default values */
    dtn_registered_cache_sizes *sizes = sizes_from_string("{\"enabled\":true}");
    dtn_registered_cache_sizes_configure(sizes);

    testrun(check_caches(sizes));

    dtn_registered_cache_free_all();

    sizes = dtn_registered_cache_sizes_free(sizes);
    testrun(0 == sizes);
    sizes = sizes_from_string("{\"enabled\":false}");

    dtn_registered_cache_sizes_configure(sizes);

    sizes = dtn_registered_cache_sizes_free(sizes);
    testrun(0 == sizes);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int all_tests() {

    testrun_init();
    testrun_test(test_dtn_registered_cache_get);
    testrun_test(test_dtn_registered_cache_put);
    testrun_test(test_dtn_registered_cache_set_element_checker);
    testrun_test(check_concurrent_access);
    testrun_test(test_dtn_registered_cache_register_extend);
    testrun_test(test_dtn_registered_cache_free_all);
    testrun_test(test_dtn_registered_cache_sizes_free);
    testrun_test(test_dtn_registered_cache_sizes_from_json);
    testrun_test(test_dtn_registered_cache_size_for);
    testrun_test(test_dtn_registered_cache_sizes_configure);

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

#else

int main(int argc, char **argv) {

    UNUSED(argc);
    UNUSED(argv);

    return 0;
}

#endif
