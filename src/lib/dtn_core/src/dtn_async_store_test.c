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
        @file           dtn_async_store_test.c
        @author         Töpfer, Markus

        @date           2025-12-06


        ------------------------------------------------------------------------
*/
#include "dtn_async_store.c"
#include <dtn_base/testrun.h>

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_async_store_create() {

    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);

    dtn_async_store *store =
        dtn_async_store_create((dtn_async_store_config){.loop = loop});

    testrun(store);
    testrun(dtn_async_store_cast(store));
    testrun(store->data.dict);
    testrun(DTN_TIMER_INVALID != store->invalidate_timer);
    testrun(store->config.limits.threadlock_timeout_usec == 100000);
    testrun(store->config.limits.invalidate_check_usec == 1000000);

    testrun(NULL == dtn_async_store_free(store));
    testrun(NULL == dtn_event_loop_free(loop));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

struct dummy_userdata {

    dtn_async_data data;
};

/*----------------------------------------------------------------------------*/

static void dummy_timeout(void *userdata, dtn_async_data data) {

    struct dummy_userdata *dummy = (struct dummy_userdata *)userdata;
    dummy->data = data;
    return;
}

/*----------------------------------------------------------------------------*/

static void dummy_callback(void *userdata, int socket, dtn_item *message) {

    struct dummy_userdata *dummy = (struct dummy_userdata *)userdata;
    dummy->data.socket = socket;
    dummy->data.message = message;
    return;
}

/*----------------------------------------------------------------------------*/

int test_dtn_async_store_free() {

    struct dummy_userdata dummy = {0};

    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);

    dtn_async_store *store =
        dtn_async_store_create((dtn_async_store_config){.loop = loop});

    testrun(store);
    testrun(NULL == dtn_async_store_free(store));

    store = dtn_async_store_create((dtn_async_store_config){.loop = loop});

    testrun(store);

    testrun(dtn_async_set(store, "1",
                          (dtn_async_data){.socket = 1,
                                           .message = NULL,
                                           .timedout.userdata = &dummy,
                                           .timedout.callback = dummy_timeout},
                          10000000));

    testrun(NULL == dtn_async_store_free(store));
    testrun(NULL == dtn_event_loop_free(loop));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_async_data_clear() {

    struct dummy_userdata dummy = {0};

    dtn_async_data data = (dtn_async_data){.socket = 1,
                                           .message = dtn_item_object(),
                                           .timedout.userdata = &dummy,
                                           .timedout.callback = dummy_timeout,
                                           .callback.userdata = &dummy,
                                           .callback.callback = dummy_callback};

    dtn_async_data_clear(&data);

    testrun(data.socket == 0);
    testrun(data.message == NULL);
    testrun(data.timedout.userdata == NULL);
    testrun(data.timedout.callback == NULL);
    testrun(data.callback.userdata == NULL);
    testrun(data.callback.callback == NULL);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_async_set() {

    struct dummy_userdata dummy = {0};

    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);

    dtn_async_store *store = dtn_async_store_create((dtn_async_store_config){
        .loop = loop, .limits.invalidate_check_usec = 50000});

    testrun(store);

    testrun(dtn_async_set(store, "1",
                          (dtn_async_data){.socket = 1,
                                           .message = NULL,
                                           .timedout.userdata = &dummy,
                                           .timedout.callback = dummy_timeout},
                          100000));

    testrun(!dtn_async_set(NULL, NULL, (dtn_async_data){0}, 0));
    testrun(!dtn_async_set(store, NULL, (dtn_async_data){0}, 0));
    testrun(!dtn_async_set(NULL, "id", (dtn_async_data){0}, 0));

    testrun(dtn_async_set(store, "2", (dtn_async_data){0}, 0));

    sleep(1);
    testrun(dtn_event_loop_run(loop, DTN_RUN_ONCE));

    testrun(0 == dtn_dict_count(store->data.dict));
    // check callback

    testrun(dummy.data.socket == 1);

    testrun(NULL == dtn_async_store_free(store));
    testrun(NULL == dtn_event_loop_free(loop));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_async_get() {

    struct dummy_userdata dummy = {0};

    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);

    dtn_async_store *store = dtn_async_store_create((dtn_async_store_config){
        .loop = loop, .limits.invalidate_check_usec = 50000});

    testrun(store);

    testrun(dtn_async_set(store, "1",
                          (dtn_async_data){.socket = 1,
                                           .message = dtn_item_object(),
                                           .timedout.userdata = &dummy,
                                           .timedout.callback = dummy_timeout},
                          1000000));

    testrun(1 == dtn_dict_count(store->data.dict));

    dtn_async_data data = dtn_async_get(store, "1");
    testrun(0 == dtn_dict_count(store->data.dict));
    testrun(1 == data.socket);
    testrun(dtn_item_is_object(data.message));
    testrun(&dummy == data.timedout.userdata);
    testrun(dummy_timeout == data.timedout.callback);
    dtn_async_data_clear(&data);

    testrun(NULL == dtn_async_store_free(store));
    testrun(NULL == dtn_event_loop_free(loop));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_async_drop() {

    struct dummy_userdata dummy = {0};

    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);

    dtn_async_store *store = dtn_async_store_create((dtn_async_store_config){
        .loop = loop, .limits.invalidate_check_usec = 50000});

    testrun(store);

    testrun(dtn_async_set(store, "1",
                          (dtn_async_data){.socket = 1,
                                           .message = dtn_item_object(),
                                           .timedout.userdata = &dummy,
                                           .timedout.callback = dummy_timeout},
                          1000000));

    testrun(dtn_async_set(store, "2",
                          (dtn_async_data){.socket = 1,
                                           .message = dtn_item_object(),
                                           .timedout.userdata = &dummy,
                                           .timedout.callback = dummy_timeout},
                          1000000));

    testrun(dtn_async_set(store, "3",
                          (dtn_async_data){.socket = 1,
                                           .message = dtn_item_object(),
                                           .timedout.userdata = &dummy,
                                           .timedout.callback = dummy_timeout},
                          1000000));

    testrun(dtn_async_set(store, "4",
                          (dtn_async_data){.socket = 2,
                                           .message = dtn_item_object(),
                                           .timedout.userdata = &dummy,
                                           .timedout.callback = dummy_timeout},
                          1000000));

    testrun(4 == dtn_dict_count(store->data.dict));
    testrun(dtn_async_drop(store, 1));
    testrun(1 == dtn_dict_count(store->data.dict));

    testrun(NULL == dtn_async_store_free(store));
    testrun(NULL == dtn_event_loop_free(loop));
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
    testrun_test(test_dtn_async_store_create);
    testrun_test(test_dtn_async_store_free);
    testrun_test(test_dtn_async_data_clear);
    testrun_test(test_dtn_async_set);
    testrun_test(test_dtn_async_get);
    testrun_test(test_dtn_async_drop);

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
