//
//  ShaderProgramComponent.h
//  AndoEngine
//
//  Created by Justin Bool on 8/6/17.
//
//

#pragma once

#include <iostream>

#include "GLVertexArrayObject.h"
#include "GLUniform.h"
#include "GLShader.h"

#include "General.h"
#include "ShaderComponent.h"
#include "MeshComponent.h"

namespace C
{
	struct ShaderProgram
	{
		vector<EntityID> LinkedShaderIDs;
		ByteStream CompiledShaderBLOB;

		GLuint ProgramID = 0;
		vector<GL::UniformInfo> Uniforms;
		GLint LinkSuccessful = false;

		void OnRetained() {}
		void OnReleased() {}

		void Use()
		{
			glUseProgram( ProgramID );
		}

		void AttachShader( GLint ShaderID )
		{
			glAttachShader( ProgramID, ShaderID );
		}

		void DetachShader( GLint ShaderID )
		{
			glDetachShader( ProgramID, ShaderID );
		}

		void Link( const vector<C::Shader*>& ShaderComps )
		{
			ProgramID = glCreateProgram();
			for( C::Shader* ShaderComp : ShaderComps )
			{
				ShaderComp->Compile();

				if( ShaderComp->CompileStatus == GL_FALSE )
				{
					cerr << "Failed to compile shader." << endl;

					int MaxInfoLength = 0;
					glGetShaderiv( ShaderComp->ShaderID, GL_INFO_LOG_LENGTH, &MaxInfoLength );

					/* The maxLength includes the NULL character */
					char* ShaderInfoLog = (char *)malloc( MaxInfoLength );

					glGetShaderInfoLog( ShaderComp->ShaderID, MaxInfoLength, &MaxInfoLength, ShaderInfoLog );

					cerr << ShaderInfoLog << endl;

					free( ShaderInfoLog );
				}
				else
				{
					AttachShader( ShaderComp->ShaderID );
				}
			}

			GL::BindAttributeNames( ProgramID );
			if( GL::Link( ProgramID ) )
			{
				GL::GetUniforms( ProgramID, Uniforms );
			}
			else
			{
				GL::DescribeLinkingErrors( std::cerr, ProgramID );
			}

			for( C::Shader* ShaderComp : ShaderComps )
			{
				DetachShader( ShaderComp->ShaderID );
				ShaderComp->Delete();
			}
		}
	};
}
