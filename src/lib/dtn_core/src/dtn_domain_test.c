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
        @file           dtn_domain_test.c
        @author         Markus Töpfer

        @date           2020-12-18


        ------------------------------------------------------------------------
*/
#include "dtn_domain.c"
#include <dtn_base/testrun.h>

#include <dtn_base/dtn_item_json.h>

#ifndef DTN_TEST_RESOURCE_DIR
#error "Must provide -D DTN_TEST_RESOURCE_DIR=value while compiling this file."
#endif

#ifndef DTN_TEST_CERT
#error "Must provide -D DTN_TEST_CERT=value while compiling this file."
#endif

#ifndef DTN_TEST_CERT_KEY
#error "Must provide -D DTN_TEST_CERT_KEY=value while compiling this file."
#endif

#ifndef DTN_TEST_CERT_ONE
#error "Must provide -D DTN_TEST_CERT_ONE=value while compiling this file."
#endif

#ifndef DTN_TEST_CERT_ONE_KEY
#error "Must provide -D DTN_TEST_CERT_ONE_KEY=value while compiling this file."
#endif

#ifndef DTN_TEST_CERT_TWO
#error "Must provide -D DTN_TEST_CERT_TWO=value while compiling this file."
#endif

#ifndef DTN_TEST_CERT_TWO_KEY
#error "Must provide -D DTN_TEST_CERT_TWO_KEY=value while compiling this file."
#endif

#include <dtn_base/dtn_file.h>
#include <sys/stat.h>
#include <sys/types.h>

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int test_dtn_domain_config_from_item() {

    dtn_domain_config config = {0};
    dtn_domain_config out = {0};
    dtn_item *cfg = dtn_domain_config_to_item(config);

    out = dtn_domain_config_from_item(NULL);
    testrun(0 == out.name.start[0]);
    testrun(0 == out.path[0]);
    testrun(0 == out.certificate.cert[0]);
    testrun(0 == out.certificate.key[0]);
    testrun(0 == out.certificate.ca.file[0]);
    testrun(0 == out.certificate.ca.path[0]);

    out = dtn_domain_config_from_item(cfg);
    testrun(0 == out.name.start[0]);
    testrun(0 == out.path[0]);
    testrun(0 == out.certificate.cert[0]);
    testrun(0 == out.certificate.key[0]);
    testrun(0 == out.certificate.ca.file[0]);
    testrun(0 == out.certificate.ca.path[0]);
    cfg = dtn_item_free(cfg);

    config = (dtn_domain_config){

        .name = "1",
        .path = "2",
        .certificate.cert = "3",
        .certificate.key = "4",
        .certificate.ca.file = "5",
        .certificate.ca.path = "6"};

    cfg = dtn_domain_config_to_item(config);

    out = dtn_domain_config_from_item(cfg);
    testrun(0 == strcmp("1", (char *)out.name.start));
    testrun(0 == strcmp("2", out.path));
    testrun(0 == strcmp("3", out.certificate.cert));
    testrun(0 == strcmp("4", out.certificate.key));
    testrun(0 == strcmp("5", out.certificate.ca.file));
    testrun(0 == strcmp("6", out.certificate.ca.path));
    cfg = dtn_item_free(cfg);

    config = (dtn_domain_config){

        .name = "1",
        .path = "2",
        .certificate.cert = "3",
        .certificate.key = "4",
    };

    cfg = dtn_domain_config_to_item(config);

    out = dtn_domain_config_from_item(cfg);
    testrun(0 == strcmp("1", (char *)out.name.start));
    testrun(0 == strcmp("2", out.path));
    testrun(0 == strcmp("3", out.certificate.cert));
    testrun(0 == strcmp("4", out.certificate.key));
    testrun(0 == out.certificate.ca.file[0]);
    testrun(0 == out.certificate.ca.path[0]);
    cfg = dtn_item_free(cfg);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_domain_config_to_item() {

    dtn_domain_config config = {0};
    dtn_item *out = dtn_domain_config_to_item(config);
    testrun(out);
    testrun(dtn_item_get(out, "/"
                              "path"));
    testrun(dtn_item_get(out, "/"
                              "name"));
    testrun(dtn_item_get(out, "/"
                              "certificate"));
    testrun(dtn_item_get(out, "/"
                              "certificate"
                              "/"
                              "key"));
    testrun(dtn_item_get(out, "/"
                              "certificate"
                              "/"
                              "file"));
    testrun(dtn_item_get(out, "/"
                              "certificate"
                              "/"
                              "ca"));
    testrun(dtn_item_get(out, "/"
                              "certificate"
                              "/"
                              "ca"
                              "/"
                              "file"));
    testrun(dtn_item_get(out, "/"
                              "certificate"
                              "/"
                              "ca"
                              "/"
                              "path"));

    testrun(dtn_item_is_null(dtn_item_get(out, "/"
                                               "path")));
    testrun(dtn_item_is_null(dtn_item_get(out, "/"
                                               "name")));
    testrun(dtn_item_is_null(dtn_item_get(out, "/"
                                               "certificate"
                                               "/"
                                               "key")));
    testrun(dtn_item_is_null(dtn_item_get(out, "/"
                                               "certificate"
                                               "/"
                                               "file")));
    testrun(dtn_item_is_null(dtn_item_get(out, "/"
                                               "certificate"
                                               "/"
                                               "ca"
                                               "/"
                                               "file")));
    testrun(dtn_item_is_null(dtn_item_get(out, "/"
                                               "certificate"
                                               "/"
                                               "ca"
                                               "/"
                                               "path")));

    out = dtn_item_free(out);

    config = (dtn_domain_config){

        .name.start = "1",
        .path = "2",
        .certificate.cert = "3",
        .certificate.key = "4",
        .certificate.ca.file = "5",
        .certificate.ca.path = "6"};

    out = dtn_domain_config_to_item(config);
    testrun(out);
    testrun(dtn_item_get(out, "/"
                              "path"));
    testrun(dtn_item_get(out, "/"
                              "name"));
    testrun(dtn_item_get(out, "/"
                              "certificate"));
    testrun(dtn_item_get(out, "/"
                              "certificate"
                              "/"
                              "key"));
    testrun(dtn_item_get(out, "/"
                              "certificate"
                              "/"
                              "file"));
    testrun(dtn_item_get(out, "/"
                              "certificate"
                              "/"
                              "ca"));
    testrun(dtn_item_get(out, "/"
                              "certificate"
                              "/"
                              "ca"
                              "/"
                              "file"));
    testrun(dtn_item_get(out, "/"
                              "certificate"
                              "/"
                              "ca"
                              "/"
                              "path"));

    testrun(0 == strcmp("1", dtn_item_get_string(dtn_item_get(out, "/"
                                                                   "name"))));
    testrun(0 == strcmp("2", dtn_item_get_string(dtn_item_get(out, "/"
                                                                   "path"))));
    testrun(0 == strcmp("3", dtn_item_get_string(dtn_item_get(out, "/"
                                                                   "certificate"
                                                                   "/"
                                                                   "file"))));
    testrun(0 == strcmp("4", dtn_item_get_string(dtn_item_get(out, "/"
                                                                   "certificate"
                                                                   "/"
                                                                   "key"))));
    testrun(0 == strcmp("5", dtn_item_get_string(dtn_item_get(out, "/"
                                                                   "certificate"
                                                                   "/"
                                                                   "ca"
                                                                   "/"
                                                                   "file"))));
    testrun(0 == strcmp("6", dtn_item_get_string(dtn_item_get(out, "/"
                                                                   "certificate"
                                                                   "/"
                                                                   "ca"
                                                                   "/"
                                                                   "path"))));
    out = dtn_item_free(out);

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_domain_init() {

    dtn_domain check = (dtn_domain){.magic_byte = DTN_DOMAIN_MAGIC_BYTE};

    dtn_domain domain = {0};
    dtn_domain_config config = (dtn_domain_config){0};

    testrun(!dtn_domain_init(NULL));

    testrun(dtn_domain_init(&domain));

    testrun(0 == memcmp(&domain.config, &config, sizeof(dtn_domain_config)));

    testrun(strcat((char *)domain.config.name.start, "test"));
    testrun(strcat(domain.config.path, "sadadadad"));
    testrun(strcat(domain.config.certificate.cert, "certificate"));
    testrun(strcat(domain.config.certificate.key, "sadadafadasfasfsaf"));
    testrun(strcat(domain.config.certificate.ca.file, "sdsdsdsdsdds"));
    testrun(strcat(domain.config.certificate.ca.path, "fonafnafpnpanfpasnfpn"));

    testrun(0 != memcmp(&check, &domain, sizeof(dtn_domain)));
    testrun(0 != memcmp(&domain.config, &config, sizeof(dtn_domain_config)));

    testrun(dtn_domain_init(&domain));

    testrun(0 == memcmp(&domain.config, &config, sizeof(dtn_domain_config)));
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_domain_config_verify() {

    testrun(dtn_dir_access_to_path(DTN_TEST_RESOURCE_DIR));
    testrun(0 != dtn_file_read_check("/filedoesnotexist"));

    dtn_domain_config config = {0};
    testrun(!dtn_domain_config_verify(NULL));
    testrun(!dtn_domain_config_verify(&config));

    // minimal config
    config = (dtn_domain_config){.name.start = "test",
                                 .path = DTN_TEST_RESOURCE_DIR,
                                 .certificate.cert = DTN_TEST_CERT,
                                 .certificate.key = DTN_TEST_CERT_KEY};
    testrun(dtn_domain_config_verify(&config));

    // no name
    config = (dtn_domain_config){.path = DTN_TEST_RESOURCE_DIR,
                                 .certificate.cert = DTN_TEST_CERT,
                                 .certificate.key = DTN_TEST_CERT_KEY};
    testrun(!dtn_domain_config_verify(&config));

    // invalid path
    config = (dtn_domain_config){.name.start = "test",
                                 .path = "/filedoesnotexist",
                                 .certificate.cert = DTN_TEST_CERT,
                                 .certificate.key = DTN_TEST_CERT_KEY};
    testrun(!dtn_domain_config_verify(&config));

    // invalid cert
    config = (dtn_domain_config){.name.start = "test",
                                 .path = DTN_TEST_RESOURCE_DIR,
                                 .certificate.cert = "/filedoesnotexist",
                                 .certificate.key = DTN_TEST_CERT_KEY};
    testrun(!dtn_domain_config_verify(&config));

    // invalid key
    config = (dtn_domain_config){.name.start = "test",
                                 .path = DTN_TEST_RESOURCE_DIR,
                                 .certificate.cert = DTN_TEST_CERT,
                                 .certificate.key = "/filedoesnotexist"};
    testrun(!dtn_domain_config_verify(&config));

    // invalid ca file
    config = (dtn_domain_config){.name.start = "test",
                                 .path = DTN_TEST_RESOURCE_DIR,
                                 .certificate.cert = DTN_TEST_CERT,
                                 .certificate.key = DTN_TEST_CERT_KEY,
                                 .certificate.ca.file = "/filedoesnotexist"};
    testrun(!dtn_domain_config_verify(&config));

    // valid ca file
    config = (dtn_domain_config){.name.start = "test",
                                 .path = DTN_TEST_RESOURCE_DIR,
                                 .certificate.cert = DTN_TEST_CERT,
                                 .certificate.key = DTN_TEST_CERT_KEY,
                                 .certificate.ca.file = DTN_TEST_CERT};
    testrun(dtn_domain_config_verify(&config));

    // invalid ca path
    config = (dtn_domain_config){.name.start = "test",
                                 .path = DTN_TEST_RESOURCE_DIR,
                                 .certificate.cert = DTN_TEST_CERT,
                                 .certificate.key = DTN_TEST_CERT_KEY,
                                 .certificate.ca.file = DTN_TEST_CERT,
                                 .certificate.ca.path = "/filedoesnotexist"};
    testrun(!dtn_domain_config_verify(&config));

    // valid ca path
    config = (dtn_domain_config){.name.start = "test",
                                 .path = DTN_TEST_RESOURCE_DIR,
                                 .certificate.cert = DTN_TEST_CERT,
                                 .certificate.key = DTN_TEST_CERT_KEY,
                                 .certificate.ca.file = DTN_TEST_CERT,
                                 .certificate.ca.path = DTN_TEST_RESOURCE_DIR};
    testrun(dtn_domain_config_verify(&config));

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_domain_load() {

    const char *file1 = DTN_TEST_RESOURCE_DIR "domain1";
    const char *file2 = DTN_TEST_RESOURCE_DIR "domain2";
    const char *file3 = DTN_TEST_RESOURCE_DIR "domain3";
    const char *file4 = DTN_TEST_RESOURCE_DIR "domain_config_invalid";

    unlink(file1);
    unlink(file2);
    unlink(file3);
    unlink(file4);

    dtn_item *conf = dtn_item_object();
    dtn_item *cert = dtn_item_object();
    testrun(dtn_item_object_set(conf, "certificate", cert));

    dtn_item *val = NULL;
    val = dtn_item_string("domain1");
    testrun(dtn_item_object_set(conf, "name", val));
    val = dtn_item_string(DTN_TEST_RESOURCE_DIR);
    testrun(dtn_item_object_set(conf, "path", val));
    val = dtn_item_string(DTN_TEST_CERT);
    testrun(dtn_item_object_set(cert, "file", val));
    val = dtn_item_string(DTN_TEST_CERT_KEY);
    testrun(dtn_item_object_set(cert, "key", val));
    // dtn_item_dump(stdout, conf);
    char *str = dtn_item_to_json(conf);
    fprintf(stdout, "%s\n", str);
    str = dtn_data_pointer_free(str);

    testrun(dtn_item_json_write_file(file1, conf));

    val = dtn_item_string("domain2");
    testrun(dtn_item_object_set(conf, "name", val));
    testrun(dtn_item_json_write_file(file2, conf));

    val = dtn_item_string("domain3");
    testrun(dtn_item_object_set(conf, "name", val));
    testrun(dtn_item_json_write_file(file3, conf));

    testrun(dtn_dir_access_to_path(DTN_TEST_RESOURCE_DIR));
    testrun(0 == dtn_file_read_check(file1));
    testrun(0 == dtn_file_read_check(file2));
    testrun(0 == dtn_file_read_check(file3));
    testrun(0 == dtn_file_read_check(DTN_TEST_RESOURCE_DIR "/dummy"));
    // file 4 not written yet
    testrun(0 != dtn_file_read_check(file4));

    size_t size = 0;
    dtn_domain *array = NULL;

    testrun(!dtn_domain_load(NULL, NULL, NULL));
    testrun(!dtn_domain_load(NULL, &size, &array));
    testrun(!dtn_domain_load(DTN_TEST_RESOURCE_DIR, NULL, &array));
    testrun(!dtn_domain_load(DTN_TEST_RESOURCE_DIR, &size, NULL));

    testrun(0 == size);
    testrun(NULL == array);
    testrun(dtn_domain_load(DTN_TEST_RESOURCE_DIR, &size, &array));

    testrun(3 == size);
    testrun(NULL != array);

    // check domain configs
    for (size_t i = 0; i < size; i++) {

        // order may be different due to keying mechanism of JSON
        if (0 != strcmp((char *)array[i].config.name.start, "domain1"))
            if (0 != strcmp((char *)array[i].config.name.start, "domain2"))
                testrun(0 ==
                        strcmp((char *)array[i].config.name.start, "domain3"));

        testrun(0 == strcmp(array[i].config.path, DTN_TEST_RESOURCE_DIR));
        testrun(0 == strcmp(array[i].config.certificate.cert, DTN_TEST_CERT));
        testrun(0 ==
                strcmp(array[i].config.certificate.key, DTN_TEST_CERT_KEY));
        testrun(0 == array[i].config.certificate.ca.file[0]);
        testrun(0 == array[i].config.certificate.ca.path[0]);
    }

    // check with array pointer
    testrun(!dtn_domain_load(DTN_TEST_RESOURCE_DIR, &size, &array));
    array = dtn_domain_array_free(size, array);
    size = 0;

    // check with authority
    dtn_item *auth = dtn_item_object();
    testrun(dtn_item_object_set(cert, "ca", auth));
    val = dtn_item_string(DTN_TEST_CERT);
    testrun(dtn_item_object_set(auth, "file", val));
    testrun(dtn_item_json_write_file(file3, conf));

    testrun(dtn_domain_load(DTN_TEST_RESOURCE_DIR, &size, &array));

    testrun(3 == size);
    testrun(NULL != array);

    // check domain configs
    for (size_t i = 0; i < size; i++) {

        // order may be different due to keying mechanism of JSON
        if (0 != strcmp((char *)array[i].config.name.start, "domain1"))
            if (0 != strcmp((char *)array[i].config.name.start, "domain2"))
                testrun(0 ==
                        strcmp((char *)array[i].config.name.start, "domain3"));

        testrun(0 == strcmp(array[i].config.path, DTN_TEST_RESOURCE_DIR));
        testrun(0 == strcmp(array[i].config.certificate.cert, DTN_TEST_CERT));
        testrun(0 ==
                strcmp(array[i].config.certificate.key, DTN_TEST_CERT_KEY));

        if (0 != strcmp((char *)array[i].config.name.start, "domain3")) {
            testrun(0 == array[i].config.certificate.ca.file[0]);
            testrun(0 == array[i].config.certificate.ca.path[0]);
        } else {
            testrun(0 ==
                    strcmp(array[i].config.certificate.ca.file, DTN_TEST_CERT));
            testrun(0 == array[i].config.certificate.ca.path[0]);
        }
    }

    array = dtn_domain_array_free(size, array);
    size = 0;

    // check with invalid config
    val = dtn_item_string("/filedoesnotexist");
    testrun(dtn_item_object_set(conf, "path", val));
    testrun(dtn_item_json_write_file(file4, conf));

    testrun(dtn_dir_access_to_path(DTN_TEST_RESOURCE_DIR));
    testrun(0 == dtn_file_read_check(file1));
    testrun(0 == dtn_file_read_check(file2));
    testrun(0 == dtn_file_read_check(file3));
    testrun(0 == dtn_file_read_check(DTN_TEST_RESOURCE_DIR "/dummy"));
    testrun(0 == dtn_file_read_check(file4));
    testrun(!dtn_domain_load(DTN_TEST_RESOURCE_DIR, &size, &array));
    testrun(0 == size);
    testrun(NULL == array);

    // reverify with valid path in file4
    val = dtn_item_string(DTN_TEST_RESOURCE_DIR);
    testrun(dtn_item_object_set(conf, "path", val));
    testrun(dtn_item_json_write_file(file4, conf));
    testrun(dtn_domain_load(DTN_TEST_RESOURCE_DIR, &size, &array));
    testrun(4 == size);
    testrun(NULL != array);
    array = dtn_domain_array_free(size, array);

    // test without any JSON config
    unlink(file1);
    unlink(file2);
    unlink(file3);
    unlink(file4);

    testrun(dtn_dir_access_to_path(DTN_TEST_RESOURCE_DIR));
    testrun(0 != dtn_file_read_check(file1));
    testrun(0 != dtn_file_read_check(file2));
    testrun(0 != dtn_file_read_check(file3));
    testrun(0 == dtn_file_read_check(DTN_TEST_RESOURCE_DIR "/dummy"));
    testrun(0 != dtn_file_read_check(file4));

    size = 150;
    testrun(!dtn_domain_load(DTN_TEST_RESOURCE_DIR, &size, &array));
    testrun(0 == size);
    testrun(NULL == array);

    size = 0;
    testrun(!dtn_domain_load(DTN_TEST_RESOURCE_DIR, &size, &array));
    testrun(0 == size);
    testrun(NULL == array);

    conf = dtn_item_free(conf);
    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_domain_array_clean() {

    dtn_domain domain = {.magic_byte = DTN_DOMAIN_MAGIC_BYTE};
    dtn_domain array[10] = {0};

    for (size_t i = 0; i < 10; i++) {
        testrun(dtn_domain_init(&array[i]));
        testrun(strcat((char *)&array[i].config.name.start, "test"));
        testrun(strcat(array[i].config.path, "sadadadad"));
        testrun(strcat(array[i].config.certificate.cert, "certificate"));
        testrun(strcat(array[i].config.certificate.key, "sadadafadasfasfsaf"));
        testrun(strcat(array[i].config.certificate.ca.file, "sdsdsdsdsdds"));
        testrun(strcat(array[i].config.certificate.ca.path,
                       "fonafnafpnpanfpasnfpn"));
    }

    dtn_domain *arr = array;

    testrun(!dtn_domain_array_clean(0, NULL));
    testrun(dtn_domain_array_clean(0, arr));
    testrun(dtn_domain_array_clean(5, arr));

    for (size_t i = 0; i < 10; i++) {

        if (i < 5) {
            testrun(0 == memcmp(&array[i], &domain, sizeof(dtn_domain)));
        } else {
            testrun(0 != memcmp(&array[i], &domain, sizeof(dtn_domain)));
        }
    }

    testrun(dtn_domain_array_clean(10, arr));

    for (size_t i = 0; i < 10; i++) {

        testrun(0 == memcmp(&array[i], &domain, sizeof(dtn_domain)));
    }

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_domain_array_free() {

    dtn_domain *array = calloc(10, sizeof(dtn_domain));

    for (size_t i = 0; i < 10; i++) {
        testrun(dtn_domain_init(&array[i]));
        testrun(strcat((char *)array[i].config.name.start, "test"));
        testrun(strcat(array[i].config.path, "sadadadad"));
        testrun(strcat(array[i].config.certificate.cert, "certificate"));
        testrun(strcat(array[i].config.certificate.key, "sadadafadasfasfsaf"));
        testrun(strcat(array[i].config.certificate.ca.file, "sdsdsdsdsdds"));
        testrun(strcat(array[i].config.certificate.ca.path,
                       "fonafnafpnpanfpasnfpn"));
    }

    array = dtn_domain_array_free(10, array);

    testrun_log("... check valgrind clean run");

    return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_domain_deinit_tls_context() {

    dtn_domain domain = {0};

    domain.context.tls = SSL_CTX_new(TLS_server_method());
    testrun(domain.context.tls);
    dtn_domain_deinit_tls_context(NULL);

    testrun(domain.context.tls);
    dtn_domain_deinit_tls_context(&domain);
    testrun(NULL == domain.context.tls);

    dtn_domain_deinit_tls_context(&domain);
    testrun(NULL == domain.context.tls);

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

    testrun_test(test_dtn_domain_init);
    testrun_test(test_dtn_domain_config_verify);

    testrun_test(test_dtn_domain_load);

    testrun_test(test_dtn_domain_array_clean);
    testrun_test(test_dtn_domain_array_free);

    testrun_test(test_dtn_domain_deinit_tls_context);

    testrun_test(test_dtn_domain_config_from_item);
    testrun_test(test_dtn_domain_config_to_item);

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
