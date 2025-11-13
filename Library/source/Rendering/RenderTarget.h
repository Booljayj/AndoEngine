#pragma
#include "Engine/Threads.h"
#include "Engine/Logging.h"
#include "Rendering/Views/View.h"

namespace Rendering {
	struct Framebuffer;
	struct FrameContext;
	struct RenderPasses;
	struct ResourcesCollection;
	
	/** A target which can perform rendering based on a set of configured views */
	struct RenderTarget {
		/** The views that define what is rendered to this target */
		ThreadSafe<std::vector<View>> ts_views;

		/** Render the renderable entities from the registry to this target */
		bool Render(RenderPasses const& passes, ResourcesCollection& previous_resources);

	protected:
		DECLARE_LOG_CATEGORY_MEMBER(RenderTarget);

		/** The total extent of this render target */
		glm::u32vec2 extent = glm::zero<glm::u32vec2>();

		virtual Framebuffer const& GetFramebuffer(uint32_t image_index) const = 0;

		virtual FrameContext* GetNextFrameContext() = 0;
		virtual void SubmitFrameContext(FrameContext const& frame) = 0;

	private:
		std::vector<ViewParameters> views_parameters;
	};
}
