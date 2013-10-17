LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := mcrypt

LOCAL_CFLAGS := -DBOOST_SYSTEM_NO_DEPRECATED -DMSTD_USE_PBUFFER=0 -D__GLIBC__

include ../AndroidLogs.mk

LOCAL_CPP_FEATURES := exceptions rtti

LOCAL_C_INCLUDES := $(BOOST_ROOT) $(OPENSSL_ROOT)/include ..

LOCAL_SRC_FILES := ../base64.cpp ../RSA.cpp ../Error.cpp ../PKey.cpp ../SHA.cpp ../Hasher.cpp

include $(BUILD_STATIC_LIBRARY)
