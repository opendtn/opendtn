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
        @file           dtn_event_loop_linux.h
        @author         Michael Beer

        @date           2019-09-04

        @ingroup        dtn_event_loop

        @see            dtn_event_loop

        Provides a linux - specific implementation of the dtn_event_loop.
        Facilitating native features of the linux kernel it ought to
        be less resource-hungry and more performant.

        However, at least memory wise, the default implementation is not far
        off, with the code being about 150% of the default implementation.

        Considerable effort had to be spent to render this loop conformant
        to the specification of an dtn_event_loop.

        It could have been substancially better be implemented if these
        requirements where somewhat relieved (e.g. having the loop close
        all open sockets automatically on exit, forcing an expensive
        book - keeping portion ...).

        As it is, it stands to reason whether the adaptions actually really pay
        off.

        BEWARE: Just as the default implementation, this event loop is

                NOT THREAD SAFE!

        ------------------------------------------------------------------------
*/
#if defined __linux__

#ifndef dtn_event_loop_linux_h
#define dtn_event_loop_linux_h

#include <dtn_base/dtn_event_loop.h>

dtn_event_loop *dtn_event_loop_linux(dtn_event_loop_config config);

#endif /* dtn_event_loop_h */

#endif /* linux */
