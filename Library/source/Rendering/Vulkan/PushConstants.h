#pragma once
#include "Engine/GLM.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	namespace Concepts {
		template<typename T>
		concept PushConstantsStruct =
			std::is_trivially_destructible_v<T> and //Push constants will be copied byte-by-byte into GPU memory, so they should be trivially destructible.
			sizeof(T) <= 128; //The minimum supported size for push constants is 128. Some systems may support larger, but that cannot be guaranteed.
	}

	struct MeshPushConstants {
		glm::mat4 mvp_matrix;
		VkDeviceAddress buffer_address;
	};
	static_assert(Concepts::PushConstantsStruct<MeshPushConstants>);
}
