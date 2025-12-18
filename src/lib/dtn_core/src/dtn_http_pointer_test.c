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
        @file           dtn_http_pointer_test.c
        @author         Markus Töpfer

        @date           2020-12-10


        ------------------------------------------------------------------------
*/
#include "dtn_http_pointer.c"
#include <dtn_base/testrun.h>

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

int check_http_is_separator() {

  for (uint8_t i = 0; i < 0xFF; i++) {

    switch (i) {

    case '(':
    case ')':
    case '<':
    case '>':
    case '@':
    case ',':
    case ';':
    case ':':
    case 0x5C:
    case 0x22:
    case '/':
    case '[':
    case ']':
    case '?':
    case '=':
    case '{':
    case '}':
    case 0x20:
    case 0x09:
      testrun(http_is_separator(i));
      break;
    default:
      testrun(!http_is_separator(i));
    }
  }

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_http_is_token_string() {

  char *str = "sometext";

  testrun(http_is_token_string((uint8_t *)str, strlen(str)));

  str = "some text";
  testrun(!http_is_token_string((uint8_t *)str, strlen(str)));

  str = "{";
  testrun(!http_is_token_string((uint8_t *)str, strlen(str)));

  str = "x{";
  testrun(!http_is_token_string((uint8_t *)str, strlen(str)));

  str = "some{text";
  testrun(!http_is_token_string((uint8_t *)str, strlen(str)));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_http_is_whitespace() {

  for (uint8_t i = 0; i < 0xFF; i++) {

    switch (i) {

    case 0x20:
    case 0x09:
      testrun(http_is_whitespace(i));
      break;
    default:
      testrun(!http_is_whitespace(i));
    }
  }

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_http_is_control() {

  for (int i = 0; i < 0xFF; i++) {

    if (i < 32) {
      testrun(http_is_control(i));
    } else if (i == 127) {
      testrun(http_is_control(i));
    } else {
      testrun(!http_is_control(i));
    }
  }

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_http_is_text() {
  /*
      char *str = "some text";

      testrun(http_is_text((uint8_t *)str, strlen(str)));

      str = "some text\r\n with \r\n\tlinear whitespace \r\n x";
      testrun(http_is_text((uint8_t *)str, strlen(str)));

      str = "some text\r\n with \r\n\tlinear whitespace error\r\n";
      testrun(!http_is_text((uint8_t *)str, strlen(str)));

      str = "some text\r\n with \r\n\tlinear whitespace and control \t
     failure"; testrun(!http_is_text((uint8_t *)str, strlen(str)));

      str = "some text\r\n with \r\n\tlinear whitespace and quoted pair \\\r
     failure"; testrun(!http_is_text((uint8_t *)str, strlen(str)));

      str = "some text\r\n with \r\n\tlinear whitespace and quoted pair \\\"
     failure"; testrun(!http_is_text((uint8_t *)str, strlen(str)));

      str = "some text\r\n with \r\n\tlinear whitespace and qd \\\" pair \\\"
     success"; testrun(http_is_text((uint8_t *)str, strlen(str)));
  */
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_http_is_token() {

  for (uint8_t i = 0; i < 0xFF; i++) {

    if (isdigit(i)) {
      testrun(http_is_token(i));
    } else if (isalpha(i)) {
      testrun(http_is_token(i));
    } else if (http_is_control(i)) {
      testrun(!http_is_token(i));
    } else if (http_is_separator(i)) {
      testrun(!http_is_token(i));
    } else if (i < 127) {
      testrun(http_is_token(i));
    } else {
      testrun(!http_is_token(i));
    }
  }

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_http_is_field_content() {

  // char buffer[100] = {0};

  char *str = "some text";

  testrun(http_is_field_content((uint8_t *)str, strlen(str)));

  str = "\"some quoted text\"";
  testrun(http_is_field_content((uint8_t *)str, strlen(str)));

  str = "AB \"some quoted text\" CD";
  testrun(http_is_field_content((uint8_t *)str, strlen(str)));

  str = "AB \"some quoted text CD";
  testrun(http_is_field_content((uint8_t *)str, strlen(str)));

  str = "AB \" (/)[]|{}} $%some quoted text CD";
  testrun(http_is_field_content((uint8_t *)str, strlen(str)));

  str = "AB \"some quoted \\x text\" CD";
  testrun(http_is_field_content((uint8_t *)str, strlen(str)));

  str = "AB\\x \"some quoted \\x text\" \\x CD";
  testrun(http_is_field_content((uint8_t *)str, strlen(str)));

  str = "\"some quoted \\x text\"";
  testrun(http_is_field_content((uint8_t *)str, strlen(str)));

  str = "\"some quoted \\x text\" abcd \"some quoted \\x text\"";
  testrun(http_is_field_content((uint8_t *)str, strlen(str)));

  /*

      buffer[0] = 'A';
      buffer[1] = 'B';
      buffer[2] = 0x5C; //  \x
      buffer[3] = 'x';
      buffer[4] = 0x5C; //  \x
      buffer[5] = 'x';
      buffer[6] = 0x5C; //  \x
      buffer[7] = 'x';
      buffer[8] = 0x5C; //  \x
      buffer[9] = 'x';
      buffer[10] = 'C'; //  \x
      buffer[11] = 'D';
      buffer[12] = 0x5C; //  \x
      buffer[13] = 'x';
      buffer[14] = 'E';
      buffer[15] = 'F';

      // incomplete quoted string
      testrun(http_is_field_content((uint8_t *)buffer, 13));

      // complete quoted string
      testrun(http_is_field_content((uint8_t *)buffer, 14));
      testrun(http_is_field_content((uint8_t *)buffer, 15));
      testrun(http_is_field_content((uint8_t *)buffer, 16));

      memset(buffer, 0, 100);

      buffer[0] = 'A';
      buffer[1] = '"';
      buffer[2] = '\t';
      buffer[3] = 'x';
      buffer[4] = '"';
      buffer[5] = 'B';
      buffer[6] = 0x5C; //  \x
      buffer[7] = 'x';
      buffer[8] = 0x5C; //  \x
      buffer[9] = 'x';

      // complete quoted string with linear whitespace
      testrun(http_is_field_content((uint8_t *)buffer, 10));

      memset(buffer, 0, 100);

      buffer[0] = 'A';
      buffer[1] = '"';
      buffer[2] = '\r';
      buffer[3] = '\n';
      buffer[4] = ' ';
      buffer[5] = 'B';
      buffer[6] = 0x5C; //  \x
      buffer[7] = 'x';
      buffer[8] = '"';
      buffer[9] = 'x';

      // quoted string with linear whitespace
      testrun(http_is_field_content((uint8_t *)buffer, 10));

      str = "token!#$";
      testrun(http_is_field_content((uint8_t *)str, strlen(str)));

      str = "token!#$ with separator () {} // <token ! in separator>";
      testrun(http_is_field_content((uint8_t *)str, strlen(str)));

      memset(buffer, 0, 100);
      buffer[0] = 'A';
      buffer[1] = '\r';
      buffer[2] = '\n';
      buffer[3] = ' ';
      buffer[4] = 0x5C;
      buffer[5] = '(';
      buffer[6] = 0x5C; //  \x
      buffer[7] = 'x';
      buffer[8] = 0x5C; //  \x
      buffer[9] = 'x';
      buffer[10] = ')';
      buffer[11] = '?';
      buffer[12] = 0x5C; //  \x
      buffer[13] = 'x';

      // multiples quoted strings mixed with token and separator
      testrun(http_is_field_content((uint8_t *)buffer, 14));
  */
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_pointer_parse_version() {

  dtn_http_version ver = {0};
  char *buf = "HTTP/1.1";
  size_t len = strlen(buf);

  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_version(NULL, 0, NULL));
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_version((uint8_t *)buf, len, NULL));
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_version(NULL, len, &ver));
  testrun(DTN_HTTP_PARSER_PROGRESS ==
          dtn_http_pointer_parse_version((uint8_t *)buf, 0, &ver));

  for (size_t i = 0; i < len; i++) {
    testrun(DTN_HTTP_PARSER_PROGRESS ==
            dtn_http_pointer_parse_version((uint8_t *)buf, i, &ver));
  }

  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_version((uint8_t *)buf, len, &ver));
  testrun(1 == ver.major);
  testrun(1 == ver.minor);

  buf = "http/2.1";
  len = strlen(buf);

  for (size_t i = 0; i < len; i++) {
    testrun(DTN_HTTP_PARSER_PROGRESS ==
            dtn_http_pointer_parse_version((uint8_t *)buf, i, &ver));
  }

  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_version((uint8_t *)buf, len, &ver));
  testrun(2 == ver.major);
  testrun(1 == ver.minor);

  buf = "http/3.763 whatever";
  len = strlen(buf);

  for (size_t i = 0; i < 8; i++) {
    testrun(DTN_HTTP_PARSER_PROGRESS ==
            dtn_http_pointer_parse_version((uint8_t *)buf, i, &ver));
  }

  for (size_t i = 8; i < len; i++) {
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_version((uint8_t *)buf, len, &ver));
    testrun(3 == ver.major);
    testrun(7 == ver.minor);
  }

  buf = "http/10.763 whatever";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_version((uint8_t *)buf, len, &ver));

  buf = "http/1:763 whatever";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_version((uint8_t *)buf, len, &ver));

  buf = "http/ 1.1";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_version((uint8_t *)buf, len, &ver));

  buf = "http1.1";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_version((uint8_t *)buf, len, &ver));

  buf = "httq/1.1";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_version((uint8_t *)buf, len, &ver));

  buf = "x";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_version((uint8_t *)buf, len, &ver));

  buf = " http/6.7";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_version((uint8_t *)buf, len, &ver));
  testrun(6 == ver.major);
  testrun(7 == ver.minor);

  buf = "\nhttp/6.7";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_version((uint8_t *)buf, len, &ver));

  buf = "\thttp/6.7";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_version((uint8_t *)buf, len, &ver));
  testrun(6 == ver.major);
  testrun(7 == ver.minor);

  buf = " \t \t http/9.9";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_version((uint8_t *)buf, len, &ver));
  testrun(9 == ver.major);
  testrun(9 == ver.minor);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_pointer_parse_status_line() {

  dtn_http_status line = {0};
  dtn_http_version version = {0};
  // 01234567890123
  char *buf = "HTTP/1.1 200 OK\r\n";
  size_t len = strlen(buf);

  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_status_line(NULL, 0, NULL, NULL, NULL));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_status_line(
                                      (uint8_t *)buf, len, NULL, NULL, NULL));
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_status_line(NULL, len, &line, &version, NULL));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_status_line(
                                      (uint8_t *)buf, len, &line, NULL, NULL));
  testrun(DTN_HTTP_PARSER_PROGRESS ==
          dtn_http_pointer_parse_status_line((uint8_t *)buf, 0, &line, &version,
                                            NULL));

  for (size_t i = 0; i < 16; i++) {
    testrun(DTN_HTTP_PARSER_PROGRESS ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  for (size_t i = 16; i < len; i++) {
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_status_line((uint8_t *)buf, len, &line,
                                            &version, NULL));
  testrun(1 == version.major);
  testrun(1 == version.minor);
  testrun(200 == line.code);
  testrun(0 == strncmp("OK", (char *)line.phrase.start, line.phrase.length));

  buf = "HTTP/5.6 201 abc\r\nsomething";
  len = strlen(buf);

  for (size_t i = 0; i < 17; i++) {
    testrun(DTN_HTTP_PARSER_PROGRESS ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  for (size_t i = 17; i < len; i++) {
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_status_line((uint8_t *)buf, len, &line,
                                            &version, NULL));
  testrun(5 == version.major);
  testrun(6 == version.minor);
  testrun(201 == line.code);
  testrun(0 == strncmp("abc", (char *)line.phrase.start, line.phrase.length));

  // 01234567890123457-8-
  buf = "HTTP/5.6 201 abc\r\n";
  len = strlen(buf);

  for (size_t i = 0; i < 17; i++) {
    testrun(DTN_HTTP_PARSER_PROGRESS ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  for (size_t i = 17; i < len; i++) {
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_status_line((uint8_t *)buf, len, &line,
                                            &version, NULL));
  testrun(5 == version.major);
  testrun(6 == version.minor);
  testrun(201 == line.code);
  testrun(0 == strncmp("abc", (char *)line.phrase.start, line.phrase.length));

  // space after version
  buf = "HTTP/5.6201 abc\r";
  len = strlen(buf);

  for (size_t i = 0; i < 9; i++) {
    testrun(DTN_HTTP_PARSER_PROGRESS ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  for (size_t i = 9; i < len; i++) {
    testrun(DTN_HTTP_PARSER_ERROR ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  // 2 digit code
  buf = "HTTP/5.6 21 abc\r";
  len = strlen(buf);

  for (size_t i = 0; i < 12; i++) {
    testrun(DTN_HTTP_PARSER_PROGRESS ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  for (size_t i = 12; i < len; i++) {
    testrun(DTN_HTTP_PARSER_ERROR ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  // 4 digit code
  buf = "HTTP/5.6 1234 abc\r";
  len = strlen(buf);

  for (size_t i = 0; i < 13; i++) {
    testrun(DTN_HTTP_PARSER_PROGRESS ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  for (size_t i = 13; i < len; i++) {
    testrun(DTN_HTTP_PARSER_ERROR ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  // no reason
  buf = "HTTP/5.6 200 \r";
  len = strlen(buf);

  for (size_t i = 0; i < 14; i++) {
    testrun(DTN_HTTP_PARSER_PROGRESS ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  for (size_t i = 14; i < len; i++) {
    testrun(DTN_HTTP_PARSER_ERROR ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  // version error
  buf = "HTTP5.6 200 \r";
  len = strlen(buf);

  for (size_t i = 0; i < 5; i++) {
    testrun(DTN_HTTP_PARSER_PROGRESS ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  for (size_t i = 5; i < len; i++) {
    testrun(DTN_HTTP_PARSER_ERROR ==
            dtn_http_pointer_parse_status_line((uint8_t *)buf, i, &line,
                                              &version, NULL));
  }

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_pointer_parse_request_line() {

  dtn_http_request line = {0};
  dtn_http_version version = {0};
  // 012345678901234-5-
  char *buf = "GET / HTTP/1.1\r\n";
  size_t len = strlen(buf);

  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_request_line(NULL, 0, NULL, NULL, 0, NULL));
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_request_line((uint8_t *)buf, len, NULL, NULL, 0,
                                             NULL));
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_request_line(NULL, len, &line, NULL, 0, NULL));
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_request_line((uint8_t *)buf, len, &line, NULL,
                                             0, NULL));
  testrun(DTN_HTTP_PARSER_PROGRESS ==
          dtn_http_pointer_parse_request_line((uint8_t *)buf, 0, &line, &version,
                                             0, NULL));

  for (size_t i = 0; i < 16; i++) {
    testrun(DTN_HTTP_PARSER_PROGRESS ==
            dtn_http_pointer_parse_request_line((uint8_t *)buf, i, &line,
                                               &version, 0, NULL));
  }

  for (size_t i = 16; i < len; i++) {
    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_request_line((uint8_t *)buf, i, &line,
                                               &version, 0, NULL));
  }

  // check method length
  for (size_t i = 1; i < 3; i++) {
    testrun(DTN_HTTP_PARSER_ERROR ==
            dtn_http_pointer_parse_request_line((uint8_t *)buf, len, &line,
                                               &version, i, NULL));
  }

  for (size_t i = 4; i < 10; i++) {

    testrun(DTN_HTTP_PARSER_SUCCESS ==
            dtn_http_pointer_parse_request_line((uint8_t *)buf, len, &line,
                                               &version, i, NULL));
  }

  buf = "whatever /wherever/subever HTTP/1.1\r\n";
  len = strlen(buf);

  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_request_line((uint8_t *)buf, len, &line,
                                             &version, 0, NULL));
  // method to long
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_request_line((uint8_t *)buf, len, &line,
                                             &version, 4, NULL));

  // early lineend
  buf = "what\r\never /wherever/subever HTTP/1.1\r\n";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_request_line((uint8_t *)buf, len, &line,
                                             &version, 0, NULL));

  // early lineend
  buf = "whatever /whe\r\nrever/subever HTTP/1.1\r\n";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_request_line((uint8_t *)buf, len, &line,
                                             &version, 0, NULL));

  // early lineend
  buf = "what\r\never /wherever/subever HTTP\r\n/1.1\r\n";
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_request_line((uint8_t *)buf, len, &line,
                                             &version, 0, NULL));

  buf = "whatever/wherever/subever HTTP/1.1\r\n";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_request_line((uint8_t *)buf, len, &line,
                                             &version, 0, NULL));

  buf = "whatever /wherever/subeverHTTP/1.1\r\n";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_request_line((uint8_t *)buf, len, &line,
                                             &version, 0, NULL));

  // invalid token in method
  buf = "what;ever /wherever/subever HTTP/1.1\r\n";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_request_line((uint8_t *)buf, len, &line,
                                             &version, 0, NULL));

  // invalid uri
  buf = "whatever scheme:error\%path HTTP/1.1\r\n";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_request_line((uint8_t *)buf, len, &line,
                                             &version, 0, NULL));

  // invalid version
  buf = "whatever /wherever/subever HTTP1.1\r\n";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_request_line((uint8_t *)buf, len, &line,
                                             &version, 0, NULL));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_pointer_parse_header_line() {

  dtn_http_header line = {0};
  // 012345678901 2 3
  char *buf = "field:value\r\nnext";
  size_t len = strlen(buf);

  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_header_line(NULL, 0, NULL, 0, NULL));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_header_line(
                                      (uint8_t *)buf, len, NULL, 0, NULL));
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_header_line(NULL, len, &line, 0, NULL));
  testrun(DTN_HTTP_PARSER_PROGRESS ==
          dtn_http_pointer_parse_header_line((uint8_t *)buf, 0, &line, 0, NULL));

  for (size_t i = 0; i < 13; i++) {
    testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_header_line(
                                           (uint8_t *)buf, i, &line, 0, NULL));
  }

  for (size_t i = 13; i < len; i++) {
    testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_header_line(
                                          (uint8_t *)buf, i, &line, 0, NULL));
  }

  uint8_t *next = NULL;

  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_header_line(
                                        (uint8_t *)buf, len, &line, 0, &next));
  testrun(next == (uint8_t *)buf + 13);
  testrun(0 == strncmp("field", (char *)line.name.start, line.name.length));
  testrun(0 == strncmp("value", (char *)line.value.start, line.value.length));

  // whitespace clean
  buf = "\t field:   \t   value   \t \r\nnext";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_header_line(
                                        (uint8_t *)buf, len, &line, 0, &next));
  testrun(next[0] == 'n');
  testrun(0 == strncmp("field", (char *)line.name.start, line.name.length));
  testrun(0 == strncmp("value", (char *)line.value.start, line.value.length));

  // check obs fold
  buf = "\t field:   \t   value\r\n 1\r\n 2\r\nnext";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_header_line(
                                        (uint8_t *)buf, len, &line, 0, &next));
  testrun(next[0] == 'n');
  testrun(0 == strncmp("field", (char *)line.name.start, line.name.length));
  testrun(0 == strncmp("value\r\n 1\r\n 2\r\n", (char *)line.value.start,
                       line.value.length));

  // error name
  buf = "fie{ld:value\r\nnext";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_header_line(
                                         (uint8_t *)buf, 4, &line, 0, &next));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_header_line(
                                      (uint8_t *)buf, len, &line, 0, &next));

  // error value
  // 01234567890
  buf = "field:val{ue\r\nnext";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_header_line(
                                         (uint8_t *)buf, 8, &line, 0, &next));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_header_line(
                                        (uint8_t *)buf, len, &line, 0, &next));

  buf = "field:val\rue\r\nnext";
  len = strlen(buf);
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_header_line(
                                         (uint8_t *)buf, 8, &line, 0, &next));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_header_line(
                                      (uint8_t *)buf, len, &line, 0, &next));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_header_get() {

  dtn_http_header array[10] = {0};
  const dtn_http_header *a = array;

  // parse some headers

  char *buf = "x:0\r\n";
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_header_line((uint8_t *)buf, strlen(buf),
                                            &array[0], 0, NULL));

  buf = "y:2\r\n";
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_header_line((uint8_t *)buf, strlen(buf),
                                            &array[2], 0, NULL));

  buf = "xz:4\r\n";
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_header_line((uint8_t *)buf, strlen(buf),
                                            &array[4], 0, NULL));

  testrun(!dtn_http_header_get(NULL, 0, NULL));

  testrun(!dtn_http_header_get(a, 1, NULL));
  testrun(!dtn_http_header_get(a, 0, "x"));
  testrun(!dtn_http_header_get(a, 1, NULL));

  const dtn_http_header *header = dtn_http_header_get(a, 1, "x");
  testrun(header);
  testrun(header == &array[0]);
  testrun(!dtn_http_header_get(a, 1, "y"));

  header = dtn_http_header_get(a, 3, "y");
  testrun(header);
  testrun(header == &array[2]);
  testrun(!dtn_http_header_get(a, 2, "y"));

  header = dtn_http_header_get(a, 3, "Y");
  testrun(header);
  testrun(header == &array[2]);

  header = dtn_http_header_get(a, 10, "xz");
  testrun(header);
  testrun(header == &array[4]);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_header_get_next() {

  dtn_http_header array[10] = {0};
  const dtn_http_header *a = array;

  // parse some headers

  char *buf = "x:0\r\n";
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_header_line((uint8_t *)buf, strlen(buf),
                                            &array[0], 0, NULL));

  buf = "x:1\r\n";
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_header_line((uint8_t *)buf, strlen(buf),
                                            &array[1], 0, NULL));

  buf = "x:2\r\n";
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_header_line((uint8_t *)buf, strlen(buf),
                                            &array[2], 0, NULL));

  buf = "x:4\r\n";
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_header_line((uint8_t *)buf, strlen(buf),
                                            &array[4], 0, NULL));

  size_t i = 0;
  testrun(!dtn_http_header_get_next(NULL, 0, 0, NULL));

  testrun(!dtn_http_header_get_next(a, 1, &i, NULL));
  testrun(!dtn_http_header_get_next(a, 0, &i, "x"));
  testrun(!dtn_http_header_get_next(a, 1, &i, NULL));

  const dtn_http_header *header = dtn_http_header_get_next(a, 10, &i, "x");
  testrun(header);
  testrun(header == &array[0]);
  testrun(i == 1);

  header = dtn_http_header_get_next(a, 10, &i, "x");
  testrun(header);
  testrun(header == &array[1]);
  testrun(i == 2);

  header = dtn_http_header_get_next(a, 10, &i, "x");
  testrun(header);
  testrun(header == &array[2]);
  testrun(i == 3);

  header = dtn_http_header_get_next(a, 10, &i, "x");
  testrun(header);
  testrun(header == &array[4]);
  testrun(i == 5);

  header = dtn_http_header_get_next(a, 10, &i, "x");
  testrun(NULL == header);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_cast() {

  for (uint16_t i = 0; i < 0xFFFF; i++) {

    if (i == DTN_HTTP_MESSAGE_MAGIC_BYTE) {
      testrun(dtn_http_message_cast(&i));
    } else {
      testrun(!dtn_http_message_cast(&i));
    }
  }

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_create() {

  dtn_http_message *msg = dtn_http_message_create((dtn_http_message_config){0});
  testrun(msg);
  testrun(dtn_http_message_cast(msg));
  testrun(msg->buffer);
  testrun(msg->config.header.capacity == IMPL_DEFAULT_HEADER_CAPACITY);
  testrun(msg->config.buffer.default_size == IMPL_DEFAULT_BUFFER_SIZE);
  testrun(msg->config.buffer.max_bytes_recache ==
          IMPL_DEFAULT_MAX_BUFFER_RECACHE);
  testrun(msg->body.start == NULL);
  testrun(msg->body.length == 0);
  testrun(msg->version.major == 0);
  testrun(msg->version.minor == 0);
  testrun(msg->request.method.start == NULL);
  testrun(msg->request.method.length == 0);
  testrun(msg->request.uri.start == NULL);
  testrun(msg->request.uri.length == 0);
  testrun(msg->status.phrase.start == NULL);
  testrun(msg->status.phrase.length == 0);
  testrun(msg->status.code == 0);
  for (size_t i = 0; i < msg->config.header.capacity; i++) {
    testrun(msg->header[i].name.start == NULL);
    testrun(msg->header[i].name.length == 0);
    testrun(msg->header[i].value.start == NULL);
    testrun(msg->header[i].value.length == 0);
  }
  testrun(msg->buffer->capacity == msg->config.buffer.default_size);
  testrun(msg->buffer->length == 0);
  msg = dtn_http_message_free(msg);

  dtn_http_message_config config = {.header.capacity = 10,
                                   .buffer.default_size = 100,
                                   .buffer.max_bytes_recache = 90};

  msg = dtn_http_message_create(config);
  testrun(dtn_http_message_cast(msg));
  testrun(msg->buffer);
  testrun(msg->config.header.capacity == 10);
  testrun(msg->config.buffer.default_size == 100);
  testrun(msg->config.buffer.max_bytes_recache == 90);
  testrun(msg->buffer->capacity == 100);
  msg = dtn_http_message_free(msg);

  // check with caching
  dtn_http_message_enable_caching(1);
  testrun(g_cache != NULL);

  msg = dtn_http_message_create((dtn_http_message_config){0});
  testrun(msg);
  testrun(dtn_http_message_cast(msg));
  dtn_http_message *msg2 = dtn_http_message_create((dtn_http_message_config){0});
  testrun(msg2);
  testrun(dtn_http_message_cast(msg2));

  dtn_http_message_free(msg);
  dtn_http_message *msg3 = dtn_http_message_create((dtn_http_message_config){0});
  testrun(msg3);
  testrun(dtn_http_message_cast(msg3));
  testrun(msg3 == msg);

  dtn_http_message_free(msg2);
  dtn_http_message *msg4 = dtn_http_message_create((dtn_http_message_config){0});
  testrun(msg4);
  testrun(dtn_http_message_cast(msg4));
  testrun(msg4 == msg2);

  dtn_http_message_free(msg3);
  dtn_http_message_free(msg4);

  // we check what we get from cache
  msg3 = dtn_registered_cache_get(g_cache);
  testrun(msg3);
  testrun(msg == msg3);
  msg4 = dtn_registered_cache_get(g_cache);
  testrun(!msg4);

  msg3 = dtn_http_message_free_uncached(msg3);
  testrun(!dtn_registered_cache_get(g_cache));

  msg = NULL;
  msg2 = NULL;
  msg3 = NULL;
  msg4 = NULL;

  config = (dtn_http_message_config){.header.capacity = 10,
                                    .buffer.default_size = 100};

  msg = dtn_http_message_create(config);
  testrun(msg);
  testrun(msg->config.header.capacity == 10);
  dtn_http_message_free(msg);

  // we request a message with a bigger header size
  msg2 =
      dtn_http_message_create((dtn_http_message_config){.header.capacity = 100});
  testrun(msg2 != msg);
  // msg will point to the message wihin the cache
  testrun(msg->config.header.capacity == 10);
  testrun(msg2->config.header.capacity == 100);

  // we request a message with the same header size
  msg3 = dtn_http_message_create(config);
  testrun(msg);
  testrun(msg->config.header.capacity == 10);
  testrun(msg == msg3);

  // order for free is import! (cache size 1)
  dtn_http_message_free(msg3);
  dtn_http_message_free(msg2);

  // we request a bigger buffer size
  msg4 = dtn_http_message_create((dtn_http_message_config){
      .header.capacity = 10, .buffer.default_size = 1000});
  testrun(msg4);
  testrun(msg == msg4);
  testrun(msg4->buffer->capacity == 1000);

  // we request a smaller buffer size
  dtn_http_message_free(msg);
  msg4 = dtn_http_message_create((dtn_http_message_config){
      .header.capacity = 10, .buffer.default_size = 100});
  testrun(msg4);
  testrun(msg == msg4);
  testrun(msg4->buffer->capacity == 1000);

  // we request a smaller buffer size and bigger header size
  // this will calloc a new msg
  dtn_http_message_free(msg);
  msg4 = dtn_http_message_create((dtn_http_message_config){
      .header.capacity = 100, .buffer.default_size = 100});
  testrun(msg4);
  testrun(msg != msg4);
  testrun(msg4->buffer->capacity == 100);
  msg4 = dtn_http_message_free(msg4);

  dtn_registered_cache_free_all();
  g_cache = NULL;

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_clear() {

  dtn_http_message *msg = dtn_http_message_create((dtn_http_message_config){0});
  testrun(msg);
  testrun(dtn_http_message_cast(msg));
  testrun(msg->buffer);
  testrun(msg->config.header.capacity == IMPL_DEFAULT_HEADER_CAPACITY);
  testrun(msg->config.buffer.default_size == IMPL_DEFAULT_BUFFER_SIZE);
  testrun(msg->config.buffer.max_bytes_recache ==
          IMPL_DEFAULT_MAX_BUFFER_RECACHE);
  testrun(msg->body.start == NULL);
  testrun(msg->body.length == 0);
  testrun(msg->version.major == 0);
  testrun(msg->version.minor == 0);
  testrun(msg->request.method.start == NULL);
  testrun(msg->request.method.length == 0);
  testrun(msg->request.uri.start == NULL);
  testrun(msg->request.uri.length == 0);
  testrun(msg->status.phrase.start == NULL);
  testrun(msg->status.phrase.length == 0);
  testrun(msg->status.code == 0);
  for (size_t i = 0; i < msg->config.header.capacity; i++) {
    testrun(msg->header[i].name.start == NULL);
    testrun(msg->header[i].name.length == 0);
    testrun(msg->header[i].value.start == NULL);
    testrun(msg->header[i].value.length == 0);
  }
  testrun(msg->buffer->capacity == msg->config.buffer.default_size);
  testrun(msg->buffer->length == 0);

  char *string = "GET / HTTP/1.1\r\nfield:value\r\n\r\n";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, NULL));

  testrun(msg->body.start == NULL);
  testrun(msg->version.major == 1);
  testrun(msg->version.minor == 1);
  testrun(0 == strncmp("GET", (char *)msg->request.method.start,
                       msg->request.method.length));
  testrun(0 == strncmp("/", (char *)msg->request.uri.start,
                       msg->request.uri.length));
  testrun(msg->status.phrase.start == NULL);
  testrun(msg->status.phrase.length == 0);
  testrun(msg->status.code == 0);
  testrun(0 == strncmp("field", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("value", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));
  testrun(0 == msg->header[1].name.start);
  testrun(0 == msg->header[1].value.start);
  testrun(msg->buffer->capacity == msg->config.buffer.default_size);
  testrun(msg->buffer->length == strlen(string));

  testrun(!dtn_http_message_clear(NULL));

  testrun(dtn_http_message_clear(msg));
  testrun(dtn_http_message_cast(msg));
  testrun(msg->buffer);
  testrun(msg->config.header.capacity == IMPL_DEFAULT_HEADER_CAPACITY);
  testrun(msg->config.buffer.default_size == IMPL_DEFAULT_BUFFER_SIZE);
  testrun(msg->config.buffer.max_bytes_recache ==
          IMPL_DEFAULT_MAX_BUFFER_RECACHE);
  testrun(msg->body.start == NULL);
  testrun(msg->body.length == 0);
  testrun(msg->version.major == 0);
  testrun(msg->version.minor == 0);
  testrun(msg->request.method.start == NULL);
  testrun(msg->request.method.length == 0);
  testrun(msg->request.uri.start == NULL);
  testrun(msg->request.uri.length == 0);
  testrun(msg->status.phrase.start == NULL);
  testrun(msg->status.phrase.length == 0);
  testrun(msg->status.code == 0);
  for (size_t i = 0; i < msg->config.header.capacity; i++) {
    testrun(msg->header[i].name.start == NULL);
    testrun(msg->header[i].name.length == 0);
    testrun(msg->header[i].value.start == NULL);
    testrun(msg->header[i].value.length == 0);
  }
  testrun(msg->buffer->capacity == msg->config.buffer.default_size);
  testrun(msg->buffer->length == 0);

  msg = dtn_http_message_free(msg);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_free() {

  dtn_http_message *msg = dtn_http_message_create((dtn_http_message_config){0});
  testrun(msg);

  testrun(NULL == dtn_http_message_free(NULL));
  testrun(NULL == dtn_http_message_free(msg));

  msg = dtn_http_message_create((dtn_http_message_config){0});

  testrun(g_cache == NULL);
  dtn_http_message_enable_caching(1);
  testrun(g_cache != NULL);

  testrun(NULL == dtn_http_message_free(msg));

  dtn_http_message *msg2 = dtn_registered_cache_get(g_cache);
  testrun(msg == msg2);

  testrun(NULL == dtn_http_message_free(msg2));
  msg2 = dtn_http_message_create((dtn_http_message_config){0});
  testrun(msg == msg2);
  testrun(NULL == dtn_registered_cache_get(g_cache));

  testrun(NULL == dtn_http_message_free(msg2));

  dtn_registered_cache_free_all();
  g_cache = NULL;

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_free_uncached() {

  dtn_http_message *msg = dtn_http_message_create((dtn_http_message_config){0});
  testrun(msg);

  testrun(NULL == dtn_http_message_free_uncached(NULL));
  testrun(NULL == dtn_http_message_free_uncached(msg));

  msg = dtn_http_message_create((dtn_http_message_config){0});

  testrun(g_cache == NULL);
  dtn_http_message_enable_caching(1);
  testrun(g_cache != NULL);

  testrun(NULL == dtn_http_message_free_uncached(msg));
  testrun(NULL == dtn_registered_cache_get(g_cache));

  dtn_registered_cache_free_all();
  g_cache = NULL;

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_enable_caching() {

  dtn_http_message *msg = dtn_http_message_create((dtn_http_message_config){0});
  testrun(msg);

  testrun(g_cache == NULL);
  dtn_http_message_enable_caching(1);
  testrun(g_cache != NULL);

  testrun(NULL == dtn_http_message_free(msg));

  dtn_http_message *msg2 = dtn_registered_cache_get(g_cache);
  testrun(msg == msg2);

  dtn_registered_cache_free_all();
  g_cache = NULL;

  dtn_http_message_free(msg2);
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_config_init() {

  dtn_http_message_config config =
      dtn_http_message_config_init((dtn_http_message_config){0});

  testrun(config.header.capacity == IMPL_DEFAULT_HEADER_CAPACITY);
  testrun(config.header.max_bytes_method_name == IMPL_DEFAULT_MAX_METHOD_NAME);
  testrun(config.header.max_bytes_line == IMPL_DEFAULT_MAX_HEADER_LINE);

  testrun(config.buffer.default_size == IMPL_DEFAULT_BUFFER_SIZE);
  testrun(config.buffer.max_bytes_recache == IMPL_DEFAULT_MAX_BUFFER_RECACHE);

  testrun(config.transfer.max == IMPL_DEFAULT_MAX_TRANSFER);

  testrun(config.chunk.max_bytes == 0);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_pointer_parse_transfer_encodings() {

  dtn_http_message *msg = dtn_http_message_create((dtn_http_message_config){0});

  char *string = "GET / HTTP/1.1\r\n\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, NULL));

  dtn_memory_pointer array[10] = {0};
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_transfer_encodings(NULL, NULL, 0));
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_transfer_encodings(msg, NULL, 0));
  testrun(DTN_HTTP_PARSER_ERROR ==
          dtn_http_pointer_parse_transfer_encodings(NULL, array, 0));

  // no transfer encoding set, no size given for array
  testrun(DTN_HTTP_PARSER_ABSENT ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 0));

  string = "GET / HTTP/1.1\r\nTransfer-Encoding:chunked\r\n\r\n"
           "4\r\nbody\r\n0\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, NULL));

  // transfer encoding set, no size given for array
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 0));
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 1));
  // check array
  testrun(0 == strncmp("chunked", (char *)array[0].start, array[0].length));
  for (size_t i = 0; i < 10; i++) {
    if (i > 0) {
      testrun(NULL == array[i].start);
      testrun(0 == array[i].length);
    }
  }

  // check whitespace
  string = "GET / HTTP/1.1\r\nTransfer-Encoding:  \t  chunked \t  \r\n\r\n"
           "4\r\nbody\r\n0\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, NULL));

  // transfer encoding set, no size given for array
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 0));
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 1));
  // check array
  testrun(0 == strncmp("chunked", (char *)array[0].start, array[0].length));
  for (size_t i = 1; i < 10; i++) {
    testrun(NULL == array[i].start);
    testrun(0 == array[i].length);
  }

  // check whitespace multiple (chunked not last)
  string = "GET / HTTP/1.1\r\nTransfer-Encoding:  \t  chunked \t , gzip \t  "
           "\r\n\r\n"
           "4\r\nbody\r\n0\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_message(msg, NULL));

  string = "GET / HTTP/1.1\r\nTransfer-Encoding:  \t  gzip \t,chunked \t  "
           "\r\n\r\n"
           "4\r\nbody\r\n0\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, NULL));

  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 0));
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 1));
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 2));
  // check array
  testrun(0 == strncmp("chunked", (char *)array[1].start, array[1].length));
  testrun(0 == strncmp("gzip", (char *)array[0].start, array[0].length));
  for (size_t i = 2; i < 10; i++) {
    testrun(NULL == array[i].start);
    testrun(0 == array[i].length);
  }

  // check non whitespace multiple
  string = "GET / HTTP/1.1\r\nTransfer-Encoding:gzip,undef,chunked\r\n\r\n"
           "4\r\nbody\r\n0\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, NULL));

  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 0));
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 1));
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 2));
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 3));
  // check array
  testrun(0 == strncmp("chunked", (char *)array[2].start, array[2].length));
  testrun(0 == strncmp("gzip", (char *)array[0].start, array[0].length));
  testrun(0 == strncmp("undef", (char *)array[1].start, array[1].length));
  for (size_t i = 3; i < 10; i++) {
    testrun(NULL == array[i].start);
    testrun(0 == array[i].length);
  }

  // check multiple headers
  string = "GET / "
           "HTTP/"
           "1.1\r\nTransfer-Encoding:gzip\r\nTransfer-Encoding:"
           "undef\r\nTransfer-Encoding:chunked\r\n\r\n"
           "4\r\nbody\r\n0\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, NULL));

  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 0));
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 1));
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 2));
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 3));
  // check array
  testrun(0 == strncmp("chunked", (char *)array[2].start, array[2].length));
  testrun(0 == strncmp("gzip", (char *)array[0].start, array[0].length));
  testrun(0 == strncmp("undef", (char *)array[1].start, array[1].length));
  for (size_t i = 3; i < 10; i++) {
    testrun(NULL == array[i].start);
    testrun(0 == array[i].length);
  }

  string = "GET / "
           "HTTP/"
           "1.1\r\nTransfer-Encoding:chunked\r\nTransfer-Encoding:"
           "undef\r\nTransfer-Encoding:gzip\r\n\r\n"
           "4\r\nbody\r\n0\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_message(msg, NULL));

  // multple chunked not last

  // check mix of headers and , separated whitespace multiple
  string = "GET / HTTP/1.1\r\nTransfer-Encoding: chunked , gzip  "
           "\r\nTransfer-Encoding: undef \r\n\r\n"
           "4\r\nbody\r\n0\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_message(msg, NULL));

  // check array cleanup (NOTE MUST be >= 3 to clean up previous set values)
  string = "GET / HTTP/1.1\r\nTransfer-Encoding:chunked\r\n\r\n"
           "4\r\nbody\r\n0\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, NULL));

  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 0));
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 3));
  // check array
  testrun(0 == strncmp("chunked", (char *)array[0].start, array[0].length));
  for (size_t i = 1; i < 10; i++) {
    testrun(NULL == array[i].start);
    testrun(0 == array[i].length);
  }

  // check mix of headers with , separated whitespace multiple and double
  // entries
  string = "GET / HTTP/1.1\r\n"
           "Transfer-Encoding: chunked , gzip  \r\n"
           "Transfer-Encoding: undef \r\n"
           "Transfer-Encoding: chunked , gzip  \r\n"
           "Transfer-Encoding: something; key=val  \r\n"
           "Transfer-Encoding: next; key = val , other; key = val  \r\n"
           "Transfer-Encoding: chunked\r\n"
           "\r\n"
           "4\r\nbody\r\n0\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, NULL));

  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 0));
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 1));
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 2));
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 3));
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 4));
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 5));
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 6));
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 7));
  testrun(DTN_HTTP_PARSER_OOB ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 8));
  testrun(DTN_HTTP_PARSER_SUCCESS ==
          dtn_http_pointer_parse_transfer_encodings(msg, array, 9));
  // check array
  testrun(0 == strncmp("chunked", (char *)array[0].start, array[0].length));
  testrun(0 == strncmp("gzip", (char *)array[1].start, array[1].length));
  testrun(0 == strncmp("undef", (char *)array[2].start, array[2].length));
  testrun(0 == strncmp("chunked", (char *)array[3].start, array[3].length));
  testrun(0 == strncmp("gzip", (char *)array[4].start, array[4].length));
  testrun(0 == strncmp("something; key=val", (char *)array[5].start,
                       array[5].length));
  testrun(0 ==
          strncmp("next; key = val", (char *)array[6].start, array[6].length));
  testrun(0 ==
          strncmp("other; key = val", (char *)array[7].start, array[7].length));
  testrun(0 == strncmp("chunked", (char *)array[8].start, array[8].length));
  for (size_t i = 9; i < 10; i++) {
    testrun(NULL == array[i].start);
    testrun(0 == array[i].length);
  }

  // check comma only
  string = "GET / HTTP/1.1\r\n"
           "Transfer-Encoding: ,\r\n"
           "\r\n"
           "4\r\nbody\r\n0\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_message(msg, NULL));

  // check comma failures
  string = "GET / HTTP/1.1\r\n"
           "Transfer-Encoding: ,chunked\r\n"
           "\r\n"
           "4\r\nbody\r\n0\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_message(msg, NULL));

  // check comma failures
  string = "GET / HTTP/1.1\r\n"
           "Transfer-Encoding: test,\r\n"
           "\r\n"
           "4\r\nbody\r\n0\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_message(msg, NULL));

  // check comma failures
  string = "GET / HTTP/1.1\r\n"
           "Transfer-Encoding: test, ,chunked\r\n"
           "\r\n"
           "4\r\nbody\r\n0\r\n";

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_message(msg, NULL));

  testrun(NULL == dtn_http_message_free(msg));
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_validate_tranfer_encoding_grammar() {

  char *string = "chunked";

  dtn_memory_pointer ptr = {0};

  testrun(!validate_tranfer_encoding_grammar(NULL, 0, NULL));
  testrun(!validate_tranfer_encoding_grammar((uint8_t *)string, 0, NULL));
  testrun(!validate_tranfer_encoding_grammar(NULL, strlen(string), NULL));

  testrun(validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                            NULL));
  testrun(validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                            &ptr));
  testrun(0 == strncmp(string, (char *)ptr.start, ptr.length));

  string = "compress";
  testrun(validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                            NULL));

  string = "deflate";
  testrun(validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                            NULL));

  string = "gzip";
  testrun(validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                            NULL));

  // non token
  string = "gz{ip";
  testrun(!validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                             NULL));

  // unsupported whitespace
  string = " compress";
  testrun(!validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                             NULL));
  string = "compress ";
  testrun(!validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                             NULL));

  // extension
  string = "extension";
  testrun(validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                            NULL));

  // extension with parameter
  string = "extension;key=val";
  testrun(validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                            NULL));

  // extension with multiple parameter
  string = "extension;key=val;key=val;key=val;key=val";
  testrun(validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                            &ptr));
  testrun(0 == strncmp("extension", (char *)ptr.start, ptr.length));

  // extension with multiple parameter and supported whitespace
  string = "extension  ;  key = val  ;  key =  val     ; key  = val";
  testrun(validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                            &ptr));
  testrun(0 == strncmp("extension", (char *)ptr.start, ptr.length));

  // extension with wrong key value paring
  string = "extension;key=";
  testrun(!validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                             NULL));
  string = "extension;key= ";
  testrun(!validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                             NULL));
  string = "extension;=val";
  testrun(!validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                             NULL));
  string = "extension;key=val;";
  testrun(!validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                             NULL));
  string = "extension;";
  testrun(!validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                             NULL));
  string = "extension;k=v;;";
  testrun(!validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                             NULL));

  // extension with non token val
  string = "extension;key=val]ue";
  testrun(!validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                             NULL));

  // extension with non token key
  string = "extension;ke[y=value";
  testrun(!validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                             NULL));

  // extension with quoted string val
  string = "extension;key=\"va[]lue\"";
  testrun(validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                            NULL));

  // extension with multiple parameter and supported whitespace and non token
  string = "extension  ;  key=val  ;  key=v}al     ; key=val ;   key=val";
  testrun(!validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                             &ptr));
  string = "extension  ;  k}ey=val  ;  key=val     ; key=val ;   key=val";
  testrun(!validate_tranfer_encoding_grammar((uint8_t *)string, strlen(string),
                                             &ptr));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_http_is_qd_char() {

  for (size_t i = 0; i < 0xff; i++) {

    // fprintf(stdout, "i %i\n", i);

    if (i >= 0x80) {
      // obs-text
      testrun(http_is_qd_char(i));

    } else if (i == 0x5C) {

      testrun(!http_is_qd_char(i));

    } else if ((i >= 0x5C) && (i <= 0x7E)) {

      testrun(http_is_qd_char(i));

    } else if ((i >= 0x23) && (i <= 0x5B)) {

      testrun(http_is_qd_char(i));

    } else {

      switch (i) {

      case 0x20: // space
      case 0x09: // HTAB
      case '!':
      case '#':
      case '-':
      case '[':
      case ']':
      case '~':
        testrun(http_is_qd_char(i));
        break;
      default:
        testrun(!http_is_qd_char(i));
      }
    }
  }

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_http_is_quoted_pair() {

  uint8_t buffer[10] = {0};

  buffer[0] = 0x5C;
  buffer[1] = 0x09;
  buffer[2] = 'a';

  testrun(!http_is_quoted_pair(NULL, 0));
  testrun(!http_is_quoted_pair(NULL, 2));
  testrun(!http_is_quoted_pair(buffer, 0));
  testrun(!http_is_quoted_pair(buffer, 1));
  testrun(http_is_quoted_pair(buffer, 2));
  testrun(http_is_quoted_pair(buffer, 3));
  testrun(http_is_quoted_pair(buffer, 4));
  testrun(http_is_quoted_pair(buffer, 5));

  for (size_t i = 0; i < 0xff; i++) {

    buffer[1] = i;

    if (http_is_whitespace(i)) {
      testrun(http_is_quoted_pair(buffer, 2));
    } else if (http_is_control(i)) {
      testrun(!http_is_quoted_pair(buffer, 2));
    } else {
      testrun(http_is_quoted_pair(buffer, 2));
    }
  }

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_http_is_quoted_string() {

  char *string = "\"test\"";

  testrun(!http_is_quoted_string(NULL, 0));
  testrun(!http_is_quoted_string((uint8_t *)string, 0));
  testrun(!http_is_quoted_string(NULL, strlen(string)));

  testrun(http_is_quoted_string((uint8_t *)string, strlen(string)));

  // no closing dquote
  testrun(!http_is_quoted_string((uint8_t *)string, strlen(string) - 1));
  // no opennig dquote
  string = "test\"";
  testrun(!http_is_quoted_string((uint8_t *)string, strlen(string)));

  string = "\"some text \"";
  testrun(http_is_quoted_string((uint8_t *)string, strlen(string)));

  // quoted pair contained
  string = "\"some text with \\\t quoted htab\"";
  testrun(http_is_quoted_string((uint8_t *)string, strlen(string)));

  // incomplete quoted pair contained
  string = "\"some text with \\\"";
  testrun(!http_is_quoted_string((uint8_t *)string, strlen(string)));

  // multiple quoted pair contained
  string = "\"some \\\ttext \\\twith \\\t quoted htab\"";
  testrun(http_is_quoted_string((uint8_t *)string, strlen(string)));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_check_chunk_extension_grammar() {

  char *string = ";key=val";

  testrun(!check_chunk_extension_grammar(NULL, 0));
  testrun(!check_chunk_extension_grammar((uint8_t *)string, 0));
  testrun(!check_chunk_extension_grammar(NULL, strlen(string)));

  testrun(check_chunk_extension_grammar((uint8_t *)string, strlen(string)));

  // no content
  string = ";";
  testrun(!check_chunk_extension_grammar((uint8_t *)string, strlen(string)));

  // multi content
  string = ";1=2;3=4;5=6";
  testrun(check_chunk_extension_grammar((uint8_t *)string, strlen(string)));

  // multi content with empty last
  string = ";1=2;3=4;5=6;";
  testrun(!check_chunk_extension_grammar((uint8_t *)string, strlen(string)));

  // multi content with empty middle
  string = ";1=2;3=4;;5=6";
  testrun(!check_chunk_extension_grammar((uint8_t *)string, strlen(string)));

  // multi content with token failure in key
  string = ";1=2;ke]y=4;5=6";
  testrun(!check_chunk_extension_grammar((uint8_t *)string, strlen(string)));

  // multi content with token failure in val
  string = ";1=2;key=va]l;5=6";
  testrun(!check_chunk_extension_grammar((uint8_t *)string, strlen(string)));

  // multi content with quoted string val
  string = ";1=2;key=\"quoted[]string\";5=6";
  testrun(check_chunk_extension_grammar((uint8_t *)string, strlen(string)));

  // multi content with non token non quoted string
  string = ";1=2;key=quoted[]string;5=6";
  testrun(!check_chunk_extension_grammar((uint8_t *)string, strlen(string)));

  // multi content with quoted string space
  string = ";1=2;key=\"quoted string\";5=6";
  testrun(check_chunk_extension_grammar((uint8_t *)string, strlen(string)));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_parse_chunked() {

  dtn_http_message *msg = dtn_http_message_create((dtn_http_message_config){0});

  char *string = "GET / HTTP/1.1\r\n\r\n";
  uint8_t *next = NULL;

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));

  testrun(msg->body.start == NULL);

  // no message body contained
  testrun(DTN_HTTP_PARSER_ERROR == parse_chunked(msg, NULL));

  // set body for testing
  //        012345678901234-5-6-7-89-0-12345-6-
  string = "GET / HTTP/1.1\r\n\r\n4\r\nbody\r\n";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(msg->body.start == NULL);
  testrun(DTN_HTTP_PARSER_ERROR == parse_chunked(msg, NULL));
  msg->body.start = msg->buffer->start + 18;
  testrun(DTN_HTTP_PARSER_SUCCESS == parse_chunked(msg, &next));
  testrun(0 == strncmp("body", (char *)msg->chunk.start, msg->chunk.length));
  testrun(next == msg->buffer->start + msg->buffer->length);
  testrun(msg->body.length == strlen("4\r\nbody\r\n"));
  size_t len = msg->buffer->length;
  for (size_t i = 18; i < len - 1; i++) {
    msg->buffer->length = i;
    testrun(DTN_HTTP_PARSER_PROGRESS == parse_chunked(msg, &next));
  }

  // parse with chunk-ext
  string = "GET / HTTP/1.1\r\n\r\n4;k=v\r\nbody\r\n";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(msg->body.start == NULL);
  testrun(DTN_HTTP_PARSER_ERROR == parse_chunked(msg, NULL));
  msg->body.start = msg->buffer->start + 18;
  testrun(DTN_HTTP_PARSER_SUCCESS == parse_chunked(msg, &next));
  testrun(0 == strncmp("body", (char *)msg->chunk.start, msg->chunk.length));
  testrun(next == msg->buffer->start + msg->buffer->length);
  testrun(msg->body.length == strlen("4;k=v\r\nbody\r\n"));

  // parse with multiple chunk-ext
  string = "GET / HTTP/1.1\r\n\r\n4;0=0;1=1;2=2\r\nbody\r\n";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(msg->body.start == NULL);
  testrun(DTN_HTTP_PARSER_ERROR == parse_chunked(msg, NULL));
  msg->body.start = msg->buffer->start + 18;
  testrun(DTN_HTTP_PARSER_SUCCESS == parse_chunked(msg, &next));
  testrun(0 == strncmp("body", (char *)msg->chunk.start, msg->chunk.length));
  testrun(next == msg->buffer->start + msg->buffer->length);
  testrun(msg->body.length == strlen("4;0=0;1=1;2=2\r\nbody\r\n"));

  // parse with invalid chunk-ext
  string = "GET / HTTP/1.1\r\n\r\n4;0=0;11;2=2\r\nbody\r\n";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(msg->body.start == NULL);
  msg->body.start = msg->buffer->start + 18;
  testrun(DTN_HTTP_PARSER_ERROR == parse_chunked(msg, &next));

  // parse with invalid chunk-ext
  string = "GET / HTTP/1.1\r\n\r\n4;0=0;1=;2=2\r\nbody\r\n";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(msg->body.start == NULL);
  msg->body.start = msg->buffer->start + 18;
  testrun(DTN_HTTP_PARSER_ERROR == parse_chunked(msg, &next));

  // parse with invalid chunk-ext
  string = "GET / HTTP/1.1\r\n\r\n4;0=0;1]=123;2=2\r\nbody\r\n";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(msg->body.start == NULL);
  msg->body.start = msg->buffer->start + 18;
  testrun(DTN_HTTP_PARSER_ERROR == parse_chunked(msg, &next));

  // parse with invalid chunk-ext
  string = "GET / HTTP/1.1\r\n\r\n4;0=0;1=[];2=2\r\nbody\r\n";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(msg->body.start == NULL);
  msg->body.start = msg->buffer->start + 18;
  testrun(DTN_HTTP_PARSER_ERROR == parse_chunked(msg, &next));

  // parse with multiple chunk-ext
  string = "GET / HTTP/1.1\r\n\r\n4;0=0;1=\"quoted\";2=2\r\nbody\r\n";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(msg->body.start == NULL);
  testrun(DTN_HTTP_PARSER_ERROR == parse_chunked(msg, NULL));
  msg->body.start = msg->buffer->start + 18;
  testrun(DTN_HTTP_PARSER_SUCCESS == parse_chunked(msg, &next));
  testrun(0 == strncmp("body", (char *)msg->chunk.start, msg->chunk.length));
  testrun(next == msg->buffer->start + msg->buffer->length);
  testrun(msg->body.length == strlen("4;0=0;1=\"quoted\";2=2\r\nbody\r\n"));

  // parse with inclomplet line
  string = "GET / HTTP/1.1\r\n\r\n4;0=0;1=\"quoted\";2=2\r\nbody\r\n";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(msg->body.start == NULL);
  testrun(DTN_HTTP_PARSER_ERROR == parse_chunked(msg, &next));
  msg->body.start = msg->buffer->start + 18;

  for (size_t i = 0; i <= strlen(string); i++) {

    msg->buffer->length = i;

    if ((size_t)(msg->body.start - msg->buffer->start) > msg->buffer->length) {
      testrun(DTN_HTTP_PARSER_ERROR == parse_chunked(msg, &next));
    } else if (i < strlen(string)) {
      testrun(DTN_HTTP_PARSER_PROGRESS == parse_chunked(msg, &next));
    } else {
      testrun(DTN_HTTP_PARSER_SUCCESS == parse_chunked(msg, &next));
    }
  }
  // reparse chunked
  testrun(DTN_HTTP_PARSER_SUCCESS == parse_chunked(msg, &next));
  testrun(0 == strncmp("body", (char *)msg->chunk.start, msg->chunk.length));
  testrun(next == msg->buffer->start + msg->buffer->length);
  testrun(msg->body.length == strlen("4;0=0;1=\"quoted\";2=2\r\nbody\r\n"));

  // parse END with multiple chunk-ext
  //        012345678901234-5-6-7-89012345678901-2-3-4-5
  string = "GET / HTTP/1.1\r\n\r\n0;0=0;1=1;2=2\r\n\r\nnext";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(msg->body.start == NULL);
  testrun(DTN_HTTP_PARSER_ERROR == parse_chunked(msg, NULL));
  msg->body.start = msg->buffer->start + 18;
  testrun(DTN_HTTP_PARSER_SUCCESS == parse_chunked(msg, &next));
  testrun(msg->chunk.length == 0);
  testrun(msg->chunk.start[0] == '\r');
  testrun(msg->chunk.start == msg->buffer->start + 33);
  testrun(msg->body.length == strlen("0;0=0;1=1;2=2\r\n\r\n"));
  testrun(next[0] == 'n');
  testrun(next == msg->buffer->start + 35);

  // parse END without chunk-ext
  //        012345678901234-5-6-7-89-0-1-2-3456
  string = "GET / HTTP/1.1\r\n\r\n0\r\n\r\nnext";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(msg->body.start == NULL);
  testrun(DTN_HTTP_PARSER_ERROR == parse_chunked(msg, NULL));
  msg->body.start = msg->buffer->start + 18;
  testrun(DTN_HTTP_PARSER_SUCCESS == parse_chunked(msg, &next));
  testrun(msg->chunk.length == 0);
  testrun(msg->chunk.start[0] == '\r');
  testrun(msg->chunk.start == msg->buffer->start + 21);
  testrun(msg->body.length == strlen("0\r\n\r\n"));
  testrun(next[0] == 'n');
  testrun(next == msg->buffer->start + 23);

  testrun(NULL == dtn_http_message_free(msg));
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_pointer_parse_message() {

  dtn_http_message *msg = dtn_http_message_create((dtn_http_message_config){0});

  //              012345678901234-5-6-7-
  char *string = "GET / HTTP/1.1\r\n\r\n";
  uint8_t *next = NULL;

  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_message(NULL, NULL));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_message(msg, NULL));

  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(msg->body.start == NULL);
  testrun(msg->body.length == 0);
  testrun(msg->chunk.start == NULL);
  testrun(msg->chunk.length == 0);
  testrun(msg->version.major == 1);
  testrun(msg->version.minor == 1);
  testrun(0 == strncasecmp("get", (char *)msg->request.method.start,
                           msg->request.method.length));
  testrun(0 == strncmp("/", (char *)msg->request.uri.start,
                       msg->request.uri.length));
  testrun(msg->status.code == 0);
  testrun(msg->status.phrase.start == NULL);
  testrun(msg->status.phrase.length == 0);
  testrun(NULL == msg->header[0].name.start);
  testrun(NULL == msg->header[0].value.start);
  testrun(next == msg->buffer->start + 18);

  string = "GET / HTTP/1.1\r\n\r\nbody\r\n";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(msg->body.start == NULL);
  testrun(msg->body.length == 0);
  testrun(msg->chunk.start == NULL);
  testrun(msg->chunk.length == 0);
  testrun(msg->version.major == 1);
  testrun(msg->version.minor == 1);
  testrun(0 == strncasecmp("get", (char *)msg->request.method.start,
                           msg->request.method.length));
  testrun(0 == strncmp("/", (char *)msg->request.uri.start,
                       msg->request.uri.length));
  testrun(msg->status.code == 0);
  testrun(msg->status.phrase.start == NULL);
  testrun(msg->status.phrase.length == 0);
  testrun(NULL == msg->header[0].name.start);
  testrun(NULL == msg->header[0].value.start);
  testrun(next == msg->buffer->start + 18);
  // this body is not a valid part of the message
  testrun(0 == strcmp((char *)next, "body\r\n"));

  // parse with content length set
  //        012345678901234-5-67890123456789012-3-4-5-67890-1-2
  string = "GET / HTTP/1.1\r\nContent-Length:4\r\n\r\nbody\r\nx";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(0 == strncmp("body", (char *)msg->body.start, msg->body.length));
  testrun(msg->body.start == msg->buffer->start + 36);
  testrun(msg->body.length == 4);
  testrun(msg->chunk.start == NULL);
  testrun(msg->chunk.length == 0);
  testrun(msg->version.major == 1);
  testrun(msg->version.minor == 1);
  testrun(0 == strncasecmp("get", (char *)msg->request.method.start,
                           msg->request.method.length));
  testrun(0 == strncmp("/", (char *)msg->request.uri.start,
                       msg->request.uri.length));
  testrun(msg->status.code == 0);
  testrun(msg->status.phrase.start == NULL);
  testrun(msg->status.phrase.length == 0);
  testrun(0 == strncmp("Content-Length", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(NULL == msg->header[1].name.start);
  testrun(NULL == msg->header[1].value.start);
  testrun(next == msg->buffer->start + 40);
  // this body is not a valid part of the message
  testrun(0 == strcmp((char *)next, "\r\nx"));
  size_t len = msg->buffer->length;
  for (size_t i = 1; i < len; i++) {
    msg->buffer->length = i;
    if (i < 40) {
      testrun(DTN_HTTP_PARSER_PROGRESS ==
              dtn_http_pointer_parse_message(msg, &next));
    } else {
      testrun(DTN_HTTP_PARSER_SUCCESS ==
              dtn_http_pointer_parse_message(msg, &next));
    }
  }

  // parse with transfer set
  //        012345678901234-5-67890123456789012345678901-2-3-4-56-7-89012-3-4
  string = "GET / "
           "HTTP/1.1\r\nTransfer-Encoding:chunked\r\n\r\n4\r\nbody\r\nx";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(0 ==
          strncmp("4\r\nbody\r\n", (char *)msg->body.start, msg->body.length));
  testrun(msg->body.start == msg->buffer->start + 45);
  testrun(msg->body.length == 9);
  testrun(0 == strncmp("body", (char *)msg->chunk.start, msg->chunk.length));
  testrun(msg->chunk.start == msg->buffer->start + 48);
  testrun(msg->chunk.length == 4);
  testrun(msg->version.major == 1);
  testrun(msg->version.minor == 1);
  testrun(0 == strncasecmp("get", (char *)msg->request.method.start,
                           msg->request.method.length));
  testrun(0 == strncmp("/", (char *)msg->request.uri.start,
                       msg->request.uri.length));
  testrun(msg->status.code == 0);
  testrun(msg->status.phrase.start == NULL);
  testrun(msg->status.phrase.length == 0);
  testrun(0 == strncmp("Transfer-Encoding", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("chunked", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));
  testrun(NULL == msg->header[1].name.start);
  testrun(NULL == msg->header[1].value.start);
  testrun(next == msg->buffer->start + 54);
  // this body is not a valid part of the message
  testrun(0 == strcmp((char *)next, "x"));
  len = msg->buffer->length;
  for (size_t i = 1; i < len; i++) {
    msg->buffer->length = i;
    if (i < 54) {
      testrun(DTN_HTTP_PARSER_PROGRESS ==
              dtn_http_pointer_parse_message(msg, &next));
    } else {
      testrun(DTN_HTTP_PARSER_SUCCESS ==
              dtn_http_pointer_parse_message(msg, &next));
    }
  }

  // parse with transfer and content length set
  //        012345678901234-5-67890123456789012345678901-2-34567890123456789-0-1-2-3
  string = "GET / "
           "HTTP/"
           "1.1\r\nTransfer-Encoding:chunked\r\nContent-Length:"
           "4\r\n\r\n4\r\nbody\r\nx";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_message(msg, &next));
  len = msg->buffer->length;
  for (size_t i = 1; i < len; i++) {
    msg->buffer->length = i;
    if (i < 63) {
      testrun(DTN_HTTP_PARSER_PROGRESS ==
              dtn_http_pointer_parse_message(msg, &next));
    } else {
      testrun(DTN_HTTP_PARSER_ERROR ==
              dtn_http_pointer_parse_message(msg, &next));
    }
  }

  // header order changed
  string = "GET / "
           "HTTP/"
           "1.1\r\nContent-Length:4\r\nTransfer-Encoding:"
           "chunked\r\n\r\n4\r\nbody\r\nx";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_message(msg, &next));
  len = msg->buffer->length;
  for (size_t i = 1; i < len; i++) {
    msg->buffer->length = i;
    if (i < 63) {
      testrun(DTN_HTTP_PARSER_PROGRESS ==
              dtn_http_pointer_parse_message(msg, &next));
    } else {
      testrun(DTN_HTTP_PARSER_ERROR ==
              dtn_http_pointer_parse_message(msg, &next));
    }
  }

  // parse with transfer and other headers set
  //        012345678901234-5-67890123456789012345678901-2-3456-7-8901-2-3-4-56-7-89012-3-4
  string = "GET / "
           "HTTP/"
           "1.1\r\nTransfer-Encoding:chunked\r\n1:1\r\n2:"
           "2\r\n\r\n4\r\nbody\r\nx";
  testrun(dtn_buffer_clear(msg->buffer));
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));
  testrun(0 ==
          strncmp("4\r\nbody\r\n", (char *)msg->body.start, msg->body.length));
  testrun(msg->body.start == msg->buffer->start + 55);
  testrun(msg->body.length == 9);
  testrun(0 == strncmp("body", (char *)msg->chunk.start, msg->chunk.length));
  testrun(msg->chunk.start == msg->buffer->start + 58);
  testrun(msg->chunk.length == 4);
  testrun(msg->version.major == 1);
  testrun(msg->version.minor == 1);
  testrun(0 == strncasecmp("get", (char *)msg->request.method.start,
                           msg->request.method.length));
  testrun(0 == strncmp("/", (char *)msg->request.uri.start,
                       msg->request.uri.length));
  testrun(msg->status.code == 0);
  testrun(msg->status.phrase.start == NULL);
  testrun(msg->status.phrase.length == 0);
  testrun(0 == strncmp("Transfer-Encoding", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("chunked", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));
  testrun(0 == strncmp("1", (char *)msg->header[1].value.start,
                       msg->header[1].value.length));
  testrun(0 == strncmp("2", (char *)msg->header[2].value.start,
                       msg->header[2].value.length));
  testrun(NULL == msg->header[3].name.start);
  testrun(NULL == msg->header[3].value.start);
  testrun(next == msg->buffer->start + 64);
  // next is not a valid part of the message
  testrun(0 == strcmp((char *)next, "x"));
  len = msg->buffer->length;
  for (size_t i = 1; i < len; i++) {
    msg->buffer->length = i;
    if (i < 64) {
      testrun(DTN_HTTP_PARSER_PROGRESS ==
              dtn_http_pointer_parse_message(msg, &next));
    } else {
      testrun(DTN_HTTP_PARSER_SUCCESS ==
              dtn_http_pointer_parse_message(msg, &next));
    }
  }

  msg = dtn_http_message_free(msg);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_shift_trailing_bytes() {

  dtn_http_message *msg = dtn_http_message_create((dtn_http_message_config){0});

  dtn_http_message *dest = NULL;

  //              012345678901234-5-6-7-890123456789012-3-4-5-
  char *string = "GET / HTTP/1.1\r\n\r\nPUT / HTTP/2.3\r\n\r\n";
  uint8_t *next = NULL;
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));

  testrun(!dtn_http_message_shift_trailing_bytes(NULL, NULL, NULL));
  testrun(!dtn_http_message_shift_trailing_bytes(msg, next, NULL));
  testrun(!dtn_http_message_shift_trailing_bytes(NULL, next, &dest));
  testrun(!dtn_http_message_shift_trailing_bytes(msg, NULL, &dest));

  testrun(!dest);
  testrun(dtn_http_message_shift_trailing_bytes(msg, next, &dest));
  testrun(dest);
  testrun(msg->buffer->start[0] == 'G');
  testrun(msg->buffer->length == 18);
  testrun(dest->buffer->start[0] == 'P');
  testrun(dest->buffer->length == 18);

  // check dest
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(dest, &next));
  testrun(dest->body.start == NULL);
  testrun(dest->body.length == 0);
  testrun(dest->chunk.start == NULL);
  testrun(dest->chunk.length == 0);
  testrun(dest->version.major == 2);
  testrun(dest->version.minor == 3);
  testrun(0 == strncasecmp("put", (char *)dest->request.method.start,
                           dest->request.method.length));
  testrun(0 == strncmp("/", (char *)dest->request.uri.start,
                       dest->request.uri.length));
  testrun(dest->status.code == 0);
  testrun(dest->status.phrase.start == NULL);
  testrun(dest->status.phrase.length == 0);
  testrun(NULL == dest->header[0].name.start);
  testrun(NULL == dest->header[0].value.start);
  testrun(next == dest->buffer->start + 18);
  dest = dtn_http_message_free(dest);

  // check shift without trailing bytes
  string = "GET / HTTP/1.1\r\n\r\n";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));

  testrun(!dest);
  testrun(dtn_http_message_shift_trailing_bytes(msg, next, &dest));
  testrun(!dest);
  testrun(msg->buffer->start[0] == 'G');
  testrun(msg->buffer->length == 18);
  dest = dtn_http_message_free(dest);

  // check shift with non http trailing bytes
  //        012345678901234-5-6-7-
  string = "GET / HTTP/1.1\r\n\r\nunspec";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, &next));

  testrun(!dest);
  testrun(dtn_http_message_shift_trailing_bytes(msg, next, &dest));
  testrun(dest);
  testrun(msg->buffer->start[0] == 'G');
  testrun(msg->buffer->length == 18);
  testrun(dest->buffer->start[0] == 'u');
  testrun(dest->buffer->length == strlen("unspec"));
  testrun(DTN_HTTP_PARSER_PROGRESS ==
          dtn_http_pointer_parse_message(dest, &next));
  // longer method than default max_method used here to force the parsing to
  // complete
  strcat((char *)dest->buffer->start, "ified");
  dest->buffer->length = strlen("unspecified");
  testrun(0 == strncmp("unspecified", (char *)dest->buffer->start,
                       dest->buffer->length));
  testrun(DTN_HTTP_PARSER_ERROR == dtn_http_pointer_parse_message(dest, &next));

  size_t len = dest->buffer->length;
  for (size_t i = 1; i < len; i++) {
    dest->buffer->length = i;
    if (i < IMPL_DEFAULT_MAX_METHOD_NAME) {
      testrun(DTN_HTTP_PARSER_PROGRESS ==
              dtn_http_pointer_parse_message(dest, &next));
    } else {
      testrun(DTN_HTTP_PARSER_ERROR ==
              dtn_http_pointer_parse_message(dest, &next));
    }
  }

  dest->buffer->length = len;

  dest = dtn_http_message_free(dest);
  msg = dtn_http_message_free(msg);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_ensure_open_capacity() {

  dtn_http_message *msg = dtn_http_message_create(
      (dtn_http_message_config){.buffer.default_size = 100});

  testrun(msg);
  testrun(msg->buffer->capacity == 100);

  testrun(!dtn_http_message_ensure_open_capacity(NULL, 0));
  testrun(!dtn_http_message_ensure_open_capacity(msg, 0));
  testrun(!dtn_http_message_ensure_open_capacity(NULL, 10));
  testrun(dtn_http_message_ensure_open_capacity(msg, 10));
  testrun(msg->buffer->start[0] == 0);
  testrun(msg->buffer->length == 0);
  testrun(msg->buffer->capacity == 100);
  for (size_t i = 0; i < msg->buffer->capacity; i++) {
    testrun(msg->buffer->start[i] == 0);
  }

  testrun(dtn_http_message_ensure_open_capacity(msg, 110));
  testrun(msg->buffer->start[0] == 0);
  testrun(msg->buffer->length == 0);
  testrun(msg->buffer->capacity == 110);
  for (size_t i = 0; i < msg->buffer->capacity; i++) {
    testrun(msg->buffer->start[i] == 0);
  }

  // extend with content
  char *string = "some content";
  testrun(dtn_buffer_set(msg->buffer, string, strlen(string)));
  testrun(0 ==
          strncmp(string, (char *)msg->buffer->start, msg->buffer->length));

  testrun(dtn_http_message_ensure_open_capacity(msg, 10));
  testrun(0 ==
          strncmp(string, (char *)msg->buffer->start, msg->buffer->length));
  testrun(msg->buffer->length == strlen(string));
  testrun(msg->buffer->capacity == 110);
  for (size_t i = 0; i < msg->buffer->capacity; i++) {
    if (i > strlen(string)) {
      testrun(msg->buffer->start[i] == 0);
    }
  }

  testrun(dtn_http_message_ensure_open_capacity(msg, 200));
  testrun(0 ==
          strncmp(string, (char *)msg->buffer->start, msg->buffer->length));
  testrun(msg->buffer->length == strlen(string));
  testrun(msg->buffer->capacity == strlen(string) + 200);
  for (size_t i = 0; i < msg->buffer->capacity; i++) {
    if (i > strlen(string)) {
      testrun(msg->buffer->start[i] == 0);
    }
  }
  msg = dtn_http_message_free(msg);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_create_status() {

  dtn_http_message *msg = NULL;

  testrun(!dtn_http_create_status((dtn_http_message_config){0},
                                 (dtn_http_version){0}, (dtn_http_status){0}));

  testrun(!dtn_http_create_status((dtn_http_message_config){0},
                                 (dtn_http_version){0},
                                 (dtn_http_status){.code = 1}));

  testrun(
      !dtn_http_create_status((dtn_http_message_config){0}, (dtn_http_version){0},
                             (dtn_http_status){.phrase.start = (uint8_t *)"x"}));

  testrun(!dtn_http_create_status((dtn_http_message_config){0},
                                 (dtn_http_version){0},
                                 (dtn_http_status){.phrase.length = 1}));

  testrun(!dtn_http_create_status(
      (dtn_http_message_config){0}, (dtn_http_version){0},
      (dtn_http_status){.phrase.start = (uint8_t *)"x", .phrase.length = 1}));

  testrun(!dtn_http_create_status(
      (dtn_http_message_config){0}, (dtn_http_version){0},
      (dtn_http_status){
          .code = 99, .phrase.start = (uint8_t *)"x", .phrase.length = 1}));

  testrun(!dtn_http_create_status(
      (dtn_http_message_config){0}, (dtn_http_version){0},
      (dtn_http_status){
          .code = 1000, .phrase.start = (uint8_t *)"x", .phrase.length = 1}));

  // min valid input
  msg = dtn_http_create_status((dtn_http_message_config){0}, (dtn_http_version){0},
                              (dtn_http_status){.code = 100,
                                               .phrase.start = (uint8_t *)"x",
                                               .phrase.length = 1});

  testrun(msg);
  uint8_t *next = NULL;
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_status_line(
                                        msg->buffer->start, msg->buffer->length,
                                        &msg->status, &msg->version, &next));

  testrun(next == msg->buffer->start + msg->buffer->length);
  testrun(100 == msg->status.code);
  testrun(0 == msg->version.major);
  testrun(0 == msg->version.minor);
  testrun(0 == strncmp("x", (char *)msg->status.phrase.start,
                       msg->status.phrase.length));
  testrun(0 == strncmp("HTTP/0.0 100 x\r\n", (char *)msg->buffer->start,
                       msg->buffer->length));
  msg = dtn_http_message_free(msg);

  dtn_http_status status = {
      .code = 0, .phrase.start = (uint8_t *)"test", .phrase.length = 4};

  // check 3 digit code only
  for (size_t i = 0; i < 2000; i++) {

    status.code = i;

    if (i < 100) {

      testrun(!dtn_http_create_status((dtn_http_message_config){0},
                                     (dtn_http_version){0}, status));

    } else if (i > 999) {

      testrun(!dtn_http_create_status((dtn_http_message_config){0},
                                     (dtn_http_version){0}, status));

    } else {

      msg = dtn_http_create_status((dtn_http_message_config){0},
                                  (dtn_http_version){0}, status);

      testrun(msg);
      testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_status_line(
                                            msg->buffer->start,
                                            msg->buffer->length, &msg->status,
                                            &msg->version, &next));

      testrun(next == msg->buffer->start + msg->buffer->length);
      testrun(i == msg->status.code);
      testrun(0 == msg->version.major);
      testrun(0 == msg->version.minor);
      testrun(0 == strncmp("test", (char *)msg->status.phrase.start,
                           msg->status.phrase.length));
      msg = dtn_http_message_free(msg);
    }
  }

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_status_not_found() {

  dtn_http_message *in = dtn_http_create_request_string(
      (dtn_http_message_config){0}, (dtn_http_version){.major = 1, .minor = 1},
      "method", "uri");

  testrun(in);

  dtn_http_message *out = dtn_http_status_not_found(in);
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(out, NULL));
  testrun(out->status.code == 404);
  testrun(0 == memcmp(DTN_HTTP_NOT_FOUND, out->status.phrase.start,
                      out->status.phrase.length));

  in = dtn_http_message_free(in);
  out = dtn_http_message_free(out);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_status_forbidden() {

  dtn_http_message *in = dtn_http_create_request_string(
      (dtn_http_message_config){0}, (dtn_http_version){.major = 1, .minor = 1},
      "method", "uri");

  testrun(in);

  dtn_http_message *out = dtn_http_status_forbidden(in);
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(out, NULL));
  testrun(out->status.code == 403);
  testrun(0 == memcmp(DTN_HTTP_FORBIDDEN, out->status.phrase.start,
                      out->status.phrase.length));

  in = dtn_http_message_free(in);
  out = dtn_http_message_free(out);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_create_status_string() {

  dtn_http_message *msg = NULL;

  testrun(!dtn_http_create_status_string((dtn_http_message_config){0},
                                        (dtn_http_version){0}, 0, "test"));

  testrun(!dtn_http_create_status_string((dtn_http_message_config){0},
                                        (dtn_http_version){0}, 100, NULL));

  // min valid input
  msg = dtn_http_create_status_string((dtn_http_message_config){0},
                                     (dtn_http_version){0}, 100, "x");

  testrun(msg);
  uint8_t *next = NULL;
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_status_line(
                                        msg->buffer->start, msg->buffer->length,
                                        &msg->status, &msg->version, &next));

  testrun(next == msg->buffer->start + msg->buffer->length);
  testrun(100 == msg->status.code);
  testrun(0 == msg->version.major);
  testrun(0 == msg->version.minor);
  testrun(0 == strncmp("x", (char *)msg->status.phrase.start,
                       msg->status.phrase.length));
  testrun(0 == strncmp("HTTP/0.0 100 x\r\n", (char *)msg->buffer->start,
                       msg->buffer->length));
  msg = dtn_http_message_free(msg);

  // check 3 digit code only
  for (size_t i = 0; i < 2000; i++) {

    if (i < 100) {

      testrun(!dtn_http_create_status_string((dtn_http_message_config){0},
                                            (dtn_http_version){0}, i, "test"));

    } else if (i > 999) {

      testrun(!dtn_http_create_status_string((dtn_http_message_config){0},
                                            (dtn_http_version){0}, i, "test"));

    } else {

      msg = dtn_http_create_status_string((dtn_http_message_config){0},
                                         (dtn_http_version){0}, i, "test");

      testrun(msg);
      testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_status_line(
                                            msg->buffer->start,
                                            msg->buffer->length, &msg->status,
                                            &msg->version, &next));

      testrun(next == msg->buffer->start + msg->buffer->length);
      testrun(i == msg->status.code);
      testrun(0 == msg->version.major);
      testrun(0 == msg->version.minor);
      testrun(0 == strncmp("test", (char *)msg->status.phrase.start,
                           msg->status.phrase.length));
      msg = dtn_http_message_free(msg);
    }
  }

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_create_request() {

  dtn_http_message *msg = NULL;

  testrun(!dtn_http_create_request((dtn_http_message_config){0},
                                  (dtn_http_version){0}, (dtn_http_request){0}));

  testrun(!dtn_http_create_request((dtn_http_message_config){0},
                                  (dtn_http_version){0},
                                  (dtn_http_request){.method.start = NULL,
                                                    .method.length = 6,
                                                    .uri.start = (uint8_t *)"/",
                                                    .uri.length = 1}));

  testrun(!dtn_http_create_request(
      (dtn_http_message_config){0}, (dtn_http_version){0},
      (dtn_http_request){.method.start = (uint8_t *)"method",
                        .method.length = 0,
                        .uri.start = (uint8_t *)"/",
                        .uri.length = 1}));

  testrun(!dtn_http_create_request(
      (dtn_http_message_config){0}, (dtn_http_version){0},
      (dtn_http_request){.method.start = (uint8_t *)"method",
                        .method.length = 6,
                        .uri.start = NULL,
                        .uri.length = 1}));

  testrun(!dtn_http_create_request(
      (dtn_http_message_config){0}, (dtn_http_version){0},
      (dtn_http_request){.method.start = (uint8_t *)"method",
                        .method.length = 6,
                        .uri.start = (uint8_t *)"/",
                        .uri.length = 0}));

  testrun(!dtn_http_create_request(
      (dtn_http_message_config){0}, (dtn_http_version){0},
      (dtn_http_request){.method.start = (uint8_t *)"methodislongerasmaxlength"
                                                   "default",
                        .method.length = IMPL_DEFAULT_MAX_METHOD_NAME + 1,
                        .uri.start = (uint8_t *)"/",
                        .uri.length = 1}));

  // min valid input
  msg = dtn_http_create_request(
      (dtn_http_message_config){0}, (dtn_http_version){0},
      (dtn_http_request){.method.start = (uint8_t *)"method",
                        .method.length = 6,
                        .uri.start = (uint8_t *)"/",
                        .uri.length = 1});

  testrun(msg);
  uint8_t *next = NULL;
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_request_line(
                                        msg->buffer->start, msg->buffer->length,
                                        &msg->request, &msg->version,
                                        IMPL_DEFAULT_MAX_METHOD_NAME, &next));

  testrun(next == msg->buffer->start + msg->buffer->length);
  testrun(0 == strncmp("method", (char *)msg->request.method.start,
                       msg->request.method.length));
  testrun(0 == strncmp("/", (char *)msg->request.uri.start,
                       msg->request.uri.length));
  testrun(0 == strncmp("method / HTTP/0.0\r\n", (char *)msg->buffer->start,
                       msg->buffer->length));
  msg = dtn_http_message_free(msg);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_create_request_string() {

  dtn_http_message *msg = NULL;

  testrun(!dtn_http_create_request_string((dtn_http_message_config){0},
                                         (dtn_http_version){0}, NULL, NULL));

  testrun(!dtn_http_create_request_string((dtn_http_message_config){0},
                                         (dtn_http_version){0}, "method", NULL));

  testrun(!dtn_http_create_request_string((dtn_http_message_config){0},
                                         (dtn_http_version){0}, NULL, "/"));

  testrun(!dtn_http_create_request_string(
      (dtn_http_message_config){0}, (dtn_http_version){0},
      "methodislongerasmaxlengthdefault", "/"));

  // min valid input
  msg = dtn_http_create_request_string((dtn_http_message_config){0},
                                      (dtn_http_version){0}, "method", "/");

  testrun(msg);
  uint8_t *next = NULL;
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_request_line(
                                        msg->buffer->start, msg->buffer->length,
                                        &msg->request, &msg->version,
                                        IMPL_DEFAULT_MAX_METHOD_NAME, &next));

  testrun(next == msg->buffer->start + msg->buffer->length);
  testrun(0 == strncmp("method", (char *)msg->request.method.start,
                       msg->request.method.length));
  testrun(0 == strncmp("/", (char *)msg->request.uri.start,
                       msg->request.uri.length));
  testrun(0 == strncmp("method / HTTP/0.0\r\n", (char *)msg->buffer->start,
                       msg->buffer->length));
  msg = dtn_http_message_free(msg);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_is_request() {

  dtn_http_message *msg = dtn_http_create_request_string(
      (dtn_http_message_config){0}, (dtn_http_version){0}, "method", "/");

  testrun(msg);
  testrun(dtn_http_message_close_header(msg));
  testrun(!dtn_http_is_request(msg, "method"));

  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, NULL));
  testrun(dtn_http_is_request(msg, "method"));
  testrun(dtn_http_is_request(msg, "Method"));
  testrun(dtn_http_is_request(msg, "METHOD"));

  testrun(!dtn_http_is_request(NULL, "METHOD"));
  testrun(!dtn_http_is_request(msg, NULL));
  testrun(!dtn_http_is_request(msg, "METHO"));

  msg = dtn_http_message_free(msg);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_add_header() {

  dtn_http_message *msg = dtn_http_message_create((dtn_http_message_config){0});

  // check empty msg (no startline)
  testrun(!dtn_http_message_add_header(
      msg, (dtn_http_header){.name.start = (uint8_t *)"x",
                            .name.length = 1,
                            .value.start = (uint8_t *)"y",
                            .value.length = 1}));

  msg = dtn_http_message_free(msg);
  msg = dtn_http_create_request_string((dtn_http_message_config){0},
                                      (dtn_http_version){.major = 1, .minor = 1},
                                      "GET", "/");

  testrun(msg);

  testrun(!dtn_http_message_add_header(
      msg, (dtn_http_header){.name.start = NULL,
                            .name.length = 1,
                            .value.start = (uint8_t *)"y",
                            .value.length = 1}));

  testrun(!dtn_http_message_add_header(
      msg, (dtn_http_header){.name.start = (uint8_t *)"x",
                            .name.length = 0,
                            .value.start = (uint8_t *)"y",
                            .value.length = 1}));

  testrun(!dtn_http_message_add_header(
      msg, (dtn_http_header){.name.start = (uint8_t *)"x",
                            .name.length = 1,
                            .value.start = NULL,
                            .value.length = 1}));

  testrun(!dtn_http_message_add_header(
      msg, (dtn_http_header){.name.start = (uint8_t *)"x",
                            .name.length = 1,
                            .value.start = (uint8_t *)"y",
                            .value.length = 0}));

  testrun(dtn_http_message_add_header(
      msg, (dtn_http_header){.name.start = (uint8_t *)"x",
                            .name.length = 1,
                            .value.start = (uint8_t *)"y",
                            .value.length = 1}));

  testrun(0 == strncmp("GET / HTTP/1.1\r\nx:y\r\n", (char *)msg->buffer->start,
                       msg->buffer->length));
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_message(msg, NULL));
  testrun(0 != msg->header[0].name.start);
  testrun(0 == msg->header[1].name.start);
  testrun(0 == strncmp("x", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("y", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));
  msg = dtn_http_message_free(msg);

  msg = dtn_http_create_status_string((dtn_http_message_config){0},
                                     (dtn_http_version){0}, 100, "c");

  testrun(msg);

  testrun(dtn_http_message_add_header(
      msg, (dtn_http_header){.name.start = (uint8_t *)"x",
                            .name.length = 1,
                            .value.start = (uint8_t *)"y",
                            .value.length = 1}));

  testrun(0 == strncmp("HTTP/0.0 100 c\r\nx:y\r\n", (char *)msg->buffer->start,
                       msg->buffer->length));
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_message(msg, NULL));
  testrun(0 != msg->header[0].name.start);
  testrun(0 == strncmp("x", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("y", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));
  msg = dtn_http_message_free(msg);

  // check max bytes header line
  msg = dtn_http_create_request_string((dtn_http_message_config){0},
                                      (dtn_http_version){.major = 1, .minor = 1},
                                      "GET", "/");

  msg->config.header.max_bytes_line = 5;
  testrun(dtn_http_message_add_header(
      msg, (dtn_http_header){.name.start = (uint8_t *)"x",
                            .name.length = 1,
                            .value.start = (uint8_t *)"y",
                            .value.length = 1}));

  testrun(0 == strncmp("GET / HTTP/1.1\r\nx:y\r\n", (char *)msg->buffer->start,
                       msg->buffer->length));
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_message(msg, NULL));
  testrun(0 != msg->header[0].name.start);
  testrun(0 == strncmp("x", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("y", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));

  testrun(!dtn_http_message_add_header(
      msg, (dtn_http_header){.name.start = (uint8_t *)"x",
                            .name.length = 1,
                            .value.start = (uint8_t *)"yz",
                            .value.length = 2}));

  // check buffer is unchanged
  testrun(0 == strncmp("GET / HTTP/1.1\r\nx:y\r\n", (char *)msg->buffer->start,
                       msg->buffer->length));
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_message(msg, NULL));
  testrun(0 != msg->header[0].name.start);
  testrun(0 == msg->header[1].name.start);
  testrun(0 == strncmp("x", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("y", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));

  msg->config.header.max_bytes_line = 6;
  testrun(dtn_http_message_add_header(
      msg, (dtn_http_header){.name.start = (uint8_t *)"x",
                            .name.length = 1,
                            .value.start = (uint8_t *)"yz",
                            .value.length = 2}));

  // check buffer contains both headers
  testrun(0 == strncmp("GET / HTTP/1.1\r\nx:y\r\nx:yz\r\n",
                       (char *)msg->buffer->start, msg->buffer->length));
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_message(msg, NULL));
  testrun(0 != msg->header[0].name.start);
  testrun(0 != msg->header[1].name.start);
  testrun(0 == strncmp("x", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("y", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));
  testrun(0 == strncmp("x", (char *)msg->header[1].name.start,
                       msg->header[1].name.length));
  testrun(0 == strncmp("yz", (char *)msg->header[1].value.start,
                       msg->header[1].value.length));
  msg = dtn_http_message_free(msg);

  // check self extending of the msg->buffer in line with message config

  msg = dtn_http_create_request_string(
      (dtn_http_message_config){.header.capacity = 1000,
                               .buffer.default_size = 50},
      (dtn_http_version){.major = 1, .minor = 1}, "GET", "/");

  testrun(msg->buffer->capacity == 50)

      size_t count = 0;
  while (msg->buffer->capacity < 250) {

    testrun(dtn_http_message_add_header(
        msg, (dtn_http_header){.name.start = (uint8_t *)"x",
                              .name.length = 1,
                              .value.start = (uint8_t *)"y",
                              .value.length = 1}));

    count++;
  }

  // fprintf(stdout, "%s", (char*) msg->buffer->start);
  testrun(msg->buffer->capacity == 250);
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_message(msg, NULL));
  for (size_t i = 0; i < count; i++) {
    testrun(0 != msg->header[i].name.start);
  }
  testrun(0 == msg->header[count].name.start);
  msg = dtn_http_message_free(msg);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_add_content_type() {

  dtn_http_message *msg = dtn_http_create_request_string(
      (dtn_http_message_config){0}, (dtn_http_version){.major = 1, .minor = 1},
      "GET", "/");

  testrun(0 == memcmp("GET / HTTP/1.1\r\n", msg->buffer->start,
                      msg->buffer->length));

  char *mime = "mime";

  testrun(!dtn_http_message_add_content_type(NULL, NULL, NULL));
  testrun(!dtn_http_message_add_content_type(msg, NULL, NULL));
  testrun(!dtn_http_message_add_content_type(NULL, mime, NULL));

  testrun(dtn_http_message_add_content_type(msg, mime, NULL));
  testrun(0 == memcmp("GET / HTTP/1.1\r\nContent-Type:mime\r\n",
                      msg->buffer->start, msg->buffer->length));

  testrun(dtn_http_message_add_content_type(msg, mime, "utf-8"));
  testrun(0 == memcmp("GET / HTTP/1.1\r\n"
                      "Content-Type:mime\r\n"
                      "Content-Type:mime;charset=utf-8\r\n",
                      msg->buffer->start, msg->buffer->length));

  msg = dtn_http_message_free(msg);
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

/*
int test_dtn_http_message_add_transfer_encodings() {

  dtn_http_message *msg = dtn_http_create_request_string(
      (dtn_http_message_config){0}, (dtn_http_version){.major = 1, .minor = 1},
      "GET", "/");

  testrun(0 == memcmp("GET / HTTP/1.1\r\n", msg->buffer->start,
                      msg->buffer->length));

  dtn_file_format_desc format = (dtn_file_format_desc){0};

  strcpy(format.desc.ext[0], "gzip");

  testrun(!dtn_http_message_add_transfer_encodings(NULL, NULL));
  testrun(dtn_http_message_add_transfer_encodings(msg, NULL));

  testrun(0 == memcmp("GET / HTTP/1.1\r\n"
                      "Transfer-Encoding:chunked\r\n",
                      msg->buffer->start, msg->buffer->length));

  testrun(dtn_http_message_add_transfer_encodings(msg, &format));

  testrun(0 == memcmp("GET / HTTP/1.1\r\n"
                      "Transfer-Encoding:chunked\r\n"
                      "Transfer-Encoding:gzip;chunked\r\n",
                      msg->buffer->start, msg->buffer->length));

  strcpy(format.desc.ext[0], "gzip");
  strcpy(format.desc.ext[1], "compress");
  strcpy(format.desc.ext[2], "deflate");
  strcpy(format.desc.ext[3], "json");

  testrun(dtn_http_message_add_transfer_encodings(msg, &format));

  testrun(0 == memcmp("GET / HTTP/1.1\r\n"
                      "Transfer-Encoding:chunked\r\n"
                      "Transfer-Encoding:gzip;chunked\r\n"
                      "Transfer-Encoding:gzip;compress;deflate;chunked\r\n",
                      msg->buffer->start, msg->buffer->length));

  strcpy(format.desc.ext[0], "compress");
  strcpy(format.desc.ext[1], "deflate");
  strcpy(format.desc.ext[2], "gzip");
  strcpy(format.desc.ext[3], "json");

  testrun(dtn_http_message_add_transfer_encodings(msg, &format));

  testrun(0 == memcmp("GET / HTTP/1.1\r\n"
                      "Transfer-Encoding:chunked\r\n"
                      "Transfer-Encoding:gzip;chunked\r\n"
                      "Transfer-Encoding:gzip;compress;deflate;chunked\r\n"
                      "Transfer-Encoding:compress;deflate;gzip;chunked\r\n",
                      msg->buffer->start, msg->buffer->length));

  strcpy(format.desc.ext[0], "compress");
  strcpy(format.desc.ext[1], "json");
  strcpy(format.desc.ext[2], "gzip");
  strcpy(format.desc.ext[3], "x");

  testrun(dtn_http_message_add_transfer_encodings(msg, &format));

  testrun(0 == memcmp("GET / HTTP/1.1\r\n"
                      "Transfer-Encoding:chunked\r\n"
                      "Transfer-Encoding:gzip;chunked\r\n"
                      "Transfer-Encoding:gzip;compress;deflate;chunked\r\n"
                      "Transfer-Encoding:compress;deflate;gzip;chunked\r\n"
                      "Transfer-Encoding:compress;chunked\r\n",
                      msg->buffer->start, msg->buffer->length));

  strcpy(format.desc.ext[0], "gzip");
  strcpy(format.desc.ext[1], "json");
  strcpy(format.desc.ext[2], "gzip");
  strcpy(format.desc.ext[3], "x");

  testrun(dtn_http_message_add_transfer_encodings(msg, &format));

  testrun(0 == memcmp("GET / HTTP/1.1\r\n"
                      "Transfer-Encoding:chunked\r\n"
                      "Transfer-Encoding:gzip;chunked\r\n"
                      "Transfer-Encoding:gzip;compress;deflate;chunked\r\n"
                      "Transfer-Encoding:compress;deflate;gzip;chunked\r\n"
                      "Transfer-Encoding:compress;chunked\r\n"
                      "Transfer-Encoding:gzip;chunked\r\n",
                      msg->buffer->start, msg->buffer->length));

  strcpy(format.desc.ext[0], "gzip");
  strcpy(format.desc.ext[1], "gzip");
  strcpy(format.desc.ext[2], "gzip");
  strcpy(format.desc.ext[3], "json");

  testrun(dtn_http_message_add_transfer_encodings(msg, &format));

  testrun(0 == memcmp("GET / HTTP/1.1\r\n"
                      "Transfer-Encoding:chunked\r\n"
                      "Transfer-Encoding:gzip;chunked\r\n"
                      "Transfer-Encoding:gzip;compress;deflate;chunked\r\n"
                      "Transfer-Encoding:compress;deflate;gzip;chunked\r\n"
                      "Transfer-Encoding:compress;chunked\r\n"
                      "Transfer-Encoding:gzip;chunked\r\n"
                      "Transfer-Encoding:gzip;gzip;gzip;chunked\r\n",
                      msg->buffer->start, msg->buffer->length));

  strcpy(format.desc.ext[0], "Compress");
  strcpy(format.desc.ext[1], "deFLATE");
  strcpy(format.desc.ext[2], "GZIP");
  strcpy(format.desc.ext[3], "x");

  testrun(dtn_http_message_add_transfer_encodings(msg, &format));

  testrun(0 == memcmp("GET / HTTP/1.1\r\n"
                      "Transfer-Encoding:chunked\r\n"
                      "Transfer-Encoding:gzip;chunked\r\n"
                      "Transfer-Encoding:gzip;compress;deflate;chunked\r\n"
                      "Transfer-Encoding:compress;deflate;gzip;chunked\r\n"
                      "Transfer-Encoding:compress;chunked\r\n"
                      "Transfer-Encoding:gzip;chunked\r\n"
                      "Transfer-Encoding:gzip;gzip;gzip;chunked\r\n"
                      "Transfer-Encoding:compress;deflate;gzip;chunked\r\n",
                      msg->buffer->start, msg->buffer->length));

  msg = dtn_http_message_free(msg);
  return testrun_log_success();
}
*/

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_set_date() {

  dtn_http_message *msg = dtn_http_create_request_string(
      (dtn_http_message_config){0}, (dtn_http_version){.major = 1, .minor = 1},
      "GET", "/");

  testrun(0 == memcmp("GET / HTTP/1.1\r\n", msg->buffer->start,
                      msg->buffer->length));

  testrun(!dtn_http_message_set_date(NULL));
  testrun(dtn_http_message_set_date(msg));

  // here we check the DATA header is added
  char *check = "GET / HTTP/1.1\r\nDate:";
  testrun(0 == memcmp(check, msg->buffer->start, strlen(check)));

  msg = dtn_http_message_free(msg);
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_set_content_length() {

  dtn_http_message *msg = dtn_http_create_request_string(
      (dtn_http_message_config){0}, (dtn_http_version){.major = 1, .minor = 1},
      "GET", "/");

  testrun(0 == memcmp("GET / HTTP/1.1\r\n", msg->buffer->start,
                      msg->buffer->length));

  testrun(!dtn_http_message_set_content_length(NULL, 0));

  testrun(dtn_http_message_set_content_length(msg, 0));
  testrun(0 == memcmp("GET / HTTP/1.1\r\n"
                      "Content-Length:0\r\n",
                      msg->buffer->start, msg->buffer->length));

  testrun(dtn_http_message_set_content_length(msg, 100));
  testrun(0 == memcmp("GET / HTTP/1.1\r\n"
                      "Content-Length:0\r\n"
                      "Content-Length:100\r\n",
                      msg->buffer->start, msg->buffer->length));

  testrun(dtn_http_message_set_content_length(msg, 123456));
  testrun(0 == memcmp("GET / HTTP/1.1\r\n"
                      "Content-Length:0\r\n"
                      "Content-Length:100\r\n"
                      "Content-Length:123456\r\n",
                      msg->buffer->start, msg->buffer->length));

  msg = dtn_http_message_free(msg);
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_add_header_string() {

  dtn_http_message *msg = dtn_http_message_create((dtn_http_message_config){0});

  // check empty msg (no startline)
  testrun(!dtn_http_message_add_header_string(msg, "x", "y"));

  msg = dtn_http_message_free(msg);
  msg = dtn_http_create_request_string((dtn_http_message_config){0},
                                      (dtn_http_version){.major = 1, .minor = 1},
                                      "GET", "/");

  testrun(msg);

  testrun(!dtn_http_message_add_header_string(msg, NULL, NULL));
  testrun(!dtn_http_message_add_header_string(msg, "x", NULL));
  testrun(!dtn_http_message_add_header_string(msg, NULL, "y"));

  testrun(dtn_http_message_add_header_string(msg, "x", "y"));

  testrun(0 == strncmp("GET / HTTP/1.1\r\nx:y\r\n", (char *)msg->buffer->start,
                       msg->buffer->length));
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_message(msg, NULL));
  testrun(0 != msg->header[0].name.start);
  testrun(0 == msg->header[1].name.start);
  testrun(0 == strncmp("x", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("y", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));
  msg = dtn_http_message_free(msg);

  msg = dtn_http_create_status_string((dtn_http_message_config){0},
                                     (dtn_http_version){0}, 100, "c");

  testrun(msg);

  testrun(dtn_http_message_add_header_string(msg, "x", "y"));

  testrun(0 == strncmp("HTTP/0.0 100 c\r\nx:y\r\n", (char *)msg->buffer->start,
                       msg->buffer->length));
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_message(msg, NULL));
  testrun(0 != msg->header[0].name.start);
  testrun(0 == strncmp("x", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("y", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));
  msg = dtn_http_message_free(msg);

  // check max bytes header line
  msg = dtn_http_create_request_string((dtn_http_message_config){0},
                                      (dtn_http_version){.major = 1, .minor = 1},
                                      "GET", "/");

  msg->config.header.max_bytes_line = 5;
  testrun(dtn_http_message_add_header_string(msg, "x", "y"));

  testrun(0 == strncmp("GET / HTTP/1.1\r\nx:y\r\n", (char *)msg->buffer->start,
                       msg->buffer->length));
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_message(msg, NULL));
  testrun(0 != msg->header[0].name.start);
  testrun(0 == strncmp("x", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("y", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));

  testrun(!dtn_http_message_add_header_string(msg, "x", "yz"));

  // check buffer is unchanged
  testrun(0 == strncmp("GET / HTTP/1.1\r\nx:y\r\n", (char *)msg->buffer->start,
                       msg->buffer->length));
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_message(msg, NULL));
  testrun(0 != msg->header[0].name.start);
  testrun(0 == msg->header[1].name.start);
  testrun(0 == strncmp("x", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("y", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));

  msg->config.header.max_bytes_line = 6;
  testrun(dtn_http_message_add_header_string(msg, "x", "yz"));

  // check buffer contains both headers
  testrun(0 == strncmp("GET / HTTP/1.1\r\nx:y\r\nx:yz\r\n",
                       (char *)msg->buffer->start, msg->buffer->length));
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_message(msg, NULL));
  testrun(0 != msg->header[0].name.start);
  testrun(0 != msg->header[1].name.start);
  testrun(0 == strncmp("x", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("y", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));
  testrun(0 == strncmp("x", (char *)msg->header[1].name.start,
                       msg->header[1].name.length));
  testrun(0 == strncmp("yz", (char *)msg->header[1].value.start,
                       msg->header[1].value.length));
  msg = dtn_http_message_free(msg);

  // check self extending of the msg->buffer in line with message config

  msg = dtn_http_create_request_string(
      (dtn_http_message_config){.header.capacity = 1000,
                               .buffer.default_size = 50},
      (dtn_http_version){.major = 1, .minor = 1}, "GET", "/");

  testrun(msg->buffer->capacity == 50)

      size_t count = 0;
  while (msg->buffer->capacity < 250) {

    testrun(dtn_http_message_add_header_string(msg, "x", "y"));
    count++;
  }

  // fprintf(stdout, "%s", (char*) msg->buffer->start);
  testrun(msg->buffer->capacity == 250);
  testrun(DTN_HTTP_PARSER_PROGRESS == dtn_http_pointer_parse_message(msg, NULL));
  for (size_t i = 0; i < count; i++) {
    testrun(0 != msg->header[i].name.start);
  }
  testrun(0 == msg->header[count].name.start);
  msg = dtn_http_message_free(msg);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_close_header() {

  dtn_http_message *msg = dtn_http_message_create((dtn_http_message_config){0});
  testrun(!dtn_http_message_close_header(NULL));
  testrun(dtn_http_message_close_header(msg));
  testrun(0 ==
          strncmp("\r\n", (char *)msg->buffer->start, msg->buffer->length));
  msg = dtn_http_message_free(msg);

  msg = dtn_http_create_request_string((dtn_http_message_config){0},
                                      (dtn_http_version){.major = 1, .minor = 1},
                                      "GET", "/");

  testrun(msg);
  testrun(dtn_http_message_add_header_string(msg, "1", "1"));
  testrun(dtn_http_message_add_header_string(msg, "2", "2"));
  testrun(dtn_http_message_add_header_string(msg, "3", "3"));
  testrun(dtn_http_message_close_header(msg));

  // 012345678901234-5-
  testrun(0 == strncmp("GET / HTTP/1.1\r\n1:1\r\n2:2\r\n3:3\r\n\r\n",
                       (char *)msg->buffer->start, msg->buffer->length));
  msg = dtn_http_message_free(msg);

  // check self extending of the msg->buffer in line with message config

  msg = dtn_http_create_request_string(
      (dtn_http_message_config){.header.capacity = 1000,
                               .buffer.default_size = 17},
      (dtn_http_version){.major = 1, .minor = 1}, "GET", "/");

  testrun(msg->buffer->capacity == 17);
  testrun(msg->buffer->length == 16);
  testrun(dtn_http_message_close_header(msg));
  testrun(0 == strncmp("GET / HTTP/1.1\r\n\r\n", (char *)msg->buffer->start,
                       msg->buffer->length));
  testrun(msg->buffer->capacity == 34);
  msg = dtn_http_message_free(msg);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_add_body() {

  dtn_http_message *msg = msg = dtn_http_create_request_string(
      (dtn_http_message_config){0}, (dtn_http_version){.major = 1, .minor = 1},
      "GET", "/");

  testrun(msg);
  testrun(dtn_http_message_add_header_string(msg, "1", "1"));
  testrun(dtn_http_message_add_header_string(msg, "2", "2"));
  testrun(dtn_http_message_add_header_string(msg, "3", "3"));
  testrun(dtn_http_message_close_header(msg));

  testrun(!dtn_http_message_add_body(
      NULL, (dtn_memory_pointer){.start = NULL, .length = 0}));

  testrun(!dtn_http_message_add_body(
      msg, (dtn_memory_pointer){.start = NULL, .length = 0}));

  testrun(!dtn_http_message_add_body(
      msg, (dtn_memory_pointer){.start = (uint8_t *)"body", .length = 0}));

  testrun(!dtn_http_message_add_body(
      NULL, (dtn_memory_pointer){.start = NULL, .length = 4}));

  testrun(!dtn_http_message_add_body(
      NULL, (dtn_memory_pointer){.start = (uint8_t *)"body", .length = 4}));

  testrun(dtn_http_message_add_body(
      msg, (dtn_memory_pointer){.start = (uint8_t *)"body", .length = 4}));

  testrun(0 == strncmp("GET / HTTP/1.1\r\n1:1\r\n2:2\r\n3:3\r\n\r\nbody",
                       (char *)msg->buffer->start, msg->buffer->length));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, NULL));
  testrun(0 == strncmp("GET", (char *)msg->request.method.start,
                       msg->request.method.length));
  testrun(0 == strncmp("/", (char *)msg->request.uri.start,
                       msg->request.uri.length));
  testrun(1 == msg->version.major);
  testrun(1 == msg->version.minor);
  testrun(0 == strncmp("body", (char *)msg->body.start, msg->body.length));
  testrun(0 == strncmp("1", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("1", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));
  testrun(0 == strncmp("2", (char *)msg->header[1].name.start,
                       msg->header[1].name.length));
  testrun(0 == strncmp("2", (char *)msg->header[1].value.start,
                       msg->header[1].value.length));
  testrun(0 == strncmp("3", (char *)msg->header[2].name.start,
                       msg->header[1].name.length));
  testrun(0 == strncmp("3", (char *)msg->header[2].value.start,
                       msg->header[1].value.length));
  testrun(0 == msg->header[3].value.start);
  msg = dtn_http_message_free(msg);

  // check adding body with unclosed header
  msg = dtn_http_create_request_string((dtn_http_message_config){0},
                                      (dtn_http_version){.major = 1, .minor = 1},
                                      "GET", "/");

  testrun(msg);
  testrun(dtn_http_message_add_header_string(msg, "1", "1"));
  testrun(dtn_http_message_add_header_string(msg, "2", "2"));
  testrun(dtn_http_message_add_header_string(msg, "3", "3"));
  testrun(!dtn_http_message_add_body(
      msg, (dtn_memory_pointer){.start = (uint8_t *)"body", .length = 4}));

  testrun(dtn_http_message_close_header(msg));
  testrun(dtn_http_message_add_body(
      msg, (dtn_memory_pointer){.start = (uint8_t *)"body", .length = 4}));
  msg = dtn_http_message_free(msg);

  // check extending buffer

  msg = dtn_http_create_request_string(
      (dtn_http_message_config){.header.capacity = 100,
                               .buffer.default_size = 20},
      (dtn_http_version){.major = 1, .minor = 1}, "GET", "/");

  testrun(dtn_http_message_close_header(msg));
  testrun(msg->buffer->capacity == 20);

  testrun(dtn_http_message_add_body(
      msg, (dtn_memory_pointer){.start = (uint8_t *)"body", .length = 4}));

  testrun(msg->buffer->capacity > 20);
  msg = dtn_http_message_free(msg);
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_message_add_body_string() {

  dtn_http_message *msg = msg = dtn_http_create_request_string(
      (dtn_http_message_config){0}, (dtn_http_version){.major = 1, .minor = 1},
      "GET", "/");

  testrun(msg);
  testrun(dtn_http_message_add_header_string(msg, "1", "1"));
  testrun(dtn_http_message_add_header_string(msg, "2", "2"));
  testrun(dtn_http_message_add_header_string(msg, "3", "3"));
  testrun(dtn_http_message_close_header(msg));

  testrun(!dtn_http_message_add_body_string(NULL, NULL));
  testrun(!dtn_http_message_add_body_string(msg, NULL));
  testrun(!dtn_http_message_add_body_string(NULL, "body"));

  testrun(dtn_http_message_add_body_string(msg, "body"));

  testrun(0 == strncmp("GET / HTTP/1.1\r\n1:1\r\n2:2\r\n3:3\r\n\r\nbody",
                       (char *)msg->buffer->start, msg->buffer->length));
  testrun(DTN_HTTP_PARSER_SUCCESS == dtn_http_pointer_parse_message(msg, NULL));
  testrun(0 == strncmp("GET", (char *)msg->request.method.start,
                       msg->request.method.length));
  testrun(0 == strncmp("/", (char *)msg->request.uri.start,
                       msg->request.uri.length));
  testrun(1 == msg->version.major);
  testrun(1 == msg->version.minor);
  testrun(0 == strncmp("body", (char *)msg->body.start, msg->body.length));
  testrun(0 == strncmp("1", (char *)msg->header[0].name.start,
                       msg->header[0].name.length));
  testrun(0 == strncmp("1", (char *)msg->header[0].value.start,
                       msg->header[0].value.length));
  testrun(0 == strncmp("2", (char *)msg->header[1].name.start,
                       msg->header[1].name.length));
  testrun(0 == strncmp("2", (char *)msg->header[1].value.start,
                       msg->header[1].value.length));
  testrun(0 == strncmp("3", (char *)msg->header[2].name.start,
                       msg->header[1].name.length));
  testrun(0 == strncmp("3", (char *)msg->header[2].value.start,
                       msg->header[1].value.length));
  testrun(0 == msg->header[3].value.start);
  msg = dtn_http_message_free(msg);

  // check adding body with unclosed header
  msg = dtn_http_create_request_string((dtn_http_message_config){0},
                                      (dtn_http_version){.major = 1, .minor = 1},
                                      "GET", "/");

  testrun(msg);
  testrun(dtn_http_message_add_header_string(msg, "1", "1"));
  testrun(dtn_http_message_add_header_string(msg, "2", "2"));
  testrun(dtn_http_message_add_header_string(msg, "3", "3"));
  testrun(!dtn_http_message_add_body_string(msg, "body"));

  testrun(dtn_http_message_close_header(msg));
  testrun(dtn_http_message_add_body_string(msg, "body"));
  msg = dtn_http_message_free(msg);

  // check extending buffer

  msg = dtn_http_create_request_string(
      (dtn_http_message_config){.header.capacity = 100,
                               .buffer.default_size = 20},
      (dtn_http_version){.major = 1, .minor = 1}, "GET", "/");

  testrun(dtn_http_message_close_header(msg));
  testrun(msg->buffer->capacity == 20);
  testrun(dtn_http_message_add_body_string(msg, "body"));
  testrun(msg->buffer->capacity == 40);
  msg = dtn_http_message_free(msg);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_offset_http_whitespace_front() {

  char *string = "test";
  uint8_t *out = NULL;

  testrun(!offset_http_whitespace_front(NULL, 0));
  testrun(!offset_http_whitespace_front((uint8_t *)string, 0));
  testrun(!offset_http_whitespace_front(NULL, strlen(string)));

  out = offset_http_whitespace_front((uint8_t *)string, strlen(string));
  testrun(out == (uint8_t *)string);

  //        012345
  string = " \t  test";
  out = offset_http_whitespace_front((uint8_t *)string, strlen(string));
  testrun(out == (uint8_t *)string + 4);

  //        012345
  string = " \t  ";
  out = offset_http_whitespace_front((uint8_t *)string, strlen(string));
  testrun(out == (uint8_t *)string + 4);

  //        012345
  string = " \n  ";
  out = offset_http_whitespace_front((uint8_t *)string, strlen(string));
  testrun(out == (uint8_t *)string + 1);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int check_offset_http_whitespace_end() {

  char *string = "test";
  uint8_t *out = NULL;

  testrun(!offset_http_whitespace_end(NULL, 0));
  testrun(!offset_http_whitespace_end((uint8_t *)string, 0));
  testrun(!offset_http_whitespace_end(NULL, strlen(string)));

  out = offset_http_whitespace_end((uint8_t *)string, strlen(string));
  testrun(out == (uint8_t *)string + 3);

  //        012345
  string = "test \t  ";
  out = offset_http_whitespace_end((uint8_t *)string, strlen(string));
  testrun(*out == 't');
  testrun(out == (uint8_t *)string + 3);

  //        012345
  string = " \t  ";
  out = offset_http_whitespace_end((uint8_t *)string, strlen(string));
  testrun(out == (uint8_t *)string);

  //        012345
  string = " \n  ";
  out = offset_http_whitespace_end((uint8_t *)string, strlen(string));
  testrun(out == (uint8_t *)string + 1);
  testrun(*out == '\n');

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_pointer_parse_comma_list_item() {

  char *str = "test";
  uint8_t *out = NULL;
  size_t len = 0;
  uint8_t *next = NULL;

  testrun(!dtn_http_pointer_parse_comma_list_item(NULL, 0, NULL, NULL, NULL));
  testrun(!dtn_http_pointer_parse_comma_list_item(NULL, strlen(str), &out, &len,
                                                 &next));
  testrun(!dtn_http_pointer_parse_comma_list_item((uint8_t *)str, 0, &out, &len,
                                                 &next));
  testrun(!dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                 NULL, &len, &next));
  testrun(!dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                 &out, NULL, &next));
  testrun(!dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                 &out, &len, NULL));

  testrun(dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                &out, &len, &next));

  testrun(out == (uint8_t *)str);
  testrun(len == strlen(str));
  testrun(next == NULL);

  //     01-23456789012345678901234567890
  str = " \t whitespace front and back \t ";

  testrun(dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                &out, &len, &next));

  testrun(out == (uint8_t *)str + 3);
  testrun(len == 25);
  testrun(next == NULL);

  //     01-23456789012345678901234567890
  str = " \t whitespace , front and back \t ";

  testrun(dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                &out, &len, &next));

  testrun(out == (uint8_t *)str + 3);
  testrun(len == 10);
  testrun(next == (uint8_t *)str + 15);

  //     0123456789012345678901234567890
  str = "a,b,c";

  testrun(dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                &out, &len, &next));

  testrun(out == (uint8_t *)str);
  testrun(len == 1);
  testrun(next == (uint8_t *)str + 2);

  //     0123456789012345678901234567890
  str = "a, b,c";

  testrun(dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                &out, &len, &next));

  testrun(out == (uint8_t *)str);
  testrun(len == 1);
  testrun(next == (uint8_t *)str + 2);

  //     0123456789012345678901234567890
  str = "a , b,c";

  testrun(dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                &out, &len, &next));

  testrun(out == (uint8_t *)str);
  testrun(len == 1);
  testrun(next == (uint8_t *)str + 3);

  //     0123456789012345678901234567890
  str = " a , b,c";

  testrun(dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                &out, &len, &next));

  testrun(out == (uint8_t *)str + 1);
  testrun(len == 1);
  testrun(next == (uint8_t *)str + 4);

  /* do not check for valid content */
  //     0123456789012345678901234567890
  str = "\ra , b,c";

  testrun(dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                &out, &len, &next));

  testrun(out == (uint8_t *)str);
  testrun(len == 2);
  testrun(next == (uint8_t *)str + 4);

  /* false with empty */
  //     0123456789012345678901234567890
  str = ",,b,c";

  testrun(!dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                 &out, &len, &next));

  str = "";
  testrun(!dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                 &out, &len, &next));

  str = " ";
  testrun(!dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                 &out, &len, &next));

  str = " ,";
  testrun(!dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                 &out, &len, &next));

  str = " \t    ,";
  testrun(!dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                 &out, &len, &next));

  //     01-2345678
  str = " \t    a,";
  testrun(dtn_http_pointer_parse_comma_list_item((uint8_t *)str, strlen(str),
                                                &out, &len, &next));
  testrun(out == (uint8_t *)str + 6);
  testrun(len == 1);
  testrun(next == (uint8_t *)str + 8);

  /* check usage */

  //     01-234-567-890-123-456-
  str = " \ta \t, \tb \t, \tc \t";

  uint8_t *ptr = (uint8_t *)str;
  size_t str_len = strlen(str);

  testrun(dtn_http_pointer_parse_comma_list_item(
      ptr, str_len - (ptr - (uint8_t *)str), &out, &len, &ptr));
  testrun(ptr == (uint8_t *)str + 6);
  testrun(out == (uint8_t *)str + 2);
  testrun(len == 1);

  testrun(dtn_http_pointer_parse_comma_list_item(
      (uint8_t *)ptr, str_len - (ptr - (uint8_t *)str), &out, &len, &ptr));
  testrun(ptr == (uint8_t *)str + 12);
  testrun(out == (uint8_t *)str + 8);
  testrun(len == 1);

  testrun(dtn_http_pointer_parse_comma_list_item(
      (uint8_t *)ptr, str_len - (ptr - (uint8_t *)str), &out, &len, &ptr));
  testrun(ptr == NULL);
  testrun(out == (uint8_t *)str + 14);
  testrun(len == 1);

  testrun(!dtn_http_pointer_parse_comma_list_item(
      (uint8_t *)ptr, len - (ptr - (uint8_t *)str), &out, &len, &ptr));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_http_pointer_find_item_in_comma_list() {

  uint8_t *str = (uint8_t *)"test";
  size_t len = strlen((char *)str);

  uint8_t *name = (uint8_t *)"test";
  size_t name_len = strlen((char *)name);

  testrun(!dtn_http_pointer_find_item_in_comma_list(NULL, len, name, name_len,
                                                   true));
  testrun(
      !dtn_http_pointer_find_item_in_comma_list(str, 0, name, name_len, true));
  testrun(
      !dtn_http_pointer_find_item_in_comma_list(str, len, NULL, name_len, true));
  testrun(!dtn_http_pointer_find_item_in_comma_list(str, len, name, 0, true));

  const uint8_t *out = NULL;
  out = dtn_http_pointer_find_item_in_comma_list(str, len, name, name_len, true);
  testrun(out);
  testrun(out == str);

  out =
      dtn_http_pointer_find_item_in_comma_list(str, len, name, name_len, false);
  testrun(out);
  testrun(out == str);

  testrun(!dtn_http_pointer_find_item_in_comma_list(str, len, name, name_len - 1,
                                                   true));
  testrun(!dtn_http_pointer_find_item_in_comma_list(str, len, name, name_len - 1,
                                                   false));

  name = (uint8_t *)"Test";
  name_len = strlen((char *)name);

  testrun(!dtn_http_pointer_find_item_in_comma_list(str, len, name, name_len,
                                                   false));
  out = dtn_http_pointer_find_item_in_comma_list(str, len, name, name_len, true);
  testrun(out);
  testrun(out == str);

  str = (uint8_t *)" \t test";
  len = strlen((char *)str);

  testrun(!dtn_http_pointer_find_item_in_comma_list(str, len, name, name_len,
                                                   false));
  out = dtn_http_pointer_find_item_in_comma_list(str, len, name, name_len, true);
  testrun(out);
  testrun(out == str + 3);

  // check first match

  //     01-2345-67890123456
  str = (uint8_t *)" \tx, \t test, Test";
  len = strlen((char *)str);

  out =
      dtn_http_pointer_find_item_in_comma_list(str, len, name, name_len, false);
  testrun(out);
  testrun(out == str + 13);

  out = dtn_http_pointer_find_item_in_comma_list(str, len, name, name_len, true);
  testrun(out);
  testrun(out == str + 7);

  str = (uint8_t *)" \tx, \t test, test";
  len = strlen((char *)str);

  testrun(!dtn_http_pointer_find_item_in_comma_list(str, len, name, name_len,
                                                   false));
  out = dtn_http_pointer_find_item_in_comma_list(str, len, name, name_len, true);
  testrun(out);
  testrun(out == str + 7);

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

  testrun_test(check_http_is_separator);
  testrun_test(check_http_is_token);
  testrun_test(check_http_is_token_string);
  testrun_test(check_http_is_whitespace);
  testrun_test(check_http_is_control);
  testrun_test(check_http_is_text);
  testrun_test(check_http_is_field_content);

  testrun_test(test_dtn_http_message_cast);
  testrun_test(test_dtn_http_message_create);
  testrun_test(test_dtn_http_message_clear);
  testrun_test(test_dtn_http_message_free);
  testrun_test(test_dtn_http_message_free_uncached);

  testrun_test(test_dtn_http_message_enable_caching);

  testrun_test(test_dtn_http_message_config_init);

  testrun_test(check_validate_tranfer_encoding_grammar);
  testrun_test(check_http_is_qd_char);
  testrun_test(check_http_is_quoted_pair);
  testrun_test(check_http_is_quoted_string);
  testrun_test(check_check_chunk_extension_grammar);
  testrun_test(check_parse_chunked);
  testrun_test(check_offset_http_whitespace_front);
  testrun_test(check_offset_http_whitespace_end);

  testrun_test(test_dtn_http_pointer_parse_transfer_encodings);
  testrun_test(test_dtn_http_pointer_parse_message);

  testrun_test(test_dtn_http_message_ensure_open_capacity);

  testrun_test(test_dtn_http_create_status);
  testrun_test(test_dtn_http_create_status_string);

  testrun_test(test_dtn_http_status_not_found);
  testrun_test(test_dtn_http_status_forbidden);

  testrun_test(test_dtn_http_create_request);
  testrun_test(test_dtn_http_create_request_string);
  testrun_test(test_dtn_http_is_request);

  testrun_test(test_dtn_http_message_add_header);
  testrun_test(test_dtn_http_message_add_header_string);

  testrun_test(test_dtn_http_message_add_content_type);
  //testrun_test(test_dtn_http_message_add_transfer_encodings);
  testrun_test(test_dtn_http_message_set_date);
  testrun_test(test_dtn_http_message_set_content_length);

  testrun_test(test_dtn_http_message_close_header);

  testrun_test(test_dtn_http_message_add_body);
  testrun_test(test_dtn_http_message_add_body_string);

  testrun_test(test_dtn_http_pointer_parse_version);
  testrun_test(test_dtn_http_pointer_parse_status_line);
  testrun_test(test_dtn_http_pointer_parse_request_line);
  testrun_test(test_dtn_http_pointer_parse_header_line);

  testrun_test(test_dtn_http_pointer_parse_comma_list_item);
  testrun_test(test_dtn_http_pointer_find_item_in_comma_list);

  testrun_test(test_dtn_http_header_get);
  testrun_test(test_dtn_http_header_get_next);

  testrun_test(test_dtn_http_message_shift_trailing_bytes);

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
