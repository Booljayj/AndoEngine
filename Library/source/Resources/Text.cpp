#include "Resources/Text.h"
#include "Resources/RegisteredResource.h"

DEFINE_STRUCT_REFLECTION_MEMBERS(Resources, Text)
	.Variables({
		MakeMember(&Resources::Text::string, "string"sv, "The raw string of text"sv),
	});

REGISTER_RESOURCE(Resources, Text);
