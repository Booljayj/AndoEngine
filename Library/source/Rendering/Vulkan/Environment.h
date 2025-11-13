#pragma once
#include "Engine/Core.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** Contains information about the vulkan environment, such as the vulkan version used to compile the executable */
	struct Environment {
		Environment();
		Environment(Environment const&) = delete;
		Environment(Environment&&) = default;

		std::vector<VkLayerProperties> supported_layers;
		std::vector<VkExtensionProperties> supported_extensions;
		
		bool SupportsLayer(char const* layer_name) const;
		bool SupportsExtension(char const* extension_name) const;
	};
}
