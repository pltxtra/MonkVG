# Copyright (C) 2014 by Anton Persson
# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
LOCAL_PATH:= $(call my-dir)
top_srcdir := $(call my-dir)/../../..
c_includes := \
		$(top_srcdir)/../precompiled/include/freetype2 \
		$(top_srcdir)/glu/include $(top_srcdir)/include \
		$(top_srcdir)/src $(top_srcdir)/src/opengl \
		$(top_srcdir)/thirdparty/gles2-bc/Sources/OpenGLES \
		$(top_srcdir)/thirdparty/gles2-bc/Sources/OpenGLES/OpenGLES11 \
		$(top_srcdir)/thirdparty/gles2-bc/Sources/OpenGLES/OpenGLES20 \
		$(top_srcdir)/thirdparty/fmemopen $(LOCAL_PATH)/boost/include

include $(CLEAR_VARS)
LOCAL_MODULE := libOpenVG
LOCAL_SRC_FILES  := ../../MonkVG-Android/obj/local/armeabi/libOpenVG.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libOpenVGU
LOCAL_SRC_FILES  := ../../MonkVG-Android/obj/local/armeabi/libOpenVGU.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libfreetype
LOCAL_SRC_FILES := ../../../../precompiled/lib/libfreetype.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/include/freetype2
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE     := libfontloader
LOCAL_CFLAGS    := -Werror
LOCAL_C_INCLUDES := $(c_includes)
LOCAL_SRC_FILES  := font_loader.c
include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := libfontloader_test
LOCAL_CPPFLAGS += -Werror --std=c++11 -O3
LOCAL_C_INCLUDES := $(c_includes)
LOCAL_SRC_FILES := fontloader_test.cc
LOCAL_LDLIBS    := -llog -lGLESv1_CM -lGLESv2
LOCAL_STATIC_LIBRARIES := libOpenVGU libOpenVG libfontloader libfreetype
include $(BUILD_SHARED_LIBRARY)
