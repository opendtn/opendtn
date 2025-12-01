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
        @file           dtn_test_config.c
        @author         Töpfer, Markus

        @date           2025-12-01


        ------------------------------------------------------------------------
*/

#include <ov_base/ov_config.h>

int main(int argc, char **argv) {

    ov_json_value *config = NULL;

    const char *path = ov_config_path_from_command_line(argc, argv);
    if (!path) {
    
      fprintf(stdout, "NO path given with -c %s\n", path);
      goto error;
    }
    
    config = ov_config_load(path);
    if (!config) {
    
      fprintf(stdout, "Failed to load JSON from %s\n", path);
      goto error;
    }

    config = ov_json_value_free(config);
    return EXIT_SUCCESS;

error:
    config = ov_json_value_free(config);
    return EXIT_FAILURE;
}