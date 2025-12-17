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
        @file           dtn_ipn_test.c
        @author         Töpfer, Markus

        @date           2025-12-17


        ------------------------------------------------------------------------
*/
#include <dtn_base/testrun.h>
#include "dtn_ipn.c"
/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_ipn_create(){
    
    dtn_ipn *self = dtn_ipn_create();
    testrun(self);

    self = dtn_ipn_free(self);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_ipn_free(){
    
    dtn_ipn *self = dtn_ipn_create();
    testrun(self);

    self = dtn_ipn_free(self);
    testrun(!self);

    self = dtn_ipn_create();
    self->scheme = dtn_string_dup("scheme");
    self->node = dtn_string_dup("node");
    self->service = dtn_string_dup("service");
    self = dtn_ipn_free(self);
    testrun(!self);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_ipn_clear(){
    
    dtn_ipn *self = dtn_ipn_create();
    testrun(self);

    testrun(dtn_ipn_clear(self));

    self->scheme = dtn_string_dup("scheme");
    self->node = dtn_string_dup("node");
    self->service = dtn_string_dup("service");
    testrun(dtn_ipn_clear(self));
    testrun(self->node == NULL);
    testrun(self->scheme == NULL);
    testrun(self->service == NULL);

    self = dtn_ipn_free(self);
    testrun(!self);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_ipn_dump(){
    
    dtn_ipn *self = dtn_ipn_create();
    testrun(self);

    testrun(dtn_ipn_dump(stdout, self));
    fprintf(stdout, "\n");

    self->scheme = dtn_string_dup("scheme");
    self->node = dtn_string_dup("node");
    self->service = dtn_string_dup("service");

    testrun(dtn_ipn_dump(stdout, self));
    fprintf(stdout, "\n");

    self = dtn_ipn_free(self);
    testrun(!self);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_ipn_copy(){
        
    dtn_ipn *copy = NULL;
    dtn_ipn *self = dtn_ipn_create();
    testrun(self);

    testrun(dtn_ipn_copy((void**)&copy, self));
    testrun(copy);
    testrun(copy->node == NULL);
    testrun(copy->scheme == NULL);
    testrun(copy->service == NULL);
    copy = dtn_ipn_free(copy);

    self->scheme = dtn_string_dup("scheme");
    self->node = dtn_string_dup("node");
    self->service = dtn_string_dup("service");
    testrun(dtn_ipn_copy((void**)&copy, self));
    testrun(copy);
    testrun(copy->node != NULL);
    testrun(copy->scheme != NULL);
    testrun(copy->service != NULL);
    testrun(0 == strcmp(copy->scheme, self->scheme));
    testrun(0 == strcmp(copy->node, self->node));
    testrun(0 == strcmp(copy->service, self->service));
    copy = dtn_ipn_free(copy);
    self = dtn_ipn_free(self);
    testrun(!self);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_ipn_decode(){
        
    dtn_ipn *self = NULL;

    self = dtn_ipn_decode("ipn:1.2");
    testrun(self);
    testrun(0 == strcmp("ipn", self->scheme));
    testrun(0 == strcmp("1", self->node));
    testrun(0 == strcmp("2", self->service));
    self = dtn_ipn_free(self);

    self = dtn_ipn_decode("ipn:12345.6789");
    testrun(self);
    testrun(0 == strcmp("ipn", self->scheme));
    testrun(0 == strcmp("12345", self->node));
    testrun(0 == strcmp("6789", self->service));
    self = dtn_ipn_free(self);

    self = dtn_ipn_decode("ipn:123a3.6789");
    testrun(!self);

    self = dtn_ipn_decode("ipn:1233.67x89");
    testrun(!self);

    self = dtn_ipn_decode("ipn:.1");
    testrun(!self);

    self = dtn_ipn_decode("ipn:1.");
    testrun(!self);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_ipn_encode(){

    dtn_ipn *self = dtn_ipn_create();

    self->scheme = dtn_string_dup("ipn");
    self->node = dtn_string_dup("1");
    self->service = dtn_string_dup("2");
    char *expect = "ipn:1.2";
    char *string = dtn_ipn_encode(self);
    testrun(0 == strcmp(expect, string));
    string = dtn_data_pointer_free(string);

    self = dtn_ipn_free(self);
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
    testrun_test(test_dtn_ipn_create);
    testrun_test(test_dtn_ipn_free);
    testrun_test(test_dtn_ipn_clear);
    testrun_test(test_dtn_ipn_dump);
    testrun_test(test_dtn_ipn_copy);
    testrun_test(test_dtn_ipn_decode);
    testrun_test(test_dtn_ipn_encode);

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
