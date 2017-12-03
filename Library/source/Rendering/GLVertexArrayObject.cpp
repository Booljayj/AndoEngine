#include <string>
#include <sstream>
#include <glm/vec3.hpp>
#include "Rendering/EBufferTarget.enum.h"
#include "Rendering/EGLBool.enum.h"
#include "Rendering/EGLType.enum.h"
#include "Rendering/GLVertexArrayObject.h"
#include "Rendering/VertexData.h"

using namespace std;

namespace GL
{
	void DescribeAttributeParameter_Bound( ostream& Stream, EAttribute::ENUM Attribute, EAttributeParameter::ENUM Param, size_t& Storage )
	{
		switch( Param )
		{
			case EAttributeParameter::Enabled:
			case EAttributeParameter::Normalized:
			//boolean
			glGetVertexAttribiv( static_cast<GLint>( Attribute ), EAttributeParameter::ToGL( Param ), reinterpret_cast<GLint*>( &Storage ) );
			Stream << EAttributeParameter::ToName( Param ) << ": " << EGLBool::ToName( EGLBool::FromGL( static_cast<GLenum>( Storage ) ) );
			break;

			case EAttributeParameter::BufferID:
			case EAttributeParameter::Size:
			case EAttributeParameter::Stride:
			case EAttributeParameter::Integer:
			case EAttributeParameter::Divisor:
			//integer
			glGetVertexAttribiv( static_cast<GLint>( Attribute ), EAttributeParameter::ToGL( Param ), reinterpret_cast<GLint*>( &Storage ) );
			Stream << EAttributeParameter::ToName( Param ) << ": " << Storage;
			break;

			case EAttributeParameter::Type:
			//type
			glGetVertexAttribiv( static_cast<GLint>( Attribute ), EAttributeParameter::ToGL( Param ), reinterpret_cast<GLint*>( &Storage ) );
			Stream << EAttributeParameter::ToName( Param ) << ": " << EGLType::ToName( EGLType::FromGL( static_cast<GLenum>( Storage ) ) );
			break;

			case EAttributeParameter::Offset:
			//pointer
			glGetVertexAttribPointerv( static_cast<GLint>( Attribute ), EAttributeParameter::ToGL( Param ), reinterpret_cast<void**>( &Storage ) );
			Stream << EAttributeParameter::ToName( Param ) << ": " << Storage;
			break;
		}
	}

	void DescribeAttribute_Bound( ostream& Stream, EAttribute::ENUM Attribute )
	{
		size_t Storage = 0;
		Stream << "[Attribute]{ " << EAttribute::ToName( Attribute ) << ", ";
		DescribeAttributeParam_Bound( Stream, Attribute, EAttributeParameter::Enabled, Storage ); Stream << ", ";
		DescribeAttributeParam_Bound( Stream, Attribute, EAttributeParameter::BufferID, Storage ); Stream << ", ";
		DescribeAttributeParam_Bound( Stream, Attribute, EAttributeParameter::Size, Storage ); Stream << ", ";
		DescribeAttributeParam_Bound( Stream, Attribute, EAttributeParameter::Stride, Storage ); Stream << ", ";
		DescribeAttributeParam_Bound( Stream, Attribute, EAttributeParameter::Offset, Storage ); Stream << ", ";
		DescribeAttributeParam_Bound( Stream, Attribute, EAttributeParameter::Type, Storage ); Stream << ", ";
		DescribeAttributeParam_Bound( Stream, Attribute, EAttributeParameter::Normalized, Storage ); Stream << " }";
	}

	void DescribeVertexArrayObject( ostream& Stream, GLuint VAOID )
	{
		size_t Storage = 0;

		Stream << "[VertexArrayObject]{\n\tID: " << VAOID << ",\n";
		glBindVertexArray( VAOID );

		for( EAttribute::TYPE AttributeIndex = 0; AttributeIndex < EAttribute::Count(); ++AttributeIndex )
		{
			EAttribute::ENUM CurAttribute = EAttribute::Cast( AttributeIndex );
			Stream << "\t";
			DescribeAttribute_Bound( Stream, CurAttribute );
			Stream << "\n";
		}
	}

	void BindAttributeNames( GLuint ProgramID )
	{
		string ShaderPrefix = "vert_";
		for( EAttribute::TYPE AttributeIndex = 0; AttributeIndex < EAttribute::Count(); ++AttributeIndex )
		{
			string ShaderAttributeName = ShaderPrefix + EAttribute::ToName( EAttribute::Cast( AttributeIndex ) );
			glBindAttribLocation( ProgramID, AttributeIndex, ShaderAttributeName.c_str() );
		}
	}

	void BindBuffersToVertexArrayObject( GLuint VAOID, const GLuint BufferID[EBuffer::Count()] )
	{
		glBindVertexArray( VAOID );

		//These could be simplified using some macros, but I think I prefer them written out explicitly. When we have a lot more attributes, some common macro patterns should emerge.
		glBindBuffer( EBufferTarget::ToGL( EBufferTarget::Array ), BufferID[EBuffer::VertexData] );
		glEnableVertexAttribArray( EAttribute::Position );
		glVertexAttribPointer( EAttribute::Position, decltype( VertexData::Position )::length(), GL_FLOAT, GL_FALSE, sizeof( VertexData ), reinterpret_cast<void*>( offsetof( VertexData, Position ) ) );

		//glEnableVertexAttribArray( EAttribute::Color );
		//glVertexAttribPointer( EAttribute::Color, decltype( VertexData::Color )::length(), GL_UNSIGNED_BYTE, GL_TRUE, sizeof( VertexData ), reinterpret_cast<void*>( offsetof( VertexData, Color ) ) );

		//glEnableVertexAttribArray( EAttribute::Normal );
		//glVertexAttribPointer( EAttribute::Normal, decltype( VertexData::Normal )::lenght(), GL_FLOAT, GL_FALSE, sizeof( VertexData ), reinterpret_cast<void*>( offsetof( VertexData, Normal ) ) );

		//glEnableVertexAttribArray( EAttribute::TexCoords );
		//glVertexAttribPointer( EAttribute::TexCoords, decltype( VertexData::TexCoords )::lenght(), GL_UNSIGNED_SHORT, GL_TRUE, sizeof( VertexData ), reinterpret_cast<void*>( offsetof( VertexData, TexCoords ) ) );
	}
}
