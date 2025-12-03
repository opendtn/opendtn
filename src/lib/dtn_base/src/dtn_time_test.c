/***
        ------------------------------------------------------------------------

        Copyright 2017 German Aerospace Center DLR e.V. (GSOC)

        Licensed under the Apache License, Version 2.0 (the "License");
        you may not use this file except in compliance with the License.
        You may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

        Unless required by applicable law or agreed to in writing, software
        distributed under the License is distributed on an "AS IS" BASIS,
        WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
        See the License for the specific language governing permissions and
        limitations under the License.

        This file is part of the openvocs project. http://openvocs.org

        ------------------------------------------------------------------------
*//**
        @file           dtn_time_tests.c
        @author         Markus Toepfer
        @author         Michael Beer

        @date           2017-12-10

        @ingroup        dtn_time

        @brief          Unit testing for dtn_time


        ------------------------------------------------------------------------
*/

#include "dtn_time.c"
#include <ctype.h>
#include "../include/testrun.h"

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST HELPER                                                     #HELPER
 *
 *      ------------------------------------------------------------------------
 */

/*----------------------------------------------------------------------------*/

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */
int test_dtn_timestamp() {

  char *result = dtn_timestamp(false);

  // 01234567890123456789
  // 2017-11-20T14:57:01Z

  // structure check only
  testrun(result != NULL);
  testrun(strlen(result) == 20);
  for (int i = 0; i < 20; i++) {

    if (i == 19) {
      testrun(result[19] == 'Z');
    } else if (i == 16) {
      testrun(result[16] == ':');
    } else if (i == 13) {
      testrun(result[13] == ':');
    } else if (i == 10) {
      testrun(result[10] == 'T');
    } else if (i == 7) {
      testrun(result[7] == '-');
    } else if (i == 4) {
      testrun(result[4] == '-');
    } else {
      testrun(isdigit(result[i]));
    }
  }
  free(result);

  // 01234567890123456789012345678
  // 2017-11-20T14:57:01.123456Z

  result = dtn_timestamp(true);
  testrun(result != NULL);
  testrun(strlen(result) == 27);
  for (int i = 0; i < 20; i++) {

    if (i == 26) {
      testrun(result[19] == 'Z');
    } else if (i == 19) {
      testrun(result[16] == ':');
    } else if (i == 16) {
      testrun(result[16] == ':');
    } else if (i == 13) {
      testrun(result[13] == ':');
    } else if (i == 10) {
      testrun(result[10] == 'T');
    } else if (i == 7) {
      testrun(result[7] == '-');
    } else if (i == 4) {
      testrun(result[4] == '-');
    } else {
      testrun(isdigit(result[i]));
    }
  }
  free(result);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_timestamp_write_to() {

  char result[30];

  testrun(!dtn_timestamp_write_to(true, NULL, 0));
  testrun(!dtn_timestamp_write_to(true, result, 0));
  testrun(!dtn_timestamp_write_to(true, result, 27));
  testrun(!dtn_timestamp_write_to(false, result, 20));

  testrun(dtn_timestamp_write_to(false, result, 21));

  // 01234567890123456789
  // 2017-11-20T14:57:01Z

  // structure check only
  testrun(strlen(result) == 20);
  for (int i = 0; i < 20; i++) {

    if (i == 19) {
      testrun(result[19] == 'Z');
    } else if (i == 16) {
      testrun(result[16] == ':');
    } else if (i == 13) {
      testrun(result[13] == ':');
    } else if (i == 10) {
      testrun(result[10] == 'T');
    } else if (i == 7) {
      testrun(result[7] == '-');
    } else if (i == 4) {
      testrun(result[4] == '-');
    } else {
      testrun(isdigit(result[i]));
    }
  }

  testrun(!dtn_timestamp_write_to(true, result, 27));
  testrun(dtn_timestamp_write_to(false, result, 21));
  // structure check only
  testrun(strlen(result) == 20);

  testrun(dtn_timestamp_write_to(true, result, 28));

  // 01234567890123456789012345678
  // 2017-11-20T14:57:01.123456Z
  testrun(result != NULL);
  testrun(strlen(result) == 27);
  for (int i = 0; i < 20; i++) {

    if (i == 26) {
      testrun(result[19] == 'Z');
    } else if (i == 19) {
      testrun(result[16] == ':');
    } else if (i == 16) {
      testrun(result[16] == ':');
    } else if (i == 13) {
      testrun(result[13] == ':');
    } else if (i == 10) {
      testrun(result[10] == 'T');
    } else if (i == 7) {
      testrun(result[7] == '-');
    } else if (i == 4) {
      testrun(result[4] == '-');
    } else {
      testrun(isdigit(result[i]));
    }
  }

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_time_string() {

  time_t t;
  time(&t);

  struct timeval tv;

  size_t size = 30;
  char time_utc[size];

  char *result = dtn_time_string(TIME_SCOPE_SECOND);

  testrun(0 == gettimeofday(&tv, NULL));

  testrun(strftime(time_utc, size, "%F %T", gmtime(&tv.tv_sec)));
  testrun(strlen(time_utc) == strlen(result));
  testrun(strncmp(result, time_utc, strlen(time_utc)) == 0);
  free(result);

  result = dtn_time_string(TIME_SCOPE_MINUTE);
  testrun(strftime(time_utc, size, "%F %R", gmtime(&tv.tv_sec)));
  testrun(strlen(time_utc) == strlen(result));
  testrun(strncmp(result, time_utc, strlen(time_utc)) == 0);
  free(result);

  result = dtn_time_string(TIME_SCOPE_HOUR);
  testrun(strftime(time_utc, size, "%F %H", gmtime(&tv.tv_sec)));
  testrun(strlen(time_utc) == strlen(result));
  testrun(strncmp(result, time_utc, strlen(time_utc)) == 0);
  free(result);

  result = dtn_time_string(TIME_SCOPE_DAY);
  testrun(strftime(time_utc, size, "%F", gmtime(&tv.tv_sec)));
  testrun(strlen(time_utc) == strlen(result));
  testrun(strncmp(result, time_utc, strlen(time_utc)) == 0);
  free(result);

  result = dtn_time_string(TIME_SCOPE_MONTH);
  testrun(strftime(time_utc, size, "%Y-%m", gmtime(&tv.tv_sec)));
  testrun(strlen(time_utc) == strlen(result));
  testrun(strncmp(result, time_utc, strlen(time_utc)) == 0);
  free(result);

  result = dtn_time_string(TIME_SCOPE_YEAR);
  testrun(strftime(time_utc, size, "%Y", gmtime(&tv.tv_sec)));
  testrun(strlen(time_utc) == strlen(result));
  testrun(strncmp(result, time_utc, strlen(time_utc)) == 0);
  free(result);

  // TIME_SCOPE_MILLISECOND

  result = dtn_time_string(TIME_SCOPE_MILLISECOND);

  // 01234567890123456789012
  // 2017-11-20 14:57:01.123

  // structure check only
  testrun(result != NULL);
  testrun(strlen(result) == 23);
  for (int i = 0; i < 23; i++) {

    if (i == 19) {
      testrun(result[19] == '.');
    } else if (i == 16) {
      testrun(result[16] == ':');
    } else if (i == 13) {
      testrun(result[13] == ':');
    } else if (i == 10) {
      testrun(result[10] == ' ');
    } else if (i == 7) {
      testrun(result[7] == '-');
    } else if (i == 4) {
      testrun(result[4] == '-');
    } else {
      testrun(isdigit(result[i]));
    }
  }
  free(result);

  result = dtn_time_string(TIME_SCOPE_MICROSECOND);
  // 01234567890123456789012345678
  // 2017-11-20 14:57:01.123456
  testrun(result != NULL);
  testrun(strlen(result) == 26);
  for (int i = 0; i < 26; i++) {

    if (i == 19) {
      testrun(result[19] == '.');
    } else if (i == 16) {
      testrun(result[16] == ':');
    } else if (i == 13) {
      testrun(result[13] == ':');
    } else if (i == 10) {
      testrun(result[10] == ' ');
    } else if (i == 7) {
      testrun(result[7] == '-');
    } else if (i == 4) {
      testrun(result[4] == '-');
    } else {
      testrun(isdigit(result[i]));
    }
  }
  free(result);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_time_write_to() {

  size_t i = 0;
  size_t size = 50;
  char result[size];
  memset(result, 0, size);

  testrun(!dtn_time_write_to(TIME_SCOPE_YEAR, NULL, 0));
  testrun(!dtn_time_write_to(TIME_SCOPE_YEAR, result, 0));

  testrun(!dtn_time_write_to(TIME_SCOPE_YEAR, result, 4));
  testrun(result[0] == 0);
  testrun(dtn_time_write_to(TIME_SCOPE_YEAR, result, 5));
  testrun(strlen(result) == 4);
  for (i = 0; i < strlen(result); i++) {
    testrun(isdigit(result[i]));
  }
  memset(result, 0, size);

  testrun(!dtn_time_write_to(TIME_SCOPE_MONTH, result, 7));
  testrun(result[0] == 0);
  testrun(dtn_time_write_to(TIME_SCOPE_MONTH, result, 8));
  testrun(strlen(result) == 7);
  for (i = 0; i < strlen(result); i++) {
    if (i == 4) {
      testrun(result[i] == '-');
    } else {
      testrun(isdigit(result[i]));
    }
  }
  memset(result, 0, size);

  testrun(!dtn_time_write_to(TIME_SCOPE_DAY, result, 10));
  testrun(result[0] == 0);
  testrun(dtn_time_write_to(TIME_SCOPE_DAY, result, 11));
  testrun(strlen(result) == 10);
  for (i = 0; i < strlen(result); i++) {

    if (i == 4) {
      testrun(result[i] == '-');
    } else if (i == 7) {
      testrun(result[i] == '-');
    } else {
      testrun(isdigit(result[i]));
    }
  }
  memset(result, 0, size);

  testrun(!dtn_time_write_to(TIME_SCOPE_HOUR, result, 13));
  testrun(result[0] == 0);
  testrun(dtn_time_write_to(TIME_SCOPE_HOUR, result, 14));
  testrun(strlen(result) == 13);
  for (i = 0; i < strlen(result); i++) {

    if (i == 4) {
      testrun(result[i] == '-');
    } else if (i == 7) {
      testrun(result[i] == '-');
    } else if (i == 10) {
      testrun(result[i] == ' ');
    } else {
      testrun(isdigit(result[i]));
    }
  }
  memset(result, 0, size);

  testrun(result[0] == 0);
  testrun(!dtn_time_write_to(TIME_SCOPE_MINUTE, result, 16));
  testrun(result[0] == 0);
  testrun(dtn_time_write_to(TIME_SCOPE_MINUTE, result, 17));
  testrun(strlen(result) == 16);
  for (i = 0; i < strlen(result); i++) {

    if (i == 4) {
      testrun(result[i] == '-');
    } else if (i == 7) {
      testrun(result[i] == '-');
    } else if (i == 10) {
      testrun(result[i] == ' ');
    } else if (i == 13) {
      testrun(result[i] == ':');
    } else {
      testrun(isdigit(result[i]));
    }
  }
  memset(result, 0, size);

  testrun(!dtn_time_write_to(TIME_SCOPE_SECOND, result, 19));
  testrun(result[0] == 0);
  testrun(dtn_time_write_to(TIME_SCOPE_SECOND, result, 20));
  testrun(strlen(result) == 19);
  for (i = 0; i < strlen(result); i++) {

    if (i == 4) {
      testrun(result[i] == '-');
    } else if (i == 7) {
      testrun(result[i] == '-');
    } else if (i == 10) {
      testrun(result[i] == ' ');
    } else if (i == 13) {
      testrun(result[i] == ':');
    } else if (i == 16) {
      testrun(result[i] == ':');
    } else {
      testrun(isdigit(result[i]));
    }
  }
  memset(result, 0, size);

  testrun(!dtn_time_write_to(TIME_SCOPE_MILLISECOND, result, 23));
  testrun(result[0] == 0);
  testrun(dtn_time_write_to(TIME_SCOPE_MILLISECOND, result, 24));
  testrun(strlen(result) == 23);
  for (i = 0; i < strlen(result); i++) {

    if (i == 4) {
      testrun(result[i] == '-');
    } else if (i == 7) {
      testrun(result[i] == '-');
    } else if (i == 10) {
      testrun(result[i] == ' ');
    } else if (i == 13) {
      testrun(result[i] == ':');
    } else if (i == 16) {
      testrun(result[i] == ':');
    } else if (i == 19) {
      testrun(result[i] == '.');
    } else {
      testrun(isdigit(result[i]));
    }
  }
  memset(result, 0, size);

  testrun(!dtn_time_write_to(TIME_SCOPE_MICROSECOND, result, 26));
  testrun(result[0] == 0);
  testrun(dtn_time_write_to(TIME_SCOPE_MICROSECOND, result, 27));
  testrun(strlen(result) == 26);
  for (i = 0; i < strlen(result); i++) {

    if (i == 4) {
      testrun(result[i] == '-');
    } else if (i == 7) {
      testrun(result[i] == '-');
    } else if (i == 10) {
      testrun(result[i] == ' ');
    } else if (i == 13) {
      testrun(result[i] == ':');
    } else if (i == 16) {
      testrun(result[i] == ':');
    } else if (i == 19) {
      testrun(result[i] == '.');
    } else {
      testrun(isdigit(result[i]));
    }
  }
  memset(result, 0, size);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int test_dtn_time_to_string() {

  uint64_t ref_epoch = 1651771106;
  char const *ref_sec_str = "2022-05-05T17:18:26Z";
  char const *ref_usec_str = "2022-05-05T17:18:26.000000Z";

  testrun(!dtn_time_to_string(0, 0, 0, 0));

  char tstr[40] = {0};

  testrun(!dtn_time_to_string(tstr, 0, 0, 0));
  testrun(!dtn_time_to_string(0, sizeof(tstr), 0, 0));
  testrun(!dtn_time_to_string(tstr, sizeof(tstr), 0, 0));
  testrun(!dtn_time_to_string(0, 0, ref_epoch, 0));
  testrun(!dtn_time_to_string(tstr, 0, ref_epoch, 0));
  testrun(!dtn_time_to_string(0, sizeof(tstr), ref_epoch, 0));
  testrun(!dtn_time_to_string(tstr, sizeof(tstr), ref_epoch, 0));
  testrun(!dtn_time_to_string(0, 0, 0, SEC));
  testrun(!dtn_time_to_string(tstr, 0, 0, SEC));
  testrun(!dtn_time_to_string(0, sizeof(tstr), 0, SEC));

  // Epoch 0 is January 1st, 00:00
  testrun(dtn_time_to_string(tstr, sizeof(tstr), 0, SEC));
  testrun(0 == strcmp("1970-01-01T00:00:00Z", tstr));

  testrun(dtn_time_to_string(tstr, sizeof(tstr), 0, USEC));
  testrun(0 == strcmp("1970-01-01T00:00:00.000000Z", tstr));

  testrun(dtn_time_to_string(tstr, sizeof(tstr), ref_epoch, SEC));
  testrun(0 == strcmp(ref_sec_str, tstr));

  testrun(dtn_time_to_string(tstr, sizeof(tstr), 1000 * 1000 * ref_epoch, USEC));
  testrun(0 == strcmp(ref_usec_str, tstr));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int test_dtn_timestamp_parse() {

  char const *ref_sec_str = "2022-05-05T17:18:26Z";

  uint16_t y = 0;
  uint8_t mo = 0;
  uint8_t d = 0;
  uint8_t h = 0;
  uint8_t m = 0;
  uint8_t s = 0;

  testrun(dtn_timestamp_parse(ref_sec_str, &y, &mo, &d, &h, &m, &s));

  testrun(s == 26);
  testrun(m == 18);
  testrun(h == 17);
  testrun(d == 5);
  testrun(mo == 5);
  testrun(y == 2022);

  testrun(!dtn_timestamp_parse("2022-05-05T17:18:26", &y, &mo, &d, &h, &m, &s));
  testrun(!dtn_timestamp_parse("2022-05-05T17:18:26z", &y, &mo, &d, &h, &m, &s));
  testrun(!dtn_timestamp_parse("2022-05-05T17-18-26Z", &y, &mo, &d, &h, &m, &s));
  testrun(!dtn_timestamp_parse("2022-05-05T17:18:2Z", &y, &mo, &d, &h, &m, &s));
  testrun(!dtn_timestamp_parse("2022-05-05T17:1:26Z", &y, &mo, &d, &h, &m, &s));
  testrun(!dtn_timestamp_parse("2022-05-05T1:18:26Z", &y, &mo, &d, &h, &m, &s));
  testrun(!dtn_timestamp_parse("2022-05-5T17:18:26Z", &y, &mo, &d, &h, &m, &s));
  testrun(!dtn_timestamp_parse("2022-5-05T17:18:26Z", &y, &mo, &d, &h, &m, &s));
  testrun(!dtn_timestamp_parse("202-05-05T17:18:26Z", &y, &mo, &d, &h, &m, &s));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int test_dtn_timestamp_from_data() {

  char *r = dtn_timestamp_from_data(2022, 12, 31, 23, 59, 59);
  testrun(0 == strcmp(r, "2022-12-31T23:59:59Z"));
  free(r);

  r = dtn_timestamp_from_data(2022, 1, 1, 0, 0, 0);
  testrun(0 == strcmp(r, "2022-01-01T00:00:00Z"));
  free(r);

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
  testrun_test(test_dtn_timestamp);
  testrun_test(test_dtn_timestamp_write_to);
  testrun_test(test_dtn_timestamp_parse);
  testrun_test(test_dtn_timestamp_from_data);
  testrun_test(test_dtn_time_string);
  testrun_test(test_dtn_time_write_to);
  testrun_test(test_dtn_time_to_string);

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
