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
        @file           dtn_webserver.c
        @author         Töpfer, Markus

        @date           2025-12-17


        ------------------------------------------------------------------------
*/
#include "../include/dtn_webserver.h"
#include "../include/dtn_mimetype.h"

#include <dtn_base/dtn_utils.h>
#include <dtn_base/dtn_string.h>
#include <dtn_base/dtn_uri.h>
#include <dtn_base/dtn_file.h>
#include <dtn_base/dtn_dump.h>
#include <dtn_base/dtn_linked_list.h>
#include <dtn_base/dtn_item_json_io_buffer.h>

/*----------------------------------------------------------------------------*/

struct dtn_webserver{

    dtn_webserver_config config;

    int socket;
    bool debug;

    dtn_dict *connections;
    dtn_dict *domains;
    dtn_dict *callbacks;

    dtn_json_io_buffer *json_io_buffer;
};

/*----------------------------------------------------------------------------*/

typedef enum ConnectionType {

    HTTP = 0,
    WEBSOCKET = 1

} ConnectionType;

/*----------------------------------------------------------------------------*/

typedef struct Connection{

    dtn_webserver *server;

    dtn_socket_data remote;
    
    int socket;
    char *domain;

    dtn_buffer *buffer;

    ConnectionType type;

    struct {

        dtn_list *queue;
        uint64_t counter;
        dtn_websocket_fragmentation_state last;

    } websocket;

} Connection;

/*----------------------------------------------------------------------------*/

typedef struct Callback{

    void *userdata;
    void (*callback)(void *userdata, int socket, dtn_item *item);

} Callback;

/*----------------------------------------------------------------------------*/

static void *connection_free(void *connection){

    Connection *conn = (Connection*) connection;
    if (!conn) return NULL;

    conn->websocket.queue = dtn_list_free(conn->websocket.queue);
    conn->buffer = dtn_buffer_free(conn->buffer);
    conn->domain = dtn_data_pointer_free(conn->domain);
    conn = dtn_data_pointer_free(conn);
    return NULL;
}

/*----------------------------------------------------------------------------*/

static bool cb_io_accept(void *userdata, int listener, int socket){

    dtn_webserver *self = (dtn_webserver*) userdata;
    if (!self) goto error;

    if (self->debug)
        dtn_log_debug("accepted socket %i at %i", socket, listener);

    Connection *conn = calloc(1, sizeof(Connection));

    conn->socket = socket;
    conn->server = self;
    conn->type = HTTP;
    conn->buffer = dtn_buffer_create(2048);
    if (!conn->buffer) goto error;

    if (!dtn_socket_get_data(socket, NULL, &conn->remote)) {
        conn = dtn_data_pointer_free(conn);
        goto error;
    }

    if (!dtn_dict_set(self->connections, (void*)(intptr_t)socket, conn, NULL)){
        conn = connection_free(conn);
        goto error;
    }

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool process_wss_control_frame(Connection *conn, 
    dtn_websocket_frame *frame){

    DTN_ASSERT(conn);
    DTN_ASSERT(frame);

    dtn_websocket_frame *response = NULL;

    switch (frame->opcode){

        case DTN_WEBSOCKET_OPCODE_PONG:
            break;

        case DTN_WEBSOCKET_OPCODE_PING:

            response = dtn_websocket_frame_create(frame->config);

            if (!response)
                goto error;

            // set fin and OV_WEBSOCKET_OPCODE_PONG
            response->buffer->start[0] = 0x8A;

            if (frame->content.start) {

                if (!dtn_websocket_frame_unmask(frame))
                    goto error;

                if (!dtn_websocket_set_data(response, frame->content.start,
                                         frame->content.length, false))
                    goto error;

            } else {

                response->buffer->length = 2;
            }

            if (!dtn_io_send(conn->server->config.io,
                conn->socket,
                (dtn_memory_pointer){
                    .start = response->buffer->start,
                    .length = response->buffer->length
                })) goto error;

            break;

        case DTN_WEBSOCKET_OPCODE_CLOSE:

            dtn_log_debug("received websocket close from %s:%i",
                conn->remote.host, conn->remote.port);

            goto error;

        default:
            goto error;
    } 

    frame = dtn_websocket_frame_free(frame);
    response = dtn_websocket_frame_free(response);
    return true;
error:
    frame = dtn_websocket_frame_free(frame);
    response = dtn_websocket_frame_free(response);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool defragmented_callback(Connection *conn){

    DTN_ASSERT(conn);
    DTN_ASSERT(conn->websocket.queue);

    dtn_websocket_frame *frame = NULL;
    dtn_buffer *buffer = dtn_buffer_create(2048);
    if (!buffer) goto error;

    frame = dtn_list_queue_pop(conn->websocket.queue);
    if (!frame) goto error;

    while(frame){

        if (!dtn_buffer_push(buffer, 
            (void*) frame->content.start, frame->content.length)){
            frame = dtn_websocket_frame_free(frame);
            goto error;
        }

        frame = dtn_websocket_frame_free(frame);
        frame = dtn_list_queue_pop(conn->websocket.queue);

    }

    // we expect only JSON websocket frames
    if (!dtn_json_io_buffer_push(
        conn->server->json_io_buffer,
        conn->socket,
        (dtn_memory_pointer){
            .start = buffer->start,
            .length = buffer->length
            })) goto error;

    buffer = dtn_buffer_free(buffer);
    conn->websocket.counter = 0;
    return true;
error:
    dtn_buffer_free(buffer);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool process_non_fragmented_frame(Connection *conn, 
    dtn_websocket_frame *frame){

    DTN_ASSERT(conn);
    DTN_ASSERT(frame);

    // we expect only JSON websocket frames
    if (!dtn_json_io_buffer_push(
        conn->server->json_io_buffer,
        conn->socket,
        (dtn_memory_pointer){
            .start = frame->content.start,
            .length = frame->content.length
            })) goto error;

    frame = dtn_websocket_frame_free(frame);
    return true;
error:
    frame = dtn_websocket_frame_free(frame);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool defragmented_wss_delivery(Connection *conn, 
    dtn_websocket_frame *frame){

    DTN_ASSERT(conn);
    DTN_ASSERT(frame);

    bool callback_queue = false;

    dtn_websocket_frame *out = NULL;

    switch (frame->state){

        case DTN_WEBSOCKET_FRAGMENTATION_NONE:

            switch (conn->websocket.last){

                case DTN_WEBSOCKET_FRAGMENTATION_NONE:
                case DTN_WEBSOCKET_FRAGMENTATION_LAST:
                    break;

                default:
                    goto error; 
            }

            // non fragmented frame
            return process_non_fragmented_frame(conn, frame);
    
        case DTN_WEBSOCKET_FRAGMENTATION_START:

            switch (conn->websocket.last){

                case DTN_WEBSOCKET_FRAGMENTATION_NONE:
                case DTN_WEBSOCKET_FRAGMENTATION_LAST:
                    break;

                default:
                    goto error; 
            }

            if (!conn->websocket.queue)
                conn->websocket.queue = dtn_linked_list_create(
                    (dtn_list_config){.item.free = dtn_websocket_frame_free});

            // at fragmentation start the queue should be empty

            out = dtn_list_queue_pop(conn->websocket.queue);
            if (out){

                out = dtn_websocket_frame_free(out);
                goto error;
            }

            // push to queue
            break;

        case DTN_WEBSOCKET_FRAGMENTATION_CONTINUE:

            switch (conn->websocket.last){

                case DTN_WEBSOCKET_FRAGMENTATION_START:
                case DTN_WEBSOCKET_FRAGMENTATION_CONTINUE:
                    break;

                default:
                    goto error; 
            }

            // push to queue
            break;

        case DTN_WEBSOCKET_FRAGMENTATION_LAST:

            switch (conn->websocket.last){

                case DTN_WEBSOCKET_FRAGMENTATION_START:
                case DTN_WEBSOCKET_FRAGMENTATION_CONTINUE:
                    break;

                default:
                    goto error; 
            }

            callback_queue = true;
            // push to queue
            break;

        default:
            // fragmentation mismatch
            goto error;
    }

    if (!dtn_list_queue_push(conn->websocket.queue, frame))
        goto error;

    conn->websocket.counter++;
    conn->websocket.last = frame->state;
    frame = NULL;

    if (callback_queue)
        return defragmented_callback(conn);

    return true;
error:
    dtn_websocket_frame_free(frame);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool process_websocket(Connection *conn, dtn_websocket_frame *frame){

    DTN_ASSERT(conn);
    DTN_ASSERT(frame);

    bool result = false;
    bool text = false;

    if (frame->opcode >= 0x08){

        result = process_wss_control_frame(conn, frame);
        goto done;
    }

    switch (frame->opcode) {

        case DTN_WEBSOCKET_OPCODE_CONTINUATION:
            break;
        case DTN_WEBSOCKET_OPCODE_TEXT:
            text = true;
            break;
        case DTN_WEBSOCKET_OPCODE_BINARY:
            text = false;
            break;
        default:
            goto error;
    }

    if (!dtn_websocket_frame_unmask(frame))
        goto error;

    UNUSED(text);
    return defragmented_wss_delivery(conn, frame);

done:
    if (!result) goto error;

    dtn_websocket_frame_free(frame);
    return true;
error:
    frame = dtn_websocket_frame_free(frame);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool io_connection_websocket(Connection *conn){

    DTN_ASSERT(conn);

    dtn_websocket_parser_state state = DTN_WEBSOCKET_PARSER_ERROR;

    bool all_done = false;
    bool result = false;

    while(!all_done){

        if (!conn->buffer) {
            conn->buffer = dtn_buffer_create(2048);
            goto done;
        }

        dtn_websocket_frame *msg = dtn_websocket_frame_pop(&conn->buffer, 
                            &conn->server->config.frame,
                            &state);
        switch (state){

            case DTN_WEBSOCKET_PARSER_SUCCESS:

                DTN_ASSERT(msg);
                result = process_websocket(conn, msg);
                break;

            case DTN_WEBSOCKET_PARSER_PROGRESS:

                if (msg) msg = dtn_websocket_frame_free(msg);
                goto done; 

            default:
                goto error;
        }

        if (!result) goto error;
    }

done:
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool cleaned_path_for_connection(Connection *conn, 
    const dtn_http_message *msg, size_t len, char *out){

    dtn_uri *uri = NULL;
    if (len < PATH_MAX) goto error;

    uri = dtn_uri_from_string((char *)msg->request.uri.start,
                           msg->request.uri.length);

    if (!uri || !uri->path)
        goto error;

    const char *domain_path = dtn_dict_get(conn->server->domains, 
        conn->domain);

    if (!domain_path){

        dtn_log_error("Access to domain %s denied "
            "- no domain root path.", conn->domain);

        goto error;
    }

    char cleaned_path[PATH_MAX + 1] = {0};
    if (!dtn_uri_path_remove_dot_segments(uri->path, cleaned_path))
        goto error;

    /* clean empty paths between document root and uri path,
     *
     * (A) document root may finish with /
     * (B) we ensure to add some /
     * (C) uri may contain some initial /
     *
     * --> delete any non requried for some clean path */

    char full_path[PATH_MAX + 1] = {0};
    
    ssize_t bytes = snprintf(full_path, PATH_MAX, "%s/%s",
                             domain_path, cleaned_path);
    
    if (bytes < 1)
        goto error;
    
    if (!dtn_uri_path_remove_dot_segments(full_path, out))
        goto error;

    if ('/' == out[strlen(out) - 1]) {

        if (PATH_MAX - bytes < 12){
            goto error;
        }

        strcat(out, "index.html");
    }

    uri = dtn_uri_free(uri);
    return true;
error:
    uri = dtn_uri_free(uri);
    return false;
} 


/*----------------------------------------------------------------------------*/

static bool parse_content_range(const dtn_http_header *range,
    size_t *from, size_t *to) {

    DTN_ASSERT(range);
    DTN_ASSERT(from);
    DTN_ASSERT(to);
    
    long n1 = 0;
    long n2 = 0;
    
    if (!dtn_string_startswith((const char *)range->value.start, "bytes="))
        goto error;
    
    char *end_ptr = NULL;
    
    char *ptr = memchr(range->value.start, '=', range->value.length);
    if (!ptr)
        goto error;
    
    ptr++;
    
    n1 = strtol(ptr, &end_ptr, 10);
    
    ptr = end_ptr;
    ptr++;
    
    n2 = strtol(ptr, &end_ptr, 10);
    
    *from = n1;
    *to = n2;
    
    return true;
error:
  return false;
}


/*----------------------------------------------------------------------------*/

static bool answer_range(Connection *conn, const char *path,
    const dtn_http_header *range,
    dtn_http_message *msg){

    dtn_http_message *response = NULL;

    uint8_t *buffer = NULL;
    size_t size = 0;

    DTN_ASSERT(conn);
    DTN_ASSERT(msg);
    DTN_ASSERT(path);
    DTN_ASSERT(range);

    size_t from = 0;
    size_t to = 0;
    size_t all = 0;
    
    if (!parse_content_range(range, &from, &to))
        goto error;

    if (DTN_FILE_SUCCESS !=
        dtn_file_read_partial(path, &buffer, &size, from, to, &all)) {
        dtn_log_error("failed to partial read file %s", path);
        goto error;
    }

    response = dtn_http_create_status_string(
        msg->config, msg->version, 206, DTN_HTTP_PARTIAL_CONTENT);

    if (!dtn_http_message_add_header_string(response, "server", 
        conn->server->config.name))
        goto error;
    
    if (!dtn_http_message_set_date(response))
        goto error;
    
    if (!dtn_http_message_set_content_length(response, size))
        goto error;
    
    if (to == 0)
        to = all;
    
    if (!dtn_http_message_set_content_range(response, all, from, to))
        goto error;
    
    if (!dtn_http_message_add_header_string(response, "Access-Control-Allow-Origin",
                                           "*"))
        goto error;
    
    if (!dtn_http_message_close_header(response))
        goto error;
    
    if (!dtn_http_message_add_body(
            response, (dtn_memory_pointer){.start = buffer, .length = size}))
        goto error;

    if (!dtn_io_send(conn->server->config.io,
        conn->socket,
        (dtn_memory_pointer){
            .start = response->buffer->start,
            .length = response->buffer->length
        })) goto error;

    response = dtn_http_message_free(response);
    buffer = dtn_data_pointer_free(buffer);
    return true;
error:
    response = dtn_http_message_free(response);
    buffer = dtn_data_pointer_free(buffer);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool process_https_get(Connection *conn, dtn_http_message *msg){

    char path[PATH_MAX] = {0};

    dtn_http_message *response = NULL;

    uint8_t *buffer = NULL;
    size_t size = 0;

    DTN_ASSERT(conn);
    DTN_ASSERT(msg);

    if (!cleaned_path_for_connection(conn, msg, PATH_MAX, path)) 
        goto error;

    const dtn_http_header *range = dtn_http_header_get(
        msg->header, msg->config.header.capacity, "Range");

    if (range)
        return answer_range(conn, path, range, msg);

    if (DTN_FILE_SUCCESS != dtn_file_read(path, &buffer, &size)) {
        dtn_log_error("failed to read file %s", path);
        goto error;
    }

    const char *ext = NULL;
    char *ptr = path + strlen(path);
    
    while(ptr[0] != '.'){
        ptr--;
        if (ptr == path) break;
    }

    ext = ptr + 1;

    const char *mimetype = dtn_mimetype_from_file_extension(ext, strlen(ext));

    response = dtn_http_create_status_string(
        conn->server->config.http, 
        (dtn_http_version){.major = 1, .minor = 1},
        200, DTN_HTTP_OK);

    if (!dtn_http_message_add_header_string(response, 
        "server", conn->server->config.name))
        goto error;

    if (!dtn_http_message_set_date(response))
        goto error;

    if (!dtn_http_message_set_content_length(response, size))
        goto error;

    if (mimetype){

        if (!dtn_http_message_add_content_type(msg, mimetype, NULL))
            goto error;

    } else {

        if (!dtn_http_message_add_content_type(msg, "text/plain", NULL))
            goto error;
    }

    if (!dtn_http_message_add_header_string(response, 
        "Accept-Ranges", "bytes"))
        goto error;

    if (!dtn_http_message_close_header(response))
        goto error;

    if (!dtn_http_message_add_body(
          response, 
          (dtn_memory_pointer){
            .start = buffer, 
            .length = size}))
        goto error;

    if (!dtn_io_send(conn->server->config.io,
        conn->socket,
        (dtn_memory_pointer){
            .start = response->buffer->start,
            .length = response->buffer->length
        })) goto error;

    response = dtn_http_message_free(response);
    buffer = dtn_data_pointer_free(buffer);
    return true;
error:
    response = dtn_http_message_free(response);
    buffer = dtn_data_pointer_free(buffer);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool process_http_message(Connection *conn, dtn_http_message *msg){

    DTN_ASSERT(conn);
    DTN_ASSERT(msg);

    const dtn_http_header *header_host = dtn_http_header_get_unique(
        msg->header, msg->config.header.capacity, DTN_HTTP_KEY_HOST);

    if (!header_host)
        goto error;
    
    uint8_t *hostname = (uint8_t *)header_host->value.start;
    size_t hostname_length = header_host->value.length;
    
    uint8_t *colon = memchr(hostname, ':', header_host->value.length);
    if (colon)
        hostname_length = colon - hostname;

    if (0 != strncmp(conn->domain, (char*)hostname, hostname_length)){

        dtn_log_error("HTTPs TLS consistency error,"
            " using domain %s and hostname %.*s at %s:%i - dropping connection",
            conn->domain,
            (int)hostname_length, (char*) hostname,
            conn->remote.host, conn->remote.port);

        goto error;
    }

    if (0 == strncasecmp(DTN_HTTP_METHOD_GET, 
        (char *)msg->request.method.start,
        msg->request.method.length)) {

        return process_https_get(conn, msg);
    }

    dtn_log_error("METHOD type |%.*s| not implemented - closing",
        (int)msg->request.method.length,
        (char *)msg->request.method.start);

    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool process_http(Connection *conn, dtn_http_message *msg){

    DTN_ASSERT(conn);
    DTN_ASSERT(msg);

    dtn_http_message *out = NULL;
    bool is_handshake = false;

    if (dtn_websocket_process_handshake_request(msg, &out, &is_handshake)){

        DTN_ASSERT(out);
        DTN_ASSERT(is_handshake);

        conn->type = WEBSOCKET;

        if (!dtn_io_send(conn->server->config.io,
            conn->socket, 
            (dtn_memory_pointer){
                .start = out->buffer->start,
                .length = out->buffer->length
            })){

            if (conn->server->debug)
                dtn_log_debug("Failed to send handshake response to %s:%i",
                    conn->remote.host, 
                    conn->remote.port);

            out = dtn_http_message_free(out);
            goto error;

        } else {

            out = dtn_http_message_free(out);
            goto done;
        }
        goto done;

    } 

    if (is_handshake) goto error;

    bool result = process_http_message(conn, msg);
    if (!result) goto error;

done:
    dtn_http_message_free(msg);
    return true;
error:
    dtn_http_message_free(msg);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool io_connection_http(Connection *conn){

    DTN_ASSERT(conn);

    dtn_http_parser_state state = DTN_HTTP_PARSER_ERROR;

    bool all_done = false;
    bool result = false;

    while(!all_done){

        if (!conn->buffer) {
            conn->buffer = dtn_buffer_create(2048);
            goto done;
        }

        dtn_http_message *msg = dtn_http_message_pop(&conn->buffer, 
                            &conn->server->config.http,
                            &state);
        switch (state){

            case DTN_HTTP_PARSER_SUCCESS:

                if (msg){
                    result = process_http(conn, msg);
                } else {
                    goto done;
                }
                break;

            case DTN_HTTP_PARSER_PROGRESS:

                if (msg) msg = dtn_http_message_free(msg);
                goto done; 

            default:
                goto error;
        }

        if (!result) goto error;
    }

done:
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static bool io_connection(dtn_webserver *self, Connection *conn, const char *domain, 
    const dtn_memory_pointer data){

    int socket = 0;

    if (!conn || !domain) goto error;

    socket = conn->socket;

    // 1st check domain input (same domain like in first contact)

    if (conn->server->debug){
        fprintf(stdout, "IO from %s:%i\n", conn->remote.host, conn->remote.port);
        dtn_dump_binary_as_hex(stdout, (uint8_t*)data.start, data.length);
        fprintf(stdout, "\n");
    }

    if (!conn->domain){
        
        conn->domain = dtn_string_dup(domain);
    
    } else {

        if (0 != dtn_string_compare(conn->domain, domain)){

            dtn_log_error("Connection %i switched from domain %s to domain %s",
                conn->socket, 
                conn->domain, 
                domain);

            goto error;
        }
    }

    if (conn->buffer == NULL)
        conn->buffer = dtn_buffer_create(2048);

    if (!dtn_buffer_push(conn->buffer, (uint8_t*) data.start, data.length))
        goto error;

    bool result = false;

    switch (conn->type){

        case HTTP:
            result = io_connection_http(conn);
            break;
        case WEBSOCKET:
            result = io_connection_websocket(conn); 
            break;
    }

    if (!result) goto error;
    return true;
error:
    if (self) dtn_io_close(self->config.io, socket);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool cb_io(void *userdata, int connection,
             const char *domain, 
             const dtn_memory_pointer data){

    dtn_webserver *self = (dtn_webserver*) userdata;
    if (!self || connection < 1 || !domain) goto error;

    Connection *conn = dtn_dict_get(self->connections, (void*)(intptr_t) connection);
    if (!conn) goto error;

    return io_connection(self, conn, domain, data);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

static void cb_close(void *userdata, int connection){

    dtn_webserver *self = (dtn_webserver*) userdata;
    
    if (!self || connection < 1) goto error;

    dtn_log_debug("closing socket %i", connection);

    dtn_dict_del(self->connections, (void*)(intptr_t) connection);
    dtn_json_io_buffer_drop(self->json_io_buffer, connection);

error:
    return;
}

/*----------------------------------------------------------------------------*/

static void cb_json_success(void *userdata, int socket, dtn_item *val){

    if (!userdata || !val) goto error;

    dtn_webserver *self = (dtn_webserver*) userdata;
    
    Connection *conn = dtn_dict_get(self->connections, (void*)(intptr_t)socket);
    if (!self || !conn) goto error;

    Callback *cb = dtn_dict_get(self->callbacks, conn->domain);
    if (!cb) goto error;

    cb->callback(cb->userdata, socket, val);
    return;
error:
    dtn_item_free(val);
    return;
}

/*----------------------------------------------------------------------------*/

static void cb_json_failure(void *userdata, int socket){

    if (!userdata) goto error;

    dtn_webserver *self = (dtn_webserver*) userdata;
    
    dtn_log_error("JSON IO failure at %i - closing", socket);
    dtn_io_close(self->config.io, socket);

error:
    return;
}

/*----------------------------------------------------------------------------*/

static bool init_config(dtn_webserver_config *config){

    if (!config || !config->loop || !config->io) goto error;

    if (0 == config->socket.host[0]){

        config->socket = (dtn_socket_configuration){
            .host = "0.0.0.0",
            .type = TLS,
            .port = 443
        };

    }

    // ensure TLS is set
    config->socket.type = TLS;

    if (0 == config->name[0])
        strncpy(config->name, "opendtn", 8);

    config->http = dtn_http_message_config_init(config->http);

    return true;
error:
    return false;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      GENERIC FUNCTIONS
 *
 *      ------------------------------------------------------------------------
 */

dtn_webserver *dtn_webserver_create(dtn_webserver_config config){

    dtn_webserver *self = NULL;

    if (!init_config(&config)) goto error;

    self = calloc(1, sizeof(dtn_webserver));
    if (!self) goto error;

    self->config = config;

    self->socket = dtn_io_open_listener(self->config.io,
        (dtn_io_socket_config){
            .socket = self->config.socket,
            .callbacks.userdata = self,
            .callbacks.accept = cb_io_accept,
            .callbacks.io = cb_io,
            .callbacks.close = cb_close
        });

    if (-1 == self->socket){

        dtn_log_error("Failed to open socket %s:%i - abort.",
            self->config.socket.host,
            self->config.socket.port);

        goto error;
    }

    dtn_dict_config d_config = dtn_dict_intptr_key_config(255);
    d_config.value.data_function.free = connection_free;

    self->connections = dtn_dict_create(d_config);
    if (!self->connections) goto error;

    d_config = dtn_dict_string_key_config(255);
    d_config.value.data_function.free = dtn_data_pointer_free;  
    self->domains = dtn_dict_create(d_config);
    if (!self->domains) goto error;

    d_config = dtn_dict_string_key_config(255);
    d_config.value.data_function.free = dtn_data_pointer_free;  
    self->callbacks = dtn_dict_create(d_config);
    if (!self->callbacks) goto error;

    self->json_io_buffer = dtn_json_io_buffer_create(
        (dtn_json_io_buffer_config){
            .callback.userdata = self,
            .callback.success = cb_json_success,
            .callback.failure = cb_json_failure
        });
    if (!self->json_io_buffer) goto error;

    return self;
error:
    dtn_webserver_free(self);
    return NULL;

}

/*----------------------------------------------------------------------------*/

dtn_webserver *dtn_webserver_free(dtn_webserver *self){

    if (!self) return self;

    self->json_io_buffer = dtn_json_io_buffer_free(self->json_io_buffer);
    self->connections = dtn_dict_free(self->connections);
    self->callbacks = dtn_dict_free(self->callbacks);
    self->domains = dtn_dict_free(self->domains);
    self = dtn_data_pointer_free(self);
    return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_webserver_set_debug(dtn_webserver *self, bool on){

    if (!self) goto error;

    self->debug = on;
    return true;
error:
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_webserver_enable_callback(
        dtn_webserver *self,
        const char *domain,
        void *userdata,
        void (*callback)(void *userdata, int socket, dtn_item *msg)){

    Callback *cb = NULL;
    char *key = NULL;

    if (!self || !domain || !userdata || !callback) goto error;

    cb = calloc(1, sizeof(Callback));
    if (!cb) goto error;

    cb->userdata = userdata;
    cb->callback = callback;

    key = dtn_string_dup(domain);
    if (!key) goto error;

    if (!dtn_dict_set(self->callbacks, key, cb, NULL))
        goto error;

    return true;
error:
    dtn_data_pointer_free(cb);
    dtn_data_pointer_free(key);
    return false;
}

/*----------------------------------------------------------------------------*/

static bool enable_domain(void *item, void *data){

    char *key = NULL;
    char *val = NULL;

    dtn_item *config = dtn_item_cast(item);
    dtn_webserver *self = (dtn_webserver*) data;

    if (!config || !self) goto error;

    dtn_item *name = dtn_item_object_get(config, "name");
    if (!name) goto error;

    dtn_item *path = dtn_item_object_get(config, "path");
    if (!path) goto error;

    key = dtn_string_dup(dtn_item_get_string(name));
    val = dtn_string_dup(dtn_item_get_string(path));

    if (!key || !val) goto error;

    if (!dtn_dict_set(self->domains, key, val, NULL)) goto error;

    return true;
error:
    key = dtn_data_pointer_free(key);
    val = dtn_data_pointer_free(val);
    return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_webserver_enable_domains(dtn_webserver *self, const dtn_item *config){

    if (!self || !config) goto error;

    dtn_item *conf = dtn_item_get(config, "/webserver/domains");
    if (!conf) goto error;

    return dtn_item_array_for_each(conf, self, enable_domain);
error:
    return false;
}

/*----------------------------------------------------------------------------*/

dtn_webserver_config dtn_webserver_config_from_item(const dtn_item *input){

    dtn_webserver_config config = {0};
    if (!input) goto error;

    const dtn_item *item = dtn_item_object_get(input, "webserver");
    if (!item) item = input;

    const char *name = dtn_item_get_string(dtn_item_object_get(item, "name"));
    if (name) strncpy(config.name, name, PATH_MAX);

    config.socket = dtn_socket_configuration_from_item(
        dtn_item_object_get(item, "socket"));

    dtn_item *http = dtn_item_object_get(item, "http");
    if (http){

        config.http.header.capacity = dtn_item_get_number(
            dtn_item_object_get(http, "capacity"));

        config.http.header.max_bytes_method_name = dtn_item_get_number(
            dtn_item_object_get(http, "max_method_name"));

        config.http.header.max_bytes_line = dtn_item_get_number(
            dtn_item_object_get(http, "max_byte_line"));

        config.http.buffer.default_size = dtn_item_get_number(
            dtn_item_object_get(http, "buffer_size"));

        config.http.buffer.max_bytes_recache = dtn_item_get_number(
            dtn_item_object_get(http, "buffer_size_recache"));

        config.http.transfer.max = dtn_item_get_number(
            dtn_item_object_get(http, "max_transfer"));

        config.http.chunk.max_bytes = dtn_item_get_number(
            dtn_item_object_get(http, "max_chunk_bytes"));

    }

    dtn_item *frame = dtn_item_object_get(item, "frame");
    if (frame){

        config.frame.buffer.default_size = dtn_item_get_number(
            dtn_item_object_get(http, "buffer_size"));

        config.frame.buffer.max_bytes_recache = dtn_item_get_number(
            dtn_item_object_get(http, "buffer_size_recache"));

    }

    return config;
error:
    return (dtn_webserver_config){0};
}

/*----------------------------------------------------------------------------*/

bool dtn_webserver_send(dtn_webserver *self, int socket, const dtn_item *msg){

    char *string = NULL;
    dtn_websocket_frame *frame = NULL;

    if (!self || !msg) goto error;

    Connection *conn = dtn_dict_get(self->connections, (void*)(intptr_t)socket);
    if (!conn) goto error;

    string = dtn_item_to_json(msg);
    if (!string) goto error;

    ssize_t length = strlen(string);

    frame = dtn_websocket_frame_create(self->config.frame);
    if (!frame) goto error;

    dtn_websocket_frame_clear(frame);

    ssize_t chunk = 1000;
    
    if (length < chunk){

        frame->buffer->start[0] = 0x80 | DTN_WEBSOCKET_OPCODE_TEXT;

        if (!dtn_websocket_set_data(frame, 
            (uint8_t *)string, length, false)) goto error;
        
        if (!dtn_io_send(self->config.io,
            conn->socket,
            (dtn_memory_pointer){
                .start = frame->buffer->start,
                .length = frame->buffer->length
            })) goto error;

        goto done;
    }
    
    // send in chunks 

    size_t counter = 0;

    uint8_t *ptr = (uint8_t *)string;
    ssize_t open = length;

    frame->buffer->start[0] = 0x00 | DTN_WEBSOCKET_OPCODE_TEXT;

    if (!dtn_websocket_set_data(frame, ptr, chunk, false)) goto error;

    if (!dtn_io_send(self->config.io,
            conn->socket,
            (dtn_memory_pointer){
                .start = frame->buffer->start,
                .length = frame->buffer->length
            })) goto error;

    counter++;
    open -= chunk;
    ptr += chunk;

    while(open > chunk){
        
        frame->buffer->start[0] = 0x00;

        if (!dtn_websocket_set_data(frame, ptr, chunk, false))
            goto error;

        if (!dtn_io_send(self->config.io,
            conn->socket,
            (dtn_memory_pointer){
                .start = frame->buffer->start,
                .length = frame->buffer->length
            })) goto error;

        open -= chunk;
        ptr += chunk;

        counter++;
    }

    frame->buffer->start[0] = 0x80;

    if (!dtn_websocket_set_data(frame, ptr, open, false))
            goto error;

    if (!dtn_io_send(self->config.io,
        conn->socket,
        (dtn_memory_pointer){
            .start = frame->buffer->start,
            .length = frame->buffer->length
        })) goto error;

done:
    dtn_data_pointer_free(string);
    dtn_websocket_frame_free(frame);
    return true;
error:
    dtn_data_pointer_free(string);
    dtn_websocket_frame_free(frame);
    return false;
}

/*----------------------------------------------------------------------------*/

dtn_event_loop *dtn_webserver_get_eventloop(const dtn_webserver *self){

    if (!self) return NULL;
    return self->config.loop;
}