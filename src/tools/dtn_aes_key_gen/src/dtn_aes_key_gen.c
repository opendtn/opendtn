/***
        ------------------------------------------------------------------------

        Copyright (c) 2026 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_aes_key_gen.c
        @author         Töpfer, Markus

        @date           2026-01-03


        ------------------------------------------------------------------------
*/

#include <dtn_core/dtn_aes_key.h>
#include <dtn_base/dtn_file.h>
#include <dtn_base/dtn_string.h>
#include <dtn_base/dtn_dump.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <limits.h>

/*---------------------------------------------------------------------------*/

static void print_usage() {

  fprintf(stdout, "\n");
  fprintf(stdout,
          "Generate a RANDOM buffer for some DTN AES KEY\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "USAGE              [OPTIONS]...\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "               -a,     --algorithm   any of (AES256, AES192, AES128)\n");
  fprintf(stdout, "               -h,     --help        print this help\n");
  fprintf(stdout, "               -f,     --file        create file with key\n");
  fprintf(stdout, "\n");
  fprintf(stdout, "\n");

  return;
}

/*---------------------------------------------------------------------------*/

bool read_command_line_input(int argc, char *argv[], 
    aes_key_gcm_algorithm *algorithm, const char **file) {

  int c = 0;
  int option_index = 0;

  while (1) {

    static struct option long_options[] = {

        /* These options don’t set a flag.
           We distinguish them by their indices. */
        {"algorithm", optional_argument, 0, 'a'},
        {"help", optional_argument, 0, 'h'},
        {"file", optional_argument, 0, 'f'},
        {0, 0, 0, 0}};

    /* getopt_long stores the option index here. */

    c = getopt_long(argc, argv, "?ha:f:", long_options, &option_index);

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

    case 'a':
        if (optarg){
            
            if (0 == dtn_string_compare(optarg, "AES128")){

                *algorithm = AES_128_GCM;

            } else if (0 == dtn_string_compare(optarg, "AES192")){

                *algorithm = AES_192_GCM;
            
            } else {

                *algorithm = AES_256_GCM;
            }
        }
      break;

    case 'f':
        if (optarg){
            *file = optarg;
        }
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

    int retval = EXIT_FAILURE;

    char path[PATH_MAX] = {0};

    aes_key_gcm_algorithm algorithm = AES_256_GCM;
    const char *file = NULL;

    if (!read_command_line_input(argc, argv, 
        &algorithm, &file)) goto error;

    dtn_buffer *buffer = dtn_aes_key_generate(algorithm);
    if (!buffer) goto error;

    if (file){

        getcwd(path, PATH_MAX);
        strcat(path, "/");
        strcat(path, file);

        if (DTN_FILE_SUCCESS != dtn_file_write(path, 
            buffer->start, buffer->length, "wr")) goto error;

        dtn_log_info("Wrote AES KEY to file %s",path);
    
    } else {

        dtn_dump_binary_as_hex(stdout, buffer->start, buffer->length);
        fprintf(stdout, "\n");

    }

    retval = EXIT_SUCCESS;
error:
    buffer = dtn_buffer_free(buffer);
    return retval;
}