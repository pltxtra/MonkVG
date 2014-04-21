/*
 *  mkMath.h
 *  MonkVG-Quartz
 *
 *  Created by Micah Pearlman on 3/11/09.
 *  Copyright 2009 Monk Games. All rights reserved.
 *
 */

#ifndef __mkMath_h__
#define __mkMath_h__

#include "MonkVG/openvg.h"
#include <cmath>
#include <cstdio>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include "mkCommon.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace MonkVG {
	
	static inline VGfloat radians (VGfloat degrees) {return (VGfloat)(degrees * (M_PI/180.0f));}	
	static inline VGfloat degrees (VGfloat radians) {return (VGfloat)(radians * (180.0f/M_PI));}
	
	//	[ sx	shx	tx
	//	 shy	sy	ty
	//	 0		0	1 ]
	//	sx and sy define scaling in the x and y directions, respectively;
	//	shx and shy define shearing in the x and y directions, respectively;
	//	tx and ty define translation in the x and y directions, respectively.
	
	class Matrix33 {
	public:
		
		union {
			struct {
				float	a, c, e,			// cos(a) -sin(a) tx
					b, d, f,			// sin(a) cos(a)  ty
					ff0, ff1, ff2;			// 0      0       1
			};
			float m[9];
			float mm[3][3];
		};
		
		Matrix33( float* t ) {
			a = t[0]; c = t[1]; e = t[2];
			b = t[3]; d = t[4]; f = t[5];
			ff0 = t[6]; ff1 = t[7];  ff2 = t[8];
		}
		
		void translate(float x, float y) {
			e = a * x + c *y + e;
			f = b * x + d * y + f;
		}

		void scale(float sx, float sy) {
			a = a * sx;
			b = b * sx;
			c = c * sy;
			d = d * sy;
		}

		void rotate(float angle_in_radians) {
			float A, B, C, D;
			float a2, b2, c2, d2;
			
			a2 = cosf(angle_in_radians); c2 = -sinf(angle_in_radians);
			b2 = sinf(angle_in_radians); d2 = cosf(angle_in_radians);
			
			A = a * a2 + c * b2;
			C = a * c2 + c * d2;
			B = b * a2 + d * b2;
			D = b * c2 + d * d2;
			
			a = A; c = C;
			b = B; d = D;
		}

		void shear(float shx, float shy) {
			float A, B, C, D;
			float a2, b2, c2, d2;
			
			a2 = 1.0f; c2 = shx;
			b2 = shy; d2 = 1.0f;
			
			A = a * a2 + c * b2;
			C = a * c2 + c * d2;
			B = b * a2 + d * b2;
			D = b * c2 + d * d2;
			
			a = A; c = C;
			b = B; d = D;
		}

		void setIdentity() {
			// set to identity
			a = d = ff2 = 1.0f;
			c = e = b = f = ff0 = ff1 = 0;
		}
		
		static void multiply( Matrix33& r, const Matrix33& a, const Matrix33& b ) {
			for ( int z = 0; z < 9; z++ )
				r.m[z] = 0;
			
			for( int i = 0; i < 3; i++ ) 
				for( int j = 0; j < 3; j++ ) 
					for ( int k = 0; k < 3; k++ ) {
						r.mm[i][j] += a.mm[i][k] * b.mm[k][j];
					}
		}
		
		Matrix33() {
			setIdentity();
		}
		
		Matrix33( const Matrix33 &m ) {
			this->copy( m );
		}
		
		inline void set( int row, int col, VGfloat v ) {
			mm[row][col] = v; 
		}
		
		inline VGfloat get( int row, int col ) const {
			return mm[row][col];
		}
		
		inline void copy( const Matrix33& m_ ) {
			for( int i = 0; i < 9; i++ )
				m[i] = m_.m[i];
		}

		inline void transpose( ) {
			Matrix33 tmp;
			for( int i = 0; i < 3; i++ )
				for( int k = 0; k < 3; k++ )
					tmp.set( i, k, get( k, i ) );
			copy( tmp );
		}

		inline void postMultiply( const Matrix33& m ) {
			Matrix33 tmp;
			Matrix33::multiply( tmp, *this, m );
			copy( tmp );
		}		
	};
	
	inline void affineTransform( float result[2], const Matrix33& m, const float v[2] )	{ 
		result[0] = v[0] * m.get(0,0) + v[1] * m.get(0,1) + m.get(0,2);
		result[1] = v[0] * m.get(1,0) + v[1] * m.get(1,1) + m.get(1,2); 
	}
	
}


#define RI_ASSERT(_) 

static inline int RI_ISNAN(float a) {
    return (a!=a)?1:0;
}

static inline float   RI_MAX(float a, float b)                            { return (a > b) ? a : b; }
static inline float   RI_MIN(float a, float b)                            { return (a < b) ? a : b; }
static inline float   RI_CLAMP(float a, float l, float h)       { if(RI_ISNAN(a)) return l; RI_ASSERT(l <= h); return (a < l) ? l : (a > h) ? h : a; }
static inline float   RI_SQR(float a)                                                       { return a * a; }
static inline float   RI_MOD(float a, float b){
	if(RI_ISNAN(a) || RI_ISNAN(b))
		return 0.0f;
    
	RI_ASSERT(b >= 0.0f);
	
	if(b == 0.0f)
		return 0.0f;
    
	float f = (float)fmod(a, b);
	
	if(f < 0.0f)
		f += b;
	RI_ASSERT(f >= 0.0f && f <= b);
	return f;
}

static inline int RI_INT_MAX(int a, int b)                      { return (a > b) ? a : b; }
static inline int RI_INT_MIN(int a, int b)                      { return (a < b) ? a : b; }
static inline uint16_t RI_UINT16_MIN(uint16_t a,uint16_t b) { return (a < b) ? a : b; } 
static inline uint32_t RI_UINT32_MIN(uint32_t a, uint32_t b)                    { return (a < b) ? a : b; }
static inline int RI_INT_MOD(int a, int b)                      { RI_ASSERT(b >= 0); if(!b) return 0; int i = a % b; if(i < 0) i += b; RI_ASSERT(i >= 0 && i < b); return i; }
static inline int RI_INT_CLAMP(int a, int l, int h)     { RI_ASSERT(l <= h); return (a < l) ? l : (a > h) ? h : a; }

static inline int RI_FLOOR_TO_INT(float value){
	if(value<0)
		return (int)floor(value);
    
	return (int)value;
}
#endif // __mkMath_h__
