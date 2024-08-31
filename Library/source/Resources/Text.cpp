#include "Resources/Text.h"
#include "Resources/RegisteredResource.h"

DEFINE_REFLECT_STRUCT(Resources, Text)
	.Variables({
		{ &Resources::Text::string, "string"sv, "The raw string of text"sv },
	});

REGISTER_RESOURCE(Resources, Text);
