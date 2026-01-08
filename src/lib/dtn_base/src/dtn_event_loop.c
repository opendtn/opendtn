/***
        ------------------------------------------------------------------------

        Copyright (c) 2018 German Aerospace Center DLR e.V. (GSOC)

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
        @file           dtn_event_loop.c
        @author         Markus Toepfer
        @author         Michael Beer

        @date           2018-12-17

        @ingroup        dtn_event_loop

        @brief          Implementation of dtn_event_loop common functions.


        ------------------------------------------------------------------------
*/

#include "dtn_event_loop.h"

#include "../include/dtn_utils.h"
#include <signal.h>
#include <sys/resource.h>
#include <sys/time.h>

#include "../include/dtn_constants.h"

// include default implementation
#include "../include/dtn_event_loop_poll.h"

#define IMPL_RECONNECT_MAGIC_BYTES 0xbbaa

/*----------------------------------------------------------------------------*/

static dtn_event_loop *g_event_loop = 0;

/*----------------------------------------------------------------------------*/

static bool register_loop(dtn_event_loop *loop) {

    if (0 == loop) {
        dtn_log_error("Got 0 pointer");
        goto error;
    }

    if (0 != g_event_loop) {
        dtn_log_critical("Another event loop already there");
        goto error;
    }

    g_event_loop = loop;

    return true;

error:

    return false;
}

/*----------------------------------------------------------------------------*/

bool unregister_event_loop(dtn_event_loop *loop) {

    if ((NULL != g_event_loop) && g_event_loop != loop) {

        dtn_log_error("Not unregistering loop - another loop was registered");
        goto error;
    }

    g_event_loop = 0;

    return true;

error:

    return false;
}

/*----------------------------------------------------------------------------*/

static void event_loop_stop_sighandler(int signum) {

    UNUSED(signum);

    if (0 == g_event_loop)
        return;

    g_event_loop->stop(g_event_loop);
}

/*----------------------------------------------------------------------------*/

bool dtn_event_loop_run(dtn_event_loop *self, uint64_t max_runtime_usec) {

    if (0 == self) {
        return false;
    } else {
        return self->run(self, max_runtime_usec);
    }
}

/*----------------------------------------------------------------------------*/

bool dtn_event_loop_stop(dtn_event_loop *self) {

    if (0 == self) {
        return false;
    } else {

        DTN_ASSERT(0 != self->stop);
        return self->stop(self);
    }
}

/*---------------------------------------------------------------------------*/

dtn_event_loop *dtn_event_loop_cast(const void *self) {

    if (!self)
        return NULL;

    const dtn_event_loop *loop = self;
    if (loop->magic_byte != DTN_EVENT_LOOP_MAGIC_BYTE)
        return NULL;

    return (dtn_event_loop *)loop;
}

/*----------------------------------------------------------------------------*/

bool dtn_event_loop_setup_signals(dtn_event_loop *loop) {

    if (!register_loop(loop)) {
        return false;
    }

    struct sigaction ignore = {
        .sa_handler = SIG_IGN,
        .sa_flags = SA_NOCLDSTOP | SA_NOCLDWAIT,

    };

    sigaction(SIGPIPE, &ignore, 0);

    struct sigaction stop_event_loop = {
        .sa_handler = event_loop_stop_sighandler,
    };

    sigaction(SIGINT, &stop_event_loop, 0);

    return true;
}

/*---------------------------------------------------------------------------*/

bool dtn_event_loop_set_type(dtn_event_loop *self, uint16_t type) {

    if (!self)
        return false;

    self->magic_byte = DTN_EVENT_LOOP_MAGIC_BYTE;
    self->type = type;

    return true;
}

/*---------------------------------------------------------------------------*/

dtn_event_loop_config dtn_event_loop_config_default() {

    dtn_event_loop_config config = {

        .max.sockets = DTN_EVENT_LOOP_MAX_SOCKETS_DEFAULT,
        .max.timers = DTN_EVENT_LOOP_MAX_TIMERS_DEFAULT};

    return config;
}

/*---------------------------------------------------------------------------*/

void *dtn_event_loop_free(void *eventloop) {

    dtn_event_loop *loop = dtn_event_loop_cast(eventloop);

    if (!loop) {
        return eventloop;
    }

    if (!loop->free) {
        dtn_log_error("eventloop free not found");
        return eventloop;
    }

    unregister_event_loop(loop);

    return loop->free(loop);
}

/*----------------------------------------------------------------------------*/

uint32_t dtn_event_loop_timer_set(dtn_event_loop *self, uint64_t relative_usec,
                                  void *data,
                                  bool (*callback)(uint32_t id, void *data)) {

    if (0 == self) {
        return DTN_TIMER_INVALID;
    } else {
        return self->timer.set(self, relative_usec, data, callback);
    }
}

/*----------------------------------------------------------------------------*/

bool dtn_event_loop_timer_unset(dtn_event_loop *self, uint32_t id,
                                void **userdata) {

    if (0 == self) {
        return false;
    } else {
        return self->timer.unset(self, id, userdata);
    }
}

/*----------------------------------------------------------------------------*/

bool dtn_event_loop_set(dtn_event_loop *self, int sfd, uint8_t events,
                        void *userdata,
                        bool (*callback)(int socket_fd, uint8_t events,
                                         void *userdata)) {

    if (dtn_cond_valid(-1 < sfd,
                       "Cannot set callback on socket fd: Invalid fd") &&
        dtn_ptr_valid(self,
                      "Cannot set callback on event loop: Invalid loop")) {

        return self->callback.set(self, sfd, events, userdata, callback);

    } else {

        return false;
    }
}

/*----------------------------------------------------------------------------*/

bool dtn_event_loop_unset(dtn_event_loop *self, int socket, void **userdata) {

    if (dtn_ptr_valid(self,
                      "Cannot remove callback on event loop: Invalid loop")) {

        return self->callback.unset(self, socket, userdata);

    } else {

        return false;
    }
}

/*---------------------------------------------------------------------------*/

dtn_event_loop *dtn_event_loop_default(dtn_event_loop_config config) {

    return dtn_event_loop_poll(config);
}

/*---------------------------------------------------------------------------*/

dtn_event_loop_config
dtn_event_loop_config_adapt_to_runtime(dtn_event_loop_config config) {

    // Extend the config to DEFAULT in case of 0 to MIN
    if (config.max.sockets == 0)
        config.max.sockets = DTN_EVENT_LOOP_SOCKETS_MIN;

    if (config.max.timers == 0)
        config.max.timers = DTN_EVENT_LOOP_TIMERS_MIN;

    struct rlimit limit = {0};

    // Limit config to system MAX

    if (0 != getrlimit(RLIMIT_NOFILE, &limit)) {
        dtn_log_error("Failed to get system limit"
                      " of open files errno %i|%s",
                      errno, strerror(errno));
    }

    if (limit.rlim_cur != RLIM_INFINITY) {

        if (config.max.sockets > limit.rlim_cur) {
            config.max.sockets = limit.rlim_cur;
            dtn_log_notice("Limited max sockets to system limit "
                           "of files open %" PRIu32,
                           config.max.sockets);
        }
    }

    if (limit.rlim_cur != RLIM_INFINITY) {

        if (config.max.timers > limit.rlim_cur) {
            config.max.timers = limit.rlim_cur;
            dtn_log_notice("Limited max timers to system limit "
                           "of signals pending %" PRIu32,
                           config.max.timers);
        }
    }

    /*

    TBD check for TIMER_MAX limit

    // Limit to max timers
    if (config.max.timers > TIMER_MAX){
            config.max.timers = TIMER_MAX;
                    dtn_log_notice(  "Limited max timers to system limit "
                                    "of TIMER_MAX %jd",
                            config.max.timers);
    }
    */
    return config;
}

/*---------------------------------------------------------------------------*/

struct accept_container {

    dtn_event_loop *loop;
    uint8_t events;
    void *data;
    bool (*callback)(int socket, uint8_t events, void *data);
};

/*---------------------------------------------------------------------------*/

static void *accept_container_free(void *data) {

    if (data)
        free(data);
    return NULL;
}

/*---------------------------------------------------------------------------*/

static bool accept_callback(int socket_fd, uint8_t events, void *data) {

    int nfd = 0;

    struct accept_container *container = (struct accept_container *)data;
    dtn_event_loop *loop = container->loop;

    if (!loop)
        goto error;

    if (socket_fd < 1)
        goto error;

    if ((events & DTN_EVENT_IO_CLOSE) || (events & DTN_EVENT_IO_ERR)) {

        void *userdata = NULL;

        if (!loop->callback.unset(loop, socket_fd, &userdata)) {
            dtn_log_error("Failed to unset accept callback");
            goto error;
        }

        accept_container_free(userdata);

        return true;
    }

    // accept MUST have some incoming IO
    if (!(events & DTN_EVENT_IO_IN))
        goto error;

    struct sockaddr_storage remote_sa = {0};
    struct sockaddr_storage local_sa = {0};
    // struct sockaddr_un un_sa;

    socklen_t remote_sa_len = sizeof(remote_sa);

    char local_ip[DTN_HOST_NAME_MAX] = {0};
    char remote_ip[DTN_HOST_NAME_MAX] = {0};

    uint16_t local_port = 0;
    uint16_t remote_port = 0;

    // get local SA
    if (!dtn_socket_get_sockaddr_storage(socket_fd, &local_sa, NULL, NULL))
        goto error;

    nfd = accept(socket_fd, (struct sockaddr *)&remote_sa, &remote_sa_len);

    // parse debug logging data
    switch (local_sa.ss_family) {

    case AF_INET:
    case AF_INET6:

        if (!dtn_socket_parse_sockaddr_storage(
                &local_sa, local_ip, DTN_HOST_NAME_MAX, &local_port)) {
            dtn_log_error("Failed to parse data "
                          "from socket fd %i",
                          socket_fd);
            goto error;
        }

        if (nfd < 0)
            break;

        if (!dtn_socket_parse_sockaddr_storage(
                &remote_sa, remote_ip, DTN_HOST_NAME_MAX, &remote_port)) {
            dtn_log_error("Failed to parse data "
                          "from socket fd %i",
                          nfd);
            goto error;
        }

        break;

    case AF_UNIX:

        break;

    default:
        dtn_log_error("Family %i not supported", local_sa.ss_family);
        goto error;
    }

    if (nfd < 0) {

        dtn_log_error("Failed to accept at socket %i", socket_fd);
        goto error;
    }

    if (!dtn_socket_ensure_nonblocking(nfd))
        goto error;

    if (local_ip[0] != 0) {

        dtn_log_debug("accepted at socket fd %i | "
                      "LOCAL %s:%i REMOTE %s:%i | "
                      "new connection fd %i",
                      socket_fd, local_ip, local_port, remote_ip, remote_port,
                      nfd);

    } else {

        dtn_log_debug("accepted at socket fd %i | "
                      "new connection fd %i",
                      socket_fd, nfd);
    }

    if (!loop->callback.set(loop, nfd, container->events, container->data,
                            container->callback))
        goto error;

    return true;

error:
    if (nfd > -1)
        close(nfd);
    return false;
}

/*---------------------------------------------------------------------------*/

bool dtn_event_add_default_connection_accept(
    dtn_event_loop *loop, int socket, uint8_t events, void *data,
    bool (*callback)(int connection_socket, uint8_t connection_events,
                     void *data)) {

    struct accept_container *container = NULL;

    if (!loop || !callback || (events == 0))
        goto error;

    int so_opt;
    socklen_t so_len = sizeof(so_opt);

    // check if socket is without error
    if (0 != getsockopt(socket, SOL_SOCKET, SO_ERROR, &so_opt, &so_len))
        goto error;

    // check if socket is streaming socket
    if (0 != getsockopt(socket, SOL_SOCKET, SO_TYPE, &so_opt, &so_len))
        goto error;

    if (so_opt != SOCK_STREAM) {
        dtn_log_error("accept applied to non streaming socket!");
        goto error;
    }

    container = calloc(1, sizeof(struct accept_container));

    container->loop = loop;
    container->events = events;
    container->data = data;
    container->callback = callback;

    if (!loop->callback.set(loop, socket,
                            DTN_EVENT_IO_IN | DTN_EVENT_IO_CLOSE |
                                DTN_EVENT_IO_ERR,
                            container, accept_callback))
        goto error;

    return true;
error:
    if (container)
        free(container);

    return false;
}

/*---------------------------------------------------------------------------*/

bool dtn_event_remove_default_connection_accept(dtn_event_loop *self, int s) {

    if (!self)
        return false;

    void *userdata = NULL;

    bool result = self->callback.unset(self, s, &userdata);

    if (userdata)
        accept_container_free(userdata);
    return result;
}

/*---------------------------------------------------------------------------*/

dtn_event_loop_config dtn_event_loop_config_from_json(const dtn_item *value) {

    dtn_event_loop_config config = {0};

    if (!value)
        goto error;

    const dtn_item *obj = dtn_item_object_get(value, DTN_EVENT_LOOP_KEY);
    if (!obj)
        obj = value;

    double sockets = dtn_item_get_number(
        dtn_item_object_get(obj, DTN_EVENT_LOOP_KEY_MAX_SOCKETS));

    double timers = dtn_item_get_number(
        dtn_item_object_get(obj, DTN_EVENT_LOOP_KEY_MAX_TIMERS));

    if (sockets > UINT32_MAX)
        goto error;

    if (timers > UINT32_MAX)
        goto error;

    config.max.sockets = (uint32_t)sockets;
    config.max.timers = (uint32_t)timers;

    return config;
error:
    return (dtn_event_loop_config){0};
}

/*---------------------------------------------------------------------------*/

dtn_item *dtn_event_loop_config_to_json(dtn_event_loop_config config) {

    dtn_item *val = NULL;
    dtn_item *out = dtn_item_object();
    if (!out)
        goto error;

    val = dtn_item_number(config.max.timers);
    if (!dtn_item_object_set(out, DTN_EVENT_LOOP_KEY_MAX_TIMERS, val))
        goto error;

    val = dtn_item_number(config.max.sockets);
    if (!dtn_item_object_set(out, DTN_EVENT_LOOP_KEY_MAX_SOCKETS, val))
        goto error;

    return out;
error:
    dtn_item_free(out);
    dtn_item_free(val);
    return NULL;
}
