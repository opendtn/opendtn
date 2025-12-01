#!/usr/bin/env bash

DTN_ROOT=$(pwd)
DTN_LIBS=${DTN_ROOT}/build/lib
LD_LIBRARY_PATH=$DTN_LIBS

OPENVOCS_ROOT=$(DTN_ROOT)/openvocs
OPENVOCS_LIBS=${OPENVOCS_ROOT}/build/lib
LD_LIBRARY_PATH=$OPENVOCS_LIBS $DTN_LIBS

# Switch on debug build

# DEBUG=1
DEBUG=

# Enable to show all commands triggered in make files in full length
# DTN_QUIET=
# export DTN_QUIET

# Compiler to use

CC=gcc
# CC=clang

# Switches for tests

DTN_VALGRIND=valgrind
DTN_TEST_SPARSE_OUT=

export DTN_ROOT
export DTN_LIBS
export OPENVOCS_ROOT
export OPENVOCS_LIBS
export LD_LIBRARY_PATH
export DEBUG
export CC

export DTN_TEST_SPARSE_OUT
export DTN_VALGRIND

# clang requires the sdk-path for compilation
if [[ "$OSTYPE" == "darwin"* ]]; then
    export SDKROOT="`xcrun --show-sdk-path`"
fi

