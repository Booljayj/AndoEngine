#pragma once
#include "Engine/GLM.h"
#include "ThirdParty/EnTT.h"

namespace Rendering {
	/** Parameters used to define the view-perspective matrix for rendering */
	struct ViewCamera {
		/** The transform that provides the location, rotation, and scale of the matrix */
		glm::mat4 transform = glm::identity<glm::mat4>();
		/** The vertical field-of-view */
		float fov = 90.0f;
		/** The aspect ratio to render with */
		float aspect = 1.0f;
		/** The near and far clip distances from the matrix location */
		struct {
			float near = 0.01f;
			float far = 10000.0f;
		} clip;
	};

	struct ViewCameraCalculator {
		virtual ~ViewCameraCalculator() = default;
		virtual ViewCamera Calculate(entt::registry const& registry, glm::uvec2 draw_size) const = 0;
	};

	/** Used in the editor where view parameters are provided by EditorCamera components */
	struct EditorViewCameraCalculator : public ViewCameraCalculator {
		virtual ViewCamera Calculate(entt::registry const& registry, glm::uvec2 draw_size) const override;
	};
}
