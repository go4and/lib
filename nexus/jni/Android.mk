LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := nexus 

LOCAL_CFLAGS := -DBOOST_SYSTEM_NO_DEPRECATED -D__GLIBC__ -std=c++0x

include ../AndroidLogs.mk

LOCAL_CPP_FEATURES := exceptions rtti

LOCAL_C_INCLUDES := $(BOOST_ROOT) ..

LOCAL_SRC_FILES := ../Utils.cpp

include $(BUILD_STATIC_LIBRARY)
