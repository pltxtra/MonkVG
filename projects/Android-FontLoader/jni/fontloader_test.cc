/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// OpenGL ES 2.0 code

#include <jni.h>
#include <android/log.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <MonkVG/openvg.h>
#include <MonkVG/vgu.h>
#include <MonkVG/vgext.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "font_loader.h"

#define  LOG_TAG    "fontloaderJNI"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}

static const char gVertexShader[] = 
    "attribute vec4 vPosition;\n"
    "void main() {\n"
    "  gl_Position = vPosition;\n"
    "}\n";

static const char gFragmentShader[] = 
    "precision mediump float;\n"
    "void main() {\n"
    "  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
    "}\n";

GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

GLuint gProgram;
GLuint gvPositionHandle;

VGPaint  _paint;
VGFont _font;
int _width = 0;
int _height = 0;

void setupGraphics(int w, int h) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    LOGI("setupGraphics(%d, %d)", w, h);
    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOGE("Could not create program.");
    }
    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
            gvPositionHandle);

    glViewport(0, 0, w, h);
    checkGlError("glViewport");

    _width = w; _height = h;
    vgCreateContextMNK(w, h, VG_RENDERING_BACKEND_TYPE_OPENGLES20 );

    // create a paint
    _paint = vgCreatePaint();
    vgSetPaint(_paint, VG_FILL_PATH );
    VGfloat color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    vgSetParameterfv(_paint, VG_PAINT_COLOR, 4, &color[0]);

    fontLoader_setup(50, 50);
    _font = fontLoader_load_font("/system/fonts/Roboto-Regular.ttf", VG_FALSE);
}

void displayText() {
	static VGfloat angle = 0.0f;
	angle += 0.005;
	angle = angle - floor(angle);
	
	vgSeti(VG_MATRIX_MODE, VG_MATRIX_GLYPH_USER_TO_SURFACE);
	vgLoadIdentity();
	vgTranslate( _width / 4, _height/2 );
	vgScale(0.4, 0.4);
	vgTranslate(1000, 50);
	vgRotate(angle * 360.0);       
	vgTranslate(-1000, -50);

	VGuint  g_idx[] = {'h', 'o', 'l', 'i', 'd', 'a', 'y'};
	VGfloat g_adx[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
	VGfloat g_ady[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

	VGfloat origin[] = {0.0f, 0.0f};
	vgSetfv(VG_GLYPH_ORIGIN, 2, origin);
	
	vgSetPaint( _paint, VG_FILL_PATH );	
	vgDrawGlyphs(
		_font, 7,
		g_idx, g_adx, g_ady,
		VG_FILL_PATH, VG_FALSE);
}

void renderFrame() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	checkGlError("glClearColor");
	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	checkGlError("glClear");
	
	glUseProgram(gProgram);
	checkGlError("glUseProgram");
	
	displayText();
}

extern "C" {
    JNIEXPORT void JNICALL Java_com_holidaystudios_fontloader_FontLoaderLib_init(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_holidaystudios_fontloader_FontLoaderLib_step(JNIEnv * env, jobject obj);
};

JNIEXPORT void JNICALL Java_com_holidaystudios_fontloader_FontLoaderLib_init(JNIEnv * env, jobject obj,  jint width, jint height)
{
    setupGraphics(width, height);
}

JNIEXPORT void JNICALL Java_com_holidaystudios_fontloader_FontLoaderLib_step(JNIEnv * env, jobject obj)
{
    renderFrame();
}
