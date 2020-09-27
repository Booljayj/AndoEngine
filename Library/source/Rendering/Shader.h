#pragma once
#include "Engine/STL.h"
#include "EntityFramework/EntityTypes.h"

struct ShaderComponent {
	std::vector<uint8_t> code;
};

struct MaterialComponent {
	EntityRuntimeID vertex;
	EntityRuntimeID fragment;
};
