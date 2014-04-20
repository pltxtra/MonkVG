/*
 * OpenVG FreeType glyph loader
 * Copyright 2014 by Anton Persson
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "font_loader.h"

#include <android/log.h>
#define  LOG_TAG    "libgl2jni"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_TYPES_H
#include FT_OUTLINE_H
#include FT_RENDER_H

static FT_Library m_library;
static int horizontal_dpi, vertical_dpi;


int fontLoader_setup(int _horizontal_dpi, int _vertical_dpi) {
	FT_Error error = FT_Err_Ok;
	m_library = 0;

	// For simplicity, the error handling is very rudimentary.
	error = FT_Init_FreeType(&m_library);
	if (!error) {
		horizontal_dpi = _horizontal_dpi;
		vertical_dpi = _vertical_dpi;
	}
	return error ? -1 : 0;
}

static int fontLoader_Outline_MoveToFunc(const FT_Vector*  to,
					 void*             user ) {
	VGPath *p = ((VGPath *)user);
	VGubyte seg = VG_MOVE_TO_ABS;
	VGfloat data[] = {
		((float)(to->x)),
		((float)(to->y))
	};
	LOGI("moveTo(%f, %f)\n", data[0], data[1]);
	vgAppendPathData(*p, 1, &seg, data);
	return 0;
}

static int fontLoader_Outline_LineToFunc(const FT_Vector*  to,
					 void*             user ) {
	VGPath *p = ((VGPath *)user);
	VGubyte seg = VG_LINE_TO_ABS;
	VGfloat data[] = {
		((float)(to->x)),
		((float)(to->y))
	};
	LOGI("lineTo(%f, %f)\n", data[0], data[1]);
	vgAppendPathData(*p, 1, &seg, data);
	return 0;
}

static int fontLoader_Outline_ConicToFunc(const FT_Vector*  control,
					  const FT_Vector*  to,
					  void*             user ) {
	VGPath *p = ((VGPath *)user);
	VGubyte seg = VG_QUAD_TO_ABS;
	VGfloat data[] = {
		((float)(control->x)),
		((float)(control->y)),
		((float)(to->x)),
		((float)(to->y))
	};
	LOGI("quadTo(%f, %f, %f, %f)\n", data[0], data[1], data[2], data[3]);
	vgAppendPathData(*p, 1, &seg, data);
	return 0;
}

static int fontLoader_Outline_CubicToFunc(const FT_Vector*  control1,
					  const FT_Vector*  control2,
					  const FT_Vector*  to,
					  void*             user ) {
	VGPath *p = ((VGPath *)user);
	VGubyte seg = VG_CUBIC_TO_ABS;
	VGfloat data[] = {
		((float)(control1->x)),
		((float)(control1->y)),
		((float)(control2->x)),
		((float)(control2->y)),
		((float)(to->x)),
		((float)(to->y))
	};
	LOGI("cubicTo(%f, %f, %f, %f, %f, %f)\n", data[0], data[1], data[2], data[3], data[4], data[5]);
	vgAppendPathData(*p, 1, &seg, data);
	return 0;
}

static FT_Outline_Funcs fot_funcs = {
	.move_to = fontLoader_Outline_MoveToFunc,
	.line_to = fontLoader_Outline_LineToFunc,
	.conic_to = fontLoader_Outline_ConicToFunc,
	.cubic_to = fontLoader_Outline_CubicToFunc,
	.shift = 0,
	.delta = 0
};

static VGPath create_path(FT_Face m_face) {
	FT_Outline* outline = &m_face->glyph->outline;

	VGPath result = vgCreatePath(VG_PATH_FORMAT_STANDARD, VG_PATH_DATATYPE_F,
				     1,0,0,0, VG_PATH_CAPABILITY_ALL);
	
	FT_Error error =
		FT_Outline_Decompose(outline,
				     &fot_funcs,
				     &result);

	return result;
}

static void load_path_to_font(VGFont vg_font, FT_Face m_face, FT_ULong charcode, FT_UInt gindex) {
	FT_Error error = FT_Err_Ok;
	VGfloat origin[] = {0.0f, 0.0f};
	VGfloat escape[] = {0.0f, 0.0f};
	
	error = FT_Load_Glyph(m_face,
			      gindex,
			      FT_LOAD_DEFAULT);

	if(!error) {
		VGPath path = create_path(m_face);
		if(path) {
			escape[0] = m_face->glyph->metrics.horiAdvance;
			escape[1] = m_face->glyph->metrics.vertAdvance;
			
			vgSetGlyphToPath(vg_font, charcode, path, VG_FALSE, origin, escape);
		}
	}
}
				

VGFont fontLoader_load_font(const char *font_path) {
	FT_Error error = FT_Err_Ok;
	VGFont vg_font = NULL;
	FT_Face m_face = 0;

	error = FT_New_Face(m_library,
			    font_path,
			    0,
			    &m_face);
	if (!error) {
		error = FT_Set_Char_Size(m_face,
					 0,
					 12 * 64,
					 horizontal_dpi,
					 vertical_dpi);
		
		if (!error) {
			vg_font = vgCreateFont(10);

			if(vg_font) {
				FT_ULong  charcode;                                              
				FT_UInt   gindex;                                                
				
				charcode = FT_Get_First_Char(m_face, &gindex );                   
				while ( gindex != 0 )                                            
				{                                                                
					load_path_to_font(vg_font, m_face, charcode, gindex);
					
					charcode = FT_Get_Next_Char(m_face, charcode, &gindex );        
				}          
			}
		}
	}
	return vg_font;
}
