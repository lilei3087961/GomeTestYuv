LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_JACK_ENABLED := disabled
LOCAL_SRC_FILES := src/com/gome/gometestyuv/TestManager.java

LOCAL_MODULE := GomeTestYuv

include $(BUILD_STATIC_JAVA_LIBRARY)
