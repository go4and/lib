LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := msqlite 

LOCAL_CFLAGS := -DSQLITE_NO_EXCEPTIONS -DMLOG_NO_LOGGING -DBOOST_SYSTEM_NO_DEPRECATED 

LOCAL_C_INCLUDES := ../.. ../../../../work/boost ../../../sqlite

LOCAL_SRC_FILES := ../SQLite.cpp

include $(BUILD_STATIC_LIBRARY)
