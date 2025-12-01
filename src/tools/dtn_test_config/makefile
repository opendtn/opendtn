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
#       ------------------------------------------------------------------------

include $(DTN_ROOT)/makefiles/makefile_const.mk

DTN_EXECUTABLE        = $(DTN_BINDIR)/$(DTN_DIRNAME)
DTN_TARGET            = $(DTN_EXECUTABLE)

#-----------------------------------------------------------------------------

DTN_FLAGS       =

DTN_LIBS     	= -L$(DTN_LIBDIR)

#-----------------------------------------------------------------------------

DTN_LIBS       	+= -l dtn_event$(DTN_EDITION)
DTN_LIBS      	+= -l ov_log$(OV_EDITION)
DTN_LIBS      	+= -l ov_base$(OV_EDITION)

#-----------------------------------------------------------------------------

include $(DTN_ROOT)/makefiles/makefile_targets.mk
