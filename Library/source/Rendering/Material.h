#pragma once
#include "Rendering/Vertex.h"
#include "Rendering/Vulkan/VulkanResources.h"
#include "Resources/Resource.h"

namespace Rendering {
	struct RenderingSystem;
	struct VertexShader;
	struct FragmentShader;

	/** Describes a method of rendering geometry, also called a GraphicsPipeline */
	struct Material : public Resources::Resource {
		REFLECT_STRUCT(Material, Resources::Resource);

		/** The vertex shader used to create this material */
		Resources::Handle<VertexShader> vertex;
		/** The fragment shader used to create this material */
		Resources::Handle<FragmentShader> fragment;

		/** Rendering resources for this material */
		VulkanPipelineResources resources;

		Material(Resources::Identifier id) : Resources::Resource(id) {}
	};
}

REFLECT(Rendering::Material, Struct);
