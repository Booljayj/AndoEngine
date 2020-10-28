#include "EntityFramework/EntityRegistry.h"
#include "Rendering/Vulkan/VulkanCommon.h"

namespace Rendering {
	struct MaterialComponent {
		/** TEMP Names of files to load to create vertex and fragment shader modules. Should be replaced by components that contain that information */
		std::string vertex;
		std::string fragment;

		VkPipeline pipeline;
		VkPipelineLayout pipelineLayout;

		static void OnCreate(entt::registry& registry, entt::entity entity);
		static void OnDestroy(entt::registry& registry, entt::entity entity);

		MaterialComponent(std::string_view inVertex, std::string_view inFragment)
		: vertex(inVertex), fragment(inFragment), pipeline(nullptr), pipelineLayout(nullptr)
		{}
	};
}
