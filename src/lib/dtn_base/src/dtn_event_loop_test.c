/***
        ------------------------------------------------------------------------

        Copyright (c) 2018 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_event_loop_test.c
        @author         Markus Toepfer
        @author         Michael Beer

        @date           2018-12-17

        @ingroup        dtn_event_loop

        @brief          Unit tests of dtn_event_loop implementation.


        ------------------------------------------------------------------------
*/
#include "../include/dtn_event_loop_test_interface.h"
#include "dtn_event_loop.c"

#define TEST_SLEEP_USEC 20000

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CASES                                                      #CASES
 *
 *      ------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------*/

int test_dtn_event_loop_config_default() {

  dtn_event_loop_config config = dtn_event_loop_config_default();

  testrun(config.max.sockets == DTN_EVENT_LOOP_MAX_SOCKETS_DEFAULT);
  testrun(config.max.timers == DTN_EVENT_LOOP_MAX_TIMERS_DEFAULT);

  return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_dtn_event_loop_config_adapt_to_runtime() {

  dtn_event_loop_config config = {0};

  struct rlimit file_limit = {0};

  testrun(0 == getrlimit(RLIMIT_NOFILE, &file_limit));

  /*
   *  NOTE on macOS we get RLIM_INFINITY
   */

  // check we got a limit range
  testrun(file_limit.rlim_max > 1);
  // testrun(file_limit.rlim_max != RLIM_INFINITY);

  config = dtn_event_loop_config_adapt_to_runtime(config);
  testrun(config.max.sockets == DTN_EVENT_LOOP_SOCKETS_MIN);
  testrun(config.max.timers == DTN_EVENT_LOOP_TIMERS_MIN);

  config.max.sockets = file_limit.rlim_max + 1;

  config = dtn_event_loop_config_adapt_to_runtime(config);
  // testrun(config.max.sockets == file_limit.rlim_max);
  testrun(config.max.timers == DTN_EVENT_LOOP_TIMERS_MIN);

  return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_dtn_event_loop_cast() {

  dtn_event_loop loop = {0};

  testrun(!dtn_event_loop_cast(&loop));
  testrun(dtn_event_loop_set_type(&loop, 0));
  testrun(dtn_event_loop_cast(&loop));

  for (uint32_t i = 0; i <= 0xffff; i++) {

    loop.magic_byte = i;
    if (i == DTN_EVENT_LOOP_MAGIC_BYTE) {
      testrun(dtn_event_loop_cast(&loop));
    } else {
      testrun(!dtn_event_loop_cast(&loop));
    }

    loop.magic_byte = DTN_EVENT_LOOP_MAGIC_BYTE;
    loop.type = i;
    testrun(dtn_event_loop_cast(&loop));
  }

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static int g_signum = -1;

static void sighandler_sigpipe(int signum) { g_signum = signum; }

/*----------------------------------------------------------------------------*/

bool g_stopped = false;

static bool event_loop_stop_mockup(dtn_event_loop *loop) {

  UNUSED(loop);
  g_stopped = true;

  return true;
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_loop_setup_signals() {

  testrun(!dtn_event_loop_setup_signals(0));

  testrun(SIGPIPE != g_signum);

  signal(SIGPIPE, sighandler_sigpipe);

  // process to send itself SIGPIPE...
  kill(getpid(), SIGPIPE);
  sleep(1);

  testrun(SIGPIPE == g_signum);
  g_signum = -1;

  dtn_event_loop loop = {

      .stop = event_loop_stop_mockup,
  };

  testrun(dtn_event_loop_set_type(&loop, 0x1234));

  testrun(dtn_event_loop_setup_signals(&loop));
  kill(getpid(), SIGINT);
  sleep(1);

  testrun(g_signum == -1);

  kill(getpid(), SIGINT);
  sleep(1);

  testrun(g_signum == -1);
  testrun(g_stopped == true);

  return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_dtn_event_loop_set_type() {

  dtn_event_loop loop = {0};

  testrun(!dtn_event_loop_set_type(NULL, 0));

  for (uint32_t i = 0; i <= 0xffff; i++) {

    loop.magic_byte = i;
    testrun(dtn_event_loop_set_type(&loop, i));
    testrun(loop.magic_byte == DTN_EVENT_LOOP_MAGIC_BYTE);
    testrun(loop.type == i);
  }

  return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_dtn_event_loop_free() {

  dtn_event_loop *loop = dtn_event_loop_default(dtn_event_loop_config_default());

  testrun(loop);
  testrun(loop->free);

  dtn_event_loop *(*free_loop)(dtn_event_loop *) = NULL;

  testrun(NULL == dtn_event_loop_free(NULL));

  loop->magic_byte = 1;
  testrun(loop == dtn_event_loop_free(loop));
  loop->magic_byte = DTN_EVENT_LOOP_MAGIC_BYTE;
  testrun(NULL == dtn_event_loop_free(loop));

  loop = dtn_event_loop_default(dtn_event_loop_config_default());

  free_loop = loop->free;
  loop->free = NULL;
  testrun(loop == dtn_event_loop_free(loop));

  loop->free = free_loop;
  testrun(NULL == dtn_event_loop_free(loop));

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_loop_default() {

  /* just check we have a default loop providing all functionatily
   * actual functionality is tested over interface tests */

  dtn_event_loop *loop = dtn_event_loop_default(dtn_event_loop_config_default());

  testrun(loop);
  testrun(loop->free);

  testrun(loop->is_running);
  testrun(loop->stop);
  testrun(loop->run);

  testrun(loop->callback.set);
  testrun(loop->callback.unset);

  testrun(loop->timer.set);
  testrun(loop->timer.unset);

  testrun(NULL == dtn_event_loop_free(loop));
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

static bool cb(int socket, uint8_t events, void *data) {

  if (socket == 0) { /* unused */
  };
  if (events == 0) { /* unused */
  };

  char *buffer = (char *)data;

  if (buffer)
    recv(socket, buffer, 512, 0);

  return true;
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_add_default_connection_accept() {

  char buffer[512] = {0};

  int max_sockets = 100;
  int max_timers = 100;

  int sockets[max_sockets];
  memset(sockets, 0, sizeof(sockets));

  dtn_event_loop_config config = {

      .max.sockets = max_sockets, .max.timers = max_timers};

  dtn_socket_configuration socket_config = {

      .host = "localhost", .type = TCP

  };

  dtn_event_loop *loop = dtn_event_loop_default(config);
  testrun(loop);
  testrun(loop->is_running(loop) == false);

  socket_config.type = TCP;
  int socketTCP = dtn_socket_create(socket_config, false, NULL);
  socket_config.type = UDP;
  int socketUDP = dtn_socket_create(socket_config, false, NULL);
  socket_config.type = LOCAL;
  int socketLOCAL = dtn_socket_create(socket_config, false, NULL);

  testrun(0 < socketTCP);
  testrun(0 < socketUDP);
  testrun(0 < socketLOCAL);

  testrun(!dtn_event_add_default_connection_accept(NULL, 0, 0, NULL, NULL));
  testrun(!dtn_event_add_default_connection_accept(NULL, socketTCP,
                                                  DTN_EVENT_IO_IN, &buffer, cb));
  testrun(!dtn_event_add_default_connection_accept(loop, 0, DTN_EVENT_IO_IN,
                                                  &buffer, cb));
  testrun(
      !dtn_event_add_default_connection_accept(loop, socketTCP, 0, &buffer, cb));
  testrun(!dtn_event_add_default_connection_accept(loop, socketTCP,
                                                  DTN_EVENT_IO_IN, NULL, NULL));

  // TCP OK
  testrun(dtn_event_add_default_connection_accept(loop, socketTCP,
                                                 DTN_EVENT_IO_IN, &buffer, cb));

  // LOCAL OK
  testrun(dtn_event_add_default_connection_accept(loop, socketLOCAL,
                                                 DTN_EVENT_IO_IN, &buffer, cb));

  // UDP NOK
  testrun(!dtn_event_add_default_connection_accept(loop, socketUDP,
                                                  DTN_EVENT_IO_IN, &buffer, cb));

  testrun(dtn_socket_get_config(socketTCP, &socket_config, NULL, NULL));
  int client = dtn_socket_create(socket_config, true, NULL);
  testrun(client > 0);
  testrun(strlen(buffer) == 0);
  loop->run(loop, DTN_RUN_ONCE);
  testrun(0 < send(client, "1234", 4, 0));
  usleep(10000);
  loop->run(loop, DTN_RUN_ONCE);

  fprintf(stdout, "buffer %s\n", buffer);

  // check data from buffer (MUST be accepted, as data was send)
  testrun(0 == strncmp(buffer, "1234", 4));

  testrun(dtn_event_remove_default_connection_accept(loop, socketTCP));
  testrun(dtn_event_remove_default_connection_accept(loop, socketLOCAL));

  // remove allready removed
  testrun(dtn_event_remove_default_connection_accept(loop, socketTCP));
  testrun(dtn_event_remove_default_connection_accept(loop, socketLOCAL));

  // remove on non accept socket
  testrun(dtn_event_remove_default_connection_accept(loop, socketUDP));

  testrun(NULL == dtn_event_loop_free(loop));
  unlink("localhost");

  close(socketTCP);
  close(socketUDP);
  close(socketLOCAL);

  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_remove_default_connection_accept() {

  // tested with @see test_dtn_event_add_default_connection_accept
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_loop_config_from_json() {

  dtn_item *obj = NULL;
  dtn_item *val = NULL;
  dtn_item *input = dtn_item_object();

  dtn_event_loop_config config = dtn_event_loop_config_from_json(NULL);
  testrun(0 == config.max.sockets);
  testrun(0 == config.max.timers);

  config = dtn_event_loop_config_from_json(input);
  testrun(0 == config.max.sockets);
  testrun(0 == config.max.timers);

  val = dtn_item_number(1);
  testrun(dtn_item_object_set(input, DTN_EVENT_LOOP_KEY_MAX_SOCKETS, val));

  config = dtn_event_loop_config_from_json(input);
  testrun(1 == config.max.sockets);
  testrun(0 == config.max.timers);

  val = dtn_item_number(2);
  testrun(dtn_item_object_set(input, DTN_EVENT_LOOP_KEY_MAX_TIMERS, val));

  config = dtn_event_loop_config_from_json(input);
  testrun(1 == config.max.sockets);
  testrun(2 == config.max.timers);

  /*
   *      Check outer object with KEY DTN_EVENT_LOOP_KEY
   */

  obj = dtn_item_object();

  config = dtn_event_loop_config_from_json(obj);
  testrun(0 == config.max.sockets);
  testrun(0 == config.max.timers);

  testrun(dtn_item_object_set(obj, DTN_EVENT_LOOP_KEY, input));

  config = dtn_event_loop_config_from_json(obj);
  testrun(1 == config.max.sockets);
  testrun(2 == config.max.timers);

  testrun(NULL == dtn_item_free(obj));

  /*
   *      Check out of range
   */

  input = dtn_item_object();

  val = dtn_item_number(UINT32_MAX + 1);
  testrun(dtn_item_object_set(input, DTN_EVENT_LOOP_KEY_MAX_SOCKETS, val));

  config = dtn_event_loop_config_from_json(input);
  testrun(0 == config.max.sockets);
  testrun(0 == config.max.timers);

  testrun(dtn_item_object_delete(input, DTN_EVENT_LOOP_KEY_MAX_SOCKETS));

  val = dtn_item_number(UINT32_MAX);
  testrun(dtn_item_object_set(input, DTN_EVENT_LOOP_KEY_MAX_SOCKETS, val));

  config = dtn_event_loop_config_from_json(input);
  testrun(UINT32_MAX == config.max.sockets);
  testrun(0 == config.max.timers);

  testrun(dtn_item_object_delete(input, DTN_EVENT_LOOP_KEY_MAX_SOCKETS));

  val = dtn_item_number(UINT32_MAX + 1);
  testrun(dtn_item_object_set(input, DTN_EVENT_LOOP_KEY_MAX_SOCKETS, val));

  config = dtn_event_loop_config_from_json(input);
  testrun(0 == config.max.sockets);
  testrun(0 == config.max.timers);

  testrun(dtn_item_object_delete(input, DTN_EVENT_LOOP_KEY_MAX_TIMERS));

  val = dtn_item_number(UINT32_MAX);
  testrun(dtn_item_object_set(input, DTN_EVENT_LOOP_KEY_MAX_TIMERS, val));

  config = dtn_event_loop_config_from_json(input);
  testrun(0 == config.max.sockets);
  testrun(UINT32_MAX == config.max.timers);

  testrun(NULL == dtn_item_free(input));
  return testrun_log_success();
}

/*----------------------------------------------------------------------------*/

int test_dtn_event_loop_config_to_json() {

  dtn_item *val = NULL;
  dtn_event_loop_config config = {0};

  val = dtn_event_loop_config_to_json(config);
  testrun(val);
  testrun(0 == dtn_item_get_number(
                   dtn_item_object_get(val, DTN_EVENT_LOOP_KEY_MAX_TIMERS)));
  testrun(0 == dtn_item_get_number(
                   dtn_item_object_get(val, DTN_EVENT_LOOP_KEY_MAX_SOCKETS)));
  val = dtn_item_free(val);

  val = dtn_event_loop_config_to_json(config);
  testrun(val);
  testrun(0 == dtn_item_get_number(
                   dtn_item_object_get(val, DTN_EVENT_LOOP_KEY_MAX_TIMERS)));
  testrun(0 == dtn_item_get_number(
                   dtn_item_object_get(val, DTN_EVENT_LOOP_KEY_MAX_SOCKETS)));
  val = dtn_item_free(val);

  config.max.sockets = 15;
  config.max.timers = 1;

  val = dtn_event_loop_config_to_json(config);
  testrun(val);
  testrun(1 == dtn_item_get_number(
                   dtn_item_object_get(val, DTN_EVENT_LOOP_KEY_MAX_TIMERS)));
  testrun(15 == dtn_item_get_number(
                    dtn_item_object_get(val, DTN_EVENT_LOOP_KEY_MAX_SOCKETS)));
  val = dtn_item_free(val);

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

  testrun_test(test_dtn_event_loop_config_from_json);
  testrun_test(test_dtn_event_loop_config_to_json);

  testrun_test(test_dtn_event_loop_config_default);
  testrun_test(test_dtn_event_loop_config_adapt_to_runtime);

  testrun_test(test_dtn_event_loop_cast);
  testrun_test(test_dtn_event_loop_setup_signals);
  testrun_test(test_dtn_event_loop_set_type);
  testrun_test(test_dtn_event_loop_free);

  testrun_test(test_dtn_event_add_default_connection_accept);
  testrun_test(test_dtn_event_remove_default_connection_accept);

  DTN_EVENT_LOOP_PERFORM_INTERFACE_TESTS(dtn_event_loop_default);

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
