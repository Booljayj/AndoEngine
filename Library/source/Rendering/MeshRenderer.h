#pragma once
#include "Resources/Resource.h"

namespace Rendering {
	struct Material;
	struct StaticMesh;

	struct MeshRenderer {
		Resources::Handle<Material> material;
		Resources::Handle<StaticMesh> mesh;
	};
}
