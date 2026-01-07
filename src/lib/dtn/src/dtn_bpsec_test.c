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
        @file           dtn_bpsec_test.c
        @author         Töpfer, Markus

        @date           2026-01-04


        ------------------------------------------------------------------------
*/
#include <dtn_base/testrun.h>
#include "dtn_bpsec.c"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_bpsec_asb_decode(){

    uint8_t buffer[1024] = {0};

    buffer[0]  = 0x81; // target
    buffer[1]  = 0x01;
    buffer[2]  = 0x01; // id
    buffer[3]  = 0x01; // flags
    buffer[4]  = 0x82; // source
    buffer[5]  = 0x02;  // ipn
    buffer[6]  = 0x82;  // 
    buffer[7]  = 0x02;  // 2
    buffer[8]  = 0x01;  // 1
    buffer[9]  = 0x82; // params
    buffer[10] = 0x82;   // 2
    buffer[11] = 0x01;   // id
    buffer[12] = 0x06;   // variant
    buffer[13] = 0x82;   // 2
    buffer[14] = 0x03;   // id
    buffer[15] = 0x07;   // all flags
    buffer[16] = 0x81; // security results
    buffer[17] = 0x81;   // 1 item
    buffer[18] = 0x82;   // 2 items
    buffer[19] = 0x01;   // id 1
    buffer[20] = 0x58;   // string
    buffer[21] = 0x30;   // 30 length
    buffer[22] = 0xf7;
    buffer[23] = 0x5f;
    buffer[24] = 0xe4;
    buffer[25] = 0xc3;
    buffer[26] = 0x7f;
    buffer[27] = 0x76;
    buffer[28] = 0xf0;
    buffer[29] = 0x46;
    buffer[30] = 0x16;
    buffer[31] = 0x58;
    buffer[32] = 0x55;
    buffer[33] = 0xbd;
    buffer[34] = 0x5f;
    buffer[35] = 0xf7;
    buffer[36] = 0x2f;
    buffer[37] = 0xbf;
    buffer[38] = 0xd4;
    buffer[39] = 0xe3;
    buffer[40] = 0xa6;
    buffer[41] = 0x4b;
    buffer[42] = 0x46;
    buffer[43] = 0x95;
    buffer[44] = 0xc4;
    buffer[45] = 0x0e;
    buffer[46] = 0x2b;
    buffer[47] = 0x78;
    buffer[48] = 0x7d;
    buffer[49] = 0xa0;
    buffer[50] = 0x05;
    buffer[51] = 0xae;
    buffer[52] = 0x81;
    buffer[53] = 0x9f;
    buffer[54] = 0x0a;
    buffer[55] = 0x2e;
    buffer[56] = 0x30;
    buffer[57] = 0xa2;
    buffer[58] = 0xe8;
    buffer[59] = 0xb3;
    buffer[60] = 0x25;
    buffer[61] = 0x52;
    buffer[62] = 0x7d;
    buffer[63] = 0xe8;
    buffer[64] = 0xae;
    buffer[65] = 0xfb;
    buffer[66] = 0x52;
    buffer[67] = 0xe7;
    buffer[68] = 0x3d;
    buffer[69] = 0x71;

    dtn_cbor *string = dtn_cbor_string(NULL);
    testrun(dtn_cbor_set_byte_string(string, buffer, 70));

    dtn_bpsec_asb *asb = dtn_bpsec_asb_decode(string);
    testrun(asb);
    testrun(dtn_cbor_is_array(asb->target));
    testrun(dtn_cbor_is_uint(asb->context_id));
    testrun(dtn_cbor_is_uint(asb->context_flags));
    testrun(dtn_cbor_is_array(asb->source));
    testrun(dtn_cbor_is_array(asb->context_parameter));
    testrun(dtn_cbor_is_array(asb->results));

    testrun(1 == dtn_cbor_array_count(asb->target));
    dtn_cbor *item = dtn_cbor_array_get(asb->target, 0);
    testrun(1 == dtn_cbor_get_uint(item));
    testrun(1 == dtn_cbor_get_uint(asb->context_id));
    testrun(1 == dtn_cbor_get_uint(asb->context_flags));
    testrun(2 == dtn_cbor_array_count(asb->source));
    testrun(2 == dtn_cbor_get_uint(dtn_cbor_array_get(asb->source, 0)));
    testrun(dtn_cbor_is_array(dtn_cbor_array_get(asb->source, 1)));
    testrun(dtn_cbor_is_array(dtn_cbor_array_get(asb->context_parameter, 0)));
    testrun(dtn_cbor_is_array(dtn_cbor_array_get(asb->context_parameter, 1)));

    dtn_cbor *out = dtn_bpsec_asb_encode(asb);
    testrun(out);
    uint8_t *buf = NULL;
    size_t size = 0;
    testrun(dtn_cbor_get_byte_string(out, &buf, &size));
    testrun(size == 70);
    testrun(0 == memcmp(buf, buffer, size));

    asb = dtn_bpsec_asb_free(asb);
    string = dtn_cbor_free(string);
    out = dtn_cbor_free(out);
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
    testrun_test(test_dtn_bpsec_asb_decode);

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
