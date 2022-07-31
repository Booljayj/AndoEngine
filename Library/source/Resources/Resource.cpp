#include "Resources/Resource.h"

DEFINE_STRUCT_REFLECTION_MEMBERS(Resources, Resource)
	.Description("An object which can be shared among several other objects")
	.Variables({
		REFLECT_MVAR(Resources::Resource, id, ""),
		REFLECT_MVAR(Resources::Resource, name, "")
	});
