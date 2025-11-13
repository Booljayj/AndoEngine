#pragma once
#include "Engine/Array.h"
#include "Engine/Core.h"
#include "Engine/GLM.h"
#include "Rendering/Views/View.h"

namespace Rendering {
	struct GraphicsPipelineResources;
	struct MeshResources;

	struct StaticMeshParameters {
		std::shared_ptr<GraphicsPipelineResources> pipeline;
		std::shared_ptr<MeshResources> mesh;
	};

	struct ThreadCullingContext {
		std::vector<StaticMeshParameters> static_meshes;
	};

	struct ViewCullingContext {
		/** The frustum for this view, calculated based on the camera */
		glm::mat4 frustum;

		std::vector<ThreadCullingContext> threads;
		std::vector<std::jthread> workers;

		void Prepare(ViewParameters view_parameters) {}
	};

	struct CullingContext {
		std::vector<ViewCullingContext> views;

		void Prepare(std::span<ViewParameters> view_parameters) {}
	};
}
