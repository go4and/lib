LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := mlog

LOCAL_CFLAGS := -DBOOST_SYSTEM_NO_DEPRECATED -DMLOG_USE_BUFFERS=0 -D__GLIBC__

LOCAL_CPP_FEATURES := exceptions rtti

LOCAL_C_INCLUDES := ../../boost ..

LOCAL_SRC_FILES := ../Logger.cpp      ../Manager.cpp     ../Output.cpp      ../ThreadTrace.cpp ../Utils.cpp

include $(BUILD_STATIC_LIBRARY)
