#pragma once
#include "EntityFramework/EntityRegistry.h"
#include "Rendering/Vertex.h"
#include "Rendering/Vulkan/VulkanResources.h"

namespace Rendering {
	/** Describes a method of rendering geometry */
	struct MaterialComponent {
		/** TEMP Names of files to load to create vertex and fragment shader modules. Should be replaced by components that contain that information */
		std::string vertex;
		std::string fragment;

		/** Rendering resources for this material */
		VulkanPipelineResources resources;
	};

	/** Combines a material with a set of parameters the material will use when rendering */
	struct MaterialInstanceComponent {
		EntityID material;
	};
}
