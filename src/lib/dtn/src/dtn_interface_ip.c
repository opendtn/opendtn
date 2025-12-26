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
        @file           dtn_interface_ip.c
        @author         Töpfer, Markus

        @date           2025-12-19


        ------------------------------------------------------------------------
*/
#include "../include/dtn_interface_ip.h"

#include "../include/dtn_bundle.h"

#include <dtn_base/dtn_utils.h>
#include <dtn_base/dtn_io_buffer.h>
#include <dtn_base/dtn_dump.h>
#include <dtn_base/dtn_thread_lock.h>

/*---------------------------------------------------------------------------*/

#define DTN_INTERFACE_IP_MAGIC_BYTE 0x1ff1

/*---------------------------------------------------------------------------*/

struct dtn_interface_ip {

    uint16_t magic_byte;
    dtn_interface_ip_config config;

    dtn_socket_data local;
    int socket;

    dtn_ip_link_state link;

    dtn_io_buffer *buffer;

    struct {

        dtn_thread_lock lock;
        dtn_list *queue;

    } out;

    struct {

        uint32_t link_check;

    } timer;
};

/*---------------------------------------------------------------------------*/

struct out_data {

    dtn_socket_configuration remote;
    dtn_buffer *buffer;
};

/*---------------------------------------------------------------------------*/

static void *out_data_free(void *data){

    if (!data) return NULL;

    struct out_data *self = (struct out_data*) data;
    self->buffer = dtn_buffer_free(self->buffer);
    self = dtn_data_pointer_free(self);
    return NULL;
}

/*---------------------------------------------------------------------------*/

static bool start_sending_queue(dtn_interface_ip *self){

    if (!dtn_thread_lock_try_lock(&self->out.lock)) goto done;

    struct out_data *data = dtn_list_queue_pop(self->out.queue);
    
    while(data){

        struct sockaddr_storage sa = {0};
        socklen_t sock_len = sizeof(sa);
    
        int type = AF_INET;
    
        char *ptr = memchr(data->remote.host, '.', strlen(data->remote.host));
        
        if (ptr){
            sock_len = sizeof(struct sockaddr_in);
            type = AF_INET;
        } else {
            sock_len = sizeof(struct sockaddr_in6);
            type = AF_INET6;
        }
    
        dtn_socket_fill_sockaddr_storage(&sa, type, data->remote.host, data->remote.port);
    
        ssize_t bytes = sendto(self->socket, data->buffer->start, data->buffer->length, 0, 
            (struct sockaddr*)&sa, sock_len);

        if (-1 == bytes){
            dtn_list_queue_push(self->out.queue, data);
            data = NULL;
        }
    
        data = out_data_free(data);
        data = dtn_list_queue_pop(self->out.queue);
    }

    if (!dtn_thread_lock_unlock(&self->out.lock)){
        dtn_log_error("failed to unlock out queue");
    }

done:
    return true;
}

/*---------------------------------------------------------------------------*/

static bool process_state_change(dtn_interface_ip *self){

    switch(self->link){

        case DTN_IP_LINK_UP:
            break;

        default:
            goto done;

    }

    // change to up, we start sending

    start_sending_queue(self);

done:
    return true;
}

/*---------------------------------------------------------------------------*/

static bool check_link_status(uint32_t timer_id, void *userdata){

    UNUSED(timer_id);
    dtn_interface_ip *self = dtn_interface_ip_cast(userdata);

    char *interface_name = dtn_ip_link_get_interface_name(self->socket);
    if (!interface_name) goto reschedule;

    dtn_ip_link_state current = dtn_ip_link_get_state(interface_name);
    interface_name = dtn_data_pointer_free(interface_name);

    if (current == self->link) goto reschedule;

    self->link = current;

    process_state_change(self);

    if (self->config.callbacks.state){

        self->config.callbacks.state(
            self->config.callbacks.userdata, 
            current,
            self->config.socket.host);
    
    }

reschedule:
    
    self->timer.link_check = dtn_event_loop_timer_set(
        self->config.loop, self->config.limits.link_check, self, check_link_status);

    if (DTN_TIMER_INVALID == self->timer.link_check) {

        dtn_log_error("LINK CHECKING ERROR.");
    }

    return true;
}

/*---------------------------------------------------------------------------*/

static bool start_link_check(dtn_interface_ip *self){

    if (!self) goto error;

    self->timer.link_check = dtn_event_loop_timer_set(
        self->config.loop, self->config.limits.link_check, self, check_link_status);

    if (DTN_TIMER_INVALID == self->timer.link_check) goto error;

    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

static bool process_bundle(dtn_interface_ip *self, 
    const dtn_socket_data *remote, dtn_bundle *bundle){

    if (!self || !remote || !bundle) goto error;

    if (self->config.callbacks.io){

        self->config.callbacks.io(
            self->config.callbacks.userdata,
            remote,
            bundle,
            self->config.socket.host);
    
    } else {

        dtn_bundle_free(bundle);
    }

    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

static bool cb_io(int socket, uint8_t event, void *userdata){

    uint8_t buffer[2048] = {0};
    size_t size = 2048;
    uint8_t *next = NULL;
    dtn_socket_data remote = {0};
    socklen_t src_addr_len = sizeof(remote.sa);

    dtn_interface_ip *self = dtn_interface_ip_cast(userdata);
    if (!self || socket < 1) goto error;

    if (event & DTN_EVENT_IO_ERR || event & DTN_EVENT_IO_CLOSE){

        dtn_log_error("failure at interface, socket closed.");
        TODO("... handle socket close");

        if (self->config.callbacks.close)
            self->config.callbacks.close(self->config.callbacks.userdata,
                dtn_interface_ip_name(self));
    }

    ssize_t bytes = recvfrom(socket, (char *)buffer, size, 0,
                           (struct sockaddr *)&remote.sa, &src_addr_len);

    if (bytes < 0) goto done;

    if (!dtn_socket_parse_sockaddr_storage(&remote.sa, remote.host,
            DTN_HOST_NAME_MAX, &remote.port)) goto error;

    dtn_buffer *out = dtn_io_buffer_pop(self->buffer, &remote);
    if (!out) out = dtn_buffer_create(bytes);
    if (!dtn_buffer_push(out, buffer, bytes)) goto error;

    dtn_bundle *bundle = NULL;
    dtn_cbor_match match = dtn_bundle_decode(
            out->start, out->length, &bundle, &next);

    switch (match){

        case DTN_CBOR_NO_MATCH:

            out = dtn_buffer_free(out);
            goto error;
            break;

        case DTN_CBOR_MATCH_PARTIAL:

            DTN_ASSERT(next);

            if (next - out->start != (int64_t) out->length){
                dtn_buffer_free(out);
                goto error;
            }

            if (!dtn_io_buffer_push(self->buffer, 
                    &remote,
                    out->start, out->length)) {
                dtn_buffer_free(out);
                goto error;
            }
            goto done;
            break;

        case DTN_CBOR_MATCH_FULL:

            DTN_ASSERT(bundle);
            DTN_ASSERT(next);

            if (next - out->start < (int64_t) out->length)
                dtn_io_buffer_push(
                        self->buffer, 
                        &remote,
                        next,
                        (out->length) - (next - out->start));

            out = dtn_buffer_free(out);

            break;
    }

    DTN_ASSERT(!out);

    while(bundle){

        process_bundle(self, &remote, bundle);
        bundle = NULL;
        
        out = dtn_io_buffer_pop(self->buffer, &remote);
        if (!out) break;

        match = dtn_bundle_decode(
            out->start, out->length, &bundle, &next);

        switch (match){

            case DTN_CBOR_NO_MATCH:

                out = dtn_buffer_free(out);
                goto error;
                break;
    
            case DTN_CBOR_MATCH_PARTIAL:

                DTN_ASSERT(next);

                if (next - out->start + 1 != (int64_t) out->length){
                    dtn_buffer_free(out);
                    goto error;
                }

                if (!dtn_io_buffer_push(self->buffer, 
                    &remote,
                    out->start, out->length)) {
                    dtn_buffer_free(out);
                    goto error;
                }
                goto done;
                break;
    
            case DTN_CBOR_MATCH_FULL:

                DTN_ASSERT(bundle);
                DTN_ASSERT(next);
    
                if (next - out->start < (int64_t) out->length)
                    dtn_io_buffer_push(
                            self->buffer, 
                            &remote,
                            next,
                            (out->length) - (next - out->start));
    
                out = dtn_buffer_free(out);
    
                break;
        }
    }
done:
    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

static bool init_config(dtn_interface_ip_config *config){

    if (!config->loop) goto error;
    if (0 == config->socket.host[0]) goto error;

    if (0 == config->limits.link_check)
        config->limits.link_check = 1000000; // every second

    if (0 == config->limits.threadlock_timeout_usecs)
        config->limits.threadlock_timeout_usecs = 100000;

    return true;
error:
    return false;
}

/*---------------------------------------------------------------------------*/

dtn_interface_ip *dtn_interface_ip_create(dtn_interface_ip_config config){

    dtn_interface_ip *self = NULL;

    if (!init_config(&config)) goto error;

    self = calloc(1, sizeof(dtn_interface_ip));
    if (!self) goto error;

    self->magic_byte = DTN_INTERFACE_IP_MAGIC_BYTE;
    self->config = config;

    self->socket = dtn_socket_create(self->config.socket, false, NULL);
    if (self->socket < 1){
        
        dtn_log_error("failed to create interface %s:%i",
            config.socket.host, config.socket.port);
        goto error;
    
    }

    if (!dtn_socket_ensure_nonblocking(self->socket)) goto error;
    if (!dtn_socket_get_data(self->socket, &self->local, NULL)) goto error;

    if (!dtn_event_loop_set(
        config.loop,
        self->socket,
        DTN_EVENT_IO_IN | DTN_EVENT_IO_ERR | DTN_EVENT_IO_CLOSE,
        self, 
        cb_io)) goto error;

    if (!start_link_check(self)) goto error;

    self->buffer = dtn_io_buffer_create((dtn_io_buffer_config){0});
    if (!self->buffer) goto error;

    if (!dtn_thread_lock_init(&self->out.lock, 
        self->config.limits.threadlock_timeout_usecs)) goto error;

    self->out.queue = dtn_list_create((dtn_list_config){
        .item.free = out_data_free
    });
    
    if (!self->out.queue) goto error;

    dtn_log_info("IP interface %s:%i activated",
        self->config.socket.host, self->config.socket.port);

    return self;
error:
    dtn_interface_ip_free(self);
    return NULL;
}

/*------------------------------------------------------------------*/

void *dtn_interface_ip_free(void *data){

    dtn_interface_ip *self = dtn_interface_ip_cast(data);
    if (!self) return data;

    if (self->socket > 0) close(self->socket);

    self->buffer = dtn_io_buffer_free(self->buffer);

    if (DTN_TIMER_INVALID != self->timer.link_check){
        dtn_event_loop_timer_unset(self->config.loop, 
            self->timer.link_check, NULL);
    }

    close(self->socket);
    self = dtn_data_pointer_free(self);
    return NULL;
}

/*------------------------------------------------------------------*/

dtn_interface_ip *dtn_interface_ip_cast(const void *data){

    if (!data) return NULL;

    if (*(uint16_t *)data != DTN_INTERFACE_IP_MAGIC_BYTE)
        return NULL;

    return (dtn_interface_ip *)data;
}

/*------------------------------------------------------------------*/

const char *dtn_interface_ip_name(const dtn_interface_ip *self){

    if (!self) return NULL;
    return self->config.socket.host;
}

/*------------------------------------------------------------------*/

bool dtn_interface_ip_send(dtn_interface_ip *self,
    dtn_socket_configuration remote,
    const uint8_t *buffer,
    size_t size){

    struct out_data *data = NULL;

    if (!self || !buffer || size < 1) goto error;

    data = calloc(1, sizeof(struct out_data));
    if (!data) goto error;

    data->remote = remote;
    data->buffer = dtn_buffer_create(size);
    dtn_buffer_push(data->buffer, (uint8_t*)buffer, size);

    if (!dtn_thread_lock_try_lock(&self->out.lock)) goto error;
    if (!dtn_list_queue_push(self->out.queue, data)) goto error;
    if (!dtn_thread_lock_unlock(&self->out.lock)){
        dtn_log_error("failed to unlock out queue");
    }

    if (DTN_IP_LINK_UP == self->link)
        start_sending_queue(self);

    return true;
error:
    return false;
}