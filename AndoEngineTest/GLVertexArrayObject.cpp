//
//  GLVertexAttributes.cpp
//  AndoEngine
//
//  Created by Justin Bool on 9/3/17.
//
//

#include <string>
#include <sstream>
using namespace std;

#include "glm/vec3.hpp"

#include "GLVertexArrayObject.h"
#include "GLVertexBufferObject.h"
#include "GLBasicEnums.h"
#include "GLVertexData.h"

namespace GL
{
	DefineEnumeration( EAttribute );
	DefineEnumerationConverter( EAttributeIntParameter );
	DefineEnumerationConverter( EAttributeBoolParameter );
	DefineEnumerationConverter( EAttributeTypeParameter );
	DefineEnumerationConverter( EAttributePtrParameter );

	void DescribeParam_Bound( ostream& Stream, EAttribute::ENUM Attribute, EAttributeIntParameter::ENUM Param, size_t& Storage )
	{
		glGetVertexAttribiv( static_cast<GLint>( Attribute ), EAttributeIntParameter::ToGlobal( Param ), reinterpret_cast<GLint*>( &Storage ) );
		Stream << EAttributeIntParameter::Name( Param ) << " = " << Storage;
	}

	void DescribeParam_Bound( ostream& Stream, EAttribute::ENUM Attribute, EAttributeTypeParameter::ENUM Param, size_t& Storage )
	{
		glGetVertexAttribiv( static_cast<GLint>( Attribute ), EAttributeTypeParameter::ToGlobal( Param ), reinterpret_cast<GLint*>( &Storage ) );
		Stream << EAttributeTypeParameter::Name( Param ) << " = " << EGLType::GlobalName( static_cast<GLenum>( Storage ) );
	}

	void DescribeParam_Bound( ostream& Stream, EAttribute::ENUM Attribute, EAttributeBoolParameter::ENUM Param, size_t& Storage )
	{
		glGetVertexAttribiv( static_cast<GLint>( Attribute ), EAttributeBoolParameter::ToGlobal( Param ), reinterpret_cast<GLint*>( &Storage ) );
		Stream << EAttributeBoolParameter::Name( Param ) << " = " << EGLBool::GlobalName( static_cast<GLenum>( Storage ) );
	}

	void DescribeParam_Bound( ostream& Stream, EAttribute::ENUM Attribute, EAttributePtrParameter::ENUM Param, size_t& Storage )
	{
		glGetVertexAttribPointerv( static_cast<GLint>( Attribute ), EAttributePtrParameter::ToGlobal( Param ), reinterpret_cast<void**>( &Storage ) );
		Stream << EAttributePtrParameter::Name( Param ) << " = " << Storage;
	}

	string DescribeVertexArrayObject( GLuint VAOID )
	{
		size_t Storage = 0;
		ostringstream Stream;

		Stream << "VertexArrayObject " << VAOID << endl;
		glBindVertexArray( VAOID );

		for( EAttribute::TYPE AttributeIndex = 0; AttributeIndex < EAttribute::Count(); ++AttributeIndex )
		{
			EAttribute::ENUM CurAttribute = EAttribute::Cast( AttributeIndex );
			Stream << "\t" << EAttribute::Name( AttributeIndex ) << ": ";
			DescribeParam_Bound( Stream, CurAttribute, EAttributeBoolParameter::Enabled, Storage ); Stream << ", ";
			DescribeParam_Bound( Stream, CurAttribute, EAttributeIntParameter::BufferID, Storage ); Stream << ", ";
			DescribeParam_Bound( Stream, CurAttribute, EAttributeIntParameter::Size, Storage ); Stream << ", ";
			DescribeParam_Bound( Stream, CurAttribute, EAttributeIntParameter::Stride, Storage ); Stream << ", ";
			DescribeParam_Bound( Stream, CurAttribute, EAttributePtrParameter::Offset, Storage ); Stream << ", ";
			DescribeParam_Bound( Stream, CurAttribute, EAttributeTypeParameter::Type, Storage ); Stream << ", ";
			DescribeParam_Bound( Stream, CurAttribute, EAttributeBoolParameter::Normalized, Storage ); Stream << "\n";
		}
		return Stream.str();
	}

	void BindAttributeNames( GLuint ProgramID )
	{
		string ShaderPrefix = "vert_";
		for( EAttribute::TYPE AttributeIndex = 0; AttributeIndex < EAttribute::Count(); ++AttributeIndex )
		{
			string ShaderAttributeName = ShaderPrefix + EAttribute::Name( AttributeIndex );
			glBindAttribLocation( ProgramID, AttributeIndex, ShaderAttributeName.c_str() );
		}
	}

	void BindBuffersToVertexArrayObject( GLuint VAOID, const GLuint BufferID[EBuffer::Count()] )
	{
		glBindVertexArray( VAOID );

		//These could be simplified using some macros, but I think I prefer them written out explicitly. When we have a lot more attributes, some common macro patterns should emerge.
		glBindBuffer( GL_ARRAY_BUFFER, BufferID[EBuffer::VertexData] );
		glEnableVertexAttribArray( EAttribute::Position );
		glVertexAttribPointer( EAttribute::Position, decltype( VertexData::Position )::length(), GL_FLOAT, GL_FALSE, sizeof( VertexData ), reinterpret_cast<void*>( offsetof( VertexData, Position ) ) );
	}
}
