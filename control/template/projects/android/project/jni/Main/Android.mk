LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

V_SRC_PATH     := ../../../../../source
V_ENG_PATH     := ../../../../../../CoreEngine/source
V_ENG_LIB_PATH := ../../../../../../CoreEngine/libraries

##########################################################################

LOCAL_MODULE := SDL2
LOCAL_SRC_FILES := $(V_ENG_LIB_PATH)/android/$(TARGET_ARCH_ABI)/libSDL2.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_ttf
LOCAL_SRC_FILES := $(V_ENG_LIB_PATH)/android/$(TARGET_ARCH_ABI)/libSDL2_ttf.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_image
LOCAL_SRC_FILES := $(V_ENG_LIB_PATH)/android/$(TARGET_ARCH_ABI)/libSDL2_image.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := OpenAL
LOCAL_SRC_FILES := $(V_ENG_LIB_PATH)/android/$(TARGET_ARCH_ABI)/libOpenAL.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := Ogg
LOCAL_SRC_FILES := $(V_ENG_LIB_PATH)/android/$(TARGET_ARCH_ABI)/libOgg.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := Vorbis
LOCAL_SRC_FILES := $(V_ENG_LIB_PATH)/android/$(TARGET_ARCH_ABI)/libVorbis.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := VorbisFile
LOCAL_SRC_FILES := $(V_ENG_LIB_PATH)/android/$(TARGET_ARCH_ABI)/libVorbisFile.so
include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

##########################################################################

LOCAL_MODULE := Main

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(V_ENG_LIB_PATH)/include \
                    $(LOCAL_PATH)/$(V_ENG_PATH)             \
                    $(LOCAL_PATH)/$(V_SRC_PATH)

LOCAL_SRC_FILES := $(V_SRC_PATH)/main.cpp                                \
                   $(V_SRC_PATH)/cLayer.cpp                              \
                   $(V_ENG_PATH)/Core.cpp                                \
                   $(V_ENG_PATH)/additional/android/glewES.cpp           \
                   $(V_ENG_PATH)/additional/android/SDL_android_main.c   \
                   $(V_ENG_PATH)/components/audio/CoreAudio.cpp          \
                   $(V_ENG_PATH)/components/audio/coreMusic.cpp          \
                   $(V_ENG_PATH)/components/audio/coreSound.cpp          \
                   $(V_ENG_PATH)/components/graphics/coreDataBuffer.cpp  \
                   $(V_ENG_PATH)/components/graphics/coreFont.cpp        \
                   $(V_ENG_PATH)/components/graphics/coreFrameBuffer.cpp \
                   $(V_ENG_PATH)/components/graphics/CoreGraphics.cpp    \
                   $(V_ENG_PATH)/components/graphics/coreModel.cpp       \
                   $(V_ENG_PATH)/components/graphics/coreShader.cpp      \
                   $(V_ENG_PATH)/components/graphics/coreSync.cpp        \
                   $(V_ENG_PATH)/components/graphics/coreTexture.cpp     \
                   $(V_ENG_PATH)/components/input/CoreInput.cpp          \
                   $(V_ENG_PATH)/components/system/CoreSystem.cpp        \
                   $(V_ENG_PATH)/components/system/coreThread.cpp        \
                   $(V_ENG_PATH)/components/system/coreTimer.cpp         \
                   $(V_ENG_PATH)/manager/coreMemory.cpp                  \
                   $(V_ENG_PATH)/manager/coreObject.cpp                  \
                   $(V_ENG_PATH)/manager/coreResource.cpp                \
                   $(V_ENG_PATH)/objects/game/coreObject2D.cpp           \
                   $(V_ENG_PATH)/objects/game/coreObject3D.cpp           \
                   $(V_ENG_PATH)/objects/game/coreParticle.cpp           \
                   $(V_ENG_PATH)/objects/menu/coreButton.cpp             \
                   $(V_ENG_PATH)/objects/menu/coreCheckBox.cpp           \
                   $(V_ENG_PATH)/objects/menu/coreLabel.cpp              \
                   $(V_ENG_PATH)/objects/menu/coreMenu.cpp               \
                   $(V_ENG_PATH)/objects/menu/coreTextBox.cpp            \
                   $(V_ENG_PATH)/utilities/data/coreData.cpp             \
                   $(V_ENG_PATH)/utilities/data/coreRand.cpp             \
                   $(V_ENG_PATH)/utilities/file/coreArchive.cpp          \
                   $(V_ENG_PATH)/utilities/file/coreConfig.cpp           \
                   $(V_ENG_PATH)/utilities/file/coreLanguage.cpp         \
                   $(V_ENG_PATH)/utilities/file/coreLog.cpp              \
                   $(V_ENG_PATH)/utilities/math/coreMath.cpp             \
                   $(V_ENG_PATH)/utilities/math/coreSpline.cpp

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_ttf SDL2_image OpenAL Ogg Vorbis VorbisFile
LOCAL_LDLIBS           := -ldl -llog -landroid -lGLESv1_CM -lGLESv2
LOCAL_CPPFLAGS         := -std=c++11

include $(BUILD_SHARED_LIBRARY)
