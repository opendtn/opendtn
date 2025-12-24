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
#       File            makefile
#
#       ------------------------------------------------------------------------

include $(DTN_ROOT)/makefiles/makefile_const.mk

DTN_LIB_DIRS      = dtn_base
DTN_LIB_DIRS      += dtn_core
DTN_LIB_DIRS      += dtn
DTN_LIB_DIRS      += dtn_nodes

all    : target_build_all
depend : target_depend

#-----------------------------------------------------------------------------
target_depend:
	@echo "[DEPEND ] src/lib: scanning dependencies"
	@for dir in $(DTN_LIB_DIRS); do \
		echo "[DEPEND ] entering $$dir ..."; \
		$(MAKE) depend -C $$dir || exit 1; \
		echo "[DEPEND ] leaving $$dir ."; \
	done
	@echo "[DEPEND ] src/lib: dependencies done"
#-----------------------------------------------------------------------------
target_build_all:
	@for dir in $(DTN_LIB_DIRS); do \
		echo "[BUILD  ] entering $$dir ..."; \
		$(MAKE) -C $$dir || exit 1; \
		echo "[BUILD  ] leaving $$dir ."; \
	done
	@echo "[BUILD  ] build done"
#-----------------------------------------------------------------------------
target_test:
	@echo "[BUILD  ] building tests"
	@for dir in $(DTN_LIB_DIRS); do \
		echo "[BUILD  ] entering $$dir ..."; \
		$(MAKE) target_test -C $$dir || exit 1; \
		echo "[BUILD  ] leaving $$dir ."; \
	done
	@echo "[BUILD  ] build tests done"
#-----------------------------------------------------------------------------


