/***
        ------------------------------------------------------------------------

        Copyright (c) 2024 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_io.h
        @author         Markus Töpfer

        @date           2024-12-20


        ------------------------------------------------------------------------
*/
#ifndef dtn_io_h
#define dtn_io_h

#include "dtn_domain.h"

#include <dtn_base/dtn_item.h>
#include <dtn_base/dtn_event_loop.h>
#include <dtn_base/dtn_memory_pointer.h>
#include <dtn_base/dtn_socket.h>

/*----------------------------------------------------------------------------*/

typedef struct dtn_io dtn_io;
typedef struct dtn_io_callback dtn_io_callback;
typedef struct dtn_io_ssl_config dtn_io_ssl_config;
typedef struct dtn_io_socket_config dtn_io_socket_config;

/*----------------------------------------------------------------------------*/

typedef struct dtn_io_config {

  dtn_event_loop *loop;

  struct {

    char path[PATH_MAX];

  } domain;

  struct {

    uint64_t reconnect_interval_usec;
    uint64_t timeout_usec;
    uint64_t threadlock_timeout_usec;

  } limits;

} dtn_io_config;

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_io *dtn_io_create(dtn_io_config config);
dtn_io *dtn_io_free(dtn_io *self);
dtn_io *dtn_io_cast(const void *data);

/*----------------------------------------------------------------------------*/

dtn_io_config dtn_io_config_from_item(const dtn_item *input);

/*
 *      ------------------------------------------------------------------------
 *
 *      SOCKET FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

struct dtn_io_callback {

  void *userdata;

  // will be called back in listener based setup
  bool (*accept)(void *userdata, int listener, int connection);

  bool (*io)(void *userdata, int connection,
             const char *optional_domain, // only transmitted for TLS
             const dtn_memory_pointer data);

  void (*close)(void *userdata, int connection);

  // will be called back in connection based setup
  void (*connected)(void *userdata, int connection);
};

/*----------------------------------------------------------------------------*/

struct dtn_io_ssl_config {

  char domain[PATH_MAX]; // hostname to use in handshake

  struct {

    char cert[PATH_MAX];
    char key[PATH_MAX];

  } certificate;

  struct {

    char file[PATH_MAX]; // path to CA verify file
    char path[PATH_MAX]; // path to CAs to use

    char client_ca[PATH_MAX]; // client CA to request for certificate auth

  } ca;

  uint8_t verify_depth;
};

/*----------------------------------------------------------------------------*/

struct dtn_io_socket_config {

  bool auto_reconnect; // only used for clients

  dtn_socket_configuration socket;

  dtn_io_callback callbacks;

  dtn_io_ssl_config ssl;
};

/*----------------------------------------------------------------------------*/

/**
 *  open a listener socket.
 *
 *  if config.ssl contains a cert this dedicated cert configuration will be
 *  used. It will build a context with certificate based authentication instead
 *  of the common SNI based Domain authentication configured.
 */
int dtn_io_open_listener(dtn_io *self, dtn_io_socket_config config);

/*----------------------------------------------------------------------------*/

int dtn_io_open_connection(dtn_io *self, dtn_io_socket_config config);

/*----------------------------------------------------------------------------*/

bool dtn_io_close(dtn_io *self, int socket);

/*----------------------------------------------------------------------------*/

/**
 *  Threadsafe send function 
 */
bool dtn_io_send(dtn_io *self, int socket, const dtn_memory_pointer buffer);

/*----------------------------------------------------------------------------*/

dtn_domain *dtn_io_get_domain(dtn_io *self, const char *name);

#endif /* dtn_io_h */
