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
        @file           dtn_websocket_message.h
        @author         Markus Töpfer

        @date           2021-01-12

        Definition of websocket message configuration for dtn_webserver_base.

        Websockets are supported in 2 modes. Frame by frame delivery, as well
        as message delivery. Control frames will alwayse be processed within
        dtn_webserver_base.

        For frame by frame delivery use config.fragmented of the config,
        to receive any content frame in order of its arrival.

        For message based delivery, independent of fragmented or unfragmented
        frames, use config.callback of the config. This will callback with
        defragmented content data of the frames. So even if some message is
        deliveres over multiple frames, the callback will return with the
        whole message.

        NOTE to limit frames max_frames may be used to prevent memory blowups
        due to unclosed websockets frame chains, which are buffered within
        dtn_webserver until the frame chain is completed.

        NOTE max_frames SHOULD be set to some reasonable value for the expected
        content.

        ------------------------------------------------------------------------
*/
#ifndef dtn_websocket_message_h
#define dtn_websocket_message_h

#include <limits.h>
#include <stdbool.h>

#include "dtn_websocket_pointer.h"
#include <dtn_base/dtn_memory_pointer.h>

/*----------------------------------------------------------------------------*/

typedef struct {

  char uri[PATH_MAX];

  void *userdata;

  /* max_frames may be used to limit the amount of allowed frames
   * in defragmented (buffered) delivery mode */

  uint32_t max_frames;

  /* This callback may be used to deliver the content of websocket frames,
   * independent of the fragmentation mode used. It will callback, once
   * some frame is completed. */

  bool (*callback)(void *userdata, int socket, const dtn_memory_pointer domain,
                   const char *uri, dtn_memory_pointer content, bool text);

  /* This callback may be used to deliver any non control frame as received */

  bool (*fragmented)(void *userdata, int socket, const dtn_memory_pointer domain,
                     const char *uri, dtn_websocket_frame *frame);

  /* This callback will forward the close calls to userdata */
  void (*close)(void *userdata, int socket);

} dtn_websocket_message_config;

/*----------------------------------------------------------------------------*/

/**
    Create some websocket upgrade message

    @param host     host to set (e.g. domain)
    @param uri      uri to use
    @param sec_key  sec_key to use

    @return upgrade message or null on error
*/
dtn_http_message *dtn_websocket_upgrade_request(const char *host, const char *uri,
                                              dtn_memory_pointer sec_key);

/*----------------------------------------------------------------------------*/

bool dtn_websocket_is_upgrade_response(const dtn_http_message *msg,
                                      dtn_memory_pointer sec_key);

#endif /* dtn_websocket_message_h */
