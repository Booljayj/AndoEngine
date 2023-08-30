#pragma once
#include "Engine/ArrayView.h"
#include "Engine/StandardTypes.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct DescriptorPool {
	public:
		DescriptorPool(VkDevice inDevice, TArrayView<VkDescriptorPoolSize> sizes, size_t maxNumSets);
		DescriptorPool(DescriptorPool const&) = delete;
		DescriptorPool(DescriptorPool&&) noexcept;
		~DescriptorPool();

		inline operator VkDescriptorPool() const { return pool; }

	private:
		VkDevice device = nullptr;
		VkDescriptorPool pool = nullptr;
	};
}
