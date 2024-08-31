#pragma once
#include "Engine/Reflection.h"
#include "Resources/Resource.h"

namespace Resources {
	/** A generic text string resource */
	struct Text : public Resources::Resource {
		REFLECT_STRUCT(Text, Resources::Resource);
		using Resources::Resource::Resource;

		std::string string;
	};
}

REFLECT(Resources::Text, Struct);
DEFINE_REFLECTED_SERIALIZATION(Resources::Text);
