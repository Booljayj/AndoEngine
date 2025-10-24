#pragma
#include "Engine/Threads.h"
#include "Rendering/Views/View.h"
#include "ThirdParty/EnTT.h"

namespace Rendering {
	struct Framebuffer;
	struct FrameResources;
	struct RenderPasses;
	struct ResourcesCollection;

	/** A target which can perform rendering based on a set of configured views */
	struct RenderTarget {
		/** The views that define what is rendered to this target */
		ThreadSafe<std::vector<View>> ts_views;

		/** Render the renderable entities from the registry to this target */
		bool Render(RenderPasses const& passes, entt::registry const& registry, ResourcesCollection& previous_resources);

		/** Get the total extent of the renderable area of this target */
		virtual glm::u32vec2 GetExtent() const = 0;

	protected:
		virtual Framebuffer const& GetFramebuffer(uint32_t image_index) const = 0;

		virtual FrameResources* PrepareFrame(std::span<ViewRenderingParameters> view_params, ResourcesCollection& previous_resources) = 0;
		virtual void SubmitFrame(FrameResources& frame) = 0;
	};
}
