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

        @author         Michael J. Beer, DLR/GSOC
        @date           2021-09-14

        ------------------------------------------------------------------------
*/

#include "dtn_config_log.c"

#include "../include/dtn_dir.h"
#include "../include/dtn_utils.h"
#include "../include/dtn_file.h"
#include "../include/testrun.h"
#include "../include/dtn_item_json.h"
#include "../include/dtn_log.h"

bool test_disable = true;

/*----------------------------------------------------------------------------*/

static bool output_empty(dtn_log_output out) {

  if (0 != out.filehandle) {
    return false;
  }

  if (0 != out.format) {
    return false;
  }

  if (out.use.systemd) {
    return false;
  }

  return true;
}

/*----------------------------------------------------------------------------*/

static void lets_log_something(dtn_log_level level) {

  dtn_log_ng(level, __FILE__, __FUNCTION__, __LINE__, "LOGGED SOMETHIN");
}

/*----------------------------------------------------------------------------*/

#define OUR_TEMP_DIR "/tmp/dtn_config_log"

/*****************************************************************************
                                     Files
 ****************************************************************************/

static bool file_object_empty(FILE *f) {

  long old_pos = ftell(f);
  DTN_ASSERT(0 <= old_pos);

  int retval = fseek(f, 0, SEEK_END);
  DTN_ASSERT(0 <= retval);

  bool is_empty = 0 == ftell(f);

  retval = fseek(f, old_pos, SEEK_SET);
  DTN_ASSERT(0 <= retval);

  return is_empty;
}

/*----------------------------------------------------------------------------*/

static bool file_empty(char *path) {

  FILE *f = fopen(path, "r");

  if (0 == f)
    return true;

  bool empty = file_object_empty(f);

  fclose(f);

  return empty;
}

/*----------------------------------------------------------------------------*/

static bool file_object_clear(FILE *f) {
  int retval = fseek(f, 0, SEEK_SET);
  DTN_ASSERT(0 <= retval);
  return 0 == ftruncate(fileno(f), 0);
}

/*----------------------------------------------------------------------------*/

static bool file_clear(char const *path) {

  FILE *f = fopen(path, "w+");
  if (0 == f)
    return false;
  bool clear = file_object_clear(f);
  fclose(f);

  return clear;
}

/*****************************************************************************
                                     STDOUT
 ****************************************************************************/

#define STDOUT_FILE OUR_TEMP_DIR "/stdout"

static bool stdout_empty() { return file_object_empty(stdout); }

/*----------------------------------------------------------------------------*/

static bool stdout_clear() { return file_object_clear(stdout); }

/*----------------------------------------------------------------------------*/

static bool stdout_redirect() {
  return stdout == freopen(STDOUT_FILE, "w+", stdout);
}

/*****************************************************************************
                                     STDERR
 ****************************************************************************/

#define STDERR_FILE OUR_TEMP_DIR "/stderr"

static bool stderr_empty() { return file_object_empty(stderr); }

/*----------------------------------------------------------------------------*/

static bool stderr_clear() { return file_object_clear(stderr); }

/*----------------------------------------------------------------------------*/

static bool stderr_redirect() {
  return stderr == freopen(STDERR_FILE, "w+", stderr);
}

/*----------------------------------------------------------------------------*/

static dtn_item *json_from_string(char const *str) {
  DTN_ASSERT(0 != str);
  return dtn_item_from_json(str);
}

/*****************************************************************************
                                     Tests
 ****************************************************************************/

static int init() {

  dtn_dir_tree_remove(OUR_TEMP_DIR);
  dtn_dir_tree_create(OUR_TEMP_DIR);
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static dtn_log_output log_output_from_string(char const *str) {

  dtn_item *log_json = dtn_item_from_json(str);

  DTN_ASSERT(0 != log_json);

  testrun_log("%s", dtn_item_to_json(log_json));

  dtn_log_output out = dtn_log_output_from_json(log_json);

  log_json = dtn_item_free(log_json);
  DTN_ASSERT(0 == log_json);

  return out;
}

/*----------------------------------------------------------------------------*/

static int test_dtn_log_output_from_json() {

  if (test_disable) goto done;

  dtn_log_output out = dtn_log_output_from_json(0);
  testrun(output_empty(out));

  out = log_output_from_string("{"
                               "\"format\" : \"json\","
                               "\"level\" : \"info\","
                               "\"systemd\" : true"
                               "}");

  testrun(!output_empty(out));

  testrun(out.use.systemd);
  testrun(0 == out.filehandle);
  testrun(!out.log_rotation.use);
  testrun(0 == out.format);

  // Log level is not part of output def. - it is not deserialized here
  // therefore

#define LOG_FILE_NAME OUR_TEMP_DIR "/output_from_string"

  memset(&out, 0, sizeof(out));

  out = log_output_from_string("{"
                               "\"file\" : \"" LOG_FILE_NAME "\","
                               "\"format\" : \"json\","
                               "\"level\" : \"info\","
                               "\"systemd\" : false"
                               "}");

  testrun(!output_empty(out));

  testrun(!out.use.systemd);
  testrun(0 != out.filehandle);
  testrun(!out.log_rotation.use);
  testrun(DTN_LOG_JSON == out.format);

  close(out.filehandle);

  memset(&out, 0, sizeof(out));

  remove(LOG_FILE_NAME);

  // Check extended file config format - no log rotation

  out = log_output_from_string("{"
                               "\"file\" : {"
                               "\"file\":\"" LOG_FILE_NAME ".r\""
                               "},"
                               "\"format\" : \"json\","
                               "\"level\" : \"info\","
                               "\"systemd\" : false"
                               "}");

  testrun(!out.use.systemd);
  testrun(0 != out.filehandle);
  testrun(!out.log_rotation.use);
  testrun(0 == out.log_rotation.path);

  testrun(DTN_LOG_JSON == out.format);

  close(out.filehandle);
  memset(&out, 0, sizeof(out));

  remove(LOG_FILE_NAME ".r");

  //
  // Check extended file config format with log rotation

  out = log_output_from_string("{"
                               "\"file\" : {"
                               "\"file\":\"" LOG_FILE_NAME ".s\","
                               "\"messages_per_file\": 7,"
                               "\"num_files\" : 3"
                               "},"
                               "\"format\" : \"json\","
                               "\"level\" : \"info\","
                               "\"systemd\" : false"
                               "}");

  testrun(!out.use.systemd);
  testrun(0 != out.filehandle);
  testrun(out.log_rotation.use);
  testrun(7 == out.log_rotation.messages_per_file);
  testrun(3 == out.log_rotation.max_num_files);
  testrun(0 != out.log_rotation.path);
  testrun(0 == strcmp(LOG_FILE_NAME ".s", out.log_rotation.path));
  testrun(DTN_LOG_JSON == out.format);

  free(out.log_rotation.path);
  close(out.filehandle);
  memset(&out, 0, sizeof(out));

  remove(LOG_FILE_NAME ".s");

  out = log_output_from_string("{"
                               "\"file\" : {"
                               "\"file\":\"" LOG_FILE_NAME ".t\","
                               "\"messages_per_file\": 7,"
                               "\"num_files\" : 3"
                               "},"
                               "\"format\" : \"json\","
                               "\"level\" : \"info\","
                               "\"systemd\" : false"
                               "}");

  testrun(!out.use.systemd);
  testrun(0 != out.filehandle);
  testrun(out.log_rotation.use);
  testrun(7 == out.log_rotation.messages_per_file);
  testrun(3 == out.log_rotation.max_num_files);
  testrun(0 != out.log_rotation.path);
  testrun(DTN_LOG_JSON == out.format);
  testrun(0 == strcmp(LOG_FILE_NAME ".t", out.log_rotation.path));

  free(out.log_rotation.path);
  close(out.filehandle);
  memset(&out, 0, sizeof(out));

  remove(LOG_FILE_NAME ".t");

#undef LOG_FILE_NAME
done:
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int test_dtn_config_log_from_json() {

  if (test_disable) goto done;

  testrun(stderr_redirect());
  testrun(stdout_redirect());
  testrun(stderr_empty());
  testrun(stdout_empty());

  // Should log stdout, up to level info
  dtn_log_error("ERROR");
  dtn_log_warning("WARNING");
  dtn_log_info("INFO");
  dtn_log_debug("DEBUG");

  testrun(!stdout_empty());
  testrun(stderr_empty());
  testrun(stdout_clear());
  testrun(stderr_clear());

  dtn_log_init();

  // Should log to stdout up to level info - JSON format
  dtn_log_error("ERROR");
  dtn_log_warning("WARNING");
  dtn_log_info("INFO");
  dtn_log_debug("DEBUG");

  testrun(!stdout_empty());
  testrun(stderr_empty());
  testrun(stdout_clear());
  testrun(stderr_clear());

  dtn_item *log_cfg =
      json_from_string("{"
                       "\"format\" : \"json\","
                       "\"level\" : \"info\","
                       "\"systemd\" : \"false\""
                       "}");

  testrun(0 != log_cfg);

  testrun(dtn_config_log_from_json(log_cfg));
  log_cfg = dtn_item_free(log_cfg);

  testrun(0 == log_cfg);

  // Nowhere
  dtn_log_error("ERROR");
  dtn_log_warning("WARNING");
  dtn_log_info("INFO");
  dtn_log_debug("DEBUG");

  testrun(stdout_empty());
  testrun(stderr_empty());

  // Install log handler for another module -
  // Since we are in module 'dtn_log_test.c', we should still log nowhere

#define AAA_FILE_NAME OUR_TEMP_DIR "/aaa"

  log_cfg = json_from_string("{"
                             "\"level\" : \"info\","
                             "\"systemd\" : \"false\","
                             "\"modules\" : {"
                             "\"aaa\" : {"
                             "\"file\" : \"" AAA_FILE_NAME "\","
                             "\"level\" : \"info\""
                             "}"
                             "}"
                             "}");

  testrun(0 != log_cfg);

  testrun(dtn_config_log_from_json(log_cfg));
  log_cfg = dtn_item_free(log_cfg);

  testrun(0 == log_cfg);

  dtn_log_error("ERROR");

  testrun(stdout_empty());
  testrun(stderr_empty());
  testrun(file_empty(AAA_FILE_NAME));

  // Install log file for another module and
  // another log file for our own module
  // SHould log to 'our' file but not the other one

#define DTN_LOG_CONFIGURE_TEST_FILE_NAME OUR_TEMP_DIR "/dtn_config_log_test_c"

  log_cfg = json_from_string("{"
                             "\"level\" : \"info\","
                             "\"systemd\" : \"false\","
                             "\"modules\" : {"
                             "\"aaa\" : {"
                             "\"file\" : \"" AAA_FILE_NAME "\","
                             "\"level\" : \"info\""
                             "},"
                             "\"dtn_config_log_test.c\" : {"
                             "\"file\" : \"" DTN_LOG_CONFIGURE_TEST_FILE_NAME "\","
                             "\"level\" : \"info\""
                             "}"
                             "}"
                             "}");

  testrun(0 != log_cfg);

  testrun(dtn_config_log_from_json(log_cfg));
  log_cfg = dtn_item_free(log_cfg);

  testrun(0 == log_cfg);

  dtn_log_error("ERROR");

  testrun(stdout_empty());
  testrun(stderr_empty());
  testrun(file_empty(AAA_FILE_NAME));
  testrun(!file_empty(DTN_LOG_CONFIGURE_TEST_FILE_NAME));

  testrun(file_clear(DTN_LOG_CONFIGURE_TEST_FILE_NAME));

  //
  // Install log file for another module and
  // another log file for our own module
  // Install handler for another function
  // SHould log to 'our' file but not the other one

#define LETS_LOG_SOMETHING_FILE_NAME OUR_TEMP_DIR "/lets_log_something"

  log_cfg = json_from_string("{"
                             "\"level\" : \"info\","
                             "\"systemd\" : \"false\","
                             "\"modules\" : {"
                             "\"aaa\" : {"
                             "\"file\" : \"" AAA_FILE_NAME "\","
                             "\"level\" : \"info\""
                             "},"
                             "\"dtn_config_log_test.c\" : {"
                             "\"file\" : \"" DTN_LOG_CONFIGURE_TEST_FILE_NAME "\","
                             "\"level\" : \"warning\","
                             "\"systemd\" : true,"
                             "\"functions\" : {"
                             "\"lets_log_something\" : {"
                             "\"file\" : \"" LETS_LOG_SOMETHING_FILE_NAME "\","
                             "\"level\" : \"warning\","
                             "\"systemd\" : true"
                             "}"
                             "}"
                             "}"
                             "}"
                             "}");

  testrun(0 != log_cfg);

  testrun(dtn_config_log_from_json(log_cfg));
  log_cfg = dtn_item_free(log_cfg);

  testrun(0 == log_cfg);

  dtn_log_error("ERROR");

  testrun(stdout_empty());
  testrun(stderr_empty());
  testrun(file_empty(AAA_FILE_NAME));
  testrun(file_empty(LETS_LOG_SOMETHING_FILE_NAME));
  testrun(!file_empty(DTN_LOG_CONFIGURE_TEST_FILE_NAME));

  testrun(file_clear(DTN_LOG_CONFIGURE_TEST_FILE_NAME));

  // Now log from out of 'lets_log_something' -> Should be logged into
  // its own log file

  lets_log_something(DTN_LOG_ERR);

  testrun(stdout_empty());
  testrun(stderr_empty());
  testrun(file_empty(AAA_FILE_NAME));
  testrun(!file_empty(LETS_LOG_SOMETHING_FILE_NAME));
  testrun(file_empty(DTN_LOG_CONFIGURE_TEST_FILE_NAME));

  testrun(file_clear(LETS_LOG_SOMETHING_FILE_NAME));

  // Since we set the logging level to `warning` for lets_log_something,
  // there should nothing be logged for lets_log_something

  lets_log_something(DTN_LOG_INFO);

  testrun(stdout_empty());
  testrun(stderr_empty());
  testrun(file_empty(AAA_FILE_NAME));
  testrun(file_empty(LETS_LOG_SOMETHING_FILE_NAME));
  testrun(file_empty(DTN_LOG_CONFIGURE_TEST_FILE_NAME));

  /*************************************************************************
                                Log rotation
   ************************************************************************/

#define LETS_LOG_SOMETHING_FILE_NAME OUR_TEMP_DIR "/lets_log_something"

  log_cfg = json_from_string("{"
                             "\"level\" : \"info\","
                             "\"systemd\" : \"false\","
                             "\"modules\" : {"
                             "\"aaa\" : {"
                             "\"file\" : \"" AAA_FILE_NAME "\","
                             "\"level\" : \"info\""
                             "},"
                             "\"dtn_config_log_test.c\" : {"
                             "\"file\" : \"" DTN_LOG_CONFIGURE_TEST_FILE_NAME "\","
                             "\"level\" : \"warning\","
                             "\"systemd\" : true,"
                             "\"functions\" : {"
                             "\"lets_log_something\" : {"
                             "\"file\" : "
                             "\"" LETS_LOG_SOMETHING_FILE_NAME "\","
                             "\"messages_per_file\": 3,"
                             "\"num_files\" : 3,"
                             "\"level\" : \"warning\","
                             "\"systemd\" : true"
                             "}"
                             "}"
                             "}"
                             "}"
                             "}");

  testrun(0 != log_cfg);

  testrun(dtn_config_log_from_json(log_cfg));
  log_cfg = dtn_item_free(log_cfg);

  testrun(0 == log_cfg);

  dtn_log_error("ERROR");

  testrun(stdout_empty());
  testrun(stderr_empty());
  testrun(file_empty(AAA_FILE_NAME));
  testrun(file_empty(LETS_LOG_SOMETHING_FILE_NAME));
  testrun(!file_empty(DTN_LOG_CONFIGURE_TEST_FILE_NAME));

  testrun(file_clear(DTN_LOG_CONFIGURE_TEST_FILE_NAME));

  // Now log from out of 'lets_log_something' -> Should be logged into
  // its own log file

  lets_log_something(DTN_LOG_ERR);

  testrun(stdout_empty());
  testrun(stderr_empty());
  testrun(file_empty(AAA_FILE_NAME));
  testrun(!file_empty(LETS_LOG_SOMETHING_FILE_NAME));
  testrun(file_empty(DTN_LOG_CONFIGURE_TEST_FILE_NAME));

  testrun(file_clear(LETS_LOG_SOMETHING_FILE_NAME));

  // Since we set the logging level to `warning` for lets_log_something,
  // there should nothing be logged for lets_log_something

  lets_log_something(DTN_LOG_INFO);

  testrun(stdout_empty());
  testrun(stderr_empty());
  testrun(file_empty(AAA_FILE_NAME));
  testrun(file_empty(LETS_LOG_SOMETHING_FILE_NAME));
  testrun(file_empty(LETS_LOG_SOMETHING_FILE_NAME ".001"));
  testrun(file_empty(LETS_LOG_SOMETHING_FILE_NAME ".002"));

  testrun(file_empty(DTN_LOG_CONFIGURE_TEST_FILE_NAME));

  // Check log rotates, we set max messages to 3
  lets_log_something(DTN_LOG_ERR);

  testrun(!file_empty(LETS_LOG_SOMETHING_FILE_NAME));
  testrun(file_empty(LETS_LOG_SOMETHING_FILE_NAME ".001"));
  testrun(file_empty(LETS_LOG_SOMETHING_FILE_NAME ".002"));

  lets_log_something(DTN_LOG_ERR);
  lets_log_something(DTN_LOG_ERR);
  lets_log_something(DTN_LOG_ERR);

  testrun(!file_empty(LETS_LOG_SOMETHING_FILE_NAME));
  testrun(!file_empty(LETS_LOG_SOMETHING_FILE_NAME ".001"));
  testrun(file_empty(LETS_LOG_SOMETHING_FILE_NAME ".002"));

  lets_log_something(DTN_LOG_ERR);
  lets_log_something(DTN_LOG_ERR);
  lets_log_something(DTN_LOG_ERR);

  testrun(!file_empty(LETS_LOG_SOMETHING_FILE_NAME));
  testrun(!file_empty(LETS_LOG_SOMETHING_FILE_NAME ".001"));
  testrun(!file_empty(LETS_LOG_SOMETHING_FILE_NAME ".002"));

  /*************************************************************************
                            Log to existing file
   ************************************************************************/

  dtn_log_close();

#define EXISTING_LOG_FILE OUR_TEMP_DIR "/we_exist"

  char const test_content[] = "denn weit und breit sieht sie ueber die "
                              "welten all\n";

  log_cfg = json_from_string("{"
                             "\"level\" : \"info\","
                             "\"systemd\" : \"false\","
                             "\"file\" : \"" EXISTING_LOG_FILE "\""
                             "}");

  testrun(0 != log_cfg);
  testrun(dtn_config_log_from_json(log_cfg));

  dtn_log_error(test_content);

  dtn_log_close();

  // Ensure we actually created the file
  ssize_t file_size = dtn_file_read_check_get_bytes(EXISTING_LOG_FILE);

  testrun(0 < file_size);

  // Now reconfigure with same file again...

  testrun(dtn_config_log_from_json(log_cfg));
  log_cfg = dtn_item_free(log_cfg);

  testrun(0 == log_cfg);

  dtn_log_error(test_content);
  dtn_log_warning(test_content);

  testrun(stdout_empty());
  testrun(stderr_empty());

  // Ensure our log file grew

  ssize_t new_file_size = dtn_file_read_check_get_bytes(EXISTING_LOG_FILE);

  testrun(0 < new_file_size);
  testrun((size_t)file_size < (size_t)new_file_size);

  /*************************************************************************
                                 Tear down
   ************************************************************************/

  dtn_log_close();

done:
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int tear_down() {

  dtn_dir_tree_remove(OUR_TEMP_DIR);
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

  testrun_test(init);
  testrun_test(test_dtn_log_output_from_json);
  testrun_test(test_dtn_config_log_from_json);
  testrun_test(tear_down);

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

