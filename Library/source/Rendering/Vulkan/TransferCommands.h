#pragma once
#include "Engine/Core.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	struct TransferCommandWriter {
		template<typename T, size_t Size>
		using Span = std::span<T const, Size>;

		static constexpr size_t Dynamic = std::dynamic_extent;

		TransferCommandWriter(VkCommandBuffer commands);
		TransferCommandWriter(TransferCommandWriter const&) = delete;
		TransferCommandWriter(TransferCommandWriter&&) = delete;
		~TransferCommandWriter();

		template<size_t Size = Dynamic>
		void Copy(VkBuffer source, VkBuffer destination, Span<VkBufferCopy, Size> regions) {
			vkCmdCopyBuffer(commands, source, destination, regions.size(), regions.data());
		}

	private:
		VkCommandBuffer commands;
	};
}
