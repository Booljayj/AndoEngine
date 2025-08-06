#include "Rendering/StaticMesh.h"
#include "Resources/RegisteredResource.h"

::Reflection::StructTypeInfo const& ::Reflect<Rendering::StaticMesh>::Get() { return Rendering::StaticMesh::info_StaticMesh; }
::Reflection::TStructTypeInfo<Rendering::StaticMesh> const Rendering::StaticMesh::info_StaticMesh{
	u"Rendering::StaticMesh"sv, u"Static Mesh Resource"sv, std::in_place_type<Rendering::StaticMesh::BaseType>,
	{

	}
};

REGISTER_RESOURCE(Rendering, StaticMesh);
