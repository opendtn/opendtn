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
#       This file will be included by a module's makefile.
#
#       ------------------------------------------------------------------------

# DTN_OBJ              = $(patsubst %.c,%.o,$(DTN_SRC))
DTN_OBJ_1       		= $(notdir $(DTN_SRC))
DTN_OBJ_2       		= $(addprefix $(DTN_OBJDIR)/$(DTN_DIRNAME)/,$(DTN_OBJ_1))
DTN_OBJ         		= $(patsubst %.c,%.o,$(DTN_OBJ_2))
DTN_DEP         		= $(patsubst %.c,%.d,$(DTN_SRC))
DTN_OBJ_EXEC    		= $(addprefix $(DTN_OBJDIR)/$(DTN_DIRNAME)/,$(patsubst %,%.o,$(notdir $(DTN_EXECUTABLE))))
DTN_OBJ_TEST    		= $(addprefix $(DTN_OBJDIR)/$(DTN_DIRNAME)/,$(patsubst %.c,%.o,$(notdir $(DTN_TEST_SOURCES))))
DTN_OBJ_IF_TEST 		= $(addprefix $(DTN_OBJDIR)/$(DTN_DIRNAME)/,$(patsubst %.c,%.o,$(notdir $(DTN_TEST_IF_SRC))))

DTN_STATIC				= $(DTN_LIBDIR)/$(DTN_LIBNAME)$(DTN_EDITION).a

DTN_SHARED_LINKER_NAME	= $(DTN_LIBNAME)$(DTN_EDITION).so
DTN_SHARED_SONAME     	= $(DTN_SHARED_LINKER_NAME).$(DTN_VERSION_MAJOR)
DTN_SHARED_REAL       	= $(DTN_SHARED_LINKER_NAME).$(DTN_VERSION)
DTN_SHARED            	= $(DTN_LIBDIR)/$(DTN_SHARED_REAL) $(DTN_LIBDIR)/$(DTN_SHARED_LINKER_NAME) $(DTN_LIBDIR)/$(DTN_SHARED_SONAME)

DTN_TEST_RESOURCE        = $(wildcard resources/test/*)
DTN_TEST_RESOURCE_TARGET = $(addprefix $(DTN_TEST_RESOURCE_DIR)/,$(notdir $(DTN_TEST_RESOURCE)))

L_SRCDIRS      		:= $(shell find . -name '*.c' -exec dirname {} \; | uniq)
VPATH          		:= $(L_SRCDIRS)

all                 : target_prepare target_build_all target_install
target_build_all    : target_sources
target_sources      : $(DTN_TARGET)
depend              : target_depend
target_install      : target_install_header
target_test         : $(DTN_TEST_RESOURCE) $(DTN_TEST_RESOURCE_TARGET) $(DTN_TEST_TARGET) $(DTN_OBJ_IF_TEST)  $(DTN_OBJ_TEST)
test 				: all target_test
install 			: target_system_install
uninstall           : target_system_uninstall

#-----------------------------------------------------------------------------
# Targets
#-----------------------------------------------------------------------------

$(DTN_OBJDIR)/$(DTN_DIRNAME)/%.o : %.c
	@echo "[CC     ] $<"
	$(DTN_QUIET)$(CC) $(BUILD_DEFINITIONS) $(TEST_DEFINITIONS)\
	$(CFLAGS) $(DTN_FLAGS) -MMD -c $< -o $@

$(DTN_OBJDIR)/$(DTN_DIRNAME)/%_test.o : %_test.c $(DTN_OBJ_IF_TEST)
	@echo "[CC     ] $<"
	$(DTN_QUIET)$(CC) \
		$(BUILD_DEFINITIONS) \
		$(TEST_DEFINITIONS) \
		$(CFLAGS) $(DTN_FLAGS) -MMD -c $< -o $@

$(DTN_OBJDIR)/$(DTN_DIRNAME)/%_test_interface.o : %_test_interface.c $(DTN_TEST_IF_SRC)
	@echo "[CC     ] $<"
	$(DTN_QUIET)$(CC) $(TEST_DEFINITIONS) \
	 $(CFLAGS) $(DTN_FLAGS) -MMD -c $< -o $@

$(DTN_TESTDIR)/$(DTN_DIRNAME)/%_test.run : $(DTN_OBJDIR)/$(DTN_DIRNAME)/%_test.o $(DTN_OBJ_IF_TEST) $(DTN_OBJ)
	$(eval NO_SELF_DEPENDENCY := $(filter-out $(<:%_test.o=%.o) , $(DTN_OBJ)))
	$(eval NO_SELF_DEPENDENCY := $(filter-out $(DTN_OBJ_EXEC) , $(NO_SELF_DEPENDENCY)))
	$(DTN_QUIET)$(CC) -o $@  $< $(NO_SELF_DEPENDENCY) $(LFLAGS) $(DTN_LIBS)
	@echo "[TEST EXEC] $(notdir $@ ) created"

# ... will create the directory for the resources and copy resources
$(DTN_TEST_RESOURCE_TARGET): target_test_resource_prepare $(DTN_TEST_RESOURCE)
	$(DTN_QUIET) $(shell cp -r $(DTN_TEST_RESOURCE) $(DTN_TEST_RESOURCE_DIR)/)

#-----------------------------------------------------------------------------

-include $(DTN_OBJDIR)/$(DTN_DIRNAME)/*.d

#-----------------------------------------------------------------------------
$(DTN_STATIC):  $(DTN_OBJ)
	$(DTN_QUIET)ar rcs $(DTN_STATIC) $(DTN_OBJ)
	$(DTN_QUIET)ranlib $(DTN_STATIC)
	@echo "[STATIC ] $(notdir $(DTN_STATIC)) created"

#-----------------------------------------------------------------------------

$(DTN_LIBDIR)/$(DTN_SHARED_REAL): $(DTN_OBJ)
ifeq ($(DTN_UNAME), Linux)
	$(DTN_QUIET)$(CC) -shared -o $@ $(DTN_OBJ) $(LFLAGS) \
		-Wl,-soname,$(DTN_SHARED_SONAME) \
		-Wl,--defsym -Wl,__DTN_LD_VERSION=0x$(DTN_VERSION_HEX) \
		-Wl,--defsym -Wl,__DTN_LD_EDITION=0x$(DTN_EDITION) \
		$(DTN_LIBS) 
else ifeq ($(DTN_UNAME), Darwin)
	$(DTN_QUIET)$(CC) -shared -o $@ $(DTN_OBJ) $(LFLAGS) \
		-compatibility_version $(DTN_VERSION) \
		-current_version $(DTN_VERSION) \
		$(DTN_LIBS) 
else
	@echo "[SHARED ] OS $(DTN_UNAME) unsupported yet."
endif
ifeq ($(DTN_BUILD_MODE), STRIP)
	$(DTN_QUIET)$(DTN_STRIP) $@
endif
	@echo "[SHARED ] $(notdir $@) created"

#-----------------------------------------------------------------------------

$(DTN_LIBDIR)/$(DTN_SHARED_LINKER_NAME): $(DTN_LIBDIR)/$(DTN_SHARED_REAL)
	$(DTN_QUIET)$(shell \
		cd $(DTN_LIBDIR) ; \
		$(DTN_SYMLINK) $(DTN_SHARED_REAL) $(DTN_SHARED_LINKER_NAME); )
	@echo "[LINK   ] $(notdir $@) created"

#-----------------------------------------------------------------------------

$(DTN_LIBDIR)/$(DTN_SHARED_SONAME): $(DTN_LIBDIR)/$(DTN_SHARED_REAL)
	$(DTN_QUIET)$(shell \
		cd $(DTN_LIBDIR) ; \
		$(DTN_SYMLINK) $(DTN_SHARED_REAL) $(DTN_SHARED_SONAME);)
	@echo "[LINK   ] $(notdir $@) created"

#-----------------------------------------------------------------------------

$(DTN_EXECUTABLE): $(DTN_OBJ)
	$(DTN_QUIET)$(CC) -o $(DTN_EXECUTABLE) $(DTN_OBJ) $(LFLAGS) $(DTN_LIBS) 
	@echo "[EXEC   ] $(notdir $(DTN_EXECUTABLE)) created"

#-----------------------------------------------------------------------------

target_test_resource_prepare:
	$(DTN_QUIET)$(DTN_MKDIR) $(DTN_TEST_RESOURCE_DIR) $(DTN_NUL_STDERR)

#-----------------------------------------------------------------------------
target_depend:
	@echo "[DEPEND ] generated automagically, skipped"
#	@echo "[DEPEND ] CURDIR         : $(CURDIR)"
#	@echo "[DEPEND ] DTN_BUILDDIR    : $(DTN_BUILDDIR)"
#	@echo "[DEPEND ] DTN_DIRNAME     : $(DTN_DIRNAME)"
#

#-----------------------------------------------------------------------------
target_install_header:
	$(DTN_QUIET)l_target=$(DTN_BUILDDIR)/include/$(DTN_DIRNAME); \
	$(DTN_MKDIR) $$l_target; \
	for f in $(DTN_HDR); do \
	  l_base=`basename $$f`; \
	  test -L  $$l_target/$$l_base || \
	  $(DTN_SYMLINK) $(abspath $$f) $$l_target/$$l_base; \
	done; \
	echo "[INSTALL] symlinks created"

#-----------------------------------------------------------------------------

target_system_install: $(DTN_EXECUTABLE)
	$(shell sudo cp $(DTN_EXECUTABLE) $(DTN_INSTALLDIR)/$(notdir $(DTN_EXECUTABLE)))
	@echo "[INSTALLED AS] $(DTN_INSTALLDIR)/$(notdir $(DTN_EXECUTABLE))"

#-----------------------------------------------------------------------------

target_system_uninstall:
	$(shell sudo rm $(DTN_INSTALLDIR)/$(notdir $(DTN_EXECUTABLE)))
	@echo "[UNINSTALLED] $(DTN_INSTALLDIR)/$(notdir $(DTN_EXECUTABLE))"

#-----------------------------------------------------------------------------
target_prepare:
	$(DTN_QUIET)$(DTN_MKDIR) $(DTN_OBJDIR)               $(DTN_NUL_STDERR)
	$(DTN_QUIET)$(DTN_MKDIR) $(DTN_OBJDIR)/$(DTN_DIRNAME) $(DTN_NUL_STDERR)
	$(DTN_QUIET)$(DTN_MKDIR) $(DTN_TESTDIR)/$(DTN_DIRNAME) $(DTN_NUL_STDERR)
	$(DTN_QUIET)$(DTN_MKDIR) $(DTN_LIBDIR)               $(DTN_NUL_STDERR)
	$(DTN_QUIET)$(DTN_MKDIR) $(DTN_BINDIR)               $(DTN_NUL_STDERR)
	