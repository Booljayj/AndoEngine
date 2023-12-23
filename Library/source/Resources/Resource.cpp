#include "Resources/Resource.h"
using namespace Reflection;

DEFINE_REFLECT_STRUCT(Resources, Resource)
	.Description("An object which can be shared between several other objects")
	.Variables({
		{ &Resources::Resource::id, "id"sv, "The unique identifier for this resource"sv, FVariableFlags::None() },
	});
