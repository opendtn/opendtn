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
        @file           dtn_websocket_message.c
        @author         Markus Töpfer

        @date           2021-01-12


        ------------------------------------------------------------------------
*/
#include "../include/dtn_websocket_message.h"

dtn_http_message *dtn_websocket_upgrade_request(const char *host, const char *uri,
                                              dtn_memory_pointer sec_key) {

  dtn_http_message *msg = NULL;

  if (!uri || !host || !sec_key.start)
    goto error;

  msg = dtn_http_create_request_string((dtn_http_message_config){0},
                                      (dtn_http_version){.major = 1, .minor = 1},
                                      DTN_HTTP_METHOD_GET, uri);

  if (!msg)
    goto error;

  if (!dtn_http_message_add_header_string(msg, DTN_HTTP_KEY_UPGRADE,
                                         DTN_WEBSOCKET_KEY))
    goto error;

  if (!dtn_http_message_add_header_string(msg, DTN_HTTP_KEY_CONNECTION,
                                         DTN_WEBSOCKET_KEY_UPGRADE))
    goto error;

  if (!dtn_http_message_add_header_string(msg, DTN_HTTP_KEY_HOST, host))
    goto error;

  if (!dtn_http_message_add_header(
          msg, (dtn_http_header){

                   .name =
                       (dtn_memory_pointer){
                           .start = (uint8_t *)DTN_WEBSOCKET_KEY_SECURE,
                           .length = strlen(DTN_WEBSOCKET_KEY_SECURE)},

                   .value = sec_key}))
    goto error;

  if (!dtn_http_message_add_header_string(msg, DTN_WEBSOCKET_KEY_SECURE_VERSION,
                                         DTN_WEBSOCKET_VERSION))
    goto error;

  if (!dtn_http_message_close_header(msg))
    goto error;

  if (DTN_HTTP_PARSER_SUCCESS != dtn_http_pointer_parse_message(msg, NULL))
    goto error;

  return msg;
error:
  dtn_http_message_free(msg);
  return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_websocket_is_upgrade_response(const dtn_http_message *msg,
                                      dtn_memory_pointer sec_key) {

  uint8_t *accept_key = NULL;
  size_t accept_key_length = 0;

  if (!msg || !sec_key.start)
    goto error;

  if (msg->status.code != 101)
    goto error;

  const dtn_http_header *header = dtn_http_header_get_unique(
      msg->header, msg->config.header.capacity, DTN_HTTP_KEY_UPGRADE);

  if (!header)
    goto error;

  if (header->value.length != strlen(DTN_WEBSOCKET_KEY))
    goto error;

  if (0 != memcmp(DTN_WEBSOCKET_KEY, header->value.start, header->value.length))
    goto error;

  header = dtn_http_header_get_unique(msg->header, msg->config.header.capacity,
                                     DTN_HTTP_KEY_CONNECTION);

  if (!header)
    goto error;

  if (header->value.length != strlen(DTN_HTTP_KEY_UPGRADE))
    goto error;

  if (0 !=
      memcmp(DTN_HTTP_KEY_UPGRADE, header->value.start, header->value.length))
    goto error;

  header = dtn_http_header_get_unique(msg->header, msg->config.header.capacity,
                                     DTN_WEBSOCKET_KEY_SECURE_ACCEPT);

  if (!header)
    goto error;

  if (!dtn_websocket_generate_secure_accept_key(sec_key.start, sec_key.length,
                                               &accept_key, &accept_key_length))
    goto error;

  if (accept_key_length != header->value.length)
    goto error;

  if (0 != memcmp(accept_key, header->value.start, header->value.length))
    goto error;

  dtn_data_pointer_free(accept_key);
  return true;
error:
  dtn_data_pointer_free(accept_key);
  return false;
}
