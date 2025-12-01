# -*- Makefile -*-


#-----------------------------------------------------------------------------

DTN_EDITION            := 1

#-----------------------------------------------------------------------------
DTN_VERSION_BUILD_ID_FILE := $(DTN_ROOT)/.DTN_build_id.txt

DTN_VERSION_MAJOR      := 0
DTN_VERSION_MINOR      := 0
DTN_VERSION_PATCH      := 1
DTN_VERSION            := $(DTN_VERSION_MAJOR).$(DTN_VERSION_MINOR).$(DTN_VERSION_PATCH)
DTN_VERSION_HEX        := $(shell printf "%02i%02i%02i\n" $(DTN_VERSION_MAJOR) $(DTN_VERSION_MINOR) $(DTN_VERSION_PATCH))
DTN_VERSION_BUILD_ID   := $(shell [ ! -e $(DTN_VERSION_BUILD_ID_FILE) ] && echo 0 > $(DTN_VERSION_BUILD_ID_FILE); cat $(DTN_VERSION_BUILD_ID_FILE))
DTN_VERSION_BUILD_DATE := $(shell date '+%Y.%m.%d_%H:%M:%S')
DTN_VERSION_COMPILER   := $(shell echo $(CC) && cc -dumpversion)
DTN_VERSION_COMMIT_ID  := $(shell git log --format="%H" -n 1)



#-----------------------------------------------------------------------------
