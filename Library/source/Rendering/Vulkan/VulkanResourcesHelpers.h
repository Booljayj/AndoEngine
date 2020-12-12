#include "Engine/Context.h"
#include "Engine/Hash.h"
#include "Rendering/Vulkan/VulkanCommon.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanResources.h"

namespace Rendering {
	/** A library of loaded shader modules which can be re-used to create pipelines */
	struct VulkanShaderModuleLibrary {
	public:
		VulkanShaderModuleLibrary(VkDevice inDevice);
		~VulkanShaderModuleLibrary();

		/** Get an already-loaded module, or load a new one */
		VkShaderModule GetModule(CTX_ARG, std::string_view name);

	private:
		struct Entry {
			Hash32 hash;
			VkShaderModule module;
		};

		VkDevice device;
		std::vector<Entry> entries;
	};

	struct VulkanMeshCreationHelper {
		std::vector<VulkanMeshCreationResults> results;
		size_t flushIterations = 0;

		VulkanLogicalDevice const* logical = nullptr;
		VkCommandPool pool = nullptr;

		VulkanMeshCreationHelper(VulkanLogicalDevice const& inLogical, VkCommandPool inPool)
		: logical(&inLogical)
		, pool(inPool)
		{}

		void Submit(VulkanMeshCreationResults const& result);
		void Flush();
	};
}
