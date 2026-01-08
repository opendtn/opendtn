/***
        ------------------------------------------------------------------------

        Copyright (c) 2021 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_password_test.c
        @author         Markus Töpfer

        @date           2021-02-20


        ------------------------------------------------------------------------
*/
#include "dtn_password.c"
#include <dtn_base/testrun.h>

#include <dtn_base/dtn_dump.h>
#include <dtn_base/dtn_item_json.h>
#include <dtn_base/dtn_time.h>

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_password_hash_scrypt() {

    /* using the example of openssl
     * https://www.openssl.org/docs/man1.1.1/man7/scrypt.html */

    const unsigned char expected[64] = {
        0xfd, 0xba, 0xbe, 0x1c, 0x9d, 0x34, 0x72, 0x00, 0x78, 0x56, 0xe7,
        0x19, 0x0d, 0x01, 0xe9, 0xfe, 0x7c, 0x6a, 0xd7, 0xcb, 0xc8, 0x23,
        0x78, 0x30, 0xe7, 0x73, 0x76, 0x63, 0x4b, 0x37, 0x31, 0x62, 0x2e,
        0xaf, 0x30, 0xd9, 0x2e, 0x22, 0xa3, 0x88, 0x6f, 0xf1, 0x09, 0x27,
        0x9d, 0x98, 0x30, 0xda, 0xc7, 0x27, 0xaf, 0xb9, 0x4a, 0x83, 0xee,
        0x6d, 0x83, 0x60, 0xcb, 0xdf, 0xa2, 0xcc, 0x06, 0x40};

    size_t size = 1024;
    char buffer[size];
    memset(buffer, 0, size);

    uint8_t *out = (uint8_t *)buffer;
    size_t len = 64;

    uint64_t start = dtn_time_get_current_time_usecs();
    testrun(dtn_password_hash_scrypt(out, &len, "password", "NaCl",
                                     (dtn_password_hash_parameter){0}));
    uint64_t end = dtn_time_get_current_time_usecs();

    testrun(len == 64);
    testrun(0 == memcmp(out, expected, 64));
    fprintf(stdout, "HASH time %" PRIu64 " usec\n", end - start);
    // dtn_dump_binary_as_hex(stdout, out, len);
    fprintf(stdout, "\n");

    // double all defaults
    start = dtn_time_get_current_time_usecs();
    memset(buffer, 0, size);
    testrun(dtn_password_hash_scrypt(
        out, &len, "password", "NaCl",
        (dtn_password_hash_parameter){
            .workfactor = 2048, .blocksize = 16, .parallel = 32}));
    end = dtn_time_get_current_time_usecs();
    fprintf(stdout, "HASH time %" PRIu64 " usec\n", end - start);
    // dtn_dump_binary_as_hex(stdout, out, len);
    testrun(0 != memcmp(out, expected, 64));
    fprintf(stdout, "\n");

    // double all defaults and len
    len = 128;
    start = dtn_time_get_current_time_usecs();
    memset(buffer, 0, size);
    testrun(dtn_password_hash_scrypt(
        out, &len, "password", "NaCl",
        (dtn_password_hash_parameter){
            .workfactor = 2048, .blocksize = 16, .parallel = 32}));
    end = dtn_time_get_current_time_usecs();
    fprintf(stdout, "HASH time %" PRIu64 " usec\n", end - start);
    // dtn_dump_binary_as_hex(stdout, out, len);
    testrun(0 != memcmp(out, expected, 64));
    fprintf(stdout, "\n");

    // check with small parameter
    len = 64;
    start = dtn_time_get_current_time_usecs();
    memset(buffer, 0, size);
    testrun(dtn_password_hash_scrypt(
        out, &len, "password", "NaCl",
        (dtn_password_hash_parameter){
            .workfactor = 1024, .blocksize = 8, .parallel = 1}));
    end = dtn_time_get_current_time_usecs();
    fprintf(stdout, "HASH time %" PRIu64 " usec\n", end - start);
    // dtn_dump_binary_as_hex(stdout, out, len);
    testrun(0 != memcmp(out, expected, 64));
    fprintf(stdout, "\n");

    // check with tiny parameter
    len = 64;
    start = dtn_time_get_current_time_usecs();
    memset(buffer, 0, size);
    testrun(dtn_password_hash_scrypt(
        out, &len, "password", "NaCl",
        (dtn_password_hash_parameter){
            .workfactor = 2, .blocksize = 2, .parallel = 1}));
    end = dtn_time_get_current_time_usecs();
    fprintf(stdout, "HASH time %" PRIu64 " usec\n", end - start);
    // dtn_dump_binary_as_hex(stdout, out, len);
    testrun(0 != memcmp(out, expected, 64));
    fprintf(stdout, "\n");

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_password_hash_pdkdf2() {

    size_t size = 1024;
    char buffer[size];
    memset(buffer, 0, size);

    uint8_t *out = (uint8_t *)buffer;
    size_t len = 64;

    uint64_t start = dtn_time_get_current_time_usecs();
    testrun(dtn_password_hash_pdkdf2(out, &len, "password", "NaCl",
                                     (dtn_password_hash_parameter){0}));
    uint64_t end = dtn_time_get_current_time_usecs();

    fprintf(stdout, "HASH time %" PRIu64 " usec\n", end - start);
    // dtn_dump_binary_as_hex(stdout, out, len);
    fprintf(stdout, "\n");

    start = dtn_time_get_current_time_usecs();
    testrun(dtn_password_hash_pdkdf2(
        out, &len, "password", "NaCl",
        (dtn_password_hash_parameter){.workfactor = 1}));
    end = dtn_time_get_current_time_usecs();

    fprintf(stdout, "HASH time %" PRIu64 " usec\n", end - start);
    // dtn_dump_binary_as_hex(stdout, out, len);
    fprintf(stdout, "\n");

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_password_params_from_item() {

    dtn_password_hash_parameter params = {0};

    params = dtn_password_params_from_item(NULL);
    testrun(0 == params.workfactor);
    testrun(0 == params.blocksize);
    testrun(0 == params.parallel);

    dtn_item *object = dtn_item_object();
    params = dtn_password_params_from_item(object);
    testrun(0 == params.workfactor);
    testrun(0 == params.blocksize);
    testrun(0 == params.parallel);

    dtn_item *val = dtn_item_number(1);
    testrun(dtn_item_object_set(object, DTN_AUTH_KEY_WORKFACTOR, val));
    val = dtn_item_number(2);
    testrun(dtn_item_object_set(object, DTN_AUTH_KEY_BLOCKSIZE, val));
    val = dtn_item_number(3);
    testrun(dtn_item_object_set(object, DTN_AUTH_KEY_PARALLEL, val));

    params = dtn_password_params_from_item(object);
    testrun(1 == params.workfactor);
    testrun(2 == params.blocksize);
    testrun(3 == params.parallel);

    testrun(NULL == dtn_item_free(object));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_password_params_to_item() {

    dtn_password_hash_parameter params = {0};

    dtn_item *val = dtn_password_params_to_item(params);
    testrun(val);
    testrun(dtn_item_object_get(val, DTN_AUTH_KEY_WORKFACTOR));
    testrun(dtn_item_object_get(val, DTN_AUTH_KEY_BLOCKSIZE));
    testrun(dtn_item_object_get(val, DTN_AUTH_KEY_PARALLEL));
    testrun(0 == dtn_item_get_number(
                     dtn_item_object_get(val, DTN_AUTH_KEY_WORKFACTOR)));
    testrun(0 == dtn_item_get_number(
                     dtn_item_object_get(val, DTN_AUTH_KEY_BLOCKSIZE)));
    testrun(0 == dtn_item_get_number(
                     dtn_item_object_get(val, DTN_AUTH_KEY_PARALLEL)));
    val = dtn_item_free(val);

    params.workfactor = 1;
    params.blocksize = 2;
    params.parallel = 3;
    val = dtn_password_params_to_item(params);
    testrun(val);
    testrun(dtn_item_object_get(val, DTN_AUTH_KEY_WORKFACTOR));
    testrun(dtn_item_object_get(val, DTN_AUTH_KEY_BLOCKSIZE));
    testrun(dtn_item_object_get(val, DTN_AUTH_KEY_PARALLEL));
    testrun(1 == dtn_item_get_number(
                     dtn_item_object_get(val, DTN_AUTH_KEY_WORKFACTOR)));
    testrun(2 == dtn_item_get_number(
                     dtn_item_object_get(val, DTN_AUTH_KEY_BLOCKSIZE)));
    testrun(3 == dtn_item_get_number(
                     dtn_item_object_get(val, DTN_AUTH_KEY_PARALLEL)));
    val = dtn_item_free(val);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_password_is_valid() {

    const char *pass = "password";
    const char *salt = "NaCl";

    /* prepare valid input */

    uint8_t buffer[64] = {0};
    size_t len = 64;
    testrun(dtn_password_hash_pdkdf2(buffer, &len, pass, salt,
                                     (dtn_password_hash_parameter){0}));

    uint8_t *base64_hash_buffer = NULL;
    size_t base64_hash_buffer_length = 0;
    testrun(dtn_base64_encode(buffer, len, &base64_hash_buffer,
                              &base64_hash_buffer_length));

    uint8_t *base64_salt_buffer = NULL;
    size_t base64_salt_buffer_length = 0;
    testrun(dtn_base64_encode((uint8_t *)salt, strlen(salt),
                              &base64_salt_buffer, &base64_salt_buffer_length));

    dtn_item *json = dtn_item_object();
    dtn_item *val = dtn_item_string((char *)base64_salt_buffer);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_SALT, val));
    val = dtn_item_string((char *)base64_hash_buffer);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_HASH, val));

    testrun(!dtn_password_is_valid(NULL, NULL));
    testrun(!dtn_password_is_valid(pass, NULL));
    testrun(!dtn_password_is_valid(NULL, json));

    char *s = dtn_item_to_json(json);
    fprintf(stdout, "%s\n", s);
    s = dtn_data_pointer_free(s);

    testrun(dtn_password_is_valid(pass, json));

    // check missing input
    testrun(dtn_item_object_delete(json, DTN_AUTH_KEY_HASH));
    testrun(!dtn_password_is_valid(pass, json));

    val = dtn_item_string((char *)base64_hash_buffer);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_HASH, val));
    testrun(dtn_item_object_delete(json, DTN_AUTH_KEY_SALT));
    testrun(!dtn_password_is_valid(pass, json));

    val = dtn_item_string((char *)base64_salt_buffer);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_SALT, val));
    testrun(dtn_password_is_valid(pass, json));

    // the hash was created without any params,
    // which means IMPL_WORKFACTOR_DEFAULT was uses as iteration
    // check with a different iteration size
    /*
        for (size_t i = 1; i < 2048; i++){

            val = dtn_item_number( i);
            testrun(dtn_item_object_set(json, DTN_AUTH_KEY_WORKFACTOR, val));
            if (i == IMPL_WORKFACTOR_DEFAULT){
                testrun(dtn_password_is_valid(pass, json));
            } else {
                testrun(!dtn_password_is_valid(pass, json));
            }
        }

        val = dtn_item_number( IMPL_WORKFACTOR_DEFAULT);
        testrun(dtn_item_object_set(json, DTN_AUTH_KEY_WORKFACTOR, val));
        testrun(dtn_password_is_valid(pass, json));
    */
    // set some parallel to force to use scrypt instead of pdkdf2
    val = dtn_item_number(1);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_PARALLEL, val));
    testrun(!dtn_password_is_valid(pass, json));
    testrun(dtn_item_object_delete(json, DTN_AUTH_KEY_PARALLEL));
    testrun(dtn_password_is_valid(pass, json));

    // set some blocksize to force to use srcypt instead of pkdf2
    val = dtn_item_number(1);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_BLOCKSIZE, val));
    testrun(!dtn_password_is_valid(pass, json));
    testrun(dtn_item_object_delete(json, DTN_AUTH_KEY_BLOCKSIZE));
    testrun(dtn_password_is_valid(pass, json));

    // check different password
    testrun(!dtn_password_is_valid("abcd", json));
    // check case sensitive password
    testrun(!dtn_password_is_valid("Password", json));

    json = dtn_item_free(json);

    // check scrypt

    memset(buffer, 0, 64);
    testrun(dtn_password_hash_scrypt(buffer, &len, pass, salt,
                                     (dtn_password_hash_parameter){0}));

    base64_hash_buffer = dtn_data_pointer_free(base64_hash_buffer);
    testrun(dtn_base64_encode(buffer, len, &base64_hash_buffer,
                              &base64_hash_buffer_length));

    json = dtn_item_object();
    val = dtn_item_string((char *)base64_salt_buffer);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_SALT, val));
    val = dtn_item_string((char *)base64_hash_buffer);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_HASH, val));

    testrun(!dtn_password_is_valid(pass, json));

    // set correct params for scrypt
    val = dtn_item_number(IMPL_BLOCKSIZE_DEFAULT);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_BLOCKSIZE, val));
    val = dtn_item_number(IMPL_WORKFACTOR_DEFAULT);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_WORKFACTOR, val));
    val = dtn_item_number(IMPL_PARALLEL_DEFAULT);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_PARALLEL, val));

    testrun(dtn_password_is_valid(pass, json));

    // wrong blocksize
    val = dtn_item_number(1);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_BLOCKSIZE, val));
    testrun(!dtn_password_is_valid(pass, json));
    val = dtn_item_number(IMPL_BLOCKSIZE_DEFAULT);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_BLOCKSIZE, val));
    testrun(dtn_password_is_valid(pass, json));

    // wrong parallel
    val = dtn_item_number(1);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_PARALLEL, val));
    testrun(!dtn_password_is_valid(pass, json));
    val = dtn_item_number(IMPL_PARALLEL_DEFAULT);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_PARALLEL, val));
    testrun(dtn_password_is_valid(pass, json));

    // wrong workfactor
    val = dtn_item_number(1);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_WORKFACTOR, val));
    testrun(!dtn_password_is_valid(pass, json));
    val = dtn_item_number(IMPL_WORKFACTOR_DEFAULT);
    testrun(dtn_item_object_set(json, DTN_AUTH_KEY_WORKFACTOR, val));
    testrun(dtn_password_is_valid(pass, json));

    // check different password
    testrun(!dtn_password_is_valid("abcd", json));
    // check case sensitive password
    testrun(!dtn_password_is_valid("Password", json));

    base64_hash_buffer = dtn_data_pointer_free(base64_hash_buffer);
    base64_salt_buffer = dtn_data_pointer_free(base64_salt_buffer);
    json = dtn_item_free(json);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_password_hash() {

    const char *pass = "password";
    char *str = NULL;

    testrun(!dtn_password_hash(NULL, (dtn_password_hash_parameter){0}, 0));

    dtn_item *out =
        dtn_password_hash(pass, (dtn_password_hash_parameter){0}, 0);

    testrun(out);
    str = dtn_item_to_json(out);
    fprintf(stdout, "%s\n", str);
    str = dtn_data_pointer_free(str);
    testrun(2 == dtn_item_count(out));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_SALT));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_HASH));
    testrun(dtn_password_is_valid(pass, out));
    out = dtn_item_free(out);

    // set explicit length
    out = dtn_password_hash(pass, (dtn_password_hash_parameter){0}, 16);

    testrun(out);
    str = dtn_item_to_json(out);
    fprintf(stdout, "%s\n", str);
    str = dtn_data_pointer_free(str);
    testrun(2 == dtn_item_count(out));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_SALT));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_HASH));
    testrun(dtn_password_is_valid(pass, out));
    out = dtn_item_free(out);

    // set explicit length and iteration
    out = dtn_password_hash(
        pass, (dtn_password_hash_parameter){.workfactor = 2048}, 0);

    testrun(out);
    str = dtn_item_to_json(out);
    fprintf(stdout, "%s\n", str);
    str = dtn_data_pointer_free(str);
    testrun(3 == dtn_item_count(out));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_SALT));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_HASH));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_WORKFACTOR));
    testrun(2048 == dtn_item_get_number(
                        dtn_item_object_get(out, DTN_AUTH_KEY_WORKFACTOR)));
    testrun(dtn_password_is_valid(pass, out));
    out = dtn_item_free(out);

    // set explicit blocksize
    out = dtn_password_hash(pass,
                            (dtn_password_hash_parameter){.blocksize = 16}, 0);

    testrun(out);
    str = dtn_item_to_json(out);
    fprintf(stdout, "%s\n", str);
    str = dtn_data_pointer_free(str);
    testrun(3 == dtn_item_count(out));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_SALT));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_HASH));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_BLOCKSIZE));
    testrun(16 == dtn_item_get_number(
                      dtn_item_object_get(out, DTN_AUTH_KEY_BLOCKSIZE)));
    testrun(dtn_password_is_valid(pass, out));
    out = dtn_item_free(out);

    // set explicit parallel
    out = dtn_password_hash(pass, (dtn_password_hash_parameter){.parallel = 16},
                            0);

    testrun(out);
    str = dtn_item_to_json(out);
    fprintf(stdout, "%s\n", str);
    str = dtn_data_pointer_free(str);
    testrun(3 == dtn_item_count(out));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_SALT));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_HASH));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_PARALLEL));
    testrun(16 == dtn_item_get_number(
                      dtn_item_object_get(out, DTN_AUTH_KEY_PARALLEL)));
    testrun(dtn_password_is_valid(pass, out));
    out = dtn_item_free(out);

    // set all
    out = dtn_password_hash(pass,
                            (dtn_password_hash_parameter){.parallel = 2,
                                                          .blocksize = 4,
                                                          .workfactor = 1024},
                            128);

    testrun(out);
    str = dtn_item_to_json(out);
    fprintf(stdout, "%s\n", str);
    str = dtn_data_pointer_free(str);
    testrun(5 == dtn_item_count(out));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_SALT));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_HASH));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_WORKFACTOR));
    testrun(1024 == dtn_item_get_number(
                        dtn_item_object_get(out, DTN_AUTH_KEY_WORKFACTOR)));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_PARALLEL));
    testrun(2 == dtn_item_get_number(
                     dtn_item_object_get(out, DTN_AUTH_KEY_PARALLEL)));
    testrun(dtn_item_object_get(out, DTN_AUTH_KEY_BLOCKSIZE));
    testrun(4 == dtn_item_get_number(
                     dtn_item_object_get(out, DTN_AUTH_KEY_BLOCKSIZE)));
    testrun(dtn_password_is_valid(pass, out));
    out = dtn_item_free(out);

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

    testrun_test(test_dtn_password_hash_scrypt);
    testrun_test(test_dtn_password_hash_pdkdf2);

    testrun_test(test_dtn_password_params_from_item);
    testrun_test(test_dtn_password_params_to_item);

    testrun_test(test_dtn_password_is_valid);
    testrun_test(test_dtn_password_hash);

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
