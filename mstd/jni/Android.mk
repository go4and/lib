LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := mstd

LOCAL_CFLAGS := -DBOOST_SYSTEM_NO_DEPRECATED -DMSTD_USE_PBUFFER=0 -D__GLIBC__

LOCAL_CPP_FEATURES := exceptions rtti

LOCAL_C_INCLUDES := ../../../boost

LOCAL_SRC_FILES := ../itoa.cpp ../random.cpp ../strings.cpp ../threads.cpp ../singleton.cpp

include $(BUILD_STATIC_LIBRARY)
