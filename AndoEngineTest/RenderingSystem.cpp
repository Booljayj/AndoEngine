//
//  RenderingSystem.cpp
//  AndoEngine
//
//  Created by Justin Bool on 8/11/17.
//
//

#include <cassert>

#include "RenderingSystem.h"
#include "Entity.h"
#include "glm/vec3.hpp"

void PrintGLErrors()
{
	GLenum ErrorCode = glGetError();
	if( ErrorCode != GL_NO_ERROR )
	{
		cerr << "OpenGL Error: " << ErrorCode << endl;
	}
}

namespace S
{
	void RenderingSystem::Update() const
	{
		MeshRendererManager->ForEach(
			[this]( C::MeshRenderer* Comp )
			{
				Render( Comp );
			}
		);
	}

	void RenderingSystem::SetMesh( const EntityID MeshRendererID, const EntityID MeshID ) const
	{
		Entity* MeshRendererEntity = EntitySys->Find( MeshRendererID );
		C::MeshRenderer* MeshRendererComp = MeshRendererEntity ? MeshRendererEntity->Get( *MeshRenderer ) : nullptr;

		Entity* MeshEntity = EntitySys->Find( MeshID );
		C::Mesh* MeshComp = MeshEntity ? MeshEntity->Get( *Mesh ) : nullptr;

		if( MeshRendererComp && MeshComp )
		{
			MeshRendererComp->CurrentMeshEntityID = MeshID;
			MeshRendererComp->MeshComp = MeshComp;
			RebindBufferData( MeshRendererComp->VertexArrayID, MeshComp );
		}
	}

	void RenderingSystem::SetShaderProgram( const EntityID MeshRendererID, const EntityID ShaderProgID ) const
	{
		Entity* MeshRendererEntity = EntitySys->Find( MeshRendererID );
		C::MeshRenderer* MeshRendererComp = MeshRendererEntity ? MeshRendererEntity->Get( *MeshRenderer ) : nullptr;

		Entity* ShaderProgEntity = EntitySys->Find( ShaderProgID );
		C::ShaderProgram* ShaderProgComp = ShaderProgEntity ? ShaderProgEntity->Get( *ShaderProgram ) : nullptr;

		if( MeshRendererComp && ShaderProgComp )
		{
			MeshRendererComp->CurrentShaderProgramID = ShaderProgID;
			MeshRendererComp->ShaderProgramComp = ShaderProgComp;
		}
	}

	void RenderingSystem::SetShaderSource( const EntityID ShaderID, const char* Source, GLenum SourceType ) const
	{
		Entity* ShaderEntity = EntitySys->Find( ShaderID );
		C::Shader* ShaderComp = ShaderEntity ? ShaderEntity->Get( *Shader ) : nullptr;
		if( ShaderComp )
		{
			ShaderComp->Source = Source;
			ShaderComp->ShaderType = SourceType;
		}
	}

	void RenderingSystem::SetProgramShaders( const EntityID ShaderProgramID, const vector<EntityID>& NewLinkedShaderIDs ) const
	{
		Entity* ShaderProgramEntity = EntitySys->Find( ShaderProgramID );
		C::ShaderProgram* ShaderProgramComp = ShaderProgramEntity ? ShaderProgramEntity->Get( *ShaderProgram ) : nullptr;

		if( ShaderProgramComp )
		{
			ShaderProgramComp->LinkedShaderIDs = NewLinkedShaderIDs;
			ShaderProgramComp->CompiledShaderBLOB.clear();
			ShaderProgramComp->LinkSuccessful = false;
		}
	}

	void RenderingSystem::Link( const EntityID ShaderProgramID ) const
	{
		Entity* ShaderProgramEntity = EntitySys->Find( ShaderProgramID );
		C::ShaderProgram* ShaderProgramComp = ShaderProgramEntity ? ShaderProgramEntity->Get( *ShaderProgram ) : nullptr;
		if( ShaderProgramComp )
		{
			Link( ShaderProgramComp );
		}
	}

/// Protected System Functions

	void RenderingSystem::RebindBufferData( GLuint VAOID, const C::Mesh* MeshComp ) const
	{
		glBindVertexArray( VAOID );
		glBindBuffer( GL_ARRAY_BUFFER, MeshComp->BufferID[C::Mesh::VertexBuffer] );
		glBufferData( GL_ARRAY_BUFFER, 9, MeshComp->VertData, GL_STATIC_DRAW );
		PrintGLErrors();
	}

	void RenderingSystem::Render( const C::MeshRenderer* MeshRendererComp ) const
	{
		if( !MeshRendererComp->IsValid() ) return;

		glUseProgram( MeshRendererComp->ShaderProgramComp->ProgramID );
		glBindVertexArray( MeshRendererComp->VertexArrayID );

		//@todo: Investigate how much of this can be moved into retain/release or even to OpenGL Init.
		glEnableVertexAttribArray( C::Mesh::VertexBuffer );
		glBindBuffer( GL_ARRAY_BUFFER, MeshRendererComp->MeshComp->BufferID[C::Mesh::VertexBuffer] );
		glVertexAttribPointer( C::Mesh::VertexBuffer, 3, GL_FLOAT, GL_FALSE, 0, nullptr );

		glDrawArrays( GL_TRIANGLES, 0, 3 );
		glDisableVertexAttribArray( C::Mesh::VertexBuffer );
		PrintGLErrors();
	}

	void RenderingSystem::Link( C::ShaderProgram* ShaderProgramComp ) const
	{
		if( ShaderProgramComp->LinkSuccessful ) return; //Already linked

		vector<C::Shader*> AttachedShaders;
		AttachedShaders.reserve( ShaderProgramComp->LinkedShaderIDs.size() );

		ShaderProgramComp->ProgramID = glCreateProgram();

		for( EntityID ShaderID : ShaderProgramComp->LinkedShaderIDs )
		{
			Entity* ShaderEntity = EntitySys->Find( ShaderID );
			C::Shader* ShaderComp = ShaderEntity ? ShaderEntity->Get( *Shader ) : nullptr;

			if( ShaderComp == nullptr || !ShaderComp->IsValid() ) continue;

			cout << "Compiling Shader " << ShaderID << endl;
			cout << "Source: " << ShaderComp->Source << endl;

			//Recreate the shader object on the GPU
			if( ShaderComp->ShaderID != 0 )
			{
				glDeleteShader( ShaderComp->ShaderID );
			}
			ShaderComp->ShaderID = glCreateShader( ShaderComp->ShaderType );

			//Send the shader source to the GPU
			glShaderSource( ShaderComp->ShaderID, 1, &ShaderComp->Source, 0 );

			//Compile the shader source into an object
			glCompileShader( ShaderComp->ShaderID );

			//Get the results of the compilation
			glGetShaderiv( ShaderComp->ShaderID, GL_COMPILE_STATUS, &ShaderComp->CompileStatus );
			//Handle failure
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
				cout << "Successfully compiled shader" << endl;
				glAttachShader( ShaderProgramComp->ProgramID, ShaderComp->ShaderID );
				AttachedShaders.push_back( ShaderComp );
			}
			PrintGLErrors();
		}

		glBindAttribLocation( ShaderProgramComp->ProgramID, 0, "in_Position" );
		glLinkProgram( ShaderProgramComp->ProgramID );
		PrintGLErrors();

		GLint IsLinked = false;
		glGetProgramiv( ShaderProgramComp->ProgramID, GL_LINK_STATUS, &IsLinked );
		if( IsLinked == GL_FALSE )
		{
			cerr << "Failed to link shader program." << endl;
			GLint MaxInfoLength = 0;
			glGetProgramiv( ShaderProgramComp->ProgramID, GL_INFO_LOG_LENGTH, &MaxInfoLength );
			char* ShaderProgramInfoLog = (char *)malloc( MaxInfoLength );
			glGetProgramInfoLog( ShaderProgramComp->ProgramID, MaxInfoLength, &MaxInfoLength, ShaderProgramInfoLog );

			cerr << ShaderProgramInfoLog << endl;
			free( ShaderProgramInfoLog );
		}
		else
		{
			cout << "Successfully linked shader program " << ShaderProgramComp->ProgramID << endl;
		}

		for( C::Shader* ShaderComp : AttachedShaders )
		{
			glDetachShader( ShaderProgramComp->ProgramID, ShaderComp->ShaderID );
			glDeleteShader( ShaderComp->ShaderID );
			ShaderComp->ShaderID = 0;
		}
		PrintGLErrors();
	}
}
