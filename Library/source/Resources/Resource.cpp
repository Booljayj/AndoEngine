#include "Resources/Resource.h"
using namespace Reflection;

DEFINE_REFLECT_STRUCT(Resources, Resource)
	.Description("An object which can be shared between several other objects")
	.Variables({
		{ &Resources::Resource::id, "id"sv, ""sv, FVariableFlags::None },
		{ &Resources::Resource::name, "name"sv, ""sv, FVariableFlags::None },
	});
