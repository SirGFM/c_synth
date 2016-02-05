LOCAL_PATH := $(call my-dir)

###########################
#
# CSynth shared library
#
###########################

include $(CLEAR_VARS)

LOCAL_MODULE := CSynth

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include $(LOCAL_PATH)/include

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

LOCAL_SRC_FILES := \
        $(LOCAL_PATH)/synth.c \
        $(LOCAL_PATH)/synth_audio.c \
        $(LOCAL_PATH)/synth_lexer.c \
        $(LOCAL_PATH)/synth_note.c \
        $(LOCAL_PATH)/synth_parser.c \
        $(LOCAL_PATH)/synth_prng.c \
        $(LOCAL_PATH)/synth_renderer.c \
        $(LOCAL_PATH)/synth_track.c \
        $(LOCAL_PATH)/synth_volume.c

LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_CFLAGS += -DUSE_SDL2
LOCAL_LDLIBS := -ldl -landroid

include $(BUILD_SHARED_LIBRARY)

