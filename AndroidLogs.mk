ifeq (1,$(MLOG_NO_LOGGING))
LOCAL_CFLAGS += -DMLOG_NO_LOGGING=1
LIBRARY_SUFFIX := _nolog
LOCAL_MODULE := $(LOCAL_MODULE)$(LIBRARY_SUFFIX)
else
LIBRARY_SUFFIX :=
endif
