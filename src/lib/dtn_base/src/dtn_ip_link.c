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
        @file           dtn_ip_link.c
        @author         Töpfer, Markus

        @date           2025-12-19


        ------------------------------------------------------------------------
*/
#include "../include/dtn_ip_link.h"

#include <stddef.h>
#include "../include/dtn_socket.h"
#include "../include/dtn_string.h"
#include <sys/types.h>
#include <ifaddrs.h>


char *dtn_ip_link_get_interface_name(int socket){

    char *name = NULL;

    if (socket < 0) goto error;

    struct sockaddr_storage sa = {0};
    socklen_t addr_len = sizeof(sa);

    struct ifaddrs* ifaddr = NULL;
    struct ifaddrs* ifa = NULL;

    int opt = 0;
    socklen_t optlen = sizeof(opt);

    int r = getsockopt(socket, SOL_SOCKET, SO_TYPE, &opt, &optlen);
    if (r != 0) goto error;

    r = getsockname(socket, (struct sockaddr*)&sa, &addr_len);
    if (r != 0) goto error;

    r = getifaddrs(&ifaddr);
    if (r != 0) goto error;

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr)
        {
            if (opt == ifa->ifa_addr->sa_family){

                switch(opt){

                    case AF_INET:

                        struct sockaddr_in* inaddr = (struct sockaddr_in*)ifa->ifa_addr;
                        struct sockaddr_in* addr = (struct sockaddr_in*)&sa;
                        
                        if (inaddr->sin_addr.s_addr == addr->sin_addr.s_addr)
                        {
                            if (ifa->ifa_name)
                            {
                                // Found it
                                name = dtn_string_dup(ifa->ifa_name);
                            }
                        }
    
                        break;
    
                    case AF_INET6:

                        struct sockaddr_in6* inaddr6 = (struct sockaddr_in6*)ifa->ifa_addr;
                        struct sockaddr_in6* addr6 = (struct sockaddr_in6*)&sa;
                        
                        if (inaddr6->sin6_addr.s6_addr == addr6->sin6_addr.s6_addr)
                        {
                            if (ifa->ifa_name)
                            {
                                // Found it
                                name = dtn_string_dup(ifa->ifa_name);
                            }
                        }
    
                        break;
    
                    default:
                        break;
                }
            }
        }
    }

    freeifaddrs(ifaddr);

    return name;

error:
    return NULL;

}

/*------------------------------------------------------------------*/

dtn_ip_link_state dtn_ip_link_get_state(const char *name){

    dtn_ip_link_state result = DTN_IP_LINK_DOWN;

    if (!name) goto error;

    struct ifaddrs *curr, *list = NULL;

    if (getifaddrs(&list) == -1)
        goto error;

    for (curr = list; curr != NULL; curr = curr->ifa_next) {
        
        if (!dtn_string_compare(name, curr->ifa_name)) {
            if (curr->ifa_addr->sa_family == AF_INET)
                result = DTN_IP_LINK_UP;
            if (curr->ifa_addr->sa_family == AF_INET6)
                result = DTN_IP_LINK_UP;
        }
    }

    freeifaddrs(list);

    return result;
error:
    return DTN_IP_LINK_ERROR;
}
