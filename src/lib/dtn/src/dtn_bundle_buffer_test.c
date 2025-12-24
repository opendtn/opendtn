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
        @file           dtn_bundle_buffer_test.c
        @author         Töpfer, Markus

        @date           2025-12-23


        ------------------------------------------------------------------------
*/
#include <dtn_base/testrun.h>
#include "dtn_bundle_buffer.c"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_bundle_buffer_create(){
    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);

    dtn_bundle_buffer *self = dtn_bundle_buffer_create(
        (dtn_bundle_buffer_config){
            .loop = loop
        });

    testrun(self);
    testrun(self->data.dict);
    testrun(DTN_TIMER_INVALID != self->timer.cleanup);

    testrun(NULL == dtn_bundle_buffer_free(self));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_buffer_free(){
    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);

    dtn_bundle_buffer *self = dtn_bundle_buffer_create(
        (dtn_bundle_buffer_config){
            .loop = loop
        });

    testrun(self);

    // add a bundle to check for freeing with data

    dtn_bundle *bundle = dtn_bundle_create();
    testrun(bundle);

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        0,
        1,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        100,
        200);

    dtn_cbor *payload = dtn_bundle_add_block(
        bundle,
        1,
        1,
        0,
        1,
        dtn_cbor_string("test"));

    testrun(payload);
    testrun(primary);

    testrun(dtn_bundle_buffer_push(self, bundle));

    testrun(NULL == dtn_bundle_buffer_free(self));
    testrun(NULL == dtn_event_loop_free(loop));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

struct dummy_data {

    dtn_buffer *buffer;
    char *source;
    char *destination;
};

/*----------------------------------------------------------------------------*/

static void dummy_data_clear(struct dummy_data *data){

    data->buffer = dtn_buffer_free(data->buffer);
    data->source = dtn_data_pointer_free(data->source);
    data->destination = dtn_data_pointer_free(data->destination);
    return;
}

/*----------------------------------------------------------------------------*/

static void dummy_callback(void *userdata, 
                           const uint8_t *payload,
                           size_t size,
                           const char *source,
                           const char *destination){

    struct dummy_data *data = (struct dummy_data*) userdata;
    
    dummy_data_clear(data);

    data->buffer = dtn_buffer_create(size);
    dtn_buffer_push(data->buffer, (uint8_t*)payload, size);
    data->source = dtn_string_dup(source);
    data->destination = dtn_string_dup(destination);
    return;

}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_buffer_push(){

    struct dummy_data dummy = {0};

    dtn_event_loop *loop = dtn_event_loop_default(
        (dtn_event_loop_config){.max.sockets = 100, .max.timers = 100});
    testrun(loop);

    dtn_bundle_buffer *self = dtn_bundle_buffer_create(
        (dtn_bundle_buffer_config){
            .loop = loop,
            .callbacks.userdata = &dummy,
            .callbacks.payload = dummy_callback
        });

    testrun(self);

    // add a complete bundle and check direct callback

    dtn_bundle *bundle = dtn_bundle_create();
    testrun(bundle);

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        0,
        0,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    dtn_cbor *payload = dtn_bundle_add_block(
        bundle,
        1,
        1,
        0,
        1,
        dtn_cbor_string("test"));

    testrun(payload);
    testrun(primary);

    testrun(dtn_bundle_buffer_push(self, bundle));
    testrun(dummy.buffer);
    testrun(0 == dtn_string_compare(dummy.source, "source"));
    testrun(0 == dtn_string_compare(dummy.destination, "destination"));
    testrun(0 == memcmp(dummy.buffer->start, "test", dummy.buffer->length));
    dummy_data_clear(&dummy);

    // add 3 bundle fragments ordered delivery

    bundle = dtn_bundle_create();
    testrun(bundle);

    primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        0,
        "destination",
        "source",
        "report",
        3,
        0,
        5,
        0,
        12);

    payload = dtn_bundle_add_block(
        bundle,
        1,
        1,
        0,
        0,
        dtn_cbor_string("test"));

    testrun(payload);
    testrun(primary);

    testrun(dtn_bundle_buffer_push(self, bundle));
    testrun(!dummy.buffer);

    bundle = dtn_bundle_create();
    testrun(bundle);

    primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        0,
        "destination",
        "source",
        "report",
        3,
        1,
        5,
        4,
        12);

    payload = dtn_bundle_add_block(
        bundle,
        1,
        1,
        0,
        0,
        dtn_cbor_string("1234"));

    testrun(payload);
    testrun(primary);

    testrun(dtn_bundle_buffer_push(self, bundle));
    testrun(!dummy.buffer);

    bundle = dtn_bundle_create();
    testrun(bundle);

    primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        0,
        "destination",
        "source",
        "report",
        3,
        3,
        5,
        8,
        12);

    payload = dtn_bundle_add_block(
        bundle,
        1,
        1,
        0,
        0,
        dtn_cbor_string("5678"));

    testrun(payload);
    testrun(primary);

    testrun(dtn_bundle_buffer_push(self, bundle));
    testrun(dummy.buffer);

    testrun(0 == memcmp(dummy.buffer->start, "test12345678", dummy.buffer->length));
    testrun(0 == dtn_string_compare(dummy.source, "source"));
    testrun(0 == dtn_string_compare(dummy.destination, "destination"));
    
    dummy_data_clear(&dummy);   

    // add 3 bundle fragments unordered delivery

    bundle = dtn_bundle_create();
    testrun(bundle);

    primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        0,
        "destination",
        "source",
        "report",
        3,
        1,
        5,
        0,
        12);

    payload = dtn_bundle_add_block(
        bundle,
        1,
        1,
        0,
        0,
        dtn_cbor_string("test"));

    testrun(payload);
    testrun(primary);

    testrun(dtn_bundle_buffer_push(self, bundle));
    testrun(!dummy.buffer);

    bundle = dtn_bundle_create();
    testrun(bundle);

    primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        0,
        "destination",
        "source",
        "report",
        3,
        0,
        5,
        4,
        12);

    payload = dtn_bundle_add_block(
        bundle,
        1,
        1,
        0,
        0,
        dtn_cbor_string("1234"));

    testrun(payload);
    testrun(primary);

    testrun(dtn_bundle_buffer_push(self, bundle));
    testrun(!dummy.buffer);

    bundle = dtn_bundle_create();
    testrun(bundle);

    primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        0,
        "destination",
        "source",
        "report",
        3,
        2,
        5,
        8,
        12);

    payload = dtn_bundle_add_block(
        bundle,
        1,
        1,
        0,
        0,
        dtn_cbor_string("5678"));

    testrun(payload);
    testrun(primary);

    testrun(dtn_bundle_buffer_push(self, bundle));
    testrun(dummy.buffer);

    testrun(0 == memcmp(dummy.buffer->start, "1234test5678", dummy.buffer->length));
    testrun(0 == dtn_string_compare(dummy.source, "source"));
    testrun(0 == dtn_string_compare(dummy.destination, "destination"));
    
    dummy_data_clear(&dummy);   


    testrun(NULL == dtn_bundle_buffer_free(self));
    testrun(NULL == dtn_event_loop_free(loop));

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
    testrun_test(test_dtn_bundle_buffer_create);
    testrun_test(test_dtn_bundle_buffer_free);
    testrun_test(test_dtn_bundle_buffer_push);

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
