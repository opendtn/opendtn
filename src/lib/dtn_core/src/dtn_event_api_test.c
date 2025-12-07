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

        This file is part of the openvocs project. https://openvocs.org

        ------------------------------------------------------------------------
*//**
        @file           dtn_event_api_test.c
        @author         Töpfer, Markus

        @date           2025-12-07


        ------------------------------------------------------------------------
*/
#include <dtn_base/testrun.h>
#include "dtn_event_api.c"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_event_message_create(){
    
    dtn_item *msg = dtn_event_message_create("1", "1");
    testrun(msg);
    testrun(0 == strcmp("1",dtn_item_get_string(dtn_item_get(msg, "/event"))));
    testrun(0 == strcmp("1",dtn_item_get_string(dtn_item_get(msg, "/uuid"))));
    msg = dtn_item_free(msg);

    msg = dtn_event_message_create(NULL, "1");
    testrun(msg);
    testrun(0 == strcmp("1",dtn_item_get_string(dtn_item_get(msg, "/event"))));
    testrun(dtn_item_get_string(dtn_item_get(msg, "/uuid")));
    msg = dtn_item_free(msg);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_message_create_reponse(){
    
    dtn_item *msg = dtn_event_message_create("1", "1");
    testrun(msg);

    dtn_item *res = dtn_event_message_create_reponse(msg);
    testrun(res);
    testrun(dtn_item_get(res, "/request"));
    testrun(dtn_item_get(res, "/response"));

    msg = dtn_item_free(msg);
    res = dtn_item_free(res);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_set_error(){
    
    dtn_item *msg = dtn_event_message_create("1", "1");
    testrun(msg);
    testrun(dtn_event_set_error(msg, 1, "error"));
    testrun(1 == dtn_event_get_error_code(msg));
    testrun(0 == strcmp("error", dtn_event_get_error_desc(msg)));

    testrun(dtn_item_get(msg, "/error/code"));
    testrun(dtn_item_get(msg, "/error/desc"));

    msg = dtn_item_free(msg);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_get_error_code(){
    
    dtn_item *msg = dtn_event_message_create("1", "1");
    testrun(msg);
    testrun(dtn_event_set_error(msg, 1, "error"));
    testrun(1 == dtn_event_get_error_code(msg));
    msg = dtn_item_free(msg);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_get_error_desc(){
    
    dtn_item *msg = dtn_event_message_create("1", "1");
    testrun(msg);
    testrun(dtn_event_set_error(msg, 1, "error"));
    testrun(0 == strcmp("error", dtn_event_get_error_desc(msg)));
    msg = dtn_item_free(msg);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_get_event(){
    
    dtn_item *msg = dtn_event_message_create("1", "2");
    testrun(msg);
    testrun(0 == strcmp("2", dtn_event_get_event(msg)));
    msg = dtn_item_free(msg);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_get_uuid(){
    
    dtn_item *msg = dtn_event_message_create("1", "2");
    testrun(msg);
    testrun(0 == strcmp("1", dtn_event_get_uuid(msg)));
    msg = dtn_item_free(msg);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_get_type(){
    
    dtn_item *msg = dtn_event_message_create("1", "2");
    testrun(msg);
    testrun(NULL == dtn_event_get_type(msg));
    testrun(dtn_item_object_set(msg, "type", dtn_item_string("some")));
    testrun(0 == strcmp("some", dtn_event_get_type(msg)));
    msg = dtn_item_free(msg);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_get_paramenter(){
    
    dtn_item *msg = dtn_event_message_create("1", "2");
    testrun(msg);
    testrun(NULL != dtn_event_get_paramenter(msg));
    testrun(dtn_item_get(msg, "/parameter") == dtn_event_get_paramenter(msg));
    msg = dtn_item_free(msg);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_get_request(){
    
    dtn_item *msg = dtn_event_message_create("1", "2");
    testrun(msg);
    testrun(NULL != dtn_event_get_request(msg));
    testrun(dtn_item_get(msg, "/request") == dtn_event_get_request(msg));
    msg = dtn_item_free(msg);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_get_response(){
    
    dtn_item *msg = dtn_event_message_create("1", "2");
    testrun(msg);
    testrun(NULL != dtn_event_get_response(msg));
    testrun(dtn_item_get(msg, "/response") == dtn_event_get_response(msg));
    msg = dtn_item_free(msg);

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
    testrun_test(test_dtn_event_message_create);
    testrun_test(test_dtn_event_message_create_reponse);
    testrun_test(test_dtn_event_set_error);
    testrun_test(test_dtn_event_get_error_code);
    testrun_test(test_dtn_event_get_error_desc);
    testrun_test(test_dtn_event_get_event);
    testrun_test(test_dtn_event_get_uuid);
    testrun_test(test_dtn_event_get_type);
    testrun_test(test_dtn_event_get_paramenter);
    testrun_test(test_dtn_event_get_request);
    testrun_test(test_dtn_event_get_response);


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
