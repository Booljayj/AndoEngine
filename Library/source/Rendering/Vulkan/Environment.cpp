#include "Rendering/Vulkan/Environment.h"

namespace Rendering {
	Environment::Environment() {
		uint32_t num_layers = 0;
		vkEnumerateInstanceLayerProperties(&num_layers, nullptr);
		supported_layers.resize(num_layers);
		vkEnumerateInstanceLayerProperties(&num_layers, supported_layers.data());

		uint32_t num_extensions = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &num_extensions, nullptr);
		supported_extensions.resize(num_extensions);
		vkEnumerateInstanceExtensionProperties(nullptr, &num_extensions, supported_extensions.data());
	}

	bool Environment::SupportsLayer(char const* layer_name) const {
		const auto MatchesLayerName = [layer_name](VkLayerProperties const& layer) { return strcmp(layer.layerName, layer_name) == 0; };
		return ranges::any_of(supported_layers, MatchesLayerName);
	}

	bool Environment::SupportsExtension(char const* extension_name) const {
		const auto MatchesExtensionName = [extension_name](VkExtensionProperties const& extension) { return strcmp(extension.extensionName, extension_name) == 0; };
		return ranges::any_of(supported_extensions, MatchesExtensionName);
	}
}
