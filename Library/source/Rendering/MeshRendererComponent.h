#include "EntityFramework/EntityRegistry.h"

namespace Rendering {
	struct MeshRendererComponent {
		EntityID material;

		MeshRendererComponent(EntityID inMaterial)
		: material(inMaterial)
		{}
	};
}
