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
        @file           dtn_item_json_test.c
        @author         Töpfer, Markus

        @date           2025-11-29


        ------------------------------------------------------------------------
*/
#include "../include/testrun.h"
#include "dtn_item_json.c"

#include "../include/dtn_string.h"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int check_json_decode() {

    dtn_item *value = NULL;
    char *string = NULL;

    testrun(-1 == json_decode(NULL, NULL, 0));
    testrun(-1 == json_decode(NULL, "{}", 2));
    testrun(-1 == json_decode(&value, NULL, 2));
    testrun(-1 == json_decode(&value, "{}", 0));
    testrun(-1 == json_decode(&value, "{}", 1));
    testrun(2 == json_decode(&value, "{}", 2));
    testrun(dtn_item_is_object(value));
    testrun(dtn_item_is_empty(value));
    value = dtn_item_free(value);

    testrun(0 == value);

    // parse to non existing object value
    testrun(2 == json_decode(&value, "{}", 2));
    testrun(dtn_item_is_object(value));
    testrun(dtn_item_is_empty(value));
    // parse to existing object value
    testrun(-1 == json_decode(&value, "{}", 100));
    value = dtn_item_free(value);
    testrun(2 == json_decode(&value, "{}", 100));
    testrun(dtn_item_is_object(value));
    testrun(dtn_item_is_empty(value));
    // parse with whitespace
    value = dtn_item_free(value);
    testrun(3 == json_decode(&value, " {}", 100));
    testrun(dtn_item_is_object(value));
    testrun(dtn_item_is_empty(value));
    value = dtn_item_free(value);
    testrun(2 == json_decode(&value, "{} ", 100));
    testrun(dtn_item_is_object(value));
    testrun(dtn_item_is_empty(value));
    value = dtn_item_free(value);
    testrun(3 == json_decode(&value, " {} ", 100));
    testrun(dtn_item_is_object(value));
    testrun(dtn_item_is_empty(value));
    value = dtn_item_free(value);
    testrun(6 == json_decode(&value, "\r\n\t {} ", 100));
    testrun(dtn_item_is_object(value));
    testrun(dtn_item_is_empty(value));
    value = dtn_item_free(value);
    // with content
    testrun(9 == json_decode(&value, "{\"key\":1}", 100));
    testrun(dtn_item_is_object(value));
    testrun(!dtn_item_is_empty(value));

    // parse to non existing array value
    value = dtn_item_free(value);
    testrun(2 == json_decode(&value, "[]", 2));
    testrun(dtn_item_is_array(value));
    testrun(dtn_item_is_empty(value));
    // parse to existing array value
    testrun(-1 == json_decode(&value, "[]", 100));
    value = dtn_item_free(value);
    testrun(2 == json_decode(&value, "[]", 100));
    testrun(dtn_item_is_array(value));
    testrun(dtn_item_is_empty(value));
    value = dtn_item_free(value);
    // parse with whitespace
    testrun(3 == json_decode(&value, " []", 100));
    testrun(dtn_item_is_array(value));
    testrun(dtn_item_is_empty(value));
    value = dtn_item_free(value);
    testrun(2 == json_decode(&value, "[] ", 100));
    testrun(dtn_item_is_array(value));
    testrun(dtn_item_is_empty(value));
    value = dtn_item_free(value);
    testrun(3 == json_decode(&value, " [] ", 100));
    testrun(dtn_item_is_array(value));
    testrun(dtn_item_is_empty(value));
    value = dtn_item_free(value);
    testrun(6 == json_decode(&value, "\r\n\t [] ", 100));
    testrun(dtn_item_is_array(value));
    testrun(dtn_item_is_empty(value));
    value = dtn_item_free(value);
    // with content
    testrun(3 == json_decode(&value, "[1]", 100));
    testrun(dtn_item_is_array(value));
    testrun(!dtn_item_is_empty(value));
    value = dtn_item_free(value);

    // parse to non existing string value
    value = dtn_item_free(value);
    testrun(3 == json_decode(&value, "\"1\"", 3));
    testrun(dtn_item_is_string(value));
    value = dtn_item_free(value);

    // parse to non existing number value
    value = dtn_item_free(value);
    testrun(1 == json_decode(&value, "1,", 2));
    testrun(dtn_item_is_number(value));
    testrun(1 == dtn_item_get_number(value));
    // parse to existing array value
    testrun(1 == json_decode(&value, "1,", 100));
    testrun(dtn_item_is_number(value));
    testrun(1 == dtn_item_get_number(value));
    // parse with whitespace
    testrun(2 == json_decode(&value, " 1,", 100));
    testrun(dtn_item_is_number(value));
    testrun(1 == dtn_item_get_number(value));
    testrun(1 == json_decode(&value, "1, ", 100));
    testrun(dtn_item_is_number(value));
    testrun(1 == dtn_item_get_number(value));
    testrun(2 == json_decode(&value, " 1, ", 100));
    testrun(dtn_item_is_number(value));
    testrun(1 == dtn_item_get_number(value));
    testrun(5 == json_decode(&value, "\r\n\t 1,", 100));
    testrun(dtn_item_is_number(value));
    testrun(1 == dtn_item_get_number(value));
    // parse to different value
    testrun(-1 == json_decode(&value, "{}", 100));
    value = dtn_item_free(value);
    // check number MUST be part of a value
    // testrun(-1 == json_decode(&value, "1e", 100));
    // testrun(-1 == json_decode(&value, "1x", 100));
    // testrun(-1 == json_decode(&value, "1g", 100));
    // testrun(-1 == json_decode(&value, "1e ", 100));

    // valid following items
    testrun(1 == json_decode(&value, "1,", 100));
    testrun(dtn_item_is_number(value));
    testrun(1 == dtn_item_get_number(value));
    testrun(1 == json_decode(&value, "1]", 100));
    testrun(dtn_item_is_number(value));
    testrun(1 == dtn_item_get_number(value));
    testrun(1 == json_decode(&value, "1}", 100));
    testrun(dtn_item_is_number(value));
    testrun(1 == dtn_item_get_number(value));
    testrun(1 == json_decode(&value, "1 ,", 100));
    testrun(dtn_item_is_number(value));
    testrun(1 == dtn_item_get_number(value));
    testrun(1 == json_decode(&value, "1\r\n\t ,\r\n\t ", 100));
    testrun(dtn_item_is_number(value));
    testrun(1 == dtn_item_get_number(value));

    // parse to non existing literal value
    value = dtn_item_free(value);
    testrun(4 == json_decode(&value, "null", 4));
    testrun(dtn_item_is_null(value));
    value = dtn_item_free(value);
    testrun(4 == json_decode(&value, "true", 100));
    testrun(dtn_item_is_true(value));
    value = dtn_item_free(value);
    testrun(5 == json_decode(&value, "false", 100));
    testrun(dtn_item_is_false(value));
    value = dtn_item_free(value);
    // parse with whitespace
    testrun(5 == json_decode(&value, " null", 100));
    testrun(dtn_item_is_null(value));
    value = dtn_item_free(value);
    testrun(4 == json_decode(&value, "true ", 100));
    testrun(dtn_item_is_true(value));
    value = dtn_item_free(value);
    testrun(6 == json_decode(&value, " false ", 100));
    testrun(dtn_item_is_false(value));
    value = dtn_item_free(value);
    testrun(8 == json_decode(&value, "\r\n\t true", 100));
    testrun(dtn_item_is_true(value));
    value = dtn_item_free(value);

    // some decode tests
    string = "{}";
    testrun(2 == json_decode(&value, string, strlen(string)));
    testrun(dtn_item_is_object(value));
    value = dtn_item_free(value);

    string = "[]";
    testrun(2 == json_decode(&value, string, strlen(string)));
    testrun(dtn_item_is_array(value));
    value = dtn_item_free(value);

    string = "\"abc\"";
    testrun(5 == json_decode(&value, string, strlen(string)));
    testrun(dtn_item_is_string(value));
    value = dtn_item_free(value);

    string = "-1,";
    testrun(2 == json_decode(&value, string, strlen(string)));
    testrun(dtn_item_is_number(value));
    value = dtn_item_free(value);

    string = "-1.1e-12,";
    testrun(8 == json_decode(&value, string, strlen(string)));
    testrun(dtn_item_is_number(value));
    value = dtn_item_free(value);

    string = "1.2E10,";
    testrun(6 == json_decode(&value, string, strlen(string)));
    testrun(dtn_item_is_number(value));
    value = dtn_item_free(value);

    string = "null";
    testrun(4 == json_decode(&value, string, strlen(string)));
    testrun(dtn_item_is_null(value));
    value = dtn_item_free(value);

    string = "true";
    testrun(4 == json_decode(&value, string, strlen(string)));
    testrun(dtn_item_is_true(value));
    value = dtn_item_free(value);

    string = "false";
    testrun(5 == json_decode(&value, string, strlen(string)));
    testrun(dtn_item_is_false(value));
    value = dtn_item_free(value);

    string = "{\"key\":[ null, true,1, 1212 \n\r, 2]}";
    testrun(strlen(string) ==
            (size_t)json_decode(&value, string, strlen(string)));
    testrun(dtn_item_is_object(value));
    testrun(1 == dtn_item_count(value));
    testrun(5 == dtn_item_count(dtn_item_object_get(value, "key")));
    testrun(dtn_item_is_null(dtn_item_get(value, "/key/0")));
    testrun(dtn_item_is_true(dtn_item_get(value, "/key/1")));
    testrun(dtn_item_is_number(dtn_item_get(value, "/key/2")));
    testrun(dtn_item_is_number(dtn_item_get(value, "/key/3")));
    testrun(dtn_item_is_number(dtn_item_get(value, "/key/4")));
    value = dtn_item_free(value);

    // Double check that parsing works with ordinary strings
    string = "{\"key\":\"abc\"}";
    testrun(strlen(string) ==
            (size_t)json_decode(&value, string, strlen(string)));
    value = dtn_item_free(value);

    // Parse quoted backslashes
    string = "{\"key\":\"abc\\\\def\"}";
    testrun(strlen(string) ==
            (size_t)json_decode(&value, string, strlen(string)));

    value = dtn_item_free(value);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_from_json() {

    dtn_item *item = NULL;
    testrun(NULL == dtn_item_from_json(""));

    item = dtn_item_from_json("{}");
    testrun(dtn_item_is_object(item));
    testrun(dtn_item_is_empty(item));
    item = dtn_item_free(item);

    item = dtn_item_from_json("[]");
    testrun(dtn_item_is_array(item));
    testrun(dtn_item_is_empty(item));
    item = dtn_item_free(item);

    item = dtn_item_from_json("\"a\"");
    testrun(dtn_item_is_string(item));
    item = dtn_item_free(item);

    item = dtn_item_from_json("1");
    testrun(dtn_item_is_number(item));
    item = dtn_item_free(item);

    item = dtn_item_from_json("1e");
    testrun(!item);
    item = dtn_item_free(item);

    item = dtn_item_from_json("null");
    testrun(dtn_item_is_null(item));
    item = dtn_item_free(item);

    item = dtn_item_from_json("true");
    testrun(dtn_item_is_true(item));
    item = dtn_item_free(item);

    item = dtn_item_from_json("false");
    testrun(dtn_item_is_false(item));
    item = dtn_item_free(item);

    item = dtn_item_from_json("{\"abc\":[1,2,[3,4,5, true, false, null]]}");
    testrun(dtn_item_is_object(item));
    testrun(!dtn_item_is_empty(item));
    item = dtn_item_free(item);

    item = dtn_item_from_json("{\"abc\":\"test\"}");
    testrun(dtn_item_is_object(item));
    testrun(!dtn_item_is_empty(item));
    testrun_log("%s", dtn_item_to_json(item));
    testrun(0 ==
            strcmp("test", dtn_item_get_string(dtn_item_get(item, "/abc"))));
    item = dtn_item_free(item);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_parser_calculate_value() {

    // check parsing
    char *expect = NULL;
    dtn_item *value = dtn_item_object();
    dtn_item_json_stringify_config conf_minimal =
        dtn_item_json_config_stringify_minimal();
    dtn_item_json_stringify_config conf_default =
        dtn_item_json_config_stringify_default();

    expect = "{}";

    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_minimal));
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));

    testrun(-1 == parser_calculate(NULL, NULL));
    testrun(-1 == parser_calculate(NULL, &conf_minimal));
    testrun(strlen(expect) == (size_t)parser_calculate(value, NULL));

    testrun(dtn_item_object_set(value, "1", dtn_item_number(1)));
    expect = "{\"1\":1}";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_minimal));
    expect = "{\n"
             "\t\"1\":1\n"
             "}";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));
    value = dtn_item_free(value);

    value = dtn_item_null();
    expect = "null";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));
    value = dtn_item_free(value);

    value = dtn_item_true();
    expect = "true";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));
    value = dtn_item_free(value);

    value = dtn_item_false();
    expect = "false";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));
    value = dtn_item_free(value);

    value = dtn_item_string("abc");
    expect = "\"abc\"";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));
    value = dtn_item_free(value);

    value = dtn_item_number(123);
    expect = "123";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));
    value = dtn_item_free(value);

    value = dtn_item_number(-1.23e5);
    expect = "-1.23e5";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));
    value = dtn_item_free(value);

    value = dtn_item_array();
    expect = "[]";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));
    testrun(dtn_item_array_push(value, dtn_item_number(1)));
    testrun(dtn_item_array_push(value, dtn_item_number(2)));
    testrun(dtn_item_array_push(value, dtn_item_number(3)));
    expect = "[1,2,3]";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_minimal));
    expect = "[\n"
             "\t1,\n"
             "\t2,\n"
             "\t3\n"
             "]";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));
    value = dtn_item_free(value);

    value = dtn_item_object();
    expect = "{}";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));
    testrun(dtn_item_object_set(value, "1", dtn_item_number(1)));
    testrun(dtn_item_object_set(value, "2", dtn_item_number(2)));
    testrun(dtn_item_object_set(value, "3", dtn_item_number(3)));
    expect = "{\"1\":1,\"2\":2,\"3\":3}";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_minimal));
    expect = "{\n"
             "\t\"1\":1,\n"
             "\t\"2\":2,\n"
             "\t\"3\":3\n"
             "}";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));
    value = dtn_item_free(value);

    // check with depth
    value = dtn_item_object();
    expect = "{}";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));
    testrun(dtn_item_object_set(value, "1", dtn_item_object()));
    testrun(dtn_item_object_set(value, "2", dtn_item_array()));
    testrun(dtn_item_object_set(value, "3", dtn_item_object()));
    expect = "{\"1\":{},\"2\":[],\"3\":{}}";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_minimal));
    expect = "{\n"
             "\t\"1\":{},\n"
             "\t\"2\":[],\n"
             "\t\"3\":{}\n"
             "}";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));
    dtn_item *child = dtn_item_object_get(value, "1");
    testrun(child);
    testrun(dtn_item_object_set(child, "1.1", dtn_item_true()));
    testrun(dtn_item_object_set(child, "1.2", dtn_item_array()));
    testrun(dtn_item_object_set(child, "1.3", dtn_item_number(1)));
    expect = "{\n"
             "\t\"1\":\n"
             "\t{\n"
             "\t\t\"1.1\":true,\n"
             "\t\t\"1.2\":[],\n"
             "\t\t\"1.3\":1\n"
             "\t},\n"
             "\t\"2\":[],\n"
             "\t\"3\":{}\n"
             "}";

    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));

    child = dtn_item_object_get(value, "2");
    testrun(dtn_item_array_push(child, dtn_item_true()));
    testrun(dtn_item_array_push(child, dtn_item_array()));
    testrun(dtn_item_array_push(child, dtn_item_number(1)));
    expect = "{\n"
             "\t\"1\":\n"
             "\t{\n"
             "\t\t\"1.1\":true,\n"
             "\t\t\"1.2\":[],\n"
             "\t\t\"1.3\":1\n"
             "\t},\n"
             "\t\"2\":\n"
             "\t[\n"
             "\t\ttrue,\n"
             "\t\t[],\n"
             "\t\t1\n"
             "\t],\n"
             "\t\"3\":{}\n"
             "}";
    testrun(strlen(expect) == (size_t)parser_calculate(value, &conf_default));
    value = dtn_item_free(value);

    dtn_item_free(value);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_item_to_json_with_config() {

    dtn_item *item = dtn_item_object();
    dtn_item *obj = dtn_item_object();
    testrun(dtn_item_object_set(obj, "1", dtn_item_number(1)));
    testrun(dtn_item_object_set(obj, "2", dtn_item_number(2)));
    testrun(dtn_item_object_set(obj, "3", dtn_item_number(3)));
    dtn_item *arr = dtn_item_array();
    testrun(dtn_item_array_push(arr, dtn_item_true()));
    testrun(dtn_item_array_push(arr, dtn_item_false()));
    testrun(dtn_item_array_push(arr, dtn_item_null()));
    testrun(dtn_item_array_push(arr, dtn_item_array()));
    testrun(dtn_item_array_push(arr, dtn_item_object()));
    testrun(dtn_item_array_push(arr, dtn_item_number(1.23e5)));
    testrun(dtn_item_array_push(arr, dtn_item_string("test")));
    testrun(dtn_item_object_set(item, "1", obj));
    testrun(dtn_item_object_set(item, "2", arr));
    testrun(dtn_item_object_set(item, "3", dtn_item_true()));
    testrun(dtn_item_object_set(item, "4", dtn_item_false()));
    testrun(dtn_item_object_set(item, "5", dtn_item_null()));
    testrun(dtn_item_object_set(item, "6", dtn_item_array()));
    testrun(dtn_item_object_set(item, "7", dtn_item_object()));
    testrun(dtn_item_object_set(item, "8", dtn_item_number(1.6)));
    testrun(dtn_item_object_set(item, "9", dtn_item_string("test")));

    char *string = dtn_item_to_json_with_config(
        item, dtn_item_json_config_stringify_default());

    char *expect = "{\n"
                   "\t\"1\":\n"
                   "\t{\n"
                   "\t\t\"1\":1,\n"
                   "\t\t\"2\":2,\n"
                   "\t\t\"3\":3\n"
                   "\t},\n"
                   "\t\"2\":\n"
                   "\t[\n"
                   "\t\ttrue,\n"
                   "\t\tfalse,\n"
                   "\t\tnull,\n"
                   "\t\t[],\n"
                   "\t\t{},\n"
                   "\t\t123000,\n"
                   "\t\t\"test\"\n"
                   "\t],\n"
                   "\t\"3\":true,\n"
                   "\t\"4\":false,\n"
                   "\t\"5\":null,\n"
                   "\t\"6\":[],\n"
                   "\t\"7\":{},\n"
                   "\t\"8\":1.6,\n"
                   "\t\"9\":\"test\"\n"
                   "}";

    // testrun_log("\n%s\n%s", string, expect);

    testrun(0 == dtn_string_compare(string, expect));
    free(string);

    testrun(NULL == dtn_item_free(item));
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
    testrun_test(check_json_decode);
    testrun_test(test_dtn_item_from_json);

    testrun_test(check_parser_calculate_value);
    testrun_test(test_dtn_item_to_json_with_config);

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
