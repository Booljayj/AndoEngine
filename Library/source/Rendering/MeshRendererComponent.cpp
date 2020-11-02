#include "Rendering/MeshRendererComponent.h"
#include "Rendering/RenderingSystem.h"

namespace Rendering {
	void MeshRendererComponent::OnCreate(entt::registry& registry, entt::entity entity) {
		RenderingSystem& rendering = *registry.ctx<RenderingSystem*>();
		rendering.shouldRebuildCommandBuffers = true;
	}

	void MeshRendererComponent::OnDestroy(entt::registry& registry, entt::entity entity) {
		RenderingSystem& rendering = *registry.ctx<RenderingSystem*>();
		rendering.shouldRebuildCommandBuffers = true;
	}
}
