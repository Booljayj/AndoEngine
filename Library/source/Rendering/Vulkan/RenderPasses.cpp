#include "Rendering/Vulkan/RenderPasses.h"
#include "Engine/Logging.h"
#include "Engine/Utility.h"

namespace Rendering {
	Framebuffer::Framebuffer(VkDevice inDevice, VkImageView inView, VkFramebuffer inFramebuffer)
		: device(inDevice), view(inView), framebuffer(inFramebuffer)
	{}

	Framebuffer::Framebuffer(Framebuffer&& other) noexcept
		: device(other.device), view(other.view), framebuffer(other.framebuffer)
	{
		other.device = nullptr;
	}

	Framebuffer::~Framebuffer() {
		if (device) {
			vkDestroyFramebuffer(device, framebuffer, nullptr);
			vkDestroyImageView(device, view, nullptr);
		}
	}

	SurfaceRenderPass::FramebufferResources::FramebufferResources(VulkanLogicalDevice const& logical, Swapchain const& swapchain, SurfaceRenderPass const& pass)
		: device(logical)
	{
		//@todo Create shared image views (i.e. the depth pass image view)
		
		framebuffers.reserve(swapchain.GetNumImages());
		for (VkImage swapchainImage : swapchain.GetImages())
		{
			VkImageViewCreateInfo viewCI{};
			viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			viewCI.image = swapchainImage;
			//Image data settings
			viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			viewCI.format = swapchain.GetSurfaceFormat().format;
			//Component swizzling settings
			viewCI.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCI.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCI.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
			viewCI.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
			//Image usage settings
			viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			viewCI.subresourceRange.baseMipLevel = 0;
			viewCI.subresourceRange.levelCount = 1;
			viewCI.subresourceRange.baseArrayLayer = 0;
			viewCI.subresourceRange.layerCount = 1;

			VkImageView imageView = nullptr;
			if (vkCreateImageView(device, &viewCI, nullptr, &imageView) != VK_SUCCESS || !imageView) {
				LOGF(Vulkan, Error, "Failed to create image view");
				throw std::runtime_error{ "Failed to create framebuffer image view" };
			}

			EnumBackedContainer<VkImageView, EAttachments> attachmentImageViews;
			attachmentImageViews[EAttachments::Color] = imageView;
			//attachmentImageViews[EAttachments::Depth] = sharedImageViews[ESharedAttachments::Depth];

			VkFramebufferCreateInfo framebufferCI{};
			framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCI.renderPass = pass;
			framebufferCI.attachmentCount = attachmentImageViews.size();
			framebufferCI.pAttachments = attachmentImageViews.data();
			swapchain.GetExtent(framebufferCI.width, framebufferCI.height);
			framebufferCI.layers = 1;

			VkFramebuffer framebuffer = nullptr;
			if (vkCreateFramebuffer(device, &framebufferCI, nullptr, &framebuffer) != VK_SUCCESS || !framebuffer) {
				LOGF(Vulkan, Error, "Failed to create frambuffer");
				vkDestroyImageView(device, imageView, nullptr);
				throw std::runtime_error{ "Failed to create framebuffer" };
			}

			framebuffers.emplace_back(device, imageView, framebuffer);
		}
	}

	SurfaceRenderPass::FramebufferResources::FramebufferResources(FramebufferResources&& other) noexcept
		: device(other.device), sharedImageViews(other.sharedImageViews), framebuffers(std::move(other.framebuffers))
	{
		other.device = nullptr;
	}

	SurfaceRenderPass::FramebufferResources::~FramebufferResources() {
		//@todo Destroy the shared image views
	}

	SurfaceRenderPass::ScopedRecord::ScopedRecord(VkCommandBuffer commands, SurfaceRenderPass const& surface, Framebuffer const& framebuffer, Geometry::ScreenRect const& rect)
	: cachedCommands(commands)
	{
		VkRenderPassBeginInfo renderPassBI{};
		renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBI.renderPass = surface;
		renderPassBI.framebuffer = framebuffer;
		renderPassBI.renderArea.offset = VkOffset2D{ rect.offset.x, rect.offset.y };
		renderPassBI.renderArea.extent = VkExtent2D{ rect.extent.x, rect.extent.y };
		
		renderPassBI.clearValueCount = surface.clearValues.size();
		renderPassBI.pClearValues = surface.clearValues.data();

		vkCmdBeginRenderPass(commands, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
	}

	SurfaceRenderPass::ScopedRecord::~ScopedRecord() {
		vkCmdEndRenderPass(cachedCommands);
	}

	SurfaceRenderPass::SurfaceRenderPass(VulkanLogicalDevice const& logical, VkFormat format) 
		: device(logical)
	{
		// Attachments
		EnumBackedContainer<VkAttachmentDescription, EAttachments> descriptions;
		{
			VkAttachmentDescription& attachment = descriptions[EAttachments::Color];
			attachment.format = format;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			clearValues[EAttachments::Color].color = VkClearColorValue{ {0.0f, 0.0f, 0.0f, 1.0f} };
		}

		//Subpasses
		enum struct ESubpasses {
			Opaque,
			//Transparent,
			//UserInterface,
			MAX
		};
		EnumBackedContainer<VkSubpassDescription, ESubpasses> subpasses;

		enum struct EOpaqueSubpassReferences {
			Color,
			MAX
		};
		EnumBackedContainer<VkAttachmentReference, EOpaqueSubpassReferences> opaqueReferences;
		{
			VkSubpassDescription& subpass = subpasses[ESubpasses::Opaque];
			{
				VkAttachmentReference& reference = opaqueReferences[EOpaqueSubpassReferences::Color];
				reference.attachment = IndexOfEnum(EAttachments::Color);
				reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}

			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = opaqueReferences.size();
			subpass.pColorAttachments = opaqueReferences.data();
		}

		//Subpass Dependencies
		enum struct EDependencies {
			ExternalToOpaque,
			MAX
		};
		EnumBackedContainer<VkSubpassDependency, EDependencies> dependencies;
		{
			VkSubpassDependency& dependency = dependencies[EDependencies::ExternalToOpaque];
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = IndexOfEnum(ESubpasses::Opaque);
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		//Information to create the full render pass, with all relevant attachments and subpasses.
		VkRenderPassCreateInfo passCI{};
		passCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		passCI.attachmentCount = descriptions.size();
		passCI.pAttachments = descriptions.data();
		passCI.subpassCount = subpasses.size();
		passCI.pSubpasses = subpasses.data();
		passCI.dependencyCount = dependencies.size();
		passCI.pDependencies = dependencies.data();

		if (vkCreateRenderPass(device, &passCI, nullptr, &pass) != VK_SUCCESS || !pass) {
			LOG(Vulkan, Error, "Failed to create main render pass");
			throw std::runtime_error("Failed to create surface render pass");
		}
	}

	SurfaceRenderPass::SurfaceRenderPass(SurfaceRenderPass&& other) noexcept
		: device(other.device), pass(other.pass)
	{
		other.device = nullptr;
	}

	SurfaceRenderPass::~SurfaceRenderPass() {
		if (device) vkDestroyRenderPass(device, pass, nullptr);
	}

	RenderPasses::RenderPasses(VulkanLogicalDevice const& logical, VkFormat format)
		: surface(logical, format)
	{}

	VulkanFramebuffers::VulkanFramebuffers(VulkanLogicalDevice const& logical, Swapchain const& swapchain, RenderPasses const& passes)
		: surface(logical, swapchain, passes.surface)
	{}
}
