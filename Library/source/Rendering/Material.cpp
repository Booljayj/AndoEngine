#include "Rendering/Material.h"
#include "Rendering/Shader.h"
#include "Resources/RegisteredResource.h"

::Reflection::StructTypeInfo const& ::Reflect<Rendering::Material>::Get() { return Rendering::Material::info_Material; }
::Reflection::TStructTypeInfo<Rendering::Material> const Rendering::Material::info_Material{
	u"Rendering::Material"sv, u"Describes a method of rendering geometry. Also sometimes called a graphics pipeline."sv, std::in_place_type<Rendering::Material::BaseType>,
	{
		MakeNestedMember(&Material::shaders, &decltype(Material::shaders)::vertex, "shaders::vertex"_h32, u"shaders::vertex"sv, u"the vertex shader used by the material"sv),
		MakeNestedMember(&Material::shaders, &decltype(Material::shaders)::fragment, "shaders::fragment"_h32, u"shaders::fragment"sv, u"the fragment shader used by the material"sv),
	}
};

REGISTER_RESOURCE(Rendering, Material);
