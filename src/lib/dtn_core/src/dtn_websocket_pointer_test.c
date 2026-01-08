/***
        ------------------------------------------------------------------------

        Copyright (c) 2020 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_websocket_pointer_test.c
        @author         Markus Töpfer

        @date           2020-12-26


        ------------------------------------------------------------------------
*/
#include "dtn_websocket_pointer.c"
#include <dtn_base/testrun.h>

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_websocket_generate_secure_websocket_key() {

    size_t amount = 100;
    uint8_t *buffer[amount];

    for (size_t i = 0; i < amount; i++) {

        buffer[i] = calloc(DTN_WEBSOCKET_SECURE_KEY_SIZE + 1, sizeof(uint8_t));
        testrun(buffer[i]);
        testrun(dtn_websocket_generate_secure_websocket_key(
            buffer[i], DTN_WEBSOCKET_SECURE_KEY_SIZE));

        for (size_t k = 0; k < i; k++) {

            testrun(0 != memcmp(buffer[k], buffer[i],
                                DTN_WEBSOCKET_SECURE_KEY_SIZE));
        }
    }

    testrun(!dtn_websocket_generate_secure_websocket_key(NULL, 0));
    testrun(!dtn_websocket_generate_secure_websocket_key(buffer[0], 0));
    testrun(!dtn_websocket_generate_secure_websocket_key(
        NULL, DTN_WEBSOCKET_SECURE_KEY_SIZE));

    testrun(!dtn_websocket_generate_secure_websocket_key(
        buffer[0], DTN_WEBSOCKET_SECURE_KEY_SIZE - 1));

    testrun(dtn_websocket_generate_secure_websocket_key(
        buffer[0], DTN_WEBSOCKET_SECURE_KEY_SIZE));

    for (size_t i = 0; i < amount; i++) {
        free(buffer[i]);
    }

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_websocket_generate_secure_accept_key() {

    size_t size = DTN_WEBSOCKET_SECURE_KEY_SIZE;
    uint8_t buffer[size];
    memset(buffer, 0, size);

    size_t hash_size = DTN_SHA1_SIZE;
    uint8_t hashed[hash_size];
    memset(hashed, 0, hash_size);

    size_t base_size = DTN_SHA1_SIZE;
    uint8_t base[base_size];
    memset(base, 0, base_size);

    size_t length = DTN_WEBSOCKET_SECURE_KEY_SIZE + GUID_LEN;
    uint8_t expect[length];
    memset(expect, 0, length);

    testrun(dtn_websocket_generate_secure_websocket_key(
        buffer, DTN_WEBSOCKET_SECURE_KEY_SIZE));

    testrun(memcpy(expect, buffer, DTN_WEBSOCKET_SECURE_KEY_SIZE));
    testrun(memcpy(expect + DTN_WEBSOCKET_SECURE_KEY_SIZE, GUID, GUID_LEN));

    uint8_t *ptr = hashed;
    testrun(dtn_hash_string(DTN_HASH_SHA1, expect, length, &ptr, &hash_size));

    uint8_t *key = NULL;
    size_t len = 0;

    testrun(!dtn_websocket_generate_secure_accept_key(NULL, 0, NULL, NULL));

    testrun(dtn_websocket_generate_secure_accept_key(
        buffer, DTN_WEBSOCKET_SECURE_KEY_SIZE, &key, &len));

    testrun(key);
    testrun(len == 28);

    ptr = base;
    testrun(dtn_base64_decode((uint8_t *)key, len, &ptr, &base_size));
    testrun(base_size == DTN_SHA1_SIZE);

    testrun(0 == memcmp(hashed, base, DTN_SHA1_SIZE));

    free(key);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_websocket_process_handshake_request() {

    uint8_t key[DTN_WEBSOCKET_SECURE_KEY_SIZE + 1];
    memset(key, 0, DTN_WEBSOCKET_SECURE_KEY_SIZE + 1);

    uint8_t *accept_key = NULL;
    size_t accept_key_length = 0;

    testrun(dtn_websocket_generate_secure_websocket_key(
        key, DTN_WEBSOCKET_SECURE_KEY_SIZE));

    testrun(dtn_websocket_generate_secure_accept_key(
        key, DTN_WEBSOCKET_SECURE_KEY_SIZE, &accept_key, &accept_key_length));

    dtn_http_message_config config = (dtn_http_message_config){0};
    dtn_http_version version = (dtn_http_version){.major = 1, .minor = 1};

    bool is_handshake = false;
    const dtn_http_header *header = NULL;

    dtn_http_message *out = NULL;
    dtn_http_message *in =
        dtn_http_create_request_string(config, version, "GET", "/");
    testrun(in);
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_HOST, "host"));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_UPGRADE,
                                               DTN_WEBSOCKET_KEY));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_CONNECTION,
                                               DTN_HTTP_KEY_UPGRADE));
    testrun(dtn_http_message_add_header_string(in, DTN_WEBSOCKET_KEY_SECURE,
                                               (char *)key));
    testrun(dtn_http_message_add_header_string(
        in, DTN_WEBSOCKET_KEY_SECURE_VERSION, "13"));
    testrun(dtn_http_message_close_header(in));
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_message(in, NULL));

    testrun(!dtn_websocket_process_handshake_request(NULL, NULL, NULL));
    testrun(!dtn_websocket_process_handshake_request(in, NULL, &is_handshake));
    testrun(
        !dtn_websocket_process_handshake_request(NULL, &out, &is_handshake));

    testrun(dtn_websocket_process_handshake_request(in, &out, &is_handshake));
    testrun(out);
    testrun(is_handshake == true);
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_message(out, NULL));
    // check out
    testrun(out->status.code == 101);
    testrun(0 == strncmp(DTN_HTTP_SWITCH_PROTOCOLS,
                         (char *)out->status.phrase.start,
                         out->status.phrase.length));
    header = dtn_http_header_get_unique(
        out->header, out->config.header.capacity, DTN_HTTP_KEY_UPGRADE);
    testrun(0 == strncmp(DTN_WEBSOCKET_KEY, (char *)header->value.start,
                         header->value.length));
    header = dtn_http_header_get_unique(
        out->header, out->config.header.capacity, DTN_HTTP_KEY_CONNECTION);
    testrun(0 == strncmp(DTN_HTTP_KEY_UPGRADE, (char *)header->value.start,
                         header->value.length));
    header =
        dtn_http_header_get_unique(out->header, out->config.header.capacity,
                                   DTN_WEBSOCKET_KEY_SECURE_ACCEPT);

    testrun(0 == memcmp(accept_key, header->value.start, header->value.length));

    out = dtn_http_message_free(out);
    is_handshake = false;

    testrun(dtn_websocket_process_handshake_request(in, &out, NULL));
    testrun(out);
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_message(out, NULL));
    // check out
    testrun(out->status.code == 101);
    testrun(0 == strncmp(DTN_HTTP_SWITCH_PROTOCOLS,
                         (char *)out->status.phrase.start,
                         out->status.phrase.length));
    header = dtn_http_header_get_unique(
        out->header, out->config.header.capacity, DTN_HTTP_KEY_UPGRADE);
    testrun(0 == strncmp(DTN_WEBSOCKET_KEY, (char *)header->value.start,
                         header->value.length));
    header = dtn_http_header_get_unique(
        out->header, out->config.header.capacity, DTN_HTTP_KEY_CONNECTION);
    testrun(0 == strncmp(DTN_HTTP_KEY_UPGRADE, (char *)header->value.start,
                         header->value.length));
    header =
        dtn_http_header_get_unique(out->header, out->config.header.capacity,
                                   DTN_WEBSOCKET_KEY_SECURE_ACCEPT);
    testrun(0 == memcmp(accept_key, header->value.start, header->value.length));

    out = dtn_http_message_free(out);
    is_handshake = false;

    // METHOD not GET
    is_handshake = false;
    in = dtn_http_message_free(in);
    in = dtn_http_create_request_string(config, version, "PUT", "/");
    testrun(in);
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_HOST, "host"));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_UPGRADE,
                                               DTN_WEBSOCKET_KEY));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_CONNECTION,
                                               DTN_HTTP_KEY_UPGRADE));
    testrun(dtn_http_message_add_header_string(in, DTN_WEBSOCKET_KEY_SECURE,
                                               (char *)key));
    testrun(dtn_http_message_add_header_string(
        in, DTN_WEBSOCKET_KEY_SECURE_VERSION, "13"));
    testrun(dtn_http_message_close_header(in));
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_message(in, NULL));
    testrun(!dtn_websocket_process_handshake_request(in, &out, &is_handshake));
    testrun(is_handshake == false);
    testrun(NULL == out);

    // NOT upgrade request
    is_handshake = false;
    in = dtn_http_message_free(in);
    in = dtn_http_create_request_string(config, version, "GET", "/");
    testrun(in);
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_HOST, "host"));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_UPGRADE,
                                               "somthing"));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_CONNECTION,
                                               DTN_HTTP_KEY_UPGRADE));
    testrun(dtn_http_message_add_header_string(in, DTN_WEBSOCKET_KEY_SECURE,
                                               (char *)key));
    testrun(dtn_http_message_add_header_string(
        in, DTN_WEBSOCKET_KEY_SECURE_VERSION, "13"));
    testrun(dtn_http_message_close_header(in));
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_message(in, NULL));
    testrun(!dtn_websocket_process_handshake_request(in, &out, &is_handshake));
    testrun(is_handshake == false);
    testrun(NULL == out);

    // No connection upgrade
    is_handshake = false;
    in = dtn_http_message_free(in);
    in = dtn_http_create_request_string(config, version, "GET", "/");
    testrun(in);
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_HOST, "host"));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_UPGRADE,
                                               DTN_WEBSOCKET_KEY));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_CONNECTION,
                                               "something"));
    testrun(dtn_http_message_add_header_string(in, DTN_WEBSOCKET_KEY_SECURE,
                                               (char *)key));
    testrun(dtn_http_message_add_header_string(
        in, DTN_WEBSOCKET_KEY_SECURE_VERSION, "13"));
    testrun(dtn_http_message_close_header(in));
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_message(in, NULL));
    testrun(!dtn_websocket_process_handshake_request(in, &out, &is_handshake));
    testrun(is_handshake == false);
    testrun(NULL == out);

    // No host
    is_handshake = false;
    in = dtn_http_message_free(in);
    in = dtn_http_create_request_string(config, version, "GET", "/");
    testrun(in);
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_UPGRADE,
                                               DTN_WEBSOCKET_KEY));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_CONNECTION,
                                               DTN_HTTP_KEY_UPGRADE));
    testrun(dtn_http_message_add_header_string(in, DTN_WEBSOCKET_KEY_SECURE,
                                               (char *)key));
    testrun(dtn_http_message_add_header_string(
        in, DTN_WEBSOCKET_KEY_SECURE_VERSION, "13"));
    testrun(dtn_http_message_close_header(in));
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_message(in, NULL));
    testrun(!dtn_websocket_process_handshake_request(in, &out, &is_handshake));
    testrun(is_handshake == false);
    testrun(NULL == out);

    // No sec_key
    is_handshake = false;
    in = dtn_http_message_free(in);
    in = dtn_http_create_request_string(config, version, "GET", "/");
    testrun(in);
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_HOST, "host"));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_UPGRADE,
                                               DTN_WEBSOCKET_KEY));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_CONNECTION,
                                               DTN_HTTP_KEY_UPGRADE));
    testrun(dtn_http_message_add_header_string(
        in, DTN_WEBSOCKET_KEY_SECURE_VERSION, "13"));
    testrun(dtn_http_message_close_header(in));
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_message(in, NULL));
    testrun(!dtn_websocket_process_handshake_request(in, &out, &is_handshake));
    testrun(is_handshake == false);
    testrun(NULL == out);

    // No sec_ver
    is_handshake = false;
    in = dtn_http_message_free(in);
    in = dtn_http_create_request_string(config, version, "GET", "/");
    testrun(in);
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_HOST, "host"));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_UPGRADE,
                                               DTN_WEBSOCKET_KEY));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_CONNECTION,
                                               DTN_HTTP_KEY_UPGRADE));
    testrun(dtn_http_message_add_header_string(in, DTN_WEBSOCKET_KEY_SECURE,
                                               (char *)key));
    testrun(dtn_http_message_close_header(in));
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_message(in, NULL));
    testrun(!dtn_websocket_process_handshake_request(in, &out, &is_handshake));
    testrun(is_handshake == false);
    testrun(NULL == out);

    // Wrong secure version
    is_handshake = false;
    in = dtn_http_message_free(in);
    in = dtn_http_create_request_string(config, version, "GET", "/");
    testrun(in);
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_HOST, "host"));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_UPGRADE,
                                               DTN_WEBSOCKET_KEY));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_CONNECTION,
                                               DTN_HTTP_KEY_UPGRADE));
    testrun(dtn_http_message_add_header_string(in, DTN_WEBSOCKET_KEY_SECURE,
                                               (char *)key));
    testrun(dtn_http_message_add_header_string(
        in, DTN_WEBSOCKET_KEY_SECURE_VERSION, "12"));
    testrun(dtn_http_message_close_header(in));
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_message(in, NULL));
    testrun(!dtn_websocket_process_handshake_request(in, &out, &is_handshake));
    testrun(is_handshake == false);
    testrun(NULL != out);
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_message(out, NULL));
    // check out
    testrun(out->status.code == 426);
    testrun(0 == strncmp(DTN_HTTP_UPGRADE_REQUIRED,
                         (char *)out->status.phrase.start,
                         out->status.phrase.length));
    header =
        dtn_http_header_get_unique(out->header, out->config.header.capacity,
                                   DTN_WEBSOCKET_KEY_SECURE_VERSION);
    testrun(0 ==
            strncmp("13", (char *)header->value.start, header->value.length));
    out = dtn_http_message_free(out);

    // secure key not 24 bytes
    char *wrong_key = "key";
    is_handshake = false;
    in = dtn_http_message_free(in);
    in = dtn_http_create_request_string(config, version, "GET", "/");
    testrun(in);
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_HOST, "host"));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_UPGRADE,
                                               DTN_WEBSOCKET_KEY));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_CONNECTION,
                                               DTN_HTTP_KEY_UPGRADE));
    testrun(dtn_http_message_add_header_string(in, DTN_WEBSOCKET_KEY_SECURE,
                                               wrong_key));
    testrun(dtn_http_message_add_header_string(
        in, DTN_WEBSOCKET_KEY_SECURE_VERSION, "13"));
    testrun(dtn_http_message_close_header(in));
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_message(in, NULL));
    testrun(!dtn_websocket_process_handshake_request(in, &out, &is_handshake));
    testrun(is_handshake == false);
    testrun(NULL == out);

    // upgrade in comma list
    is_handshake = false;
    in = dtn_http_message_free(in);
    in = dtn_http_create_request_string(config, version, "GET", "/");
    testrun(in);
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_HOST, "host"));
    testrun(dtn_http_message_add_header_string(in, DTN_HTTP_KEY_UPGRADE,
                                               DTN_WEBSOCKET_KEY));
    testrun(dtn_http_message_add_header_string(
        in, DTN_HTTP_KEY_CONNECTION, "x, y, z,  upgrade , whatever "));
    testrun(dtn_http_message_add_header_string(in, DTN_WEBSOCKET_KEY_SECURE,
                                               (char *)key));
    testrun(dtn_http_message_add_header_string(
        in, DTN_WEBSOCKET_KEY_SECURE_VERSION, "13"));
    testrun(dtn_http_message_close_header(in));
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_message(in, NULL));
    testrun(dtn_websocket_process_handshake_request(in, &out, &is_handshake));
    testrun(is_handshake == true);
    testrun(NULL != out);
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_message(out, NULL));
    // check out
    testrun(out->status.code == 101);
    testrun(0 == strncmp(DTN_HTTP_SWITCH_PROTOCOLS,
                         (char *)out->status.phrase.start,
                         out->status.phrase.length));
    header = dtn_http_header_get_unique(
        out->header, out->config.header.capacity, DTN_HTTP_KEY_UPGRADE);
    testrun(0 == strncmp(DTN_WEBSOCKET_KEY, (char *)header->value.start,
                         header->value.length));
    header = dtn_http_header_get_unique(
        out->header, out->config.header.capacity, DTN_HTTP_KEY_CONNECTION);
    testrun(0 == strncmp(DTN_HTTP_KEY_UPGRADE, (char *)header->value.start,
                         header->value.length));
    header =
        dtn_http_header_get_unique(out->header, out->config.header.capacity,
                                   DTN_WEBSOCKET_KEY_SECURE_ACCEPT);
    testrun(0 == memcmp(accept_key, header->value.start, header->value.length));

    out = dtn_http_message_free(out);

    testrun(NULL == dtn_http_message_free(in));
    testrun(NULL == dtn_http_message_free(out));

    accept_key = dtn_data_pointer_free(accept_key);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_fragmentation_state() {

    uint8_t buffer = 0;

    bool fin = false;
    bool opcode = false;

    dtn_websocket_fragmentation_state state;

    for (size_t i = 0; i < 0xff; i++) {

        buffer = i;

        fin = false;
        opcode = false;

        if (buffer & 0x80)
            fin = true;

        if (buffer & 0x0F)
            opcode = true;

        state = fragmentation_state(buffer);

        if (fin && opcode) {
            testrun(state == DTN_WEBSOCKET_FRAGMENTATION_NONE);
        } else if (fin) {
            testrun(state == DTN_WEBSOCKET_FRAGMENTATION_LAST);
        } else if (opcode) {
            testrun(state == DTN_WEBSOCKET_FRAGMENTATION_START);
        } else {
            testrun(state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
        }
    }

    testrun(DTN_WEBSOCKET_FRAGMENTATION_CONTINUE == fragmentation_state(0));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_parse_payload_length() {

    uint8_t *next = NULL;

    dtn_websocket_frame *frame = dtn_websocket_frame_create(
        (dtn_websocket_frame_config){.buffer.default_size = 0xFffff});

    testrun(frame);
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 0xFffff);

    // set min valid for OK
    frame->buffer->start[1] = 0;
    frame->buffer->length = 2;

    testrun(DTN_WEBSOCKET_PARSER_ERROR == parse_payload_length(NULL, NULL));
    testrun(DTN_WEBSOCKET_PARSER_ERROR == parse_payload_length(NULL, &next));

    // parse without next
    testrun(DTN_WEBSOCKET_PARSER_SUCCESS == parse_payload_length(frame, NULL));
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);
    // parse with next
    testrun(DTN_WEBSOCKET_PARSER_SUCCESS == parse_payload_length(frame, &next));
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);
    testrun(next == frame->buffer->start + frame->buffer->length);

    // reset
    memset(frame->buffer->start, 0, frame->buffer->capacity);
    frame->buffer->length = 0;

    memset(frame->buffer->start, 'A', 150);

    // check length < 126 without masking
    for (size_t i = 0; i < 126; i++) {

        frame->buffer->start[1] = i;
        for (size_t x = 0; x < i + 1; x++) {

            frame->buffer->length = x;
            testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                    parse_payload_length(frame, NULL));
        }

        frame->buffer->length = i + 2;
        testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                parse_payload_length(frame, NULL));
    }

    // check length < 126 with masking
    for (size_t i = 0; i < 126; i++) {

        frame->buffer->start[1] = i | 0x80;

        for (size_t x = 0; x < i + 6; x++) {

            frame->buffer->length = x;

            if (i == 0) {

                if (x < 2) {
                    testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                            parse_payload_length(frame, NULL));
                } else {
                    // Zero content but masking set
                    testrun(DTN_WEBSOCKET_PARSER_ERROR ==
                            parse_payload_length(frame, NULL));
                }

            } else {
                testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                        parse_payload_length(frame, NULL));
            }
        }

        frame->buffer->length = i + 6;
        if (i == 0) {
            // Zero content but masking set
            testrun(DTN_WEBSOCKET_PARSER_ERROR ==
                    parse_payload_length(frame, NULL));
        } else {
            testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                    parse_payload_length(frame, NULL));
        }
    }

    // check next we start at 1 to avoid testing for 0 with mask (tested above)
    for (size_t i = 1; i < 126; i++) {

        frame->buffer->start[1] = i;
        next = NULL;

        for (size_t x = 0; x < i + 1; x++) {

            frame->buffer->length = x;
            testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                    parse_payload_length(frame, &next));
            testrun(next == NULL);
        }

        frame->buffer->length = i + 2;
        testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                parse_payload_length(frame, &next));
        testrun(next == frame->buffer->start + i + 2);

        // check with masking
        frame->buffer->start[1] = i | 0x80;

        next = NULL;

        for (size_t x = 0; x < i + 6; x++) {

            frame->buffer->length = x;

            testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                    parse_payload_length(frame, &next));
            testrun(next == NULL);
        }

        frame->buffer->length = i + 6;
        testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                parse_payload_length(frame, &next));
        testrun(next == frame->buffer->start + i + 6);
    }

    // check 1byte length max
    next = NULL;
    frame->buffer->start[127] = 'B';
    frame->buffer->length = 135;
    frame->buffer->start[1] = 125;
    frame->buffer->length = 127;
    testrun(DTN_WEBSOCKET_PARSER_SUCCESS == parse_payload_length(frame, &next));
    testrun(next = frame->buffer->start + 127);
    testrun(*next == 'B');
    testrun(*(next - 1) == 'A');
    testrun(*(next + 1) == 'A');

    // check max 1byte length with mask
    next = NULL;
    frame->buffer->start[127] = 'A';
    frame->buffer->start[131] = 'B';
    frame->buffer->start[132] = 'C';
    frame->buffer->start[1] = 125 | 0x80;
    frame->buffer->length = 131;
    testrun(DTN_WEBSOCKET_PARSER_SUCCESS == parse_payload_length(frame, &next));
    testrun(next = frame->buffer->start + 131);
    testrun(*next == 'B');
    testrun(*(next - 1) == 'A');
    testrun(*(next + 1) == 'C');

    // reset
    memset(frame->buffer->start, 'A', frame->buffer->capacity);
    frame->buffer->length = 0;

    // check with length byte 126 (masking and non masking)
    for (size_t i = 126; i < 0xffff; i++) {

        frame->buffer->start[1] = 126;
        frame->buffer->start[2] = i >> 8;
        frame->buffer->start[3] = i;

        next = NULL;

        // we test from i to limit the runtime of the test
        for (size_t x = i; x < i + 3; x++) {

            frame->buffer->length = x;
            testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                    parse_payload_length(frame, &next));
            testrun(next == NULL);
        }

        frame->buffer->length = i + 4;
        testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                parse_payload_length(frame, &next));
        testrun(next == frame->buffer->start + i + 4);

        // check with masking
        next = NULL;

        frame->buffer->start[1] = 126 | 0x80;

        // we test from i to limit the runtime of the test
        for (size_t x = i; x < i + 7; x++) {

            frame->buffer->length = x;
            testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                    parse_payload_length(frame, &next));
            testrun(next == NULL);
        }

        frame->buffer->length = i + 8;
        testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                parse_payload_length(frame, &next));
        testrun(next == frame->buffer->start + i + 8);
    }

    // check with length byte 127 (masking and non masking)

    // NOTE limited size due to linux create error for buffer of uin64_t
    for (size_t i = 0xF000; i < 0xFffff; i += 0x10001) {

        frame->buffer->start[1] = 127;
        frame->buffer->start[2] = i >> 56;
        frame->buffer->start[3] = i >> 48;
        frame->buffer->start[4] = i >> 40;
        frame->buffer->start[5] = i >> 32;
        frame->buffer->start[6] = i >> 24;
        frame->buffer->start[7] = i >> 16;
        frame->buffer->start[8] = i >> 8;
        frame->buffer->start[9] = i;

        if (i < 0x10000) {
            testrun(DTN_WEBSOCKET_PARSER_ERROR ==
                    parse_payload_length(frame, &next));
            continue;
        }

        if (frame->buffer->start[2] & 0x80) {
            testrun(DTN_WEBSOCKET_PARSER_ERROR ==
                    parse_payload_length(frame, &next));
            continue;
        }

        next = NULL;

        // we test from i to limit the runtime of the test
        for (size_t x = i; x < i + 10; x++) {

            frame->buffer->length = x;
            testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                    parse_payload_length(frame, &next));
            testrun(next == NULL);
        }

        frame->buffer->length = i + 10;
        testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                parse_payload_length(frame, &next));
        testrun(next == frame->buffer->start + i + 10);

        // check with masking
        next = NULL;

        frame->buffer->start[1] = 127 | 0x80;

        // we test from i to limit the runtime of the test
        for (size_t x = i; x < i + 14; x++) {

            frame->buffer->length = x;
            testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                    parse_payload_length(frame, &next));
            testrun(next == NULL);
        }

        frame->buffer->length = i + 14;
        testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                parse_payload_length(frame, &next));
        testrun(next == frame->buffer->start + i + 14);
    }

    // check edge case with length byte 127 (masking and non masking)

    // NOTE this test increases the test runtime quite significiant,
    for (size_t i = 0xF0000; i < 0xFffff; i += 0x10001) {

        frame->buffer->start[1] = 127;
        frame->buffer->start[2] = i >> 56;
        frame->buffer->start[3] = i >> 48;
        frame->buffer->start[4] = i >> 40;
        frame->buffer->start[5] = i >> 32;
        frame->buffer->start[6] = i >> 24;
        frame->buffer->start[7] = i >> 16;
        frame->buffer->start[8] = i >> 8;
        frame->buffer->start[9] = i;

        // 64-bit unsigned integer (the most significant bit MUST be 0)
        if (frame->buffer->start[2] & 0x80) {
            testrun(DTN_WEBSOCKET_PARSER_ERROR ==
                    parse_payload_length(frame, &next));
            continue;
        }

        next = NULL;

        // we test from i to limit the runtime of the test
        for (size_t x = i; x < i + 10; x++) {

            frame->buffer->length = x;
            testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                    parse_payload_length(frame, &next));
            testrun(next == NULL);
        }

        frame->buffer->length = i + 10;
        testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                parse_payload_length(frame, &next));
        testrun(next == frame->buffer->start + i + 10);

        // check with masking
        next = NULL;

        frame->buffer->start[1] = 127 | 0x80;

        // we test from i to limit the runtime of the test
        for (size_t x = i; x < i + 14; x++) {

            frame->buffer->length = x;
            testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                    parse_payload_length(frame, &next));
            testrun(next == NULL);
        }

        frame->buffer->length = i + 14;
        testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                parse_payload_length(frame, &next));
        testrun(next == frame->buffer->start + i + 14);
    }

    testrun(NULL == dtn_websocket_frame_free(frame));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_websocket_frame_cast() {

    for (uint16_t i = 0; i < 0xFFFF; i++) {

        if (i == DTN_WEBSOCKET_MAGIC_BYTE) {
            testrun(dtn_websocket_frame_cast(&i));
            testrun(!dtn_http_message_cast(&i));
        } else {
            testrun(!dtn_websocket_frame_cast(&i));
        }
    }

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_websocket_frame_create() {

    dtn_websocket_frame_config config = {0};
    dtn_websocket_frame *frame = dtn_websocket_frame_create(config);
    testrun(frame);
    testrun(dtn_websocket_frame_cast(frame));
    testrun(frame->buffer);
    testrun(frame->config.buffer.default_size ==
            DTN_WEBSOCKET_FRAME_DEFAULT_BUFFER_SIZE);
    testrun(frame->buffer->capacity == DTN_WEBSOCKET_FRAME_DEFAULT_BUFFER_SIZE);
    frame = dtn_websocket_frame_free(frame);

    // check with caching
    dtn_websocket_enable_caching(1);
    testrun(g_cache != NULL);

    dtn_websocket_frame *frame1 = NULL;
    dtn_websocket_frame *frame2 = NULL;
    dtn_websocket_frame *frame3 = NULL;
    dtn_websocket_frame *frame4 = NULL;

    frame = dtn_websocket_frame_create(config);
    testrun(frame);
    testrun(dtn_websocket_frame_cast(frame));

    frame2 = dtn_websocket_frame_create(config);
    testrun(frame2);
    testrun(dtn_websocket_frame_cast(frame2));

    dtn_websocket_frame_free(frame);
    frame3 = dtn_websocket_frame_create(config);
    testrun(frame3);
    testrun(dtn_websocket_frame_cast(frame3));
    testrun(frame3 == frame);

    dtn_websocket_frame_free(frame2);
    frame4 = dtn_websocket_frame_create(config);
    testrun(frame4);
    testrun(dtn_websocket_frame_cast(frame4));
    testrun(frame4 == frame2);

    dtn_websocket_frame_free(frame3);
    dtn_websocket_frame_free(frame4);

    // we check what we get from cache
    frame3 = dtn_registered_cache_get(g_cache);
    testrun(frame3);
    testrun(frame3 == frame);
    frame4 = dtn_registered_cache_get(g_cache);
    testrun(!frame4);

    frame3 = dtn_websocket_frame_free_uncached(frame3);
    testrun(!dtn_registered_cache_get(g_cache));

    frame = NULL;
    frame1 = NULL;
    frame2 = NULL;
    frame3 = NULL;

    config = (dtn_websocket_frame_config){.buffer.default_size = 100};

    frame = dtn_websocket_frame_create(config);
    testrun(frame);
    testrun(frame->buffer->capacity == 100);

    dtn_websocket_frame_free(frame);

    // we request a bigger size
    config = (dtn_websocket_frame_config){.buffer.default_size = 200};

    frame1 = dtn_websocket_frame_create(config);
    testrun(frame1);
    testrun(frame1 == frame);
    testrun(frame->buffer == frame1->buffer);
    testrun(frame1->buffer->capacity == 200);

    dtn_websocket_frame_free(frame1);

    // we request a smaller buffer size

    config = (dtn_websocket_frame_config){.buffer.default_size = 50};

    frame1 = dtn_websocket_frame_create(config);
    testrun(frame1);
    testrun(frame1 == frame);
    testrun(frame->buffer == frame1->buffer);
    testrun(frame1->buffer->capacity == 50);

    frame1->buffer = dtn_buffer_free_uncached(frame1->buffer);
    dtn_websocket_frame_free(frame1);
    frame2 = dtn_websocket_frame_create(config);
    testrun(frame2);
    testrun(frame2 == frame1);
    testrun(frame2->buffer != NULL);
    testrun(frame2->buffer->capacity == 50);

    frame = dtn_websocket_frame_free_uncached(frame);
    dtn_registered_cache_free_all();
    g_cache = NULL;

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_websocket_frame_clear() {

    dtn_websocket_frame_config config = {0};
    dtn_websocket_frame *frame = dtn_websocket_frame_create(config);
    testrun(frame);
    testrun(dtn_websocket_frame_cast(frame));
    testrun(frame->buffer);
    testrun(frame->config.buffer.default_size ==
            DTN_WEBSOCKET_FRAME_DEFAULT_BUFFER_SIZE);
    testrun(frame->buffer->capacity == DTN_WEBSOCKET_FRAME_DEFAULT_BUFFER_SIZE);

    testrun(!dtn_websocket_frame_clear(NULL));
    testrun(dtn_websocket_frame_clear(frame));

    testrun(frame->buffer->capacity == DTN_WEBSOCKET_FRAME_DEFAULT_BUFFER_SIZE);
    testrun(frame->buffer->length == 0);
    testrun(frame->opcode == 0);
    testrun(frame->state == 0);
    testrun(frame->mask == NULL);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);

    char *content = "some content";
    testrun(dtn_buffer_set(frame->buffer, content, strlen(content)));
    testrun(frame->buffer->start[0] != 0);
    testrun(frame->buffer->length != 0);
    frame->opcode = DTN_WEBSOCKET_OPCODE_CLOSE;
    frame->mask = frame->buffer->start;
    frame->content.start = frame->buffer->start;
    frame->content.length = frame->buffer->length;

    testrun(dtn_websocket_frame_clear(frame));

    testrun(frame->buffer->capacity == DTN_WEBSOCKET_FRAME_DEFAULT_BUFFER_SIZE);
    testrun(frame->buffer->length == 0);
    testrun(frame->opcode == 0);
    testrun(frame->state == 0);
    testrun(frame->mask == NULL);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);

    // check with max_recache < buffer->capacity
    frame->config.buffer.max_bytes_recache =
        DTN_WEBSOCKET_FRAME_DEFAULT_BUFFER_SIZE;

    testrun(dtn_websocket_frame_clear(frame));

    testrun(frame->buffer->capacity == DTN_WEBSOCKET_FRAME_DEFAULT_BUFFER_SIZE);
    testrun(frame->buffer->length == 0);
    testrun(frame->opcode == 0);
    testrun(frame->state == 0);
    testrun(frame->mask == NULL);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);

    // check with max_recache > buffer->capacity
    frame->config.buffer.max_bytes_recache =
        DTN_WEBSOCKET_FRAME_DEFAULT_BUFFER_SIZE - 1;

    testrun(dtn_websocket_frame_clear(frame));

    testrun(frame->buffer != NULL);
    testrun(frame->opcode == 0);
    testrun(frame->state == 0);
    testrun(frame->mask == NULL);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);

    testrun(NULL == dtn_websocket_frame_free(frame));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_websocket_frame_free() {

    dtn_websocket_frame_config config = {0};
    dtn_websocket_frame *frame = dtn_websocket_frame_create(config);
    testrun(frame);

    testrun(NULL == dtn_websocket_frame_free(NULL));
    testrun(NULL == dtn_websocket_frame_free(frame));

    // check with caching
    dtn_websocket_enable_caching(1);
    testrun(g_cache != NULL);

    frame = dtn_websocket_frame_create(config);
    testrun(frame);
    testrun(NULL == dtn_websocket_frame_free(frame));

    void *data = dtn_registered_cache_get(g_cache);
    testrun(data);
    testrun(data == frame);

    testrun(NULL == dtn_websocket_frame_free_uncached(frame));
    testrun(!dtn_registered_cache_get(g_cache));

    dtn_registered_cache_free_all();
    g_cache = NULL;

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_websocket_frame_free_uncached() {

    dtn_websocket_frame_config config = {0};
    dtn_websocket_frame *frame = dtn_websocket_frame_create(config);
    testrun(frame);

    testrun(NULL == dtn_websocket_frame_free_uncached(NULL));
    testrun(NULL == dtn_websocket_frame_free_uncached(frame));

    // check with caching
    dtn_websocket_enable_caching(1);
    testrun(g_cache != NULL);

    frame = dtn_websocket_frame_create(config);
    testrun(frame);
    testrun(NULL == dtn_websocket_frame_free_uncached(frame));

    testrun(!dtn_registered_cache_get(g_cache));
    dtn_registered_cache_free_all();
    g_cache = NULL;

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_websocket_parse_frame() {

    uint8_t *next = NULL;
    dtn_websocket_frame_config config = {.buffer.default_size = 0xffFFFF};
    dtn_websocket_frame *frame = dtn_websocket_frame_create(config);
    testrun(frame);

    testrun(DTN_WEBSOCKET_PARSER_ERROR ==
            dtn_websocket_parse_frame(NULL, NULL));
    testrun(DTN_WEBSOCKET_PARSER_ERROR ==
            dtn_websocket_parse_frame(NULL, &next));
    testrun(DTN_WEBSOCKET_PARSER_ERROR ==
            dtn_websocket_parse_frame(frame, NULL));

    frame->buffer->length = 1;
    testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
            dtn_websocket_parse_frame(frame, NULL));

    // check length of 0 with complete structure
    frame->buffer->length = 2;
    testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
            dtn_websocket_parse_frame(frame, &next));
    testrun(next == frame->buffer->start + 2);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);
    testrun(frame->mask == NULL);
    testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_CONTINUATION);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    for (size_t i = 1; i < 2; i++) {
        next = NULL;
        frame->buffer->length = i;
        testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                dtn_websocket_parse_frame(frame, &next));
        testrun(next == NULL);
    }

    for (size_t i = 2; i < 10; i++) {
        frame->buffer->length = i;
        next = NULL;
        testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                dtn_websocket_parse_frame(frame, &next));
        testrun(next == frame->buffer->start + 2);
    }

    frame->buffer->length = 2;

    // fin no set opcode not set
    next = NULL;
    frame->buffer->start[0] = 0;
    testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
            dtn_websocket_parse_frame(frame, &next));
    testrun(next == frame->buffer->start + 2);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);
    testrun(frame->mask == NULL);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_CONTINUATION);

    // fin set
    next = NULL;
    frame->buffer->start[0] = 0x80;
    testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
            dtn_websocket_parse_frame(frame, &next));
    testrun(next == frame->buffer->start + 2);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);
    testrun(frame->mask == NULL);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_LAST);
    testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_CONTINUATION);

    // fin set opcode set
    next = NULL;
    frame->buffer->start[0] = 0x80 | DTN_WEBSOCKET_OPCODE_TEXT;
    testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
            dtn_websocket_parse_frame(frame, &next));
    testrun(next == frame->buffer->start + 2);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);
    testrun(frame->mask == NULL);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_NONE);
    testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_TEXT);

    // fin set opcode set
    next = NULL;
    frame->buffer->start[0] = 0x80 | DTN_WEBSOCKET_OPCODE_BINARY;
    testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
            dtn_websocket_parse_frame(frame, &next));
    testrun(next == frame->buffer->start + 2);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);
    testrun(frame->mask == NULL);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_NONE);
    testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_BINARY);

    // fin set opcode set
    next = NULL;
    frame->buffer->start[0] = 0x80 | DTN_WEBSOCKET_OPCODE_PING;
    testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
            dtn_websocket_parse_frame(frame, &next));
    testrun(next == frame->buffer->start + 2);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);
    testrun(frame->mask == NULL);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_NONE);
    testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_PING);

    // fin no set opcode set
    next = NULL;
    frame->buffer->start[0] = DTN_WEBSOCKET_OPCODE_PONG;
    testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
            dtn_websocket_parse_frame(frame, &next));
    testrun(next == frame->buffer->start + 2);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);
    testrun(frame->mask == NULL);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_START);
    testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_PONG);

    // fin no set opcode set
    next = NULL;
    frame->buffer->start[0] = DTN_WEBSOCKET_OPCODE_CLOSE;
    testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
            dtn_websocket_parse_frame(frame, &next));
    testrun(next == frame->buffer->start + 2);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);
    testrun(frame->mask == NULL);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_START);
    testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_CLOSE);

    // mask set no content
    next = NULL;
    frame->buffer->start[0] = 0;
    frame->buffer->start[1] = 0x80;
    testrun(DTN_WEBSOCKET_PARSER_ERROR ==
            dtn_websocket_parse_frame(frame, &next));

    // mask set no content
    next = NULL;
    frame->buffer->start[0] = DTN_WEBSOCKET_OPCODE_CLOSE;
    frame->buffer->start[1] = 0x80;
    testrun(DTN_WEBSOCKET_PARSER_ERROR ==
            dtn_websocket_parse_frame(frame, &next));

    // check RSV ignore (FIN set OPCODE wrong)
    next = NULL;
    frame->buffer->start[0] = 0xFF;
    frame->buffer->start[1] = 0x00;
    testrun(DTN_WEBSOCKET_PARSER_ERROR ==
            dtn_websocket_parse_frame(frame, &next));

    // check first byte all
    for (int i = 0; i <= 0xff; i++) {

        next = NULL;
        frame->buffer->start[0] = i;

        switch (0x0F & i) {

        case DTN_WEBSOCKET_OPCODE_CONTINUATION:
        case DTN_WEBSOCKET_OPCODE_TEXT:
        case DTN_WEBSOCKET_OPCODE_BINARY:
        case DTN_WEBSOCKET_OPCODE_CLOSE:
        case DTN_WEBSOCKET_OPCODE_PING:
        case DTN_WEBSOCKET_OPCODE_PONG:
            testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                    dtn_websocket_parse_frame(frame, &next));
            break;
        default:
            testrun(DTN_WEBSOCKET_PARSER_ERROR ==
                    dtn_websocket_parse_frame(frame, &next));
        }
    }

    // check size

    // valid 1st byte length < 125
    frame->buffer->start[0] = 0x80 | DTN_WEBSOCKET_OPCODE_TEXT;

    for (size_t i = 1; i < 126; i++) {

        frame->buffer->start[1] = i;

        for (size_t x = i; x < 10; x++) {

            frame->buffer->length = x;
            next = NULL;

            if (x < (i + 2)) {
                testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                        dtn_websocket_parse_frame(frame, &next));
            } else {
                testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                        dtn_websocket_parse_frame(frame, &next));
                testrun(next == frame->buffer->start + i + 2);
                testrun(frame->content.start == frame->buffer->start + 2);
                testrun(frame->content.length == i);
                testrun(frame->mask == NULL);
                testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_NONE);
                testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_TEXT);
            }
        }
    }

    // valid 1st byte length > 125
    frame->buffer->start[0] = 0x80 | DTN_WEBSOCKET_OPCODE_TEXT;

    for (size_t i = 126; i < 0xffff; i += 100) {

        frame->buffer->start[1] = 126;
        frame->buffer->start[2] = i >> 8;
        frame->buffer->start[3] = i;

        for (size_t x = i; x < 10; x++) {

            frame->buffer->length = x;
            next = NULL;

            if (x < (i + 4)) {
                testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                        dtn_websocket_parse_frame(frame, &next));
            } else {
                testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                        dtn_websocket_parse_frame(frame, &next));
                testrun(next == frame->buffer->start + i + 4);
                testrun(frame->content.start == frame->buffer->start + 4);
                testrun(frame->content.length == i);
                testrun(frame->mask == NULL);
                testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_NONE);
                testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_TEXT);
            }
        }

        // set masking
        frame->buffer->start[2] |= 0x80;

        for (size_t x = i; x < 10; x++) {

            frame->buffer->length = x;
            next = NULL;

            if (x < (i + 8)) {
                testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                        dtn_websocket_parse_frame(frame, &next));
            } else {
                testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                        dtn_websocket_parse_frame(frame, &next));
                testrun(next == frame->buffer->start + i + 8);
                testrun(frame->content.start == frame->buffer->start + 8);
                testrun(frame->content.length == i);
                testrun(frame->mask == frame->buffer->start + i + 4);
                testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_NONE);
                testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_TEXT);
            }
        }
    }

    // valid 1st byte length > 125 (edge case)
    frame->buffer->start[0] = 0x80 | DTN_WEBSOCKET_OPCODE_TEXT;
    frame->buffer->start[1] = 126;
    frame->buffer->start[2] = 0xFF;
    frame->buffer->start[3] = 0XFF;

    for (size_t x = 0; x < 10; x++) {

        frame->buffer->length = 0xFFFF + x;
        next = NULL;

        if (x < 4) {
            testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                    dtn_websocket_parse_frame(frame, &next));
        } else {
            testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                    dtn_websocket_parse_frame(frame, &next));
            testrun(next == frame->buffer->start + 0xFFFF + 4);
            testrun(frame->content.start == frame->buffer->start + 4);
            testrun(frame->content.length == 0xFFFF);
            testrun(frame->mask == NULL);
            testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_NONE);
            testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_TEXT);
        }
    }

    // set masking
    frame->buffer->start[1] |= 0x80;

    for (size_t x = 0; x < 10; x++) {

        frame->buffer->length = 0xFFFF + x;
        next = NULL;

        if (x < 8) {
            testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                    dtn_websocket_parse_frame(frame, &next));
        } else {
            testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                    dtn_websocket_parse_frame(frame, &next));
            testrun(next == frame->buffer->start + 0xFFFF + 8);
            testrun(frame->content.start == frame->buffer->start + 8);
            testrun(frame->content.length == 0xFFFF);
            testrun(frame->mask == frame->buffer->start + 4);
            testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_NONE);
            testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_TEXT);
        }
    }

    // valid 1st byte length > 0xFFFF
    frame->buffer->start[0] = 0x80 | DTN_WEBSOCKET_OPCODE_TEXT;

    for (uint64_t i = 0xffff; i < 0xfFFFF; i += 0x10001) {

        frame->buffer->start[1] = 127;
        frame->buffer->start[2] = i >> 56;
        frame->buffer->start[3] = i >> 48;
        frame->buffer->start[4] = i >> 40;
        frame->buffer->start[5] = i >> 32;
        frame->buffer->start[6] = i >> 24;
        frame->buffer->start[7] = i >> 16;
        frame->buffer->start[8] = i >> 8;
        frame->buffer->start[9] = i;

        for (size_t x = i + 6; x < 12; x++) {

            frame->buffer->length = x;
            next = NULL;

            if (x < (i + 10)) {
                testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                        dtn_websocket_parse_frame(frame, &next));
            } else {
                testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                        dtn_websocket_parse_frame(frame, &next));
                testrun(next == frame->buffer->start + i + 10);
                testrun(frame->content.start == frame->buffer->start + 10);
                testrun(frame->content.length == i);
                testrun(frame->mask == NULL);
                testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_NONE);
                testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_TEXT);
            }
        }

        // set masking
        frame->buffer->start[2] |= 0x80;

        for (size_t x = i + 6; x < 16; x++) {

            frame->buffer->length = x;
            next = NULL;

            if (x < (i + 14)) {
                testrun(DTN_WEBSOCKET_PARSER_PROGRESS ==
                        dtn_websocket_parse_frame(frame, &next));
            } else {
                testrun(DTN_WEBSOCKET_PARSER_SUCCESS ==
                        dtn_websocket_parse_frame(frame, &next));
                testrun(next == frame->buffer->start + i + 14);
                testrun(frame->content.start == frame->buffer->start + 14);
                testrun(frame->content.length == i);
                testrun(frame->mask == frame->buffer->start + i + 10);
                testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_NONE);
                testrun(frame->opcode == DTN_WEBSOCKET_OPCODE_TEXT);
            }
        }
    }

    testrun(NULL == dtn_websocket_frame_free(frame));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_generate_masking_key() {

    uint8_t empty[5] = {0};

    uint8_t *array[20] = {0};
    for (size_t i = 0; i < 20; i++) {
        array[i] = calloc(1, 5);
    }

    testrun(!generate_masking_key(NULL, 0));
    testrun(!generate_masking_key(array[0], 0));
    testrun(!generate_masking_key(array[0], 1));
    testrun(!generate_masking_key(array[0], 2));
    testrun(!generate_masking_key(array[0], 3));
    testrun(!generate_masking_key(NULL, 4));

    for (size_t i = 0; i < 20; i++) {
        testrun(generate_masking_key(array[i], 5));
        testrun(0 != memcmp(array[i], &empty, 5));
    }

    for (size_t i = 0; i < 20; i++) {

        for (size_t x = 0; x < 20; x++) {

            if (x == i)
                continue;

            testrun(0 != memcmp(array[i], array[x], 5));
        }
    }

    for (size_t i = 0; i < 20; i++) {
        array[i] = dtn_data_pointer_free(array[i]);
    }

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_mask_data() {

    uint8_t mask[5] = {0};
    uint8_t data[100] = {0};
    uint8_t source[100] = {0};

    testrun(generate_masking_key(mask, 5));
    testrun(dtn_random_bytes(data, 100));
    memcpy(&source, &data, 100);

    testrun(0 == memcmp(&source, &data, 100));

    testrun(!mask_data(NULL, 0, NULL));
    testrun(!mask_data(NULL, 100, mask));
    testrun(!mask_data(data, 0, mask));
    testrun(!mask_data(data, 100, NULL));

    testrun(mask_data(data, 100, mask));

    uint8_t j = 0;

    for (size_t i = 0; i < 100; i++) {
        j = i % 4;
        data[i] = source[i] ^ mask[j];
    }

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_websocket_set_data() {

    dtn_websocket_frame_config config = {.buffer.default_size = 10};

    dtn_websocket_frame *frame = dtn_websocket_frame_create(config);
    testrun(frame);
    testrun(frame->buffer->capacity == 10);

    char *data = "test";
    testrun(!dtn_websocket_set_data(NULL, NULL, 0, false));

    // basically some reset of the content
    testrun(dtn_websocket_set_data(frame, NULL, 0, false));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 10);
    testrun(frame->buffer->length == 2);
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == NULL);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);

    // masking without content requested
    testrun(!dtn_websocket_set_data(frame, NULL, 0, true));

    testrun(
        dtn_websocket_set_data(frame, (uint8_t *)data, strlen(data), false));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 10);
    testrun(frame->buffer->length == 2 + strlen(data));
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == NULL);
    testrun(frame->content.start == frame->buffer->start + 2);
    testrun(frame->content.length == strlen(data));
    testrun(0 == memcmp(data, frame->content.start, frame->content.length));

    testrun(dtn_websocket_set_data(frame, (uint8_t *)data, strlen(data), true));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 10);
    testrun(frame->buffer->length == 6 + strlen(data));
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == frame->buffer->start + 2);
    testrun(frame->content.start == frame->buffer->start + 6);
    testrun(frame->content.length == strlen(data));
    testrun(0 != memcmp(data, frame->content.start, frame->content.length));

    // NOTE only content will be unmaked!
    testrun(dtn_websocket_frame_unmask(frame));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 10);
    testrun(frame->buffer->length == 6 + strlen(data));
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == frame->buffer->start + 2);
    testrun(frame->content.start == frame->buffer->start + 6);
    testrun(frame->content.length == strlen(data));
    testrun(0 == memcmp(data, frame->content.start, frame->content.length));

    // check reset without data
    testrun(dtn_websocket_set_data(frame, NULL, strlen(data), false));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 10);
    testrun(frame->buffer->length == 2);
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == NULL);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);

    data = "something longer as buffer capacity";
    testrun(dtn_websocket_set_data(frame, (uint8_t *)data, strlen(data), true));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity > 10);
    testrun(frame->buffer->capacity == strlen(data) + 6);
    testrun(frame->buffer->length == 6 + strlen(data));
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == frame->buffer->start + 2);
    testrun(frame->content.start == frame->buffer->start + 6);
    testrun(frame->content.length == strlen(data));
    testrun(0 != memcmp(data, frame->content.start, frame->content.length));

    // check reset without data length
    testrun(dtn_websocket_set_data(frame, (uint8_t *)data, 0, false));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == strlen(data) + 6);
    testrun(frame->buffer->length == 2);
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == NULL);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);

    // check with length > 125
    uint8_t buffer[200] = {0};
    memset(buffer, 'A', 200);

    testrun(dtn_websocket_set_data(frame, buffer, 200, false));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 204);
    testrun(frame->buffer->length == 204);
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == NULL);
    testrun(frame->content.start == frame->buffer->start + 4);
    testrun(frame->content.length == 200);
    testrun(0 == memcmp(buffer, frame->content.start, frame->content.length));

    // check with mask
    testrun(dtn_websocket_set_data(frame, buffer, 200, true));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 208);
    testrun(frame->buffer->length == 208);
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == frame->buffer->start + 4);
    testrun(frame->content.start == frame->buffer->start + 8);
    testrun(frame->content.length == 200);
    testrun(0 != memcmp(buffer, frame->content.start, frame->content.length));
    // NOTE only content will be unmaked!
    testrun(dtn_websocket_frame_unmask(frame));
    testrun(frame->buffer->capacity == 208);
    testrun(frame->buffer->length == 208);
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == frame->buffer->start + 4);
    testrun(frame->content.start == frame->buffer->start + 8);
    testrun(frame->content.length == 200);
    testrun(0 == memcmp(buffer, frame->content.start, frame->content.length));

    testrun(NULL == dtn_websocket_frame_free(frame));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_websocket_frame_unmask() {

    dtn_websocket_frame_config config = {.buffer.default_size = 10};

    dtn_websocket_frame *frame = dtn_websocket_frame_create(config);
    testrun(frame);
    testrun(frame->buffer->capacity == 10);

    char *data = "test";

    // create unmasked frame
    testrun(
        dtn_websocket_set_data(frame, (uint8_t *)data, strlen(data), false));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 10);
    testrun(frame->buffer->length == 2 + strlen(data));
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == NULL);
    testrun(frame->content.start == frame->buffer->start + 2);
    testrun(frame->content.length == strlen(data));
    testrun(0 == memcmp(data, frame->content.start, frame->content.length));

    testrun(!dtn_websocket_frame_unmask(NULL));
    testrun(dtn_websocket_frame_unmask(frame));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 10);
    testrun(frame->buffer->length == 2 + strlen(data));
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == NULL);
    testrun(frame->content.start == frame->buffer->start + 2);
    testrun(frame->content.length == strlen(data));
    testrun(0 == memcmp(data, frame->content.start, frame->content.length));

    // created masked frame
    testrun(dtn_websocket_set_data(frame, (uint8_t *)data, strlen(data), true));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 10);
    testrun(frame->buffer->length == 6 + strlen(data));
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == frame->buffer->start + 2);
    testrun(frame->content.start == frame->buffer->start + 6);
    testrun(frame->content.length == strlen(data));
    testrun(0 != memcmp(data, frame->content.start, frame->content.length));
    // unmask content
    testrun(dtn_websocket_frame_unmask(frame));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 10);
    testrun(frame->buffer->length == 6 + strlen(data));
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == frame->buffer->start + 2);
    testrun(frame->content.start == frame->buffer->start + 6);
    testrun(frame->content.length == strlen(data));
    testrun(0 == memcmp(data, frame->content.start, frame->content.length));

    // reset frame
    testrun(dtn_websocket_set_data(frame, NULL, 0, false));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 10);
    testrun(frame->buffer->length == 2);
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == NULL);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);

    testrun(dtn_websocket_frame_unmask(frame));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 10);
    testrun(frame->buffer->length == 2);
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == NULL);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);

    // set mask, but no content
    frame->mask = (uint8_t *)data;
    testrun(!dtn_websocket_frame_unmask(frame));
    testrun(frame->buffer);
    testrun(frame->buffer->capacity == 10);
    testrun(frame->buffer->length == 2);
    testrun(frame->opcode == 0);
    testrun(frame->state == DTN_WEBSOCKET_FRAGMENTATION_CONTINUE);
    testrun(frame->mask == (uint8_t *)data);
    testrun(frame->content.start == NULL);
    testrun(frame->content.length == 0);

    testrun(NULL == dtn_websocket_frame_free(frame));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_websocket_frame_shift_trailing_bytes() {

    dtn_websocket_frame *frame =
        dtn_websocket_frame_create((dtn_websocket_frame_config){0});

    testrun(frame);

    for (size_t i = 0; i < 200; i++) {

        if (i < 100) {
            frame->buffer->start[i] = 'A';
        } else {
            frame->buffer->start[i] = 'B';
        }
    }

    frame->buffer->length = 200;

    dtn_websocket_frame *dest = NULL;
    testrun(!dtn_websocket_frame_shift_trailing_bytes(NULL, NULL, NULL));
    testrun(!dtn_websocket_frame_shift_trailing_bytes(frame, NULL, &dest));
    testrun(!dtn_websocket_frame_shift_trailing_bytes(
        frame, frame->buffer->start + 100, NULL));
    testrun(!dtn_websocket_frame_shift_trailing_bytes(
        NULL, frame->buffer->start + 100, &dest));

    testrun(dtn_websocket_frame_shift_trailing_bytes(
        frame, frame->buffer->start + 100, &dest));
    testrun(dest);

    for (size_t i = 0; i < 200; i++) {

        if (i < 100) {
            testrun(frame->buffer->start[i] == 'A');
            testrun(dest->buffer->start[i] == 'B');
        } else {
            testrun(frame->buffer->start[i] == 0);
            testrun(dest->buffer->start[i] == 0);
        }
    }

    dest = dtn_websocket_frame_free(dest);

    testrun(dtn_websocket_frame_shift_trailing_bytes(
        frame, frame->buffer->start + 100, &dest));
    testrun(dest);

    for (size_t i = 0; i < 200; i++) {

        if (i < 100) {
            testrun(frame->buffer->start[i] == 'A');
            testrun(dest->buffer->start[i] == 0);
        } else {
            testrun(frame->buffer->start[i] == 0);
            testrun(dest->buffer->start[i] == 0);
        }
    }

    dest = dtn_websocket_frame_free(dest);
    frame = dtn_websocket_frame_free(frame);

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

    testrun_test(test_dtn_websocket_generate_secure_websocket_key);
    testrun_test(test_dtn_websocket_generate_secure_accept_key);

    testrun_test(test_dtn_websocket_frame_cast);
    testrun_test(test_dtn_websocket_frame_create);
    testrun_test(test_dtn_websocket_frame_clear);
    testrun_test(test_dtn_websocket_frame_free);
    testrun_test(test_dtn_websocket_frame_free_uncached);

    testrun_test(test_dtn_websocket_process_handshake_request);
    testrun_test(check_fragmentation_state);
    testrun_test(check_parse_payload_length);
    testrun_test(test_dtn_websocket_parse_frame);

    testrun_test(check_generate_masking_key);
    testrun_test(check_mask_data);

    testrun_test(test_dtn_websocket_set_data);
    testrun_test(test_dtn_websocket_frame_unmask);

    testrun_test(test_dtn_websocket_frame_shift_trailing_bytes);

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
