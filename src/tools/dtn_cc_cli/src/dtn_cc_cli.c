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
        @file           dtn_cc_cli.c
        @author         Töpfer, Markus

        @date           2025-12-22


        ------------------------------------------------------------------------
*/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <dtn_base/dtn_getopt.h>
#include <dtn_base/dtn_convert.h>
#include <dtn_base/dtn_string.h>
#include <dtn_base/dtn_event_loop.h>
#include <dtn_base/dtn_item.h>
#include <dtn_base/dtn_item_json.h>
#include <dtn_base/dtn_utils.h>

#include <dtn_core/dtn_app.h>
#include <dtn_core/dtn_event_api.h>

#include <dtn_os/dtn_os_event_loop.h>

/*---------------------------------------------------------------------------*/

struct userdata {

    dtn_event_loop *loop;
    dtn_app *app;
    int socket;
    char *password;
    pthread_t thread;

};

/*---------------------------------------------------------------------------*/

static bool cb_login(void *userdata, int socket, dtn_item *msg){

    UNUSED(socket);

    struct userdata *self = (struct userdata*)(userdata);

    UNUSED(self);

    if (!dtn_event_get_response(msg)) goto error;

    if (0 != dtn_event_get_error_code(msg)){

        char *string = dtn_item_to_json(msg);
        dtn_log_error("LOGIN FAILED %s", string);
        string = dtn_data_pointer_free(string);
        exit(1);

    } else {

        dtn_log_info("LOGIN SUCCESS.");
    }

    msg = dtn_item_free(msg);
    return true;
error:
    msg = dtn_item_free(msg);
    return false;
}

/*---------------------------------------------------------------------------*/

static bool cb_load_routes(void *userdata, int socket, dtn_item *msg){

    UNUSED(socket);

    struct userdata *self = (struct userdata*)(userdata);

    UNUSED(self);

    if (!dtn_event_get_response(msg)) goto error;

    if (0 != dtn_event_get_error_code(msg)){

        char *string = dtn_item_to_json(msg);
        dtn_log_error("LOAD ROUTES FAILED %s", string);
        string = dtn_data_pointer_free(string);

    } else {

        dtn_log_info("LOAD ROUTES SUCCESS.");
    }

    msg = dtn_item_free(msg);
    return true;
error:
    msg = dtn_item_free(msg);
    return false;
}

/*---------------------------------------------------------------------------*/

static bool cb_send_file(void *userdata, int socket, dtn_item *msg){

    UNUSED(socket);

    struct userdata *self = (struct userdata*)(userdata);

    UNUSED(self);

    if (!dtn_event_get_response(msg)) goto error;

    if (0 != dtn_event_get_error_code(msg)){

        char *string = dtn_item_to_json(msg);
        dtn_log_error("SEND FILE FAILED %s", string);
        string = dtn_data_pointer_free(string);

    } else {

        dtn_log_info("SEND FILE SUCCESS.");
    }

    msg = dtn_item_free(msg);
    return true;
error:
    msg = dtn_item_free(msg);
    return false;
}

/*---------------------------------------------------------------------------*/

static bool send_login(dtn_app *app, int socket, const char *password){

    dtn_item *msg = dtn_event_message_create(NULL, "login");
    dtn_item *par = dtn_event_get_paramenter(msg);
    dtn_item_object_set(par, "password", dtn_item_string(password));

    if (!dtn_app_send_json(app, socket, msg)) goto error;

    dtn_log_debug("LOGIN send at %i", socket);
    
    msg = dtn_item_free(msg);
    return true;
error:
    dtn_log_debug("LOGIN send failed.");
    dtn_item_free(msg);
    return false;
}

/*---------------------------------------------------------------------------*/

static bool send_load_routes(dtn_app *app, int socket, const char *path){

    dtn_item *msg = dtn_event_message_create(NULL, "load_routes");
    dtn_item *par = dtn_event_get_paramenter(msg);
    dtn_item_object_set(par, "path", dtn_item_string(path));

    if (!dtn_app_send_json(app, socket, msg)) goto error;

    dtn_log_debug("LOAD_ROUTES send at %i", socket);
    
    msg = dtn_item_free(msg);
    return true;
error:
    dtn_log_debug("LOAD_ROUTES send failed.");
    dtn_item_free(msg);
    return false;
}

/*---------------------------------------------------------------------------*/

static bool send_send_file(dtn_app *app, int socket, 
    const char *source, const char *dest, const char *uri){

    dtn_item *msg = dtn_event_message_create(NULL, "send_file");
    dtn_item *par = dtn_event_get_paramenter(msg);
    dtn_item *path = dtn_item_object();
    dtn_item_object_set(par, "path", path);
    dtn_item_object_set(path, "source", dtn_item_string(source));
    dtn_item_object_set(path, "destination", dtn_item_string(dest));
    dtn_item_object_set(par, "uri", dtn_item_string(uri));

    if (!dtn_app_send_json(app, socket, msg)) goto error;

    dtn_log_debug("SEND_FILE send at %i", socket);
   /*
    char *str = dtn_item_to_json(msg);
    dtn_log_debug("%s", str);
    str = dtn_data_pointer_free(str);
    */
    msg = dtn_item_free(msg);
    return true;
error:
    dtn_log_debug("SEND_FILE send failed.");
    dtn_item_free(msg);
    return false;
}

/*---------------------------------------------------------------------------*/

static bool print_help(){

    fprintf(stdout, "\nThe following commands are recognized: \n\n");
    fprintf(stdout, "   help        :   print this help\n");
    fprintf(stdout, "   quit        :   quit program\n");
    fprintf(stdout, "   login       :   send login to server (your password is stored for convinience)\n");
    fprintf(stdout, "   load_routes :   send load_routes to server\n");
    fprintf(stdout, "   send_file   :   send send_file from path and uri to server\n");

    return true;
}

/*---------------------------------------------------------------------------*/

static bool interact_with_user(struct userdata *userdata){

    char buffer[2048]; 
    int size = 2028;

    char buffer2[2048]; 
    char buffer3[2048]; 

    if (!userdata) goto error;

    while(1){

        fprintf(stdout, "Command:\n");
        fgets(buffer, size, stdin);

        if (0 == strcmp(buffer, "quit\n"))
            break;

        if (0 == strcmp(buffer, "q\n"))
            break;

        if (0 == strcmp(buffer, "help\n"))
            print_help();

        if (0 == strcmp(buffer, "h\n"))
            print_help();

        if (0 == strcmp(buffer, "?\n"))
            print_help();

        if (0 == strcmp(buffer, "login\n"))
            send_login(userdata->app, userdata->socket, userdata->password);

        if (0 == strcmp(buffer, "load_routes\n")){

            fprintf(stdout, "PATH:\n");
            fgets(buffer, size, stdin);
            buffer[strlen(buffer) - 1 ] = 0;

            send_load_routes(userdata->app, userdata->socket, buffer);
        }

        if (0 == strcmp(buffer, "send_file\n")){

            fprintf(stdout, "SOURCE PATH:\n");
            fgets(buffer, size, stdin);
            buffer[strlen(buffer) - 1 ] = 0;

            fprintf(stdout, "DEST PATH:\n");
            fgets(buffer2, size, stdin);
            buffer2[strlen(buffer2) - 1 ] = 0;

            fprintf(stdout, "DTN_URI:\n");
            fgets(buffer3, size, stdin);
            buffer3[strlen(buffer3) - 1 ] = 0;

            send_send_file(userdata->app, userdata->socket, buffer, buffer2, buffer3);
        }

    }

    dtn_event_loop_stop(userdata->loop);
    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

void *thread_base_user_interaction(void *x){

    struct userdata *userdata = (struct userdata*)x;

    interact_with_user(userdata); 

    dtn_log_debug("User interaction thread exit.");
    return NULL;
}


/*---------------------------------------------------------------------------*/

static void cb_connected(void *userdata, int socket){

    struct userdata *self = (struct userdata*)(userdata);
    self->socket = socket;
    dtn_log_debug("socket is now %i", self->socket);
    dtn_log_info("You need to relogin to server.");
    return;
}

/*---------------------------------------------------------------------------*/

static void cb_close(void *userdata, int socket){

    UNUSED(socket);

    struct userdata *self = (struct userdata*)(userdata);
    self->socket = -1;
    dtn_log_debug("socket is now unconnected");

    return;
}

/*---------------------------------------------------------------------------*/

static void print_usage() {

  fprintf(stdout, "\n");
  fprintf(stdout,
          "Connect to Command and Control socket of a DTN node\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "USAGE              [OPTIONS]...\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "               -x,     --password  password for the connection\n");
  fprintf(stdout, "               -i,     --host      host to connect to\n");
  fprintf(stdout, "               -p,     --port      port to connect to\n");
  fprintf(stdout, "               -h,     --help      print this help\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "\n");

  return;
}

/*---------------------------------------------------------------------------*/

bool read_command_line_input(int argc, char *argv[], 
    char **host, uint16_t *port, char **password) {

  int c = 0;
  int option_index = 0;
  bool ok = true;

  while (1) {

    static struct option long_options[] = {

        /* These options don’t set a flag.
           We distinguish them by their indices. */
        {"host", required_argument, 0, 'i'},
        {"port", required_argument, 0, 'p'},
        {"password", optional_argument, 0, 'x'},
        {"help", optional_argument, 0, 'h'},
        {0, 0, 0, 0}};

    /* getopt_long stores the option index here. */

    c = getopt_long(argc, argv, "i:p:?hx:", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {

    case 'h':
      print_usage();
      goto error;
      break;

    case '?':
      print_usage();
      goto error;
      break;

    case 'p':
        if (optarg){
            *port = dtn_string_to_uint16(optarg, &ok);
            if (!ok) goto error;
        }
      break;

    case 'x':
        *password = optarg;
      break;

    case 'i':
        *host = optarg;
      break;

    default:
      print_usage();
      goto error;
    }
  }

  return true;
error:
  return false;
}

/*---------------------------------------------------------------------------*/

int main(int argc, char **argv) {

    struct userdata userdata = {0};

    int retval = EXIT_FAILURE;

    dtn_event_loop *loop = NULL;
    dtn_io *io = NULL;
    dtn_app *app = NULL;

    dtn_event_loop_config loop_config = (dtn_event_loop_config){
        .max.sockets = dtn_socket_get_max_supported_runtime_sockets(0),
        .max.timers = dtn_socket_get_max_supported_runtime_sockets(0)};

    loop = dtn_os_event_loop(loop_config);
    if (!loop) goto error;

    io = dtn_io_create((dtn_io_config){.loop = loop});
    if (!io) goto error;

    app = dtn_app_create((dtn_app_config){
        .loop = loop,
        .io = io
    });

    if (!app) goto error;

    dtn_socket_configuration socket_config = {0};
    socket_config.type = TCP;

    char *password = NULL;
    char *host = NULL;

    if (!read_command_line_input(argc, argv, 
        &host, &socket_config.port, &password)) goto error;

    if (!host) goto error;
    strncpy(socket_config.host, host, DTN_HOST_NAME_MAX);

    dtn_log_debug("Connecting to host %s:%i", socket_config.host, socket_config.port);

    if (!password){
        password = getpass("Password: ");
    }

    userdata.app = app;
    userdata.loop = loop;
    userdata.password = password;
    dtn_app_register_connected(app, &userdata, cb_connected);
    dtn_app_register_close(app, &userdata, cb_close);

    int socket = dtn_app_open_connection(app, socket_config, (dtn_io_ssl_config){0});
    UNUSED(socket);
    
    userdata.socket = socket;
    
    if (!dtn_app_register(app, "login", cb_login, &userdata))
        goto error;

    if (!dtn_app_register(app, "load_routes", cb_load_routes, &userdata))
        goto error;

    if (!dtn_app_register(app, "send_file", cb_send_file, &userdata))
        goto error;

    int res = pthread_create(&userdata.thread, NULL, thread_base_user_interaction, &userdata);
    if (res)
    {
        printf ("error %d\n", res);
    }

    dtn_event_loop_run(loop, DTN_RUN_MAX);

    retval = EXIT_SUCCESS;
error:
    dtn_io_free(io);
    dtn_app_free(app);
    dtn_event_loop_free(loop);
    return retval;
}