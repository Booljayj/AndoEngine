#pragma once
#include "Engine/ArrayView.h"
#include "Engine/StandardTypes.h"
#include "Geometry/ScreenRect.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"

/**
 * At a high level, each render pass is defined using a type. Static members of that type provide information about the render pass,
 * and instances of the type are equivalent to an instance of the render pass. render-pass-dependent objects such as framebuffers
 * also exist as unique types within the render pass type.
 */

namespace Rendering {
	struct SurfaceRenderPass {
		struct EAttachments { enum : uint8_t {
			Color,
			//Depth,
			//Normal,
			MAX
		};};
		using AttachmentImageViews = VkImageView[EAttachments::MAX];

		struct ESubpasses { enum : uint8_t {
			Opaque,
			//Transparent,
			//UserInterface,
			MAX
		};};

		struct EDependencies { enum : uint8_t {
			ExternalToOpaque,
			MAX
		};};

		/** A framebuffer which can be used with this pass */
		struct Framebuffer {
			VkFramebuffer internal = nullptr;
		};
		using Framebuffers = std::vector<Framebuffer>;

		/** Created to mark a scope within which commands are recorded for the pass */
		struct ScopedRecord {
		public:
			ScopedRecord(VkCommandBuffer commands, SurfaceRenderPass const& surface, Framebuffer framebuffer, Geometry::ScreenRect const& rect);
			~ScopedRecord();
		private:
			VkCommandBuffer cachedCommands;
		};

		VkRenderPass pass = nullptr;
		VkClearValue clearValues[EAttachments::MAX];

		static void DestroyFramebuffers(VulkanLogicalDevice const& logical, Framebuffers& framebuffers);

		bool Create(VulkanLogicalDevice const& logical, VkFormat format);
		void Destroy(VulkanLogicalDevice const& logical);

		bool CreateFramebuffers(VulkanLogicalDevice const& logical, AttachmentImageViews const& sharedImageViews, TArrayView<VkImageView> colorImageViews, glm::u32vec2 const& size, Framebuffers& framebuffers) const;
	};

	// struct PostProcessRenderPass {
	// 	struct EAttachments { enum : uint8_t {
	// 		Screen,
	// 		MAX
	// 	};};
	//
	// 	struct ESubpasses { enum : uint8_t {
	// 		Main,
	// 		MAX
	// 	};};
	//
	// 	struct EDependencies { enum : uint8_t {
	// 		ExternalToMain,
	// 		MAX
	// 	};};
	//
	// 	VkRenderPass pass;
	// 	VkClearValue clearValues[EAttachments::MAX];
	//
	// 	bool Create(VulkanLogicalDevice const& logical, VkFormat format);
	// 	void Destroy(VulkanLogicalDevice const& logical);
	// };

	/** Render passes */
	struct VulkanRenderPasses {
		SurfaceRenderPass surface;
		//PostProcessRenderPass postProcess;

		bool Create(VulkanLogicalDevice const& logical, VkFormat format);
		void Destroy(VulkanLogicalDevice const& logical);
	};

	/** Framebuffers which can be used with different render passes */
	struct VulkanFramebuffers {
		SurfaceRenderPass::Framebuffers surface;
		//PostProcessRenderPass::Framebuffers postProcess;

		void Destroy(VulkanLogicalDevice const& logical);
	};
}
