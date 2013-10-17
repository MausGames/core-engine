LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

V_SRC_PATH := ../../../../source
V_LIB_PATH := ../../../../libraries

##########################################################################

LOCAL_MODULE := SDL2
LOCAL_SRC_FILES := $(V_LIB_PATH)/android/$(TARGET_ARCH_ABI)/libSDL2.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_ttf
LOCAL_SRC_FILES := $(V_LIB_PATH)/android/$(TARGET_ARCH_ABI)/libSDL2_ttf.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_image
LOCAL_SRC_FILES := $(V_LIB_PATH)/android/$(TARGET_ARCH_ABI)/libSDL2_image.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := OpenAL
LOCAL_SRC_FILES := $(V_LIB_PATH)/android/$(TARGET_ARCH_ABI)/libOpenAL.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := Ogg
LOCAL_SRC_FILES := $(V_LIB_PATH)/android/$(TARGET_ARCH_ABI)/libOgg.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := Vorbis
LOCAL_SRC_FILES := $(V_LIB_PATH)/android/$(TARGET_ARCH_ABI)/libVorbis.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := VorbisFile
LOCAL_SRC_FILES := $(V_LIB_PATH)/android/$(TARGET_ARCH_ABI)/libVorbisFile.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

##########################################################################

LOCAL_MODULE := main

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(V_LIB_PATH)/include \
		    $(LOCAL_PATH)/$(V_SRC_PATH)

LOCAL_SRC_FILES := ../../../main.cpp \
		   $(V_SRC_PATH)/Core.cpp \
		   $(V_SRC_PATH)/additional/android/glewES.cpp \
		   $(V_SRC_PATH)/additional/android/SDL_android_main.c \
		   $(V_SRC_PATH)/components/audio/CoreAudio.cpp \
		   $(V_SRC_PATH)/components/audio/coreMusic.cpp \
		   $(V_SRC_PATH)/components/audio/coreSound.cpp \
		   $(V_SRC_PATH)/components/graphics/CoreGraphics.cpp \
		   $(V_SRC_PATH)/components/graphics/coreModel.cpp \
		   $(V_SRC_PATH)/components/graphics/coreShader.cpp \
		   $(V_SRC_PATH)/components/graphics/coreTexture.cpp \
		   $(V_SRC_PATH)/components/input/CoreInput.cpp \
		   $(V_SRC_PATH)/components/system/CoreSystem.cpp \
		   $(V_SRC_PATH)/components/system/coreThread.cpp \
		   $(V_SRC_PATH)/components/system/coreTimer.cpp \
		   $(V_SRC_PATH)/manager/coreMemory.cpp \
		   $(V_SRC_PATH)/manager/coreObject.cpp \
		   $(V_SRC_PATH)/manager/coreResource.cpp \
		   $(V_SRC_PATH)/objects/game/coreObject3D.cpp \
		   $(V_SRC_PATH)/utilities/data/coreData.cpp \
		   $(V_SRC_PATH)/utilities/data/coreRand.cpp \
		   $(V_SRC_PATH)/utilities/file/coreArchive.cpp \
		   $(V_SRC_PATH)/utilities/file/coreConfig.cpp \
		   $(V_SRC_PATH)/utilities/file/coreLog.cpp \
		   $(V_SRC_PATH)/utilities/math/coreMath.cpp \
		   $(V_SRC_PATH)/utilities/math/coreMatrix.cpp \
		   $(V_SRC_PATH)/utilities/math/coreSpline.cpp \
		   $(V_SRC_PATH)/utilities/math/coreVector.cpp

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_ttf SDL2_image OpenAL Ogg Vorbis VorbisFile
LOCAL_LDLIBS	       := -ldl -llog -lGLESv1_CM -lGLESv2
LOCAL_CPPFLAGS	       := -std=c++11

include $(BUILD_SHARED_LIBRARY)
