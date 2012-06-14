LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS := -DSQLITE_NO_EXCEPTIONS -DBOOST_SYSTEM_NO_DEPRECATED 

LOCAL_MODULE := msqlite

include ../AndroidLogs.mk

LOCAL_CPP_FEATURES := exceptions rtti

LOCAL_C_INCLUDES := .. $(BOOST_ROOT) ../../sqlite3

LOCAL_SRC_FILES := ../SQLite.cpp

include $(BUILD_STATIC_LIBRARY)
