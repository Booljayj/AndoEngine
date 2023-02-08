#pragma once
#include "EntityFramework/EntityRegistry.h"
#include "Resources/Resource.h"

namespace Rendering {
	struct Material;

	struct MeshRendererComponent {
		Resources::Handle<Material> material;
		EntityID mesh;
	};
}
