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
        @file           dtn_aes_key_wrap_test.c
        @author         Töpfer, Markus

        @date           2025-12-30


        ------------------------------------------------------------------------
*/
#include <dtn_base/testrun.h>
#include "dtn_aes_key_wrap.c"

#include <dtn_base/dtn_dump.h>

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_aes_key_wrap(){
    
    uint8_t key[256] = {0};
    key[0]  = 0x00;
    key[1]  = 0x01;
    key[2]  = 0x02;
    key[3]  = 0x03;
    key[4]  = 0x04;
    key[5]  = 0x05;
    key[6]  = 0x06;
    key[7]  = 0x07;
    key[8]  = 0x08;
    key[9]  = 0x09;
    key[10] = 0x0A;
    key[11] = 0x0B;
    key[12] = 0x0C;
    key[13] = 0x0D;
    key[14] = 0x0E;
    key[15] = 0x0F;

    uint8_t data[256] = {0};

    data[0]  = 0x00;
    data[1]  = 0x11;
    data[2]  = 0x22;
    data[3]  = 0x33;
    data[4]  = 0x44;
    data[5]  = 0x55;
    data[6]  = 0x66;
    data[7]  = 0x77;
    data[8]  = 0x88;
    data[9]  = 0x99;
    data[10] = 0xAA;
    data[11] = 0xBB;
    data[12] = 0xCC;
    data[13] = 0xDD;
    data[14] = 0xEE;
    data[15] = 0xFF;

    uint8_t expect[256] = {0};
    expect[0]  = 0x1F;
    expect[1]  = 0xA6;
    expect[2]  = 0x8B;
    expect[3]  = 0x0A;
    expect[4]  = 0x81;
    expect[5]  = 0x12;
    expect[6]  = 0xB4;
    expect[7]  = 0x47;
    expect[8]  = 0xAE;
    expect[9]  = 0xF3;
    expect[10] = 0x4B;
    expect[11] = 0xD8;
    expect[12] = 0xFB;
    expect[13] = 0x5A;
    expect[14] = 0x7B;
    expect[15] = 0x82;
    expect[16] = 0x9D;
    expect[17] = 0x3E;
    expect[18] = 0x86;
    expect[19] = 0x23;
    expect[20] = 0x71;
    expect[21] = 0xD2;
    expect[22] = 0xCF;
    expect[23] = 0xE5;
    expect[24] = 0x00;
    expect[25] = 0x00;
    expect[26] = 0x00;
    expect[27] = 0x00;

    uint8_t buffer[1024] = { 0 };
    size_t len = 1024;

    testrun(dtn_aes_key_wrap(
        data,
        16,
        buffer,
        &len,
        key,
        16));

    testrun(len == 24);
    //dtn_dump_binary_as_hex(stdout, buffer, 24);

    testrun(0 == memcmp(buffer, expect, len));

    memset(key, 0, 256);
    key[0]  = 0x00;
    key[1]  = 0x01;
    key[2]  = 0x02;
    key[3]  = 0x03;
    key[4]  = 0x04;
    key[5]  = 0x05;
    key[6]  = 0x06;
    key[7]  = 0x07;
    key[8]  = 0x08;
    key[9]  = 0x09;
    key[10] = 0x0A;
    key[11] = 0x0B;
    key[12] = 0x0C;
    key[13] = 0x0D;
    key[14] = 0x0E;
    key[15] = 0x0F;
    key[16] = 0x10;
    key[17] = 0x11;
    key[18] = 0x12;
    key[19] = 0x13;
    key[20] = 0x14;
    key[21] = 0x15;
    key[22] = 0x16;
    key[23] = 0x17;
    key[24] = 0x00;

    memset(buffer, 0, 1024);

    expect[0]  = 0x96;
    expect[1]  = 0x77;
    expect[2]  = 0x8B;
    expect[3]  = 0x25;
    expect[4]  = 0xAE;
    expect[5]  = 0x6C;
    expect[6]  = 0xa4;
    expect[7]  = 0x35;
    expect[8]  = 0xF9;
    expect[9]  = 0x2B;
    expect[10] = 0x5b;
    expect[11] = 0x97;
    expect[12] = 0xc0;
    expect[13] = 0x50;
    expect[14] = 0xAE;
    expect[15] = 0xD2;
    expect[16] = 0x46;
    expect[17] = 0x8A;
    expect[18] = 0xB8;
    expect[19] = 0xA1;
    expect[20] = 0x7A;
    expect[21] = 0xD8;
    expect[22] = 0x4E;
    expect[23] = 0x5D;
    expect[24] = 0x00;
    expect[25] = 0x00;
    expect[26] = 0x00;
    expect[27] = 0x00;

    len = 1024;
    testrun(dtn_aes_key_wrap(
        data,
        16,
        buffer,
        &len,
        key,
        24));

    //dtn_dump_binary_as_hex(stdout, buffer, 24);
    testrun(len == 24)
    testrun(0 == memcmp(buffer, expect, len));

    memset(key, 0, 256);
    key[0]  = 0x00;
    key[1]  = 0x01;
    key[2]  = 0x02;
    key[3]  = 0x03;
    key[4]  = 0x04;
    key[5]  = 0x05;
    key[6]  = 0x06;
    key[7]  = 0x07;
    key[8]  = 0x08;
    key[9]  = 0x09;
    key[10] = 0x0A;
    key[11] = 0x0B;
    key[12] = 0x0C;
    key[13] = 0x0D;
    key[14] = 0x0E;
    key[15] = 0x0F;
    key[16] = 0x10;
    key[17] = 0x11;
    key[18] = 0x12;
    key[19] = 0x13;
    key[20] = 0x14;
    key[21] = 0x15;
    key[22] = 0x16;
    key[23] = 0x17;
    key[24] = 0x18;
    key[25] = 0x19;
    key[26] = 0x1A;
    key[27] = 0x1B;
    key[28] = 0x1C;
    key[29] = 0x1D;
    key[30] = 0x1E;
    key[31] = 0x1F;
    key[32] = 0x00;

    memset(buffer, 0, 1024);

    expect[0]  = 0x64;
    expect[1]  = 0xE8;
    expect[2]  = 0xc3;
    expect[3]  = 0xf9;
    expect[4]  = 0xce;
    expect[5]  = 0x0f;
    expect[6]  = 0x5b;
    expect[7]  = 0xa2;
    expect[8]  = 0x63;
    expect[9]  = 0xe9;
    expect[10] = 0x77;
    expect[11] = 0x79;
    expect[12] = 0x05;
    expect[13] = 0x81;
    expect[14] = 0x8a;
    expect[15] = 0x2a;
    expect[16] = 0x93;
    expect[17] = 0xc8;
    expect[18] = 0x19;
    expect[19] = 0x1e;
    expect[20] = 0x7d;
    expect[21] = 0x6e;
    expect[22] = 0x8a;
    expect[23] = 0xe7;
    expect[24] = 0x00;
    expect[25] = 0x00;
    expect[26] = 0x00;
    expect[27] = 0x00;

    len = 1024;
    testrun(dtn_aes_key_wrap(
        data,
        16,
        buffer,
        &len,
        key,
        32));

    //dtn_dump_binary_as_hex(stdout, buffer, 24);
    testrun(len == 24);
    testrun(0 == memcmp(buffer, expect, 24));

    memset(key, 0, 256);
    key[0]  = 0x00;
    key[1]  = 0x01;
    key[2]  = 0x02;
    key[3]  = 0x03;
    key[4]  = 0x04;
    key[5]  = 0x05;
    key[6]  = 0x06;
    key[7]  = 0x07;
    key[8]  = 0x08;
    key[9]  = 0x09;
    key[10] = 0x0A;
    key[11] = 0x0B;
    key[12] = 0x0C;
    key[13] = 0x0D;
    key[14] = 0x0E;
    key[15] = 0x0F;
    key[16] = 0x10;
    key[17] = 0x11;
    key[18] = 0x12;
    key[19] = 0x13;
    key[20] = 0x14;
    key[21] = 0x15;
    key[22] = 0x16;
    key[23] = 0x17;

    memset(data, 0, 256);

    data[0]  = 0x00;
    data[1]  = 0x11;
    data[2]  = 0x22;
    data[3]  = 0x33;
    data[4]  = 0x44;
    data[5]  = 0x55;
    data[6]  = 0x66;
    data[7]  = 0x77;
    data[8]  = 0x88;
    data[9]  = 0x99;
    data[10] = 0xAA;
    data[11] = 0xBB;
    data[12] = 0xCC;
    data[13] = 0xDD;
    data[14] = 0xEE;
    data[15] = 0xFF;
    data[16] = 0x00;
    data[17] = 0x01;
    data[18] = 0x02;
    data[19] = 0x03;
    data[20] = 0x04;
    data[21] = 0x05;
    data[22] = 0x06;
    data[23] = 0x07;
    data[24] = 0x00;

    memset(buffer, 0, 1024);

    expect[0]  = 0x03;
    expect[1]  = 0x1D;
    expect[2]  = 0x33;
    expect[3]  = 0x26;
    expect[4]  = 0x4E;
    expect[5]  = 0x15;
    expect[6]  = 0xd3;
    expect[7]  = 0x32;
    expect[8]  = 0x68;
    expect[9]  = 0xf2;
    expect[10] = 0x4E;
    expect[11] = 0xc2;
    expect[12] = 0x60;
    expect[13] = 0x74;
    expect[14] = 0x3e;
    expect[15] = 0xdc;
    expect[16] = 0xe1;
    expect[17] = 0xc6;
    expect[18] = 0xc7;
    expect[19] = 0xdd;
    expect[20] = 0xee;
    expect[21] = 0x72;
    expect[22] = 0x5a;
    expect[23] = 0x93;
    expect[24] = 0x6b;
    expect[25] = 0xa8;
    expect[26] = 0x14;
    expect[27] = 0x91;
    expect[28] = 0x5c;
    expect[29] = 0x67;
    expect[30] = 0x62;
    expect[31] = 0xd2;
    expect[32] = 0x00;
    expect[33] = 0x00;
    expect[34] = 0x00;

    len = 1024;
    testrun(dtn_aes_key_wrap(
        data,
        24,
        buffer,
        &len,
        key,
        24));

    //dtn_dump_binary_as_hex(stdout, buffer, 24);

    testrun(len == 32);
    testrun(0 == memcmp(buffer, expect, 32));

    memset(key, 0, 256);
    key[0]  = 0x00;
    key[1]  = 0x01;
    key[2]  = 0x02;
    key[3]  = 0x03;
    key[4]  = 0x04;
    key[5]  = 0x05;
    key[6]  = 0x06;
    key[7]  = 0x07;
    key[8]  = 0x08;
    key[9]  = 0x09;
    key[10] = 0x0A;
    key[11] = 0x0B;
    key[12] = 0x0C;
    key[13] = 0x0D;
    key[14] = 0x0E;
    key[15] = 0x0F;
    key[16] = 0x10;
    key[17] = 0x11;
    key[18] = 0x12;
    key[19] = 0x13;
    key[20] = 0x14;
    key[21] = 0x15;
    key[22] = 0x16;
    key[23] = 0x17;
    key[24] = 0x18;
    key[25] = 0x19;
    key[26] = 0x1A;
    key[27] = 0x1B;
    key[28] = 0x1C;
    key[29] = 0x1D;
    key[30] = 0x1E;
    key[31] = 0x1F;
    key[32] = 0x00;

    memset(buffer, 0, 1024);

    expect[0]  = 0xA8;
    expect[1]  = 0xf9;
    expect[2]  = 0xbc;
    expect[3]  = 0x16;
    expect[4]  = 0x12;
    expect[5]  = 0xc6;
    expect[6]  = 0x8b;
    expect[7]  = 0x3f;
    expect[8]  = 0xf6;
    expect[9]  = 0xe6;
    expect[10] = 0xf4;
    expect[11] = 0xfb;
    expect[12] = 0xe3;
    expect[13] = 0x0e;
    expect[14] = 0x71;
    expect[15] = 0xe4;
    expect[16] = 0x76;
    expect[17] = 0x9c;
    expect[18] = 0x8b;
    expect[19] = 0x80;
    expect[20] = 0xa3;
    expect[21] = 0x2c;
    expect[22] = 0xb8;
    expect[23] = 0x95;
    expect[24] = 0x8c;
    expect[25] = 0xd5;
    expect[26] = 0xd1;
    expect[27] = 0x7d;
    expect[28] = 0x6b;
    expect[29] = 0x25;
    expect[30] = 0x4d;
    expect[31] = 0xa1;
    expect[32] = 0x00;
    expect[33] = 0x00;
    expect[34] = 0x00;

    len = 1024;
    testrun(dtn_aes_key_wrap(
        data,
        24,
        buffer,
        &len,
        key,
        32));

    testrun(len == 32);
    testrun(0 == memcmp(buffer, expect, 32));

    memset(data, 0, 256);

    data[0]  = 0x00;
    data[1]  = 0x11;
    data[2]  = 0x22;
    data[3]  = 0x33;
    data[4]  = 0x44;
    data[5]  = 0x55;
    data[6]  = 0x66;
    data[7]  = 0x77;
    data[8]  = 0x88;
    data[9]  = 0x99;
    data[10] = 0xAA;
    data[11] = 0xBB;
    data[12] = 0xCC;
    data[13] = 0xDD;
    data[14] = 0xEE;
    data[15] = 0xFF;
    data[16] = 0x00;
    data[17] = 0x01;
    data[18] = 0x02;
    data[19] = 0x03;
    data[20] = 0x04;
    data[21] = 0x05;
    data[22] = 0x06;
    data[23] = 0x07;
    data[24] = 0x08;
    data[25] = 0x09;
    data[26] = 0x0A;
    data[27] = 0x0B;
    data[28] = 0x0C;
    data[29] = 0x0D;
    data[30] = 0x0E;
    data[31] = 0x0F;
    data[32] = 0x00;

    memset(buffer, 0, 1024);

    expect[0]  = 0x28;
    expect[1]  = 0xc9;
    expect[2]  = 0xf4;
    expect[3]  = 0x04;
    expect[4]  = 0xc4;
    expect[5]  = 0xb8;
    expect[6]  = 0x10;
    expect[7]  = 0xf4;
    expect[8]  = 0xcb;
    expect[9]  = 0xcc;
    expect[10] = 0xb3;
    expect[11] = 0x5c;
    expect[12] = 0xfb;
    expect[13] = 0x87;
    expect[14] = 0xf8;
    expect[15] = 0x26;
    expect[16] = 0x3f;
    expect[17] = 0x57;
    expect[18] = 0x86;
    expect[19] = 0xe2;
    expect[20] = 0xd8;
    expect[21] = 0x0e;
    expect[22] = 0xd3;
    expect[23] = 0x26;
    expect[24] = 0xcb;
    expect[25] = 0xc7;
    expect[26] = 0xf0;
    expect[27] = 0xe7;
    expect[28] = 0x1A;
    expect[29] = 0x99;
    expect[30] = 0xf4;
    expect[31] = 0x3b;
    expect[32] = 0xfb;
    expect[33] = 0x98;
    expect[34] = 0x8b;
    expect[35] = 0x9b;
    expect[36] = 0x7a;
    expect[37] = 0x02;
    expect[38] = 0xDD;
    expect[39] = 0x21;

    len = 1024;
    testrun(dtn_aes_key_wrap(
        data,
        32,
        buffer,
        &len,
        key,
        32));

    testrun(len == 40);
    testrun(0 == memcmp(buffer, expect, 40));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_aes_key_unwrap(){
    
    uint8_t key[256] = {0};
    key[0]  = 0x00;
    key[1]  = 0x01;
    key[2]  = 0x02;
    key[3]  = 0x03;
    key[4]  = 0x04;
    key[5]  = 0x05;
    key[6]  = 0x06;
    key[7]  = 0x07;
    key[8]  = 0x08;
    key[9]  = 0x09;
    key[10] = 0x0A;
    key[11] = 0x0B;
    key[12] = 0x0C;
    key[13] = 0x0D;
    key[14] = 0x0E;
    key[15] = 0x0F;

    uint8_t data[256] = {0};

    data[0]  = 0x00;
    data[1]  = 0x11;
    data[2]  = 0x22;
    data[3]  = 0x33;
    data[4]  = 0x44;
    data[5]  = 0x55;
    data[6]  = 0x66;
    data[7]  = 0x77;
    data[8]  = 0x88;
    data[9]  = 0x99;
    data[10] = 0xAA;
    data[11] = 0xBB;
    data[12] = 0xCC;
    data[13] = 0xDD;
    data[14] = 0xEE;
    data[15] = 0xFF;

    uint8_t expect[256] = {0};
    expect[0]  = 0x1F;
    expect[1]  = 0xA6;
    expect[2]  = 0x8B;
    expect[3]  = 0x0A;
    expect[4]  = 0x81;
    expect[5]  = 0x12;
    expect[6]  = 0xB4;
    expect[7]  = 0x47;
    expect[8]  = 0xAE;
    expect[9]  = 0xF3;
    expect[10] = 0x4B;
    expect[11] = 0xD8;
    expect[12] = 0xFB;
    expect[13] = 0x5A;
    expect[14] = 0x7B;
    expect[15] = 0x82;
    expect[16] = 0x9D;
    expect[17] = 0x3E;
    expect[18] = 0x86;
    expect[19] = 0x23;
    expect[20] = 0x71;
    expect[21] = 0xD2;
    expect[22] = 0xCF;
    expect[23] = 0xE5;
    expect[24] = 0x00;
    expect[25] = 0x00;
    expect[26] = 0x00;
    expect[27] = 0x00;

    uint8_t buffer[1024] = { 0 };
    size_t len = 1024;

    testrun(dtn_aes_key_unwrap(
        expect,
        24,
        buffer,
        &len,
        key,
        16));

    testrun(len == 16);
    //dtn_dump_binary_as_hex(stdout, buffer, 24);

    testrun(0 == memcmp(buffer, data, len));

    memset(key, 0, 256);
    key[0]  = 0x00;
    key[1]  = 0x01;
    key[2]  = 0x02;
    key[3]  = 0x03;
    key[4]  = 0x04;
    key[5]  = 0x05;
    key[6]  = 0x06;
    key[7]  = 0x07;
    key[8]  = 0x08;
    key[9]  = 0x09;
    key[10] = 0x0A;
    key[11] = 0x0B;
    key[12] = 0x0C;
    key[13] = 0x0D;
    key[14] = 0x0E;
    key[15] = 0x0F;
    key[16] = 0x10;
    key[17] = 0x11;
    key[18] = 0x12;
    key[19] = 0x13;
    key[20] = 0x14;
    key[21] = 0x15;
    key[22] = 0x16;
    key[23] = 0x17;
    key[24] = 0x00;

    memset(buffer, 0, 1024);

    expect[0]  = 0x96;
    expect[1]  = 0x77;
    expect[2]  = 0x8B;
    expect[3]  = 0x25;
    expect[4]  = 0xAE;
    expect[5]  = 0x6C;
    expect[6]  = 0xa4;
    expect[7]  = 0x35;
    expect[8]  = 0xF9;
    expect[9]  = 0x2B;
    expect[10] = 0x5b;
    expect[11] = 0x97;
    expect[12] = 0xc0;
    expect[13] = 0x50;
    expect[14] = 0xAE;
    expect[15] = 0xD2;
    expect[16] = 0x46;
    expect[17] = 0x8A;
    expect[18] = 0xB8;
    expect[19] = 0xA1;
    expect[20] = 0x7A;
    expect[21] = 0xD8;
    expect[22] = 0x4E;
    expect[23] = 0x5D;
    expect[24] = 0x00;
    expect[25] = 0x00;
    expect[26] = 0x00;
    expect[27] = 0x00;

    len = 1024;
    testrun(dtn_aes_key_unwrap(
        expect,
        24,
        buffer,
        &len,
        key,
        24));

    testrun(len == 16);
    //dtn_dump_binary_as_hex(stdout, buffer, 24);

    testrun(0 == memcmp(buffer, data, len));



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
    testrun_test(test_dtn_aes_key_wrap);
    testrun_test(test_dtn_aes_key_unwrap);

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
