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
        @file           dtn_bundle_test.c
        @author         Töpfer, Markus

        @date           2025-12-15


        ------------------------------------------------------------------------
*/
#include <dtn_base/testrun.h>
#include "dtn_bundle.c"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_bundle_create(){
    
    dtn_bundle *self = dtn_bundle_create();
    testrun(self);
    testrun(NULL == self->data);
    testrun(NULL == dtn_bundle_free(self));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_free(){
    
    dtn_bundle *self = dtn_bundle_create();
    testrun(self);
    testrun(NULL == self->data);
    testrun(NULL == dtn_bundle_free(self));

    self = dtn_bundle_create();
    testrun(self);
    testrun(NULL == self->data);
    self->data = dtn_cbor_array();
    testrun(NULL == dtn_bundle_free(self));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_check_primary_block(){

    dtn_bundle *self = dtn_bundle_create();
    testrun(self);
    testrun(!check_primary_block(self));

    testrun(dtn_bundle_primary_set_version(self));
    testrun(dtn_bundle_primary_set_flags(self, 0));
    testrun(dtn_bundle_primary_set_crc_type(self, 0));
    testrun(dtn_bundle_primary_set_destination(self, "dtn:1"));
    testrun(dtn_bundle_primary_set_source(self, "dtn:2"));
    testrun(dtn_bundle_primary_set_report(self, "dtn:3"));
    testrun(dtn_bundle_primary_set_timestamp(self, 1, 2));
    testrun(dtn_bundle_primary_set_lifetime(self, 3));
    testrun(check_primary_block(self));

    testrun(NULL == dtn_bundle_free(self));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_check_blocks(){

    dtn_bundle *self = dtn_bundle_create();
    testrun(self);
    testrun(!check_primary_block(self));
    testrun(check_canonical_blocks(self));

    testrun(dtn_bundle_primary_set_version(self));
    testrun(dtn_bundle_primary_set_flags(self, 0));
    testrun(dtn_bundle_primary_set_crc_type(self, 0));
    testrun(dtn_bundle_primary_set_destination(self, "dtn:1"));
    testrun(dtn_bundle_primary_set_source(self, "dtn:2"));
    testrun(dtn_bundle_primary_set_report(self, "dtn:3"));
    testrun(dtn_bundle_primary_set_timestamp(self, 1, 2));
    testrun(dtn_bundle_primary_set_lifetime(self, 3));
    testrun(check_primary_block(self));
    testrun(check_canonical_blocks(self));

    dtn_cbor *payload = dtn_cbor_string("test");
    dtn_cbor *block = dtn_bundle_add_block(self, 1,2,3,0,payload);
    testrun(block);
    testrun(check_canonical_blocks(self));

    payload = dtn_cbor_string("test");
    block = dtn_bundle_add_block(self, 1,3,0,0,payload);
    testrun(block);
    testrun(check_canonical_blocks(self));

    // add incomplete block
    block = dtn_cbor_array();
    dtn_cbor_array_push(self->data, block);
    testrun(!check_canonical_blocks(self));

    testrun(NULL == dtn_bundle_free(self));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_check_payload_block(){

    dtn_bundle *self = dtn_bundle_create();
    testrun(self);
    testrun(!check_primary_block(self));
    testrun(!check_payload_block(self));

    testrun(dtn_bundle_primary_set_version(self));
    testrun(dtn_bundle_primary_set_flags(self, 0));
    testrun(dtn_bundle_primary_set_crc_type(self, 0));
    testrun(dtn_bundle_primary_set_destination(self, "dtn:1"));
    testrun(dtn_bundle_primary_set_source(self, "dtn:2"));
    testrun(dtn_bundle_primary_set_report(self, "dtn:3"));
    testrun(dtn_bundle_primary_set_timestamp(self, 1, 2));
    testrun(dtn_bundle_primary_set_lifetime(self, 3));
    testrun(check_primary_block(self));
    testrun(check_canonical_blocks(self));

    dtn_cbor *payload = dtn_cbor_string("test");
    dtn_cbor *block = dtn_bundle_add_block(self, 1,2,3,0,payload);
    testrun(block);
    testrun(check_payload_block(self));

    // payload block not last block
    payload = dtn_cbor_string("test");
    block = dtn_bundle_add_block(self, 6,3,0,0,payload);
    testrun(block);
    testrun(!check_payload_block(self));

    testrun(NULL == dtn_bundle_free(self));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_verify(){

    dtn_bundle *self = dtn_bundle_create();
    testrun(self);
    testrun(!dtn_bundle_verify(self));

    testrun(dtn_bundle_primary_set_version(self));
    testrun(dtn_bundle_primary_set_flags(self, 0));
    testrun(dtn_bundle_primary_set_crc_type(self, 0));
    testrun(dtn_bundle_primary_set_destination(self, "dtn:1"));
    testrun(dtn_bundle_primary_set_source(self, "dtn:2"));
    testrun(dtn_bundle_primary_set_report(self, "dtn:3"));
    testrun(dtn_bundle_primary_set_timestamp(self, 1, 2));
    testrun(dtn_bundle_primary_set_lifetime(self, 3));
    testrun(!dtn_bundle_verify(self));

    dtn_cbor *payload = dtn_cbor_string("test");
    dtn_cbor *block = dtn_bundle_add_block(self, 1,2,3,0,payload);
    testrun(block);
    testrun(dtn_bundle_verify(self));

    // payload block not last block
    payload = dtn_cbor_string("test");
    block = dtn_bundle_add_block(self, 6,3,0,0,payload);
    testrun(block);
    testrun(!dtn_bundle_verify(self));

    testrun(NULL == dtn_bundle_free(self));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_decode(){
    
    dtn_bundle *out = NULL;

    uint8_t buffer[0xffff] = {0};
    size_t size = 0xffff;
    uint8_t *next = NULL;

    // min valid, primary block and payload block
    buffer[0]  = 0x9f;
    buffer[1]  = 0x88;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x00; // flags
    buffer[4]  = 0x00; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x85; // payload
    buffer[28] = 0x01; // code
    buffer[29] = 0x02; // number
    buffer[30] = 0x00; // flags
    buffer[31] = 0x00; // crc_type
    buffer[32] = 0x44;
    buffer[33] = 't';
    buffer[34] = 'e';
    buffer[35] = 's';
    buffer[36] = 't';
    buffer[37] = 0xFF;

    for (int i = 1; i < 37; i++){

        testrun(DTN_CBOR_MATCH_PARTIAL == 
                dtn_bundle_decode(buffer, i, &out, &next));
    }

    testrun(DTN_CBOR_MATCH_FULL ==
        dtn_bundle_decode(buffer, 38, &out, &next));
    testrun(out);
    testrun(next == buffer + 38);

    out = dtn_bundle_free(out);

    testrun(DTN_CBOR_MATCH_FULL ==
        dtn_bundle_decode(buffer, size, &out, &next));
    testrun(out);
    testrun(next == buffer + 38);

    out = dtn_bundle_free(out);

    // check failure in items of payload
    buffer[0]  = 0x9f;
    buffer[1]  = 0x88;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x00; // flags
    buffer[4]  = 0x00; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x85; // payload
    buffer[28] = 0x01; // code
    buffer[29] = 0x02; // number
    buffer[30] = 0x00; // flags
                        // crc_type missing
    buffer[31] = 0x44;
    buffer[32] = 't';
    buffer[33] = 'e';
    buffer[34] = 's';
    buffer[35] = 't';
    buffer[36] = 0xFF;

    testrun(DTN_CBOR_NO_MATCH ==
        dtn_bundle_decode(buffer, 38, &out, &next));
    testrun(!out);

    testrun(NULL == dtn_bundle_free(out));

    // check failure in items of primary
    buffer[0]  = 0x9f;
    buffer[1]  = 0x88;
    buffer[2]  = 0x07; // version
     // flags missing
    buffer[3]  = 0x00; // crc_type
    buffer[4]  = 0x45; // destination
    buffer[5]  = 'd';
    buffer[6]  = 't';
    buffer[7]  = 'n';
    buffer[8]  = ':';
    buffer[9]  = '1';
    buffer[10] = 0x45; // source
    buffer[11] = 'd';
    buffer[12] = 't';
    buffer[13] = 'n';
    buffer[14] = ':';
    buffer[15] = '2';
    buffer[16] = 0x45; // report
    buffer[17] = 'd';
    buffer[18] = 't';
    buffer[19] = 'n';
    buffer[20] = ':';
    buffer[21] = '3';
    buffer[22] = 0x82; // timestamp
    buffer[23] = 0x01;          // time
    buffer[24] = 0x02;          // sequence number
    buffer[25] = 0x03; // lifetime
    buffer[26] = 0x85; // payload
    buffer[27] = 0x01; // code
    buffer[28] = 0x02; // number
    buffer[29] = 0x00; // flags
    buffer[30] = 0x00; // crc_type
    buffer[31] = 0x44;
    buffer[32] = 't';
    buffer[33] = 'e';
    buffer[34] = 's';
    buffer[35] = 't';
    buffer[36] = 0xFF;

    testrun(DTN_CBOR_NO_MATCH ==
        dtn_bundle_decode(buffer, 38, &out, &next));
    testrun(!out);

    testrun(NULL == dtn_bundle_free(out));

    // min valid with one additional block 
    buffer[0]  = 0x9f;
    buffer[1]  = 0x88;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x00; // flags
    buffer[4]  = 0x00; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x85; // addtional block
    buffer[28] = 0x02; // code
    buffer[29] = 0x03; // number
    buffer[30] = 0x00; // flags
    buffer[31] = 0x00; // crc_type
    buffer[32] = 0x44;
    buffer[33] = 't';
    buffer[34] = 'e';
    buffer[35] = 's';
    buffer[36] = 't';
    buffer[37] = 0x85; // payload
    buffer[38] = 0x01; // code
    buffer[39] = 0x02; // number
    buffer[40] = 0x00; // flags
    buffer[41] = 0x00; // crc_type
    buffer[42] = 0x44;
    buffer[43] = 't';
    buffer[44] = 'e';
    buffer[45] = 's';
    buffer[46] = 't';
    buffer[47] = 0xFF;

    for (int i = 1; i < 48; i++){

        testrun(DTN_CBOR_MATCH_PARTIAL == 
                dtn_bundle_decode(buffer, i, &out, &next));
    }

    testrun(DTN_CBOR_MATCH_FULL ==
        dtn_bundle_decode(buffer, 48, &out, &next));
    testrun(out);
    testrun(next == buffer + 48);

    out = dtn_bundle_free(out);

    // failure in additional block 
    buffer[0]  = 0x9f;
    buffer[1]  = 0x88;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x00; // flags
    buffer[4]  = 0x00; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x84; // additional block (only 4 elements)
     // code missing
    buffer[28] = 0x03; // number
    buffer[29] = 0x00; // flags
    buffer[30] = 0x00; // crc_type
    buffer[31] = 0x44;
    buffer[32] = 't';
    buffer[33] = 'e';
    buffer[34] = 's';
    buffer[35] = 't';
    buffer[36] = 0x85; // payload
    buffer[37] = 0x01; // code
    buffer[38] = 0x02; // number
    buffer[39] = 0x00; // flags
    buffer[40] = 0x00; // crc_type
    buffer[41] = 0x44;
    buffer[42] = 't';
    buffer[43] = 'e';
    buffer[44] = 's';
    buffer[45] = 't';
    buffer[46] = 0xFF;

    for (int i = 1; i < 27; i++){

        testrun(DTN_CBOR_MATCH_PARTIAL == 
                dtn_bundle_decode(buffer, i, &out, &next));
    }

    for (int i = 28; i < 46; i++){

        testrun(DTN_CBOR_NO_MATCH == 
                dtn_bundle_decode(buffer, i, &out, &next));
    }

    // check CBOR is valid
    dtn_cbor *item = NULL;

    testrun(DTN_CBOR_MATCH_FULL == dtn_cbor_decode(
        buffer, 47, &item, &next));
    testrun(item);
    testrun(next == buffer + 47);
    item = dtn_cbor_free(item);

    // check bundle is NOT valid due to missing block item in additional
    testrun(DTN_CBOR_NO_MATCH ==
        dtn_bundle_decode(buffer, 47, &out, &next));
    testrun(!out);

    out = dtn_bundle_free(out);

    // primary block and payload block

    // check CRC field missing, flag is set
    buffer[0]  = 0x9f;
    buffer[1]  = 0x88;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x00; // flags
    buffer[4]  = 0x01; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x85; // payload
    buffer[28] = 0x01; // code
    buffer[29] = 0x02; // number
    buffer[30] = 0x00; // flags
    buffer[31] = 0x00; // crc_type
    buffer[32] = 0x44;
    buffer[33] = 't';
    buffer[34] = 'e';
    buffer[35] = 's';
    buffer[36] = 't';
    buffer[37] = 0xFF;

    for (int i = 1; i < 27; i++){

        testrun(DTN_CBOR_MATCH_PARTIAL == 
                dtn_bundle_decode(buffer, i, &out, &next));
    }

    for (int i = 27; i < 37; i++){

        testrun(DTN_CBOR_NO_MATCH == 
                dtn_bundle_decode(buffer, i, &out, &next));
    }

    testrun(DTN_CBOR_NO_MATCH ==
        dtn_bundle_decode(buffer, 38, &out, &next));
    testrun(!out);

    // check CRC field present but wrong, flag is set
    buffer[0]  = 0x9f;
    buffer[1]  = 0x89;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x00; // flags
    buffer[4]  = 0x01; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x42; // crc16 string of 2 bytes
    buffer[28] = 0x0F; // fake CRC
    buffer[29] = 0x1F; // fake CRC
    buffer[30] = 0x85; // payload
    buffer[31] = 0x01; // code
    buffer[32] = 0x02; // number
    buffer[33] = 0x00; // flags
    buffer[34] = 0x00; // crc_type
    buffer[35] = 0x44;
    buffer[36] = 't';
    buffer[37] = 'e';
    buffer[38] = 's';
    buffer[39] = 't';
    buffer[40] = 0xFF;

    for (int i = 1; i < 30; i++){

        testrun(DTN_CBOR_MATCH_PARTIAL == 
                dtn_bundle_decode(buffer, i, &out, &next));
    }

    for (int i = 30; i < 40; i++){

        testrun(DTN_CBOR_NO_MATCH == 
                dtn_bundle_decode(buffer, i, &out, &next));
    }

    testrun(DTN_CBOR_NO_MATCH ==
        dtn_bundle_decode(buffer, 41, &out, &next));
  
    // set correct CRC16 

    buffer[28] = 0x00;
    buffer[29] = 0x00;

    uint16_t crc = crc16x25(buffer + 1, 29);
    buffer[28] = crc >> 8;
    buffer[29] = crc;

    testrun(DTN_CBOR_MATCH_FULL ==
        dtn_bundle_decode(buffer, 41, &out, &next));

    testrun(out);
    testrun(next == buffer + 41);
    out = dtn_bundle_free(out);

    // check CRC absent fragmentation set
    buffer[0]  = 0x9f;
    buffer[1]  = 0x8A;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x01; // flags
    buffer[4]  = 0x00; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x03; // fragment offset
    buffer[28] = 0x06;  // total application data
    buffer[29] = 0x85; // payload
    buffer[30] = 0x01; // code
    buffer[31] = 0x02; // number
    buffer[32] = 0x00; // flags
    buffer[33] = 0x00; // crc_type
    buffer[34] = 0x44;
    buffer[35] = 't';
    buffer[36] = 'e';
    buffer[37] = 's';
    buffer[38] = 't';
    buffer[39] = 0xFF;

    for (int i = 1; i < 40; i++){

        testrun(DTN_CBOR_MATCH_PARTIAL == 
                dtn_bundle_decode(buffer, i, &out, &next));
    }

    testrun(DTN_CBOR_MATCH_FULL ==
        dtn_bundle_decode(buffer, 40, &out, &next));

    testrun(out);
    testrun(next == buffer + 40);
    out = dtn_bundle_free(out);

    // check CRC set fragmentation set
    buffer[0]  = 0x9f;
    buffer[1]  = 0x8B;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x01; // flags
    buffer[4]  = 0x02; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x03; // fragment offset
    buffer[28] = 0x06;  // total application data
    buffer[29] = 0x44;  // crc32
    buffer[30] = 0x00; 
    buffer[31] = 0x00; 
    buffer[32] = 0x00; 
    buffer[33] = 0x00; 
    buffer[34] = 0x85; // payload
    buffer[35] = 0x01; // code
    buffer[36] = 0x02; // number
    buffer[37] = 0x00; // flags
    buffer[38] = 0x00; // crc_type
    buffer[39] = 0x44;
    buffer[40] = 't';
    buffer[41] = 'e';
    buffer[42] = 's';
    buffer[43] = 't';
    buffer[44] = 0xFF;

    uint32_t crc32 = dtn_crc32c(buffer + 1, 33);
    buffer[30] = crc32 >> 24;
    buffer[31] = crc32 >> 16;
    buffer[32] = crc32 >> 8;
    buffer[33] = crc32;

    for (int i = 1; i < 44; i++){

        testrun(DTN_CBOR_MATCH_PARTIAL == 
                dtn_bundle_decode(buffer, i, &out, &next));
    }

    testrun(DTN_CBOR_MATCH_FULL ==
        dtn_bundle_decode(buffer, 45, &out, &next));

    testrun(out);
    testrun(next == buffer + 45);
    out = dtn_bundle_free(out);

    // check CRC32 wrong
    buffer[30] = 0x00;

    testrun(DTN_CBOR_NO_MATCH ==
        dtn_bundle_decode(buffer, 45, &out, &next));

    // check CRC set fragmentation set primary bundle ok
    // no crc in payload set, but flag set
    buffer[34] = 0x85; // payload
    buffer[35] = 0x01; // code
    buffer[36] = 0x02; // number
    buffer[37] = 0x00; // flags
    buffer[38] = 0x01; // crc_type
    buffer[39] = 0x44;
    buffer[40] = 't';
    buffer[41] = 'e';
    buffer[42] = 's';
    buffer[43] = 't';
    buffer[44] = 0xFF;

    testrun(DTN_CBOR_NO_MATCH ==
        dtn_bundle_decode(buffer, 45, &out, &next));

    buffer[30] = crc32 >> 24;
    buffer[31] = crc32 >> 16;
    buffer[32] = crc32 >> 8;
    buffer[33] = crc32;

    // check CRC set fragmentation set primary bundle ok
    // crc and flag in payload are ok 
    buffer[34] = 0x86; // payload
    buffer[35] = 0x01; // code
    buffer[36] = 0x02; // number
    buffer[37] = 0x00; // flags
    buffer[38] = 0x01; // crc_type
    buffer[39] = 0x44;
    buffer[40] = 't';
    buffer[41] = 'e';
    buffer[42] = 's';
    buffer[43] = 't';
    buffer[44] = 0x42;
    buffer[45] = 0x00;
    buffer[46] = 0x00;
    buffer[47] = 0xFF;

    crc = crc16x25(buffer + 34, 13);
    buffer[45] = crc >> 8;
    buffer[46] = crc;

    testrun(DTN_CBOR_MATCH_FULL ==
        dtn_bundle_decode(buffer, 48, &out, &next));
    testrun(out);
    testrun(next == buffer + 48);
    out = dtn_bundle_free(out);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_add_primary_block(){

    dtn_bundle *bundle = dtn_bundle_create();

    uint64_t timestamp = 0;
    uint64_t sequence_number = 0;

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);
    testrun(primary == dtn_cbor_array_get(bundle->data, 0));
    testrun(0x07 == dtn_bundle_primary_get_version(bundle));
    testrun(1 == dtn_bundle_primary_get_flags(bundle));
    testrun(2 == dtn_bundle_primary_get_crc_type(bundle));
    testrun(0 == strcmp("destination", 
        dtn_bundle_primary_get_destination(bundle)));
    testrun(0 == strcmp("source", 
        dtn_bundle_primary_get_source(bundle)));
    testrun(0 == strcmp("report", 
        dtn_bundle_primary_get_report(bundle)));
    testrun(dtn_bundle_primary_get_timestamp(
        bundle, &timestamp, &sequence_number));
    testrun(3 == timestamp);
    testrun(4 == sequence_number);
    testrun(5 == dtn_bundle_primary_get_lifetime(bundle));
    // FRAGMENT and TOTAL are set
    testrun(10 == dtn_cbor_array_count(primary));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_get_version(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(0x07 == dtn_bundle_primary_get_version(bundle));

    dtn_cbor *item = dtn_cbor_array_get(primary, 0);
    testrun(item);
    testrun(dtn_cbor_set_uint(item, 123));
    testrun(123 == dtn_bundle_primary_get_version(bundle));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_set_version(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(0x07 == dtn_bundle_primary_get_version(bundle));

    dtn_cbor *item = dtn_cbor_array_get(primary, 0);
    testrun(item);
    testrun(dtn_cbor_set_uint(item, 123));
    testrun(123 == dtn_bundle_primary_get_version(bundle));
    testrun(dtn_bundle_primary_set_version(bundle));
    testrun(0x07 == dtn_bundle_primary_get_version(bundle));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_get_flags(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(1 == dtn_bundle_primary_get_flags(bundle));

    dtn_cbor *item = dtn_cbor_array_get(primary, 1);
    testrun(item);
    testrun(dtn_cbor_set_uint(item, 123));
    testrun(123 == dtn_bundle_primary_get_flags(bundle));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_set_flags(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(1 == dtn_bundle_primary_get_flags(bundle));

    dtn_cbor *item = dtn_cbor_array_get(primary, 1);
    testrun(item);
    testrun(dtn_cbor_set_uint(item, 123));
    testrun(123 == dtn_bundle_primary_get_flags(bundle));
    testrun(dtn_bundle_primary_set_flags(bundle, 998));
    testrun(998 == dtn_cbor_get_uint(item));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_get_crc_type(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(2 == dtn_bundle_primary_get_crc_type(bundle));

    dtn_cbor *item = dtn_cbor_array_get(primary, 2);
    testrun(item);
    testrun(dtn_cbor_set_uint(item, 123));
    testrun(123 == dtn_bundle_primary_get_crc_type(bundle));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_set_crc_type(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    dtn_cbor *item = dtn_cbor_array_get(primary, 2);
    testrun(item);
    testrun(dtn_cbor_set_uint(item, 123));
    testrun(123 == dtn_bundle_primary_get_crc_type(bundle));
    testrun(dtn_bundle_primary_set_crc_type(bundle, 998));
    testrun(998 == dtn_cbor_get_uint(item));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_get_destination(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(0 == strcmp("destination",
        dtn_bundle_primary_get_destination(bundle)));

    dtn_cbor *item = dtn_cbor_array_get(primary, 3);
    testrun(item);
    testrun(dtn_cbor_set_string(item, "somewhere"));
    testrun(0 == strcmp("somewhere",
        dtn_bundle_primary_get_destination(bundle)));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_set_destination(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(0 == strcmp("destination",
        dtn_bundle_primary_get_destination(bundle)));

    dtn_cbor *item = dtn_cbor_array_get(primary, 3);
    testrun(item);
    testrun(dtn_cbor_set_string(item, "somewhere"));
    testrun(0 == strcmp("somewhere",
        dtn_bundle_primary_get_destination(bundle)));
    testrun(dtn_bundle_primary_set_destination(bundle, "stars"));
    testrun(0 == strcmp("stars",
        dtn_cbor_get_string(item)));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_get_source(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(0 == strcmp("source",
        dtn_bundle_primary_get_source(bundle)));

    dtn_cbor *item = dtn_cbor_array_get(primary, 4);
    testrun(item);
    testrun(dtn_cbor_set_string(item, "somewhere"));
    testrun(0 == strcmp("somewhere",
        dtn_bundle_primary_get_source(bundle)));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_set_source(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(0 == strcmp("source",
        dtn_bundle_primary_get_source(bundle)));

    dtn_cbor *item = dtn_cbor_array_get(primary, 4);
    testrun(item);
    testrun(dtn_cbor_set_string(item, "somewhere"));
    testrun(0 == strcmp("somewhere",
        dtn_bundle_primary_get_source(bundle)));
    testrun(dtn_bundle_primary_set_source(bundle, "stars"));
    testrun(0 == strcmp("stars",
        dtn_cbor_get_string(item)));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_get_report(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(0 == strcmp("report",
        dtn_bundle_primary_get_report(bundle)));

    dtn_cbor *item = dtn_cbor_array_get(primary, 5);
    testrun(item);
    testrun(dtn_cbor_set_string(item, "somewhere"));
    testrun(0 == strcmp("somewhere",
        dtn_bundle_primary_get_report(bundle)));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_set_report(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(0 == strcmp("report",
        dtn_bundle_primary_get_report(bundle)));

    dtn_cbor *item = dtn_cbor_array_get(primary,5);
    testrun(item);
    testrun(dtn_cbor_set_string(item, "somewhere"));
    testrun(0 == strcmp("somewhere",
        dtn_bundle_primary_get_report(bundle)));
    testrun(dtn_bundle_primary_set_report(bundle, "stars"));
    testrun(0 == strcmp("stars",
        dtn_cbor_get_string(item)));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_get_timestamp(){

    dtn_bundle *bundle = dtn_bundle_create();

    uint64_t time = 0;
    uint64_t seq = 0;

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(dtn_bundle_primary_get_timestamp(bundle, &time, &seq));
    testrun(time == 3);
    testrun(seq == 4);

    testrun(dtn_bundle_primary_set_timestamp(bundle, 5, 6));
    testrun(dtn_bundle_primary_get_timestamp(bundle, &time, &seq));
    testrun(time == 5);
    testrun(seq == 6);

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_set_timestamp(){

    dtn_bundle *bundle = dtn_bundle_create();

    uint64_t time = 0;
    uint64_t seq = 0;

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(dtn_bundle_primary_get_timestamp(bundle, &time, &seq));
    testrun(time == 3);
    testrun(seq == 4);

    testrun(dtn_bundle_primary_set_timestamp(bundle, 5, 6));
    testrun(dtn_bundle_primary_get_timestamp(bundle, &time, &seq));
    testrun(time == 5);
    testrun(seq == 6);

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_get_lifetime(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(5 == dtn_bundle_primary_get_lifetime(bundle));

    dtn_cbor *item = dtn_cbor_array_get(primary, 7);
    testrun(item);
    testrun(dtn_cbor_set_uint(item, 123));
    testrun(123 == dtn_bundle_primary_get_lifetime(bundle));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_set_lifetime(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    dtn_cbor *item = dtn_cbor_array_get(primary, 7);
    testrun(item);
    testrun(dtn_cbor_set_uint(item, 123));
    testrun(123 == dtn_bundle_primary_get_lifetime(bundle));
    testrun(dtn_bundle_primary_set_lifetime(bundle, 998));
    testrun(998 == dtn_cbor_get_uint(item));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}
/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_get_fragment_offset(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(0 == dtn_bundle_primary_get_fragment_offset(bundle));
    testrun(dtn_bundle_primary_set_fragment_offset(bundle, 123))
    testrun(123 == dtn_bundle_primary_get_fragment_offset(bundle));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_set_fragment_offset(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    testrun(0 == dtn_bundle_primary_get_fragment_offset(bundle));
    testrun(dtn_bundle_primary_set_fragment_offset(bundle, 123))
    testrun(123 == dtn_bundle_primary_get_fragment_offset(bundle));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_get_total_data_length(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    // MUST set a fragment before otherwise the array order doesnt fit
    testrun(0 == dtn_bundle_primary_get_fragment_offset(bundle));
    testrun(dtn_bundle_primary_set_fragment_offset(bundle, 123))
    testrun(123 == dtn_bundle_primary_get_fragment_offset(bundle));

    testrun(0 == dtn_bundle_primary_get_totel_data_length(bundle));
    testrun(dtn_bundle_primary_set_total_data_length(bundle, 123))
    testrun(123 == dtn_bundle_primary_get_totel_data_length(bundle));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_primary_set_total_data_length(){

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    // MUST set a fragment before otherwise the array order doesnt fit
    testrun(0 == dtn_bundle_primary_get_fragment_offset(bundle));
    testrun(dtn_bundle_primary_set_fragment_offset(bundle, 123))
    testrun(123 == dtn_bundle_primary_get_fragment_offset(bundle));

    testrun(0 == dtn_bundle_primary_get_totel_data_length(bundle));
    testrun(dtn_bundle_primary_set_total_data_length(bundle, 123))
    testrun(123 == dtn_bundle_primary_get_totel_data_length(bundle));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_add_block(){

    // NOTE this will add a block at the end of some bundle. 
    // we preset a primary bundle here for potential further tests. 

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    dtn_cbor *block = dtn_bundle_add_block(bundle, 
        1,
        2,
        3,
        4,
        dtn_cbor_string("testpayload"));

    testrun(block);
    testrun(block == dtn_bundle_get_block(bundle, 2));
    testrun(1 == dtn_bundle_get_code(block));
    testrun(2 == dtn_bundle_get_number(block));
    testrun(3 == dtn_bundle_get_flags(block));
    testrun(4 == dtn_bundle_get_crc_type(block));
    dtn_cbor *data = dtn_bundle_get_data(block);
    testrun(dtn_cbor_is_string(data));
    testrun(0 == strcmp("testpayload", dtn_cbor_get_string(data)));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_get_code(){

    // NOTE this will add a block at the end of some bundle. 
    // we preset a primary bundle here for potential further tests. 

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    dtn_cbor *block = dtn_bundle_add_block(bundle, 
        1,
        2,
        3,
        4,
        dtn_cbor_string("testpayload"));

    testrun(block);
    testrun(1 == dtn_bundle_get_code(block));
    testrun(dtn_bundle_set_code(block, 123));
    testrun(123 == dtn_bundle_get_code(block));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_set_code(){

    // NOTE this will add a block at the end of some bundle. 
    // we preset a primary bundle here for potential further tests. 

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    dtn_cbor *block = dtn_bundle_add_block(bundle, 
        1,
        2,
        3,
        4,
        dtn_cbor_string("testpayload"));

    testrun(block);
    testrun(1 == dtn_bundle_get_code(block));
    testrun(dtn_bundle_set_code(block, 123));
    testrun(123 == dtn_bundle_get_code(block));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_get_number(){

    // NOTE this will add a block at the end of some bundle. 
    // we preset a primary bundle here for potential further tests. 

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    dtn_cbor *block = dtn_bundle_add_block(bundle, 
        1,
        2,
        3,
        4,
        dtn_cbor_string("testpayload"));

    testrun(block);
    testrun(2 == dtn_bundle_get_number(block));
    testrun(dtn_bundle_set_number(block, 123));
    testrun(123 == dtn_bundle_get_number(block));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_set_number(){

    // NOTE this will add a block at the end of some bundle. 
    // we preset a primary bundle here for potential further tests. 

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    dtn_cbor *block = dtn_bundle_add_block(bundle, 
        1,
        2,
        3,
        4,
        dtn_cbor_string("testpayload"));

    testrun(block);
    testrun(2 == dtn_bundle_get_number(block));
    testrun(dtn_bundle_set_number(block, 123));
    testrun(123 == dtn_bundle_get_number(block));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_get_flags(){

    // NOTE this will add a block at the end of some bundle. 
    // we preset a primary bundle here for potential further tests. 

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    dtn_cbor *block = dtn_bundle_add_block(bundle, 
        1,
        2,
        3,
        4,
        dtn_cbor_string("testpayload"));

    testrun(block);
    testrun(3 == dtn_bundle_get_flags(block));
    testrun(dtn_bundle_set_flags(block, 123));
    testrun(123 == dtn_bundle_get_flags(block));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_set_flags(){

    // NOTE this will add a block at the end of some bundle. 
    // we preset a primary bundle here for potential further tests. 

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    dtn_cbor *block = dtn_bundle_add_block(bundle, 
        1,
        2,
        3,
        4,
        dtn_cbor_string("testpayload"));

    testrun(block);
    testrun(3 == dtn_bundle_get_flags(block));
    testrun(dtn_bundle_set_flags(block, 123));
    testrun(123 == dtn_bundle_get_flags(block));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_get_crc_type(){

    // NOTE this will add a block at the end of some bundle. 
    // we preset a primary bundle here for potential further tests. 

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    dtn_cbor *block = dtn_bundle_add_block(bundle, 
        1,
        2,
        3,
        4,
        dtn_cbor_string("testpayload"));

    testrun(block);
    testrun(4 == dtn_bundle_get_crc_type(block));
    testrun(dtn_bundle_set_crc_type(block, 123));
    testrun(123 == dtn_bundle_get_crc_type(block));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_set_crc_type(){

    // NOTE this will add a block at the end of some bundle. 
    // we preset a primary bundle here for potential further tests. 

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    dtn_cbor *block = dtn_bundle_add_block(bundle, 
        1,
        2,
        3,
        4,
        dtn_cbor_string("testpayload"));

    testrun(block);
    testrun(4 == dtn_bundle_get_crc_type(block));
    testrun(dtn_bundle_set_crc_type(block, 123));
    testrun(123 == dtn_bundle_get_crc_type(block));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_get_data(){

    // NOTE this will add a block at the end of some bundle. 
    // we preset a primary bundle here for potential further tests. 

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    dtn_cbor *block = dtn_bundle_add_block(bundle, 
        1,
        2,
        3,
        4,
        dtn_cbor_string("testpayload"));

    testrun(block);
    dtn_cbor *data = dtn_bundle_get_data(block);
    testrun(0 == strcmp("testpayload", dtn_cbor_get_string(data)));
    testrun(dtn_bundle_set_data(block, dtn_cbor_string("something")));
    testrun(0 == strcmp("something", dtn_cbor_get_string(
        dtn_bundle_get_data(block))));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_set_data(){

    // NOTE this will add a block at the end of some bundle. 
    // we preset a primary bundle here for potential further tests. 

    dtn_bundle *bundle = dtn_bundle_create();

    dtn_cbor *primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);

    testrun(primary);

    dtn_cbor *block = dtn_bundle_add_block(bundle, 
        1,
        2,
        3,
        4,
        dtn_cbor_string("testpayload"));

    testrun(block);
    dtn_cbor *data = dtn_bundle_get_data(block);
    testrun(0 == strcmp("testpayload", dtn_cbor_get_string(data)));
    testrun(dtn_bundle_set_data(block, dtn_cbor_string("something")));
    testrun(0 == strcmp("something", dtn_cbor_get_string(
        dtn_bundle_get_data(block))));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_set_crc_primary(){

    // NOTE this will add a block at the end of some bundle. 
    // we preset a primary bundle here for potential further tests. 

    dtn_bundle *bundle = dtn_bundle_create();

    // crc_type 0 8 block bundle
    dtn_cbor *block = dtn_bundle_add_primary_block(
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

    testrun(set_crc_primary(block));
    testrun(8 == dtn_cbor_array_count(block));

    // crc_type 1 8 block bundle
    testrun(dtn_bundle_primary_set_crc_type(bundle, 1));
    testrun(set_crc_primary(block));
    testrun(9 == dtn_cbor_array_count(block));
    testrun(check_primary_block(bundle));
    bundle = dtn_bundle_free(bundle);

    // crc_type 2 8 block
    bundle = dtn_bundle_create();
    block = dtn_bundle_add_primary_block(
        bundle,
        0,
        1,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        0,
        0);
    testrun(set_crc_primary(block));
    testrun(9 == dtn_cbor_array_count(block));
    testrun(check_primary_block(bundle));
    bundle = dtn_bundle_free(bundle);

    // crc_type 2 10 block
    bundle = dtn_bundle_create();
    block = dtn_bundle_add_primary_block(
        bundle,
        1,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        123,
        456);
    testrun(set_crc_primary(block));
    testrun(11 == dtn_cbor_array_count(block));
    testrun(check_primary_block(bundle));
    bundle = dtn_bundle_free(bundle);

    // crc_type 1 10 block
    bundle = dtn_bundle_create();
    block = dtn_bundle_add_primary_block(
        bundle,
        1,
        1,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        123,
        456);
    testrun(set_crc_primary(block));
    testrun(11 == dtn_cbor_array_count(block));
    testrun(check_primary_block(bundle));
    bundle = dtn_bundle_free(bundle);

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_set_crc_block(){

    // NOTE we use the primary block based tests here to be able to
    // use the canonical checks. 

    dtn_bundle *bundle = dtn_bundle_create();

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

    testrun(primary);

    dtn_cbor *block = dtn_bundle_add_block(
        bundle,
        0,
        0,
        0,
        0,
        dtn_cbor_string("test"));
    testrun(block);

    testrun(set_crc_block(block));
    testrun(5 == dtn_cbor_array_count(block));
    testrun(check_canonical_blocks(bundle));

    dtn_bundle_set_crc_type(block, 1);
    testrun(set_crc_block(block));
    testrun(6 == dtn_cbor_array_count(block));
    testrun(check_canonical_blocks(bundle));

    bundle = dtn_bundle_free(bundle);
    bundle = dtn_bundle_create();

    primary = dtn_bundle_add_primary_block(
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

    testrun(primary);

    block = dtn_bundle_add_block(
        bundle,
        0,
        0,
        2,
        0,
        dtn_cbor_string("test"));
    testrun(block);

    dtn_bundle_set_crc_type(block, 2);
    testrun(set_crc_block(block));
    testrun(6 == dtn_cbor_array_count(block));
    testrun(check_canonical_blocks(bundle));

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_encode(){

    uint8_t buffer[0xffff] = {0};
    uint8_t *next = NULL;
    size_t size = 0xffff;

    dtn_cbor *block = 0;
    dtn_bundle *bundle = dtn_bundle_create();

    // check min valid
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
        0,
        dtn_cbor_string("test"));

    testrun(primary);
    testrun(payload);

    testrun(dtn_bundle_encode(bundle, buffer, size, &next));

    testrun(buffer[0]  == 0x9F);
    testrun(buffer[1]  == 0x88);
    testrun(buffer[2]  == 0x07);
    testrun(buffer[3]  == 0x00);
    testrun(buffer[4]  == 0x00);
    testrun(buffer[5]  == 0x4b);
    testrun(buffer[6]  == 'd');
    testrun(buffer[7]  == 'e');
    testrun(buffer[8]  == 's');
    testrun(buffer[9]  == 't');
    testrun(buffer[10] == 'i');
    testrun(buffer[11] == 'n');
    testrun(buffer[12] == 'a');
    testrun(buffer[13] == 't');
    testrun(buffer[14] == 'i');
    testrun(buffer[15] == 'o');
    testrun(buffer[16] == 'n');
    testrun(buffer[17] == 0x46);
    testrun(buffer[18] == 's');
    testrun(buffer[19] == 'o');
    testrun(buffer[20] == 'u');
    testrun(buffer[21] == 'r');
    testrun(buffer[22] == 'c');
    testrun(buffer[23] == 'e');
    testrun(buffer[24] == 0x46);
    testrun(buffer[25] == 'r');
    testrun(buffer[26] == 'e');
    testrun(buffer[27] == 'p');
    testrun(buffer[28] == 'o');
    testrun(buffer[29] == 'r');
    testrun(buffer[30] == 't');
    testrun(buffer[31] == 0x82);
    testrun(buffer[32] == 0x03);
    testrun(buffer[33] == 0x04);
    testrun(buffer[34] == 0x05);
    testrun(buffer[35] == 0x85);
    testrun(buffer[36] == 0x01);
    testrun(buffer[37] == 0x01);
    testrun(buffer[38] == 0x00);
    testrun(buffer[39] == 0x00);
    testrun(buffer[40] == 0x44);
    testrun(buffer[41] == 't');
    testrun(buffer[42] == 'e');
    testrun(buffer[43] == 's');
    testrun(buffer[44] == 't');
    testrun(buffer[45] == 0xFF);
    testrun(buffer[46] == 0x00);
    testrun(buffer[47] == 0x00);
    testrun(buffer[48] == 0x00);
    testrun(buffer[49] == 0x00);
    testrun(buffer[50] == 0x00);

    bundle = dtn_bundle_free(bundle);
    bundle = dtn_bundle_create();

    // check max valid with some additional blocks
    primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        1,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        100,
        200);

    testrun(primary);

    block = dtn_bundle_add_block(
        bundle,
        20,
        20,
        0,
        1,
        dtn_cbor_string("one"));

    testrun(block);

    block = dtn_bundle_add_block(
        bundle,
        12,
        12,
        0,
        2,
        dtn_cbor_string("two"));

    testrun(block);

    payload = dtn_bundle_add_block(
        bundle,
        1,
        1,
        0,
        2,
        dtn_cbor_string("payload"));
   
    testrun(payload);

    testrun(dtn_bundle_encode(bundle, buffer, size, &next));

    testrun(buffer[0]  == 0x9F);
    testrun(buffer[1]  == 0x8B);
    testrun(buffer[2]  == 0x07);
    testrun(buffer[3]  == 0x01);
    testrun(buffer[4]  == 0x01);
    testrun(buffer[5]  == 0x4b);
    testrun(buffer[6]  == 'd');
    testrun(buffer[7]  == 'e');
    testrun(buffer[8]  == 's');
    testrun(buffer[9]  == 't');
    testrun(buffer[10] == 'i');
    testrun(buffer[11] == 'n');
    testrun(buffer[12] == 'a');
    testrun(buffer[13] == 't');
    testrun(buffer[14] == 'i');
    testrun(buffer[15] == 'o');
    testrun(buffer[16] == 'n');
    testrun(buffer[17] == 0x46);
    testrun(buffer[18] == 's');
    testrun(buffer[19] == 'o');
    testrun(buffer[20] == 'u');
    testrun(buffer[21] == 'r');
    testrun(buffer[22] == 'c');
    testrun(buffer[23] == 'e');
    testrun(buffer[24] == 0x46);
    testrun(buffer[25] == 'r');
    testrun(buffer[26] == 'e');
    testrun(buffer[27] == 'p');
    testrun(buffer[28] == 'o');
    testrun(buffer[29] == 'r');
    testrun(buffer[30] == 't');
    testrun(buffer[31] == 0x82);
    testrun(buffer[32] == 0x03);
    testrun(buffer[33] == 0x04);
    testrun(buffer[34] == 0x05);
    testrun(buffer[35] == 0x18);
    testrun(buffer[36] == 100);
    testrun(buffer[37] == 0x18);
    testrun(buffer[38] == 200);
    testrun(buffer[39] == 0x42); // CRC16
    testrun(buffer[40] == 0x14);
    testrun(buffer[41] == 0x21);
    testrun(buffer[42] == 0x86); // block 2
    testrun(buffer[43] == 0x14);
    testrun(buffer[44] == 0x14);
    testrun(buffer[45] == 0x00);
    testrun(buffer[46] == 0x01);
    testrun(buffer[47] == 0x43);
    testrun(buffer[48] == 'o');
    testrun(buffer[49] == 'n');
    testrun(buffer[50] == 'e');
    testrun(buffer[51] == 0x42); // CRC16
    testrun(buffer[52] == 0x54);
    testrun(buffer[53] == 0x94);
    testrun(buffer[54] == 0x86); // block 3
    testrun(buffer[55] == 0x0c);
    testrun(buffer[56] == 0x0C);
    testrun(buffer[57] == 0x00);
    testrun(buffer[58] == 0x02);
    testrun(buffer[59] == 0x43);
    testrun(buffer[60] == 't');
    testrun(buffer[61] == 'w');
    testrun(buffer[62] == 'o');
    testrun(buffer[63] == 0x44); // crc32
    testrun(buffer[64] == 0x8F);
    testrun(buffer[65] == 0x24);
    testrun(buffer[66] == 0x68);
    testrun(buffer[67] == 0x6d);
    testrun(buffer[68] == 0x86); // block 4 payload
    testrun(buffer[69] == 0x01);
    testrun(buffer[70] == 0x01);
    testrun(buffer[71] == 0x00);
    testrun(buffer[72] == 0x02);
    testrun(buffer[73] == 0x47);
    testrun(buffer[74] == 'p');
    testrun(buffer[75] == 'a');
    testrun(buffer[76] == 'y');
    testrun(buffer[77] == 'l');
    testrun(buffer[78] == 'o');
    testrun(buffer[79] == 'a');
    testrun(buffer[80] == 'd');
    testrun(buffer[81] == 0x44); // crc32
    testrun(buffer[82] == 0xdc);
    testrun(buffer[83] == 0xb4);
    testrun(buffer[84] == 0x9b);
    testrun(buffer[85] == 0xa2);
    testrun(buffer[86] == 0xff);
    testrun(buffer[87] == 0x00);
    testrun(buffer[88] == 0x00);
    testrun(buffer[89] == 0x00);
    testrun(buffer[90] == 0x00);
    testrun(next == buffer + 87);

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_clear(){

    dtn_cbor *block = NULL;
    dtn_bundle *bundle = dtn_bundle_create();

    testrun(dtn_bundle_clear(bundle));
    testrun(bundle->data == NULL);

    // check min valid
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
        0,
        dtn_cbor_string("test"));

    testrun(primary);
    testrun(payload);

    testrun(dtn_bundle_clear(bundle));
    testrun(bundle->data == NULL);

    // check max valid with some additional blocks
    primary = dtn_bundle_add_primary_block(
        bundle,
        1,
        1,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        100,
        200);

    testrun(primary);

    block = dtn_bundle_add_block(
        bundle,
        20,
        20,
        0,
        1,
        dtn_cbor_string("one"));

    testrun(block);

    block = dtn_bundle_add_block(
        bundle,
        12,
        12,
        0,
        2,
        dtn_cbor_string("two"));

    testrun(block);

    payload = dtn_bundle_add_block(
        bundle,
        1,
        1,
        0,
        2,
        dtn_cbor_string("payload"));
   
    testrun(payload);

    testrun(dtn_bundle_clear(bundle));
    testrun(bundle->data == NULL);

    bundle = dtn_bundle_free(bundle);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_free_void(){

    dtn_bundle *bundle = dtn_bundle_create();

    bundle = dtn_bundle_free_void(bundle);
    testrun(!bundle);

    bundle = dtn_bundle_create();

    // check min valid
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
        0,
        dtn_cbor_string("test"));

    testrun(primary);
    testrun(payload);

    bundle = dtn_bundle_free_void(bundle);
    testrun(!bundle);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_dump(){

    dtn_bundle *bundle = dtn_bundle_create();

    bundle = dtn_bundle_free_void(bundle);
    testrun(!bundle);

    bundle = dtn_bundle_create();

    // check min valid
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

    testrun(!dtn_bundle_verify(bundle));
    testrun(!dtn_bundle_dump(stdout, bundle));

    dtn_cbor *payload = dtn_bundle_add_block(
        bundle,
        1,
        1,
        0,
        0,
        dtn_cbor_string("test"));

    testrun(primary);
    testrun(payload);

    testrun(dtn_bundle_verify(bundle));
    testrun(dtn_bundle_dump(stdout, bundle));

    bundle = dtn_bundle_free_void(bundle);
    testrun(!bundle);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_bundle_copy(){

    dtn_bundle *bundle = dtn_bundle_create();
    dtn_bundle *copy = NULL;

    testrun(dtn_bundle_copy((void**)&copy, bundle));
    copy = dtn_bundle_free(copy);

    // check min valid
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

    testrun(dtn_bundle_copy((void**)&copy, bundle));
    copy = dtn_bundle_free(copy);

    dtn_cbor *payload = dtn_bundle_add_block(
        bundle,
        1,
        1,
        0,
        0,
        dtn_cbor_string("test"));

    testrun(primary);
    testrun(payload);

    testrun(dtn_bundle_copy((void**)&copy, bundle));
    copy = dtn_bundle_free(copy);

    bundle = dtn_bundle_free_void(bundle);
    testrun(!bundle);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_check_primary_bytes(){

    uint8_t buffer[100] = {0};
    // min valid 
    buffer[0]  = 0x9f;
    buffer[1]  = 0x88;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x00; // flags
    buffer[4]  = 0x00; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime

    testrun(check_primary_bytes(buffer + 1 , 26));

    // full valid without crc 
    buffer[0]  = 0x9f;
    buffer[1]  = 0x8B;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x00; // flags
    buffer[4]  = 0x00; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x18;
    buffer[28] = 100;
    buffer[29] = 0x18;
    buffer[30] = 200;

    testrun(check_primary_bytes(buffer + 1 , 30));

    // app data not int
    buffer[0]  = 0x9f;
    buffer[1]  = 0x8B;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x01; // flags
    buffer[4]  = 0x00; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x18;
    buffer[28] = 100;
    buffer[29] = 0x41;
    buffer[30] = 200;

    testrun(!check_primary_bytes(buffer + 1 , 30));

    // fragment not int
    buffer[0]  = 0x9f;
    buffer[1]  = 0x8B;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x01; // flags
    buffer[4]  = 0x00; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x8B;
    buffer[28] = 100;
    buffer[29] = 0x18;
    buffer[30] = 200;

    // destination not string
    buffer[0]  = 0x9f;
    buffer[1]  = 0x8B;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x01; // flags
    buffer[4]  = 0x00; // crc_type
    buffer[5]  = 0x18; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x18;
    buffer[28] = 100;
    buffer[29] = 0x18;
    buffer[30] = 200;

    testrun(!check_primary_bytes(buffer + 1 , 30));

    // source not string
    buffer[0]  = 0x9f;
    buffer[1]  = 0x8B;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x01; // flags
    buffer[4]  = 0x00; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x18; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x18;
    buffer[28] = 100;
    buffer[29] = 0x18;
    buffer[30] = 200;

    testrun(!check_primary_bytes(buffer + 1 , 30));

    // report not string
    buffer[0]  = 0x9f;
    buffer[1]  = 0x8B;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x01; // flags
    buffer[4]  = 0x00; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x18; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x82; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x18;
    buffer[28] = 100;
    buffer[29] = 0x18;
    buffer[30] = 200;

    testrun(!check_primary_bytes(buffer + 1 , 30));

    // timestamp not array
    buffer[0]  = 0x9f;
    buffer[1]  = 0x8B;
    buffer[2]  = 0x07; // version
    buffer[3]  = 0x01; // flags
    buffer[4]  = 0x00; // crc_type
    buffer[5]  = 0x45; // destination
    buffer[6]  = 'd';
    buffer[7]  = 't';
    buffer[8]  = 'n';
    buffer[9]  = ':';
    buffer[10] = '1';
    buffer[11] = 0x45; // source
    buffer[12] = 'd';
    buffer[13] = 't';
    buffer[14] = 'n';
    buffer[15] = ':';
    buffer[16] = '2';
    buffer[17] = 0x45; // report
    buffer[18] = 'd';
    buffer[19] = 't';
    buffer[20] = 'n';
    buffer[21] = ':';
    buffer[22] = '3';
    buffer[23] = 0x18; // timestamp
    buffer[24] = 0x01;          // time
    buffer[25] = 0x02;          // sequence number
    buffer[26] = 0x03; // lifetime
    buffer[27] = 0x18;
    buffer[28] = 100;
    buffer[29] = 0x18;
    buffer[30] = 200;

    testrun(!check_primary_bytes(buffer + 1 , 30));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_crc_generation(){

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
        dtn_cbor_string("test12345678901234567890"));

    testrun(payload);
    testrun(primary);

    uint8_t *next = NULL;
    dtn_bundle *out = NULL;

    uint8_t buffer[2048] = {0};
    size_t size = 2048;

    testrun(dtn_bundle_encode(bundle, buffer, size, &next));

    out = NULL;
    testrun(DTN_CBOR_MATCH_FULL == dtn_bundle_decode(
        buffer, next - buffer, &out, &next));
    testrun(out);

    // check we can encode (set crc) more than once
    testrun(dtn_bundle_encode(bundle, buffer, size, &next));
    testrun(dtn_bundle_encode(bundle, buffer, size, &next));

    out = dtn_bundle_free(out);
    bundle = dtn_bundle_free(out);

    bundle = dtn_bundle_create();
    testrun(bundle);

    primary = dtn_bundle_add_primary_block(
        bundle,
        0,
        2,
        "destination",
        "source",
        "report",
        3,
        4,
        5,
        100,
        200);

    payload = dtn_bundle_add_block(
        bundle,
        1,
        1,
        0,
        2,
        dtn_cbor_string("test12345678901234567890"));

    testrun(payload);
    testrun(primary);

    testrun(dtn_bundle_encode(bundle, buffer, size, &next));

    out = NULL;
    testrun(DTN_CBOR_MATCH_FULL == dtn_bundle_decode(
        buffer, next - buffer, &out, &next));
    testrun(out);

     // check we can encode (set crc) more than once
    testrun(dtn_bundle_encode(bundle, buffer, size, &next));
    testrun(dtn_bundle_encode(bundle, buffer, size, &next));

    out = dtn_bundle_free(out);
    bundle = dtn_bundle_free(out);

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
    testrun_test(test_check_primary_bytes);
    testrun_test(test_dtn_bundle_create);
    testrun_test(test_dtn_bundle_free);
    testrun_test(test_check_primary_block);
    testrun_test(test_check_blocks);
    testrun_test(test_check_payload_block);
    testrun_test(test_dtn_bundle_verify);
    testrun_test(test_dtn_bundle_decode);
    testrun_test(test_dtn_bundle_add_primary_block);
    testrun_test(test_dtn_bundle_primary_get_version);
    testrun_test(test_dtn_bundle_primary_set_version);
    testrun_test(test_dtn_bundle_primary_get_flags);
    testrun_test(test_dtn_bundle_primary_set_flags);
    testrun_test(test_dtn_bundle_primary_get_crc_type);
    testrun_test(test_dtn_bundle_primary_set_crc_type);
    testrun_test(test_dtn_bundle_primary_get_destination);
    testrun_test(test_dtn_bundle_primary_set_destination);
    testrun_test(test_dtn_bundle_primary_get_source);
    testrun_test(test_dtn_bundle_primary_set_source);
    testrun_test(test_dtn_bundle_primary_get_report);
    testrun_test(test_dtn_bundle_primary_set_report);
    testrun_test(test_dtn_bundle_primary_get_timestamp);
    testrun_test(test_dtn_bundle_primary_set_timestamp);
    testrun_test(test_dtn_bundle_primary_get_lifetime);
    testrun_test(test_dtn_bundle_primary_set_lifetime);
    testrun_test(test_dtn_bundle_primary_get_fragment_offset);
    testrun_test(test_dtn_bundle_primary_set_fragment_offset);
    testrun_test(test_dtn_bundle_primary_get_total_data_length);
    testrun_test(test_dtn_bundle_primary_set_total_data_length);
    testrun_test(test_dtn_bundle_add_block);
    testrun_test(test_dtn_bundle_get_code);
    testrun_test(test_dtn_bundle_set_code);
    testrun_test(test_dtn_bundle_get_number);
    testrun_test(test_dtn_bundle_set_number);
    testrun_test(test_dtn_bundle_get_flags);
    testrun_test(test_dtn_bundle_set_flags);
    testrun_test(test_dtn_bundle_get_crc_type);
    testrun_test(test_dtn_bundle_set_crc_type);
    testrun_test(test_dtn_bundle_get_data);
    testrun_test(test_dtn_bundle_set_data);
    testrun_test(check_set_crc_primary);
    testrun_test(check_set_crc_block);
    testrun_test(test_dtn_bundle_encode);
    testrun_test(test_dtn_bundle_clear);
    testrun_test(test_dtn_bundle_free_void);
    testrun_test(test_dtn_bundle_dump);
    testrun_test(test_dtn_bundle_copy);
    testrun_test(check_crc_generation);
    

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
