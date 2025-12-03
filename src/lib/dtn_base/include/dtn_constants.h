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
        See the License for the specific language gDTNerning permissions and
        limitations under the License.

        This file is part of the openvocs project. https://openvocs.org

        ------------------------------------------------------------------------
*//**

        @author         Michael J. Beer

        Global compile time constants.

        Value of DTN_CONSTANT might be DTNerwritten by
        -DDTN_CONSTANT compiler flag like
        -DDTN_UDP_PAYLOAD_OCTETS=512 to set max supported UDP payload length to 512 octets

        ------------------------------------------------------------------------
*/
#ifndef DTN_CONSTANTS_H
#define DTN_CONSTANTS_H

#include <stdint.h>

#ifndef DTN_ERROR_NO_ERROR

#define DTN_ERROR_NO_ERROR 0

#endif

/*****************************************************************************
                                     DEBUG
 ****************************************************************************/

#ifdef DEBUG

#define DTN_SIP_APP_DEBUG

#endif

/*****************************************************************************
                                  DEFAULTS ETC
 ****************************************************************************/

#ifndef DTN_MAX_FRAME_LENGTH_MS

#define DTN_MAX_FRAME_LENGTH_MS 20

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_FRAME_LENGTH_MS

#define DTN_DEFAULT_FRAME_LENGTH_MS 20

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_MAX_CSRCS

#define DTN_MAX_CSRCS 15

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_UDP_PAYLOAD_OCTETS

/* RFC  1122 requires a MINIMUM of 576 octets for the EMTU_R.
 * 576 minus the UDP header is the MINIMUM supported payload size.
 * However, DTNer ethernet, the MTU is up to 1500 (dep. on the flavor),
 * thus DTNer ethernet, we might encounter UDP datagrams of of up to 1500 octets.
 * Let's play it safe and assume 2k = 2048
 */
#define DTN_UDP_PAYLOAD_OCTETS 2048

#endif

// #define DTN_DISABLE_CACHING 1

/*----------------------------------------------------------------------------*/

/* You might also define `DTN_ARCH` to force compilation for a specific
 * architecture like
 *
 * -DDTN_ARCH=2 to enforce Linux
 *
 *  See DTN_arch/DTN_arch.h for possible options
 */

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_CACHE_SIZE

#define DTN_DEFAULT_CACHE_SIZE 50

#endif

/*----------------------------------------------------------------------------*/

/**
 * Maximum length of a hostname
 * Under Linux, this would be the constant DTN_HOST_NAME_MAX,
 * under BSDs, it would be _POSIX_DTN_HOST_NAME_MAX.
 *
 * The maximum allowed hostname, in general, is 255 however.
 */
#ifndef DTN_HOST_NAME_MAX

#define DTN_HOST_NAME_MAX 255

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_USER

#define DTN_DEFAULT_USER "unspecified"

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_RECORDER_REPOSITORY_ROOT_PATH

#define DTN_DEFAULT_RECORDER_REPOSITORY_ROOT_PATH "/tmp"

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_NUM_THREADS

#define DTN_DEFAULT_NUM_THREADS 4

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_LOCK_TIMEOUT_MSECS

#define DTN_DEFAULT_LOCK_TIMEOUT_MSECS 500

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_REQUEST_TIMEOUT_MSECS

#define DTN_DEFAULT_REQUEST_TIMEOUT_MSECS (3 * 1000)

#endif

/*----------------------------------------------------------------------------*/

/**
 * How many signaling requests are supported in parallel?
 * Used at least within SIP gateway to communicating with resource manager
 */
#ifndef DTN_MAX_NUM_ACTIVE_REQUESTS

#define DTN_MAX_NUM_ACTIVE_REQUESTS 50

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_RECONNECT_INTERVAL_SECS

#define DTN_DEFAULT_RECONNECT_INTERVAL_SECS 10

#endif

/*----------------------------------------------------------------------------*/

/**
 * Maximum number of supported translations in a translation table,
 * i.e. maximum number of sources and destinations supported by a minion.
 */
#ifndef DTN_DEFAULT_MAX_NUM_TRANSLATIONS

#define DTN_DEFAULT_MAX_NUM_TRANSLATIONS DTN_MAX_CSRCS

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_MAX_TALKERS_PER_LOOP

#define DTN_MAX_TALKERS_PER_LOOP DTN_MAX_CSRCS

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_MAX_LISTENERS_PER_LOOP

#define DTN_MAX_LISTENERS_PER_LOOP DTN_MAX_CSRCS

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_MAX_RECORDINGS_PER_LOOP

#define DTN_MAX_RECORDINGS_PER_LOOP DTN_MAX_CSRCS

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_MAX_PLAYBACKS_PER_LOOP

#define DTN_MAX_PLAYBACKS_PER_LOOP DTN_MAX_CSRCS

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_SSID

#define DTN_DEFAULT_SSID 123456789

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_EC_FRAMES_PER_SSID

#define DTN_DEFAULT_EC_FRAMES_PER_SSID 3

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_CONFIG_DIRECTORY

#define DTN_DEFAULT_CONFIG_DIRECTORY "/etc/openvocs"

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_LOG_DIRECTORY

#define DTN_DEFAULT_LOG_DIRECTORY "/tmp"

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_LOG_LEVEL_STRING

#define DTN_DEFAULT_LOG_LEVEL_STRING "info"

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_LOG_MAX_NUM_FILES

#define DTN_DEFAULT_LOG_MAX_NUM_FILES 5

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_LOG_MESSAGES_PER_FILE

#define DTN_DEFAULT_LOG_MESSAGES_PER_FILE 10000

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_PATH_DELIMITER

#define DTN_PATH_DELIMITER '/'

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_MAX_SAMPLERATE_HZ

#define DTN_MAX_SAMPLERATE_HZ 48000

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_SAMPLERATE

#define DTN_DEFAULT_SAMPLERATE 48000

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_CODEC

#define DTN_DEFAULT_CODEC "opus"

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_COMFORT_NOISE_LEVEL_DB

#define DTN_DEFAULT_COMFORT_NOISE_LEVEL_DB ((int16_t) - 50.0)

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_NORMALIZE_INPUT

#define DTN_DEFAULT_NORMALIZE_INPUT true

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_RTP_PORT

#define DTN_DEFAULT_RTP_PORT 12345

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_KEEPALIVE_RTP_FRAMES

#define DTN_DEFAULT_KEEPALIVE_RTP_FRAMES true

#endif

/*----------------------------------------------------------------------------*/

/**
 * The first frame of an RTP stream ought to be marked to indicate start of
 * stream.
 * If the first (marked) frame is lost, the stream will never be considered
 * 'established'.
 * Hence, the first frame of an RTP stream is repeated several times.
 * This constant defines how often it should be repeated.
 */
#ifndef DTN_DEFAULT_RTP_REPETITIONS_START_FRAME

#define DTN_DEFAULT_RTP_REPETITIONS_START_FRAME 5

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_OCTETS_PER_SAMPLE

#define DTN_DEFAULT_OCTETS_PER_SAMPLE 2

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_PAYLOAD_TYPE

#define DTN_DEFAULT_PAYLOAD_TYPE 96

#endif

/*----------------------------------------------------------------------------*/

/**
 * Max difference for two floating points to still be considered equal
 */
#ifndef DTN_MAX_FLOAT_DELTA

#define DTN_MAX_FLOAT_DELTA 0.0000000000001

#endif

/*----------------------------------------------------------------------------*/

/**
 * MAX volume value in the backend
 */
#define DTN_MAX_VOLUME UINT16_MAX

/*----------------------------------------------------------------------------*/

/**
 * For noise detection
 */
#ifndef DTN_DEFAULT_POWERLEVEL_DENSITY_THRESHOLD_DB

#define DTN_DEFAULT_POWERLEVEL_DENSITY_THRESHOLD_DB ((double)-50.0)

#endif

/**
 * For noise detection
 */
#ifndef DTN_DEFAULT_ZERO_CROSSINGS_RATE_THRESHOLD_HZ

// Voice does not contain frequencies higher than 10kHz, thus normal voice
// cannot contain more than 10 zero crossings per 1msec.
// the unit is 'per sample'
#define DTN_DEFAULT_ZERO_CROSSINGS_RATE_THRESHOLD_HZ 10000

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_MAX_BACKTRACE_DEPTH

#define DTN_MAX_BACKTRACE_DEPTH 300

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_VM_TIMER_INTERVAL_USECS

#define DTN_DEFAULT_VM_TIMER_INTERVAL_USECS (1 * 1000 * 1000)

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_SIP_NUM_HEADER

#define DTN_DEFAULT_SIP_NUM_HEADER 15

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_DEFAULT_SIP_CSEQ

#define DTN_DEFAULT_SIP_CSEQ 10

#endif

/*----------------------------------------------------------------------------*/

#ifndef DTN_MAX_ANALOG_SSRC

#define DTN_MAX_ANALOG_SSRC 66000

#endif

#ifndef DTN_DEFAULT_CUTOFF_AFTER_FRAMES

/**
 * If Voice detection is active, determines after how many frames of no
 * voice having been detected the voice should be considered gone.
 * Used e.g. by the recorder to stop a recording if VAD is active.
 */
#define DTN_DEFAULT_CUTOFF_AFTER_FRAMES 100

#endif

/*****************************************************************************
 *                           DERIVED CONSTANTS
 ****************************************************************************/

#define DTN_DEFAULT_LOCK_TIMEOUT_USECS (1000 * DTN_DEFAULT_LOCK_TIMEOUT_MSECS)

#define DTN_DEFAULT_LOG_LEVEL                                                   \
  DTN_log_level_from_string(DTN_DEFAULT_LOG_LEVEL_STRING)

/*----------------------------------------------------------------------------*/

#define DTN_MAX_FRAME_LENGTH_SAMPLES                                            \
  (DTN_MAX_SAMPLERATE_HZ * DTN_MAX_FRAME_LENGTH_MS / 1000)

#define DTN_MAX_FRAME_LENGTH_BYTES                                              \
  (DTN_MAX_FRAME_LENGTH_SAMPLES * sizeof(int16_t))

#define DTN_DEFAULT_FRAME_LENGTH_SAMPLES                                        \
  (DTN_DEFAULT_SAMPLERATE * DTN_DEFAULT_FRAME_LENGTH_MS / 1000)

/*----------------------------------------------------------------------------*/

#define DTN_INTERNAL_CODEC                                                      \
  "{"                                                                          \
  "\"sample_rate_hz\": " TO_STR(DTN_DEFAULT_SAMPLERATE) ","                     \
                                                       "\"codec\": "           \
                                                       "\"" DTN_DEFAULT_CODEC   \
                                                       "\""                    \
                                                       "}"

/*----------------------------------------------------------------------------*/
#endif
