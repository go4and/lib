include ../ApplicationLogs.mk

APP_ABI := all
APP_STL := gnustl_static
APP_MODULES := mlog$(LIBRARY_SUFFIX)
APP_CPPFLAGS += -std=c++11
