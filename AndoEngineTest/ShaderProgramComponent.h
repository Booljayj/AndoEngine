//
//  ShaderProgramComponent.h
//  AndoEngine
//
//  Created by Justin Bool on 8/6/17.
//
//

#pragma once

#include "General.h"
#include "ShaderComponent.h"

namespace C
{
	struct ShaderProgram
	{
		vector<EntityID> LinkedShaderIDs;
		ByteStream CompiledShaderBLOB;

		GLuint ProgramID;
		bool LinkSuccessful;

		void OnRetained() {}
		void OnReleased() {}
	};
}
