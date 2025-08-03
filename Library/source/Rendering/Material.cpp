#include "Rendering/Material.h"
#include "Rendering/Shader.h"
#include "Resources/RegisteredResource.h"

DEFINE_STRUCT_REFLECTION_MEMBERS(Rendering, Material)
	.Description("Describes a method of rendering geometry. Also sometimes called a graphics pipeline.")
	.Variables({
		MakeNestedMember(&Material::shaders, &decltype(Material::shaders)::vertex, "shaders::vertex"sv, "the vertex shader used by the material"sv),
		MakeNestedMember(&Material::shaders, &decltype(Material::shaders)::fragment, "shaders::fragment"sv, "the fragment shader used by the material"sv),
	});

REGISTER_RESOURCE(Rendering, Material);
