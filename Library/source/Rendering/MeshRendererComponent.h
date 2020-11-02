#include "EntityFramework/EntityRegistry.h"

namespace Rendering {
	struct MeshRendererComponent {
		EntityID material;

		static void OnCreate(entt::registry& registry, entt::entity entity);
		static void OnDestroy(entt::registry& registry, entt::entity entity);

		MeshRendererComponent(EntityID inMaterial)
		: material(inMaterial)
		{}
	};
}
