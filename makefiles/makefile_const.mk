# -*- Makefile -*-
#       ------------------------------------------------------------------------
#
#       Copyright 2020 German Aerospace Center DLR e.V. (GSOC)
#
#       Licensed under the Apache License, Version 2.0 (the "License");
#       you may not use this file except in compliance with the License.
#       You may obtain a copy of the License at
#
#               http://www.apache.org/licenses/LICENSE-2.0
#
#       Unless required by applicable law or agreed to in writing, software
#       distributed under the License is distributed on an "AS IS" BASIS,
#       WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#       See the License for the specific language governing permissions and
#       limitations under the License.
#
#       This file is part of the openvocs project. http://openvocs.org
#
#       ------------------------------------------------------------------------
#
#       Authors         Udo Haering, Michael J. Beer, Markus Töpfer
#       Date            2020-01-21
#
#       ------------------------------------------------------------------------

include $(DTN_ROOT)/makefiles/makefile_version.mk

# generic source files
G_HEADERS           := $(wildcard include/*.h)
G_SOURCES_C         := $(wildcard src/*.c)
G_TEST_SOURCES      := $(wildcard src/*_test.c)
G_TEST_IF_SRC       := $(wildcard src/*_test_interface.c)
G_SOURCES           := $(filter-out $(G_TEST_SOURCES), $(G_SOURCES_C))

# const variables to use (generic may be overriden using L_* )
DTN_HDR             := $(if $(L_HEADERS),$(L_HEADERS),$(G_HEADERS))
DTN_SRC             := $(if $(L_SOURCES),$(L_SOURCES),$(G_SOURCES))
DTN_TEST_SOURCES    := $(if $(L_TEST_SOURCES),$(L_TEST_SOURCES),$(G_TEST_SOURCES))
DTN_TEST_IF_SRC     := $(if $(L_TEST_IF_SRC),$(L_TEST_IF_SRC),$(G_TEST_IF_SRC))

# shell commands
DTN_MKDIR           := mkdir -p
DTN_RM              := rm -f
DTN_RMDIR           := rm -rf
DTN_SYMLINK         := ln -s
DTN_COPY            := cp -r

# redirection (suppressing printouts from command line tools))
DTN_NUL             := /dev/null
DTN_NUL_STDERR      := 2>$(DTN_NUL) || true
DTN_NUL_STDOUT      := 1>$(DTN_NUL) || true
DTN_DEV_NULL        := $(DTN_NUL_STDOUT) $(DTN_NUL_STDERR)

# make commands quiet
DTN_QUIET           ?= @
#DTN_QUIET            =

# prevent make from displaying 'Entering/Leaving directory ...'
DTN_QUIET_MAKE      ?= --no-print-directory

# directories
DTN_SOURCEDIR       := $(DTN_ROOT)/src
DTN_BUILDDIR        := $(DTN_ROOT)/build
DTN_BUILDDIR        := $(abspath $(DTN_BUILDDIR))
DTN_OBJDIR          := $(DTN_BUILDDIR)/obj
DTN_LIBDIR          := $(DTN_BUILDDIR)/lib
DTN_BINDIR          := $(DTN_BUILDDIR)/bin
DTN_TESTDIR         := $(DTN_BUILDDIR)/test
DTN_PLUGINDIR       := $(DTN_BUILDDIR)/plugins
DTN_INSTALLDIR      := /usr/local/bin
DTN_PLUGINS_INSTALLDIR   := /usr/lib/openvocs/plugins

OPENVOCS_BUILD_DIR  := $(DTN_ROOT)/openvocs/build
OPENVOCS_LIBDIR     := $(DTN_ROOT)/openvocs/build/lib

DTN_TEMPDIR         := /tmp

DTN_INSTALL_PREFIX  := /usr/local


# # dependencies
# # ==> to be changed !!
# DTN_DEPENDFILE      := .depend

# include paths
DTN_INC             := -Iinclude -I$(DTN_BUILDDIR) -I$(DTN_BUILDDIR)/include -I$(OPENVOCS_BUILD_DIR)/include

DTN_DIRNAME         := $(notdir $(CURDIR))
DTN_LIBNAME         := lib$(DTN_DIRNAME)

DTN_UNAME             := $(shell uname)

#.............................................................................
# distinguish libs and executables
#
# IMPORTANT:
# this _MUST_ be defined as a recursively expanded variable!!
DTN_TARGET           = $(DTN_STATIC) $(DTN_SHARED)
DTN_TEST_TARGET      = $(addprefix $(DTN_TESTDIR)/$(DTN_DIRNAME)/,$(patsubst %.c,%.run, $(notdir $(DTN_TEST_SOURCES))))
#.............................................................................
# test sources directory
DTN_TEST_RESOURCE_DIR := $(DTN_BUILDDIR)/test/$(DTN_DIRNAME)/resources/

#.............................................................................

BUILD_DEFINITIONS = -D DTN_ROOT='"$(DTN_ROOT)"' \
                    -D DTN_VERSION='"$(DTN_VERSION)"' \
                    -D DTN_VERSION_BUILD_ID='"$(DTN_VERSION_BUILD_ID)"' \
                    -D DTN_VERSION_COMMIT_ID='"$(DTN_VERSION_COMMIT_ID)"' \
                    -D DTN_VERSION_BUILD_DATE='"$(DTN_VERSION_BUILD_DATE)"' \
                    -D DTN_VERSION_COMPILER='"$(DTN_VERSION_COMPILER)"' \
                    -D DTN_PLUGINS_INSTALLDIR='"$(DTN_PLUGINS_INSTALLDIR)"' \

#.............................................................................

TEST_DEFINITIONS =  -D DTN_TEST_RESOURCE_DIR='"$(DTN_TEST_RESOURCE_DIR)"' \

#.............................................................................

# CLANG specifics
CFLAGS             += -fstrict-aliasing -Wno-trigraphs -O0

#.............................................................................
#
# these _must_ not be prefixed ...
# -Wno-missing-braces is disabled with GCC by default, but clang still uses it
#

CFLAGS             += -Werror -Wall -Wextra -fPIC $(DTN_INC)
CFLAGS             += -std=c11 -D _DEFAULT_SOURCE -D _POSIX_C_SOURCE=200809
CFLAGS             += -D _XOPEN_SOURCE=500
CFLAGS             += -DDEBUG -g
# To make clang accept C11 '{0}' initialisation ...
CFLAGS             +=  -Wno-missing-braces

ifeq ($(DTN_UNAME), Darwin)
    CFLAGS         +=  -D _DARWIN_C_SOURCE=__DARWIN_C_FULL
endif

# when clang is used write some compile_command.json per file
ifeq ($(CC), clang)
    CFLAGS         +=  -MJ $@.json
endif

# Debug flags - switch on on command line like
# DEBUG=1 make test -j 4

ifdef DEBUG

CFLAGS             += -DDEBUG
CFLAGS             += -g

endif

LFLAGS              = -rdynamic