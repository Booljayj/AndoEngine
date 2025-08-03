#include "Rendering/Vulkan/RenderPasses.h"
#include "Engine/Logging.h"
#include "Engine/TemporaryStrings.h"
#include "Engine/Utility.h"
#include "Rendering/Vulkan/Handles.h"

namespace Rendering {
	Framebuffer::Framebuffer(VkDevice device, VkImageView view, VkFramebuffer framebuffer)
		: device(device), view(view), framebuffer(framebuffer)
	{}

	Framebuffer::~Framebuffer() {
		if (device) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
			vkDestroyImageView(device, view, nullptr);
		}
	}

	SurfaceRenderPass::FramebufferResources::FramebufferResources(VkDevice device, Swapchain const& swapchain, SurfaceRenderPass const& pass)
		: device(device)
	{
		//@todo Create shared image views (i.e. the depth pass image view)
		
		framebuffers.reserve(swapchain.GetNumImages());
		for (VkImage swapchainImage : swapchain.GetImages())
		{
			VkImageViewCreateInfo const viewCI{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = swapchainImage,
				//Image data settings
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = swapchain.GetSurfaceFormat().format,
				//Component swizzling settings
				.components = VkComponentMapping{
					.r = VK_COMPONENT_SWIZZLE_IDENTITY,
					.g = VK_COMPONENT_SWIZZLE_IDENTITY,
					.b = VK_COMPONENT_SWIZZLE_IDENTITY,
					.a = VK_COMPONENT_SWIZZLE_IDENTITY,
				},
				//Image usage settings
				.subresourceRange = VkImageSubresourceRange{
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1,
				},
			};

			auto imageViewHandle = ImageView::Create(device, viewCI, "Failed to create framebuffer image view");
			
			EnumArray<VkImageView, EAttachments> attachmentImageViews;
			attachmentImageViews[EAttachments::Color] = imageViewHandle;
			//attachmentImageViews[EAttachments::Depth] = sharedImageViews[ESharedAttachments::Depth];

			glm::u32vec2 const extent = swapchain.GetExtent();
			VkFramebufferCreateInfo const framebufferCI{
				.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
				.renderPass = pass,
				.attachmentCount = attachmentImageViews.size(),
				.pAttachments = attachmentImageViews.data(),
				.width = extent.x,
				.height = extent.y,
				.layers = 1,
			};

			VkFramebuffer framebuffer = nullptr;
			if (vkCreateFramebuffer(device, &framebufferCI, nullptr, &framebuffer) != VK_SUCCESS || !framebuffer) {
				throw FormatType<std::runtime_error>("Failed to create framebuffer");
			}

			framebuffers.emplace_back(device, imageViewHandle.Release(), framebuffer);
		}
	}

	SurfaceRenderPass::FramebufferResources::~FramebufferResources() {
		//@todo Destroy the shared image views
	}

	SurfaceRenderPass::ScopedRecord::ScopedRecord(VkCommandBuffer commands, SurfaceRenderPass const& surface, Framebuffer const& framebuffer, Geometry::ScreenRect const& rect)
	: cachedCommands(commands)
	{
		VkRenderPassBeginInfo const renderPassBI{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.renderPass = surface,
			.framebuffer = framebuffer,
			.renderArea = VkRect2D{
				.offset = VkOffset2D{ rect.offset.x, rect.offset.y },
				.extent = VkExtent2D{ rect.extent.x, rect.extent.y },
			},
			.clearValueCount = surface.clearValues.size(),
			.pClearValues = surface.clearValues.data(),
		};
		vkCmdBeginRenderPass(commands, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
	}

	SurfaceRenderPass::ScopedRecord::~ScopedRecord() {
		vkCmdEndRenderPass(cachedCommands);
	}

	SurfaceRenderPass::SurfaceRenderPass(Device const& inDevice, VkFormat format) 
		: device(inDevice)
	{
		// Attachments
		EnumArray<VkAttachmentDescription, EAttachments> descriptions;
		{
			descriptions[EAttachments::Color] = VkAttachmentDescription{
				.format = format,
				.samples = VK_SAMPLE_COUNT_1_BIT,
				.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
				.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
				.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
				.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
				.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
			};

			clearValues[EAttachments::Color].color = VkClearColorValue{ {0.0f, 0.0f, 0.0f, 1.0f} };
		}

		//Subpasses
		enum struct ESubpasses {
			Opaque,
			//Transparent,
			//UserInterface,
			MAX
		};
		EnumArray<VkSubpassDescription, ESubpasses> subpasses;

		enum struct EOpaqueSubpassReferences {
			Color,
			MAX
		};
		EnumArray<VkAttachmentReference, EOpaqueSubpassReferences> opaqueReferences;

		opaqueReferences[EOpaqueSubpassReferences::Color] = VkAttachmentReference{
			.attachment = std::to_underlying(EAttachments::Color),
			.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};

		subpasses[ESubpasses::Opaque] = VkSubpassDescription{
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.colorAttachmentCount = opaqueReferences.size(),
			.pColorAttachments = opaqueReferences.data(),
		};

		//Subpass Dependencies
		enum struct EDependencies {
			ExternalToOpaque,
			MAX
		};
		EnumArray<VkSubpassDependency, EDependencies> dependencies;
		
		dependencies[EDependencies::ExternalToOpaque] = VkSubpassDependency{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = std::to_underlying(ESubpasses::Opaque),
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		};

		//Information to create the full render pass, with all relevant attachments and subpasses.
		VkRenderPassCreateInfo const passCI{
			.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = descriptions.size(),
			.pAttachments = descriptions.data(),
			.subpassCount = subpasses.size(),
			.pSubpasses = subpasses.data(),
			.dependencyCount = dependencies.size(),
			.pDependencies = dependencies.data(),
		};

		if (vkCreateRenderPass(device, &passCI, nullptr, &pass) != VK_SUCCESS || !pass) {
			LOG(Vulkan, Error, "Failed to create main render pass");
			throw FormatType<std::runtime_error>("Failed to create surface render pass");
		}
	}

	SurfaceRenderPass::~SurfaceRenderPass() {
		if (device) vkDestroyRenderPass(device, pass, nullptr);
	}

	RenderPasses::RenderPasses(Device const& inDevice, VkFormat format)
		: surface(inDevice, format)
	{}

	Framebuffers::Framebuffers(VkDevice device, Swapchain const& swapchain, RenderPasses const& passes)
		: surface(device, swapchain, passes.surface)
	{}
}
