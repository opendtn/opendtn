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
        @file           dtn_uri_test.c
        @author         Töpfer, Markus

        @date           2025-12-17


        ------------------------------------------------------------------------
*/
#include <dtn_base/testrun.h>
#include "dtn_uri.c"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_uri_create(){
    
    dtn_uri *self = dtn_uri_create();
    testrun(self);

    self = dtn_uri_free(self);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_uri_free(){
    
    dtn_uri *self = dtn_uri_create();
    testrun(self);

    self = dtn_uri_free(self);
    testrun(!self);

    self = dtn_uri_create();
    self->scheme = dtn_string_dup("scheme");
    self->name = dtn_string_dup("name");
    self->demux = dtn_string_dup("demux");
    self = dtn_uri_free(self);
    testrun(!self);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_uri_clear(){
    
    dtn_uri *self = dtn_uri_create();
    testrun(self);

    testrun(dtn_uri_clear(self));

    self->scheme = dtn_string_dup("scheme");
    self->name = dtn_string_dup("name");
    self->demux = dtn_string_dup("demux");
    testrun(dtn_uri_clear(self));
    testrun(self->name == NULL);
    testrun(self->scheme == NULL);
    testrun(self->demux == NULL);

    self = dtn_uri_free(self);
    testrun(!self);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_uri_dump(){
    
    dtn_uri *self = dtn_uri_create();
    testrun(self);

    testrun(dtn_uri_dump(stdout, self));
    fprintf(stdout, "\n");

    self->scheme = dtn_string_dup("scheme");
    self->name = dtn_string_dup("name");
    self->demux = dtn_string_dup("demux");

    testrun(dtn_uri_dump(stdout, self));
    fprintf(stdout, "\n");

    self = dtn_uri_free(self);
    testrun(!self);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_uri_copy(){
        
    dtn_uri *copy = NULL;
    dtn_uri *self = dtn_uri_create();
    testrun(self);

    testrun(dtn_uri_copy((void**)&copy, self));
    testrun(copy);
    testrun(copy->name == NULL);
    testrun(copy->scheme == NULL);
    testrun(copy->demux == NULL);
    copy = dtn_uri_free(copy);

    self->scheme = dtn_string_dup("scheme");
    self->name = dtn_string_dup("name");
    self->demux = dtn_string_dup("demux");
    testrun(dtn_uri_copy((void**)&copy, self));
    testrun(copy);
    testrun(copy->name != NULL);
    testrun(copy->scheme != NULL);
    testrun(copy->demux != NULL);
    testrun(0 == strcmp(copy->name, self->name));
    testrun(0 == strcmp(copy->scheme, self->scheme));
    testrun(0 == strcmp(copy->demux, self->demux));
    copy = dtn_uri_free(copy);
    self = dtn_uri_free(self);
    testrun(!self);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_uri_decode(){
        
    dtn_uri *self = NULL;

    self = dtn_uri_decode("scheme:none");
    testrun(self);
    testrun(0 == strcmp(self->scheme, "scheme"));
    testrun(0 == strcmp(self->name, "none"));
    self = dtn_uri_free(self);

    self = dtn_uri_decode("scheme://name/demux");
    testrun(self);
    testrun(0 == strcmp(self->scheme, "scheme"));
    testrun(0 == strcmp(self->name, "name"));
    testrun(0 == strcmp(self->demux, "demux"));
    self = dtn_uri_free(self);

    self = dtn_uri_decode("scheme://name/~");
    testrun(self);
    testrun(0 == strcmp(self->scheme, "scheme"));
    testrun(0 == strcmp(self->name, "name"));
    testrun(0 == strcmp(self->demux, "~"));
    self = dtn_uri_free(self);

    self = dtn_uri_decode("scheme://name/");
    testrun(self);
    testrun(0 == strcmp(self->scheme, "scheme"));
    testrun(0 == strcmp(self->name, "name"));
    testrun(0 != self->demux);
    testrun(self->demux[0] == 0x00);
    self = dtn_uri_free(self);

    self = dtn_uri_decode("scheme://name");
    testrun(self);
    testrun(0 == strcmp(self->scheme, "scheme"));
    testrun(0 == strcmp(self->name, "name"));
    testrun(0 == self->demux);
    self = dtn_uri_free(self);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_uri_encode(){

    dtn_uri *self = dtn_uri_create();

    char *str = dtn_uri_encode(self);
    testrun(str);
    str = dtn_data_pointer_free(str);

    self->scheme = dtn_string_dup("scheme");
    self->name = dtn_string_dup("name");
    self->demux = dtn_string_dup("demux");
    str = dtn_uri_encode(self);
    char *expect = "scheme://name/demux";
    testrun(0 == strcmp(str, expect));
    str = dtn_data_pointer_free(str);

    self->demux = dtn_data_pointer_free(self->demux);
    str = dtn_uri_encode(self);
    expect = "scheme://name/";
    fprintf(stdout, "%s\n", str);
    testrun(0 == strcmp(str, expect));
    str = dtn_data_pointer_free(str);

    self->name = dtn_data_pointer_free(self->name);
    str = dtn_uri_encode(self);
    expect = "scheme:none";
    testrun(0 == strcmp(str, expect));
    str = dtn_data_pointer_free(str);

    self = dtn_uri_free(self);
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
    testrun_test(test_dtn_uri_create);
    testrun_test(test_dtn_uri_free);
    testrun_test(test_dtn_uri_clear);
    testrun_test(test_dtn_uri_dump);
    testrun_test(test_dtn_uri_copy);
    testrun_test(test_dtn_uri_decode);
    testrun_test(test_dtn_uri_encode);

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
