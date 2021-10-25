#pragma once
#include "Geometry/GLM.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** Object for uniforms that are modified per-frame */
	struct GlobalUniforms {
		glm::mat4 viewProjection = glm::identity<glm::mat4>();
		glm::mat4 viewProjectionInverse = glm::identity<glm::mat4>();
		float time = 0;

		static VkDescriptorSetLayoutBinding GetBinding();
	};

	/** Object for uniforms that are modified per-draw-call */
	struct ObjectUniforms {
		glm::mat4 modelViewProjection = glm::identity<glm::mat4>();

		static VkDescriptorSetLayoutBinding GetBinding();
	};
}
