//
//  GLBasicEnums.hpp
//  AndoEngine
//
//  Created by Justin Bool on 9/3/17.
//
//

#pragma once

#include <cstdint>
#include "GL/glew.h"

#include "EnumMacros.h"

namespace GL
{
	DeclareEnumerationConverter(
		EGLType,
		( uint16_t, GLenum ),
		(
			( Bool, GL_BOOL ),

			( Byte, GL_BYTE ),
			( UByte, GL_UNSIGNED_BYTE ),
			( Int, GL_INT ),
			( UInt, GL_UNSIGNED_INT ),
			( Short, GL_SHORT ),
			( UShort, GL_UNSIGNED_SHORT ),

			( Float, GL_FLOAT ),
			( Double, GL_DOUBLE ),
			( Half, GL_HALF_FLOAT ),
			( Fixed, GL_FIXED ),

			( BVec2, GL_BOOL_VEC2 ),
			( BVec3, GL_BOOL_VEC3 ),
			( BVec4, GL_BOOL_VEC4 ),

			( IVec2, GL_INT_VEC2 ),
			( IVec3, GL_INT_VEC3 ),
			( IVec4, GL_INT_VEC4 ),

			( UVec2, GL_UNSIGNED_INT_VEC2 ),
			( UVec3, GL_UNSIGNED_INT_VEC3 ),
			( UVec4, GL_UNSIGNED_INT_VEC4 ),

			( FVec2, GL_FLOAT_VEC2 ),
			( FVec3, GL_FLOAT_VEC3 ),
			( FVec4, GL_FLOAT_VEC4 ),

			( DVec2, GL_DOUBLE_VEC2 ),
			( DVec3, GL_DOUBLE_VEC3 ),
			( DVec4, GL_DOUBLE_VEC4 ),

			( FMat2x2, GL_FLOAT_MAT2 ),
			( FMat2x3, GL_FLOAT_MAT2x3 ),
			( FMat2x4, GL_FLOAT_MAT2x4 ),
			( FMat3x2, GL_FLOAT_MAT3x2 ),
			( FMat3x3, GL_FLOAT_MAT3 ),
			( FMat3x4, GL_FLOAT_MAT3x4 ),
			( FMat4x2, GL_FLOAT_MAT4x2 ),
			( FMat4x3, GL_FLOAT_MAT4x3 ),
			( FMat4x4, GL_FLOAT_MAT4 ),

			( DMat2x2, GL_DOUBLE_MAT2 ),
			( DMat2x3, GL_DOUBLE_MAT2x3 ),
			( DMat2x4, GL_DOUBLE_MAT2x4 ),
			( DMat3x2, GL_DOUBLE_MAT3x2 ),
			( DMat3x3, GL_DOUBLE_MAT3 ),
			( DMat3x4, GL_DOUBLE_MAT3x4 ),
			( DMat4x2, GL_DOUBLE_MAT4x2 ),
			( DMat4x3, GL_DOUBLE_MAT4x3 ),
			( DMat4x4, GL_DOUBLE_MAT4 ),

			( FSampler1D, GL_SAMPLER_1D ),
			( FSampler2D, GL_SAMPLER_2D ),
			( FSampler3D, GL_SAMPLER_3D ),
			( FSamplerCube, GL_SAMPLER_CUBE ),

			( ISampler1D, GL_INT_SAMPLER_1D ),
			( ISampler2D, GL_INT_SAMPLER_2D ),
			( ISampler3D, GL_INT_SAMPLER_3D ),
			( ISamplerCube, GL_INT_SAMPLER_CUBE ),

			( USampler1D, GL_UNSIGNED_INT_SAMPLER_1D ),
			( USampler2D, GL_UNSIGNED_INT_SAMPLER_2D ),
			( USampler3D, GL_UNSIGNED_INT_SAMPLER_3D ),
			( USamplerCube, GL_UNSIGNED_INT_SAMPLER_CUBE ),

			( Int_2_10_10_10_REV, GL_INT_2_10_10_10_REV ),
			( UInt_2_10_10_10_REV, GL_UNSIGNED_INT_2_10_10_10_REV ),
			( UInt_10F_11F_11F_REV, GL_UNSIGNED_INT_10F_11F_11F_REV ),
		 )
	);

	DeclareEnumerationConverter(
		EGLBool,
		( uint8_t, GLenum ),
		(
			( False, GL_FALSE ),
			( True, GL_TRUE )
		)
	);
}
