/***
        ------------------------------------------------------------------------

        Copyright 2021 German Aerospace Center DLR e.V. (GSOC)

        Licensed under the Apache License, Version 2.0 (the "License");
        you may not use this file except in compliance with the License.
        You may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

        Unless required by applicable law or agreed to in writing, software
        distributed under the License is distributed on an "AS IS" BASIS,
        WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
        See the License for the specific language gdtnerning permissions and
        limitations under the License.

        This file is part of the openvocs project. http://openvocs.org

        ------------------------------------------------------------------------
*//**
        @file           dtn_log.h
        @author         Michael Beer

        ------------------------------------------------------------------------
*/
#ifndef dtn_log_ng_h
#define dtn_log_ng_h

#include <stdbool.h>
#include <stddef.h>

/*----------------------------------------------------------------------------*/

typedef enum {

    DTN_LOG_EMERG = 0,
    DTN_LOG_ALERT = 1,
    DTN_LOG_CRIT,
    DTN_LOG_ERR,
    DTN_LOG_WARNING,
    DTN_LOG_NOTICE,
    DTN_LOG_INFO,
    DTN_LOG_DEBUG,
    DTN_LOG_DEV,
    DTN_LOG_INVALID,

} dtn_log_level;

dtn_log_level dtn_log_level_from_string(char const *level);

/**
 * @return 0 if level unknown
 */
char const *dtn_log_level_to_string(dtn_log_level level);

/*----------------------------------------------------------------------------*/

/**
 * Initializes logging facility.
 * If not done, logging will still be performed, but just to stderr.
 */
bool dtn_log_init();

bool dtn_log_close();

/*----------------------------------------------------------------------------*/

typedef enum {

    DTN_LOG_FORMAT_INVALID = -1,
    DTN_LOG_TEXT = 0, // Should be the default
    DTN_LOG_JSON,

} dtn_log_format;

typedef struct {

    /* Flags that enable a particular logging facility or not */
    struct {
        bool systemd : 1;
    } use;

    /* Optional: stream to log to */
    int filehandle;
    dtn_log_format format;

    struct {

        bool use;
        size_t messages_per_file;
        size_t max_num_files;
        char *path;

    } log_rotation;

} dtn_log_output;

/*----------------------------------------------------------------------------*/

#define dtn_log_configure(module, function, level, ...)                        \
    dtn_log_set_output(module, function, level, (dtn_log_output){__VA_ARGS__})

/**
 * Set custom logger for a file or file/function
 * @return File handle if there was a file handle associated
 */
int dtn_log_set_output(char const *module_name, char const *function_name,
                       dtn_log_level level, const dtn_log_output output);

/*----------------------------------------------------------------------------*/

bool dtn_log_ng(dtn_log_level level, char const *file, char const *function,
                size_t line, char const *format, ...);

/*----------------------------------------------------------------------------*/

/**
 * Stop logging anything but error messages
 */
void dtn_log_mute();

/**
 * After this call, everything will be logged again.
 *
 * Enable full logging again after call to `dtn_log_mute` .
 * If `dtn_log_mute` was not called before, has no effect.
 */
void dtn_log_unmute();

/*----------------------------------------------------------------------------*/

#undef dtn_log_dev
#undef dtn_log_debug
#undef dtn_log_info
#undef dtn_log_notice
#undef dtn_log_warning
#undef dtn_log_error
#undef dtn_log_critical
#undef dtn_log_alert
#undef dtn_log_emergency

#define dtn_log_dev(M, ...)                                                    \
    dtn_log_ng(DTN_LOG_DEV, __FILE__, __FUNCTION__, __LINE__, M, ##__VA_ARGS__)

#define dtn_log_debug(M, ...)                                                  \
    dtn_log_ng(DTN_LOG_DEBUG, __FILE__, __FUNCTION__, __LINE__, M,             \
               ##__VA_ARGS__)

#define dtn_log_info(M, ...)                                                   \
    dtn_log_ng(DTN_LOG_INFO, __FILE__, __FUNCTION__, __LINE__, M, ##__VA_ARGS__)

#define dtn_log_notice(M, ...)                                                 \
    dtn_log_ng(DTN_LOG_NOTICE, __FILE__, __FUNCTION__, __LINE__, M,            \
               ##__VA_ARGS__)

#define dtn_log_warning(M, ...)                                                \
    dtn_log_ng(DTN_LOG_WARNING, __FILE__, __FUNCTION__, __LINE__, M,           \
               ##__VA_ARGS__)

#define dtn_log_error(M, ...)                                                  \
    dtn_log_ng(DTN_LOG_ERR, __FILE__, __FUNCTION__, __LINE__, M, ##__VA_ARGS__)

#define dtn_log_critical(M, ...)                                               \
    dtn_log_ng(DTN_LOG_CRIT, __FILE__, __FUNCTION__, __LINE__, M, ##__VA_ARGS__)

#define dtn_log_alert(M, ...)                                                  \
    dtn_log_ng(DTN_LOG_ALERT, __FILE__, __FUNCTION__, __LINE__, M,             \
               ##__VA_ARGS__)

#define dtn_log_emergency(M, ...)                                              \
    dtn_log_ng(DTN_LOG_EMERG, __FILE__, __FUNCTION__, __LINE__, M,             \
               ##__VA_ARGS__)

#endif /* dtn_log_ng_h */
