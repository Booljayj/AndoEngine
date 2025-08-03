#pragma once
#include "Rendering/Vertex.h"
#include "Rendering/Vulkan/Resources.h"
#include "Resources/Resource.h"

namespace Rendering {
	struct RenderingSystem;
	struct VertexShader;
	struct FragmentShader;

	/** Describes a method of rendering geometry, also called a GraphicsPipeline */
	struct Material : public Resources::Resource {
		DECLARE_STRUCT_REFLECTION_MEMBERS(Material, Resources::Resource);
		using Resources::Resource::Resource;

		struct {
			Resources::Handle<VertexShader> vertex;
			Resources::Handle<FragmentShader> fragment;
		} shaders;

		std::shared_ptr<GraphicsPipelineResources> objects;
	};
}

REFLECT(Rendering::Material, Struct);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(Rendering::Material);
DEFINE_DEFAULT_YAML_SERIALIZATION(Rendering::Material);
