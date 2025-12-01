# -*- Makefile -*-
#       ------------------------------------------------------------------------
#
#       Copyright 2025 German Aerospace Center DLR e.V. (GSOC)
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
#       Authors         Markus Töpfer
#
#       ------------------------------------------------------------------------
#
#       Install lib 
#
#    

-include makefiles/makefile_const.mk

DTN_LIB_INSTALL_DIR      := /usr

DTN_LIB_SOURCE_ROOT 		:= $(DTN_ROOT)/src/lib
DTN_LIB_SOURCE_DIR 		:= $(sort $(dir $(wildcard $(DTN_LIB_SOURCE_ROOT)/*/)))
DTN_LIB_NAMES            := $(notdir $(patsubst %/,%,$(dir $(DTN_LIB_SOURCE_DIR))))
DTN_LIB_NAME_EDITION     := $(addsuffix $(DTN_EDITION), $(DTN_LIB_NAMES))

DTN_LIB_FILES_0=$(addsuffix $(DTN_EDITION).so, $(DTN_LIB_NAMES))
DTN_LIB_FILES_1=$(addsuffix $(DTN_EDITION).so.$(DTN_VERSION_MAJOR), $(DTN_LIB_NAMES))
DTN_LIB_FILES_2=$(addsuffix $(DTN_EDITION).so.$(DTN_VERSION_MAJOR).$(DTN_VERSION_MINOR).$(DTN_VERSION_PATCH), $(DTN_LIB_NAMES))
DTN_LIB_FILES_ABSPATH=$(addprefix $(DTN_LIB_INSTALL_DIR)/lib/lib, $(DTN_LIB_FILES_0) $(DTN_LIB_FILES_1) $(DTN_LIB_FILES_2))

target_install_lib_header : 
	$(DTN_QUIET) sudo $(DTN_MKDIR) $(DTN_LIB_INSTALL_DIR)/include/openvocs
	$(DTN_QUIET)for item in $(DTN_LIB_NAMES) ; do \
        sudo $(DTN_MKDIR) $(DTN_LIB_INSTALL_DIR)/include/openvocs/$$item"$(DTN_EDITION)"; \
        sudo $(DTN_COPY) $(DTN_LIB_SOURCE_ROOT)/$$item/include/*.h $(DTN_LIB_INSTALL_DIR)/include/openvocs/$$item"$(DTN_EDITION)"/. ;\
    done

target_install_lib_sources : $(LIB_FILES_ABSPATH)
	$(DTN_QUIET) echo "Creating $?"
	$(DTN_QUIET) sudo $(DTN_MKDIR) $(DTN_LIB_INSTALL_DIR)/lib/openvocs
	for f in $?; do sudo $(DTN_COPY) $$f $(DTN_LIB_INSTALL_DIR)/lib/openvocs; done

target_install_lib_pkg_config :
	$(DTN_QUIET)for item in $(DTN_LIB_NAME_EDITION) ; do \
		sudo touch $(DTN_LIB_INSTALL_DIR)/lib/pkgconfig/lib$$item.pc ;\
		sudo chmod a+w $(DTN_LIB_INSTALL_DIR)/lib/pkgconfig/lib$$item.pc ;\
		sudo echo $$item.pc > $(DTN_LIB_INSTALL_DIR)/lib/pkgconfig/lib$$item.pc ;\
		sudo echo "prefix=$(DTN_LIB_INSTALL_DIR)" >> $(DTN_LIB_INSTALL_DIR)/lib/pkgconfig/lib$$item.pc ;\
		sudo echo "exec_prefix=$(DTN_LIB_INSTALL_DIR)" >> $(DTN_LIB_INSTALL_DIR)/lib/pkgconfig/lib$$item.pc ;\
		sudo echo "includedir=$(DTN_LIB_INSTALL_DIR)/include" >> $(DTN_LIB_INSTALL_DIR)/lib/pkgconfig/lib$$item.pc ;\
		sudo echo "libdir=$(DTN_LIB_INSTALL_DIR)/lib" >> $(DTN_LIB_INSTALL_DIR)/lib/pkgconfig/lib$$item.pc ;\
		sudo echo "" >> $(DTN_LIB_INSTALL_DIR)/lib/pkgconfig/lib$$item.pc ;\
		sudo echo "Name:lib$$item" >> $(DTN_LIB_INSTALL_DIR)/lib/pkgconfig/lib$$item.pc ;\
		sudo echo "Description: The $$item Library of openvocs" >> $(DTN_LIB_INSTALL_DIR)/lib/pkgconfig/lib$$item.pc ;\
		sudo echo "Version:$(DTN_VERSION_MAJOR).$(DTN_VERSION_MINOR).$(DTN_VERSION_PATCH)" >> $(DTN_LIB_INSTALL_DIR)/lib/pkgconfig/lib$$item.pc ;\
		sudo echo "Cflags: -I$(DTN_LIB_INSTALL_DIR)/include/openvocs/$$item" >> $(DTN_LIB_INSTALL_DIR)/lib/pkgconfig/lib$$item.pc ;\
		sudo echo "Libs: -L$(DTN_LIB_INSTALL_DIR)/lib/openvocs/ -llib$$item" >> $(DTN_LIB_INSTALL_DIR)/lib/pkgconfig/lib$$item.pc ;\
    done

.phony: target_install_lib
target_install_lib :  target_install_lib_header target_install_lib_sources target_install_lib_pkg_config