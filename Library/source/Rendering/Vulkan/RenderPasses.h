#pragma once
#include "Engine/Array.h"
#include "Engine/Core.h"
#include "Engine/EnumArray.h"
#include "Engine/MoveOnly.h"
#include "Geometry/ScreenRect.h"
#include "Rendering/Views/ViewRect.h"
#include "Rendering/Vulkan/Device.h"
#include "Rendering/Vulkan/GraphicsQueue.h"
#include "Rendering/Vulkan/Swapchain.h"
#include "Rendering/Vulkan/Vulkan.h"

/**
 * At a high level, each render pass is defined using a type. Static members of that type provide information about the render pass,
 * and instances of the type are equivalent to an instance of the render pass. render-pass-dependent objects such as framebuffers
 * also exist as unique types within the render pass type.
 */

namespace Rendering {
	struct Framebuffer {
	public:
		inline operator VkFramebuffer() const { return framebuffer; }

		Framebuffer(VkDevice device, VkImageView view, VkFramebuffer framebuffer);
		Framebuffer(Framebuffer const&) = delete;
		Framebuffer(Framebuffer&& other) noexcept = default;
		~Framebuffer();

	private:
		MoveOnly<VkDevice> device;
		VkImageView view = nullptr;
		VkFramebuffer framebuffer = nullptr;
	};

	/** The primary render pass responsible for rendering surfaces on 3D geometry */
	struct SurfaceRenderPass {
		enum struct EAttachments {
			Color,
			//Normal,
			//Depth,
			MAX
		};
		
		enum struct ESharedAttachments {
			//Normal,
			Depth,
			MAX
		};
		using SharedAttachmentImageViews = EnumArray<VkImageView, ESharedAttachments>;

		struct FramebufferResources {
		public:
			const Framebuffer& operator[](size_t index) const { return framebuffers[index]; }

			FramebufferResources(VkDevice device, Swapchain const& swapchain, SurfaceRenderPass const& pass);
			FramebufferResources(FramebufferResources const&) = delete;
			FramebufferResources(FramebufferResources&& other) noexcept = default;
			~FramebufferResources();

		private:
			MoveOnly<VkDevice> device;
			SharedAttachmentImageViews sharedImageViews;
			std::vector<Framebuffer> framebuffers;
		};

		/** Created to mark a scope within which commands are recorded for the pass */
		struct ScopedRecord {
		public:
			ScopedRecord(GraphicsCommandWriter const& commands, SurfaceRenderPass const& surface, Framebuffer const& framebuffer, ViewRect const& rect);
			~ScopedRecord();
		private:
			VkCommandBuffer cachedCommands;
		};

		inline operator VkRenderPass() const { return pass; }

		SurfaceRenderPass(Device const& inDevice, VkFormat format);
		SurfaceRenderPass(const SurfaceRenderPass&) = delete;
		SurfaceRenderPass(SurfaceRenderPass&&) noexcept = default;
		~SurfaceRenderPass();

	private:
		MoveOnly<VkDevice> device;
		VkRenderPass pass = nullptr;
		EnumArray<VkClearValue, EAttachments> clearValues;
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
	struct RenderPasses {
		SurfaceRenderPass surface;
		//PostProcessRenderPass postProcess;

		RenderPasses(Device const& device, VkFormat format);
		RenderPasses(RenderPasses const&) = delete;
		RenderPasses(RenderPasses&&) = default;
	};

	/** Framebuffers which can be used with different render passes */
	struct Framebuffers {
		SurfaceRenderPass::FramebufferResources surface;
		//PostProcessRenderPass::Framebuffers postProcess;

		Framebuffers(VkDevice device, Swapchain const& swapchain, RenderPasses const& passes);
		Framebuffers(const Framebuffers&) = delete;
		Framebuffers(Framebuffers&&) = default;

		inline Framebuffers& operator=(Framebuffers&&) = default;
	};
}
