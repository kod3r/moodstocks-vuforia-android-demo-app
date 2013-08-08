LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := QCAR-prebuilt
LOCAL_SRC_FILES = $(VFR_SDK)/build/lib/$(TARGET_ARCH_ABI)/libQCAR.so
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/$(VFR_SDK)/build/include
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE := MoodstocksVuforia
OPENGLES_LIB  := -lGLESv2
OPENGLES_DEF  := -DUSE_OPENGL_ES_2_0
LOCAL_CFLAGS := -Wno-write-strings -Wno-psabi $(OPENGLES_DEF)
LOCAL_LDLIBS := -llog $(OPENGLES_LIB)
LOCAL_SHARED_LIBRARIES := QCAR-prebuilt
LOCAL_SRC_FILES := \
           MSController.cpp \
           MSTargetManager.cpp \
           MSTargetInfo.cpp \
           MSRenderer.cpp \
           MSTracker.cpp \
           MSCallback.cpp \
           MSModel.cpp \
           MSTexture.cpp \
           JNILayer.cpp
LOCAL_ARM_MODE := arm
include $(BUILD_SHARED_LIBRARY)
