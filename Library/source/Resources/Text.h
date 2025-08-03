#pragma once
#include "Engine/Reflection.h"
#include "Resources/Resource.h"

namespace Resources {
	/** A generic text string resource */
	struct Text : public Resources::Resource {
		DECLARE_STRUCT_REFLECTION_MEMBERS(Text, Resources::Resource);
		using Resources::Resource::Resource;

		std::string string;
	};
}

REFLECT(Resources::Text, Struct);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(Resources::Text);
DEFINE_DEFAULT_YAML_SERIALIZATION(Resources::Text);
