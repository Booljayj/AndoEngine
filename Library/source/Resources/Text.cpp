#include "Resources/Text.h"
#include "Resources/RegisteredResource.h"

::Reflection::StructTypeInfo const& ::Reflect<Resources::Text>::Get() { return Resources::Text::info_Text; }
::Reflection::TStructTypeInfo<Resources::Text> const Resources::Text::info_Text{
	u"Resources::Text"sv, u"Text Resource"sv, std::in_place_type<Resources::Text::BaseType>,
	{
		MakeMember(&Resources::Text::string, "string"_h32, u"string"sv, u"The raw string of text"sv)
	}
};

REGISTER_RESOURCE(Resources, Text);
