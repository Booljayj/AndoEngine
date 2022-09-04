#include "Rendering/Vulkan/VulkanRenderPasses.h"
#include "Engine/Logging.h"

namespace Rendering {
	SurfaceRenderPass::ScopedRecord::ScopedRecord(VkCommandBuffer commands, SurfaceRenderPass const& surface, Framebuffer framebuffer, Geometry::ScreenRect const& rect)
	: cachedCommands(commands)
	{
		VkRenderPassBeginInfo renderPassBI{};
		renderPassBI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBI.renderPass = surface.pass;
		renderPassBI.framebuffer = framebuffer.internal;
		renderPassBI.renderArea.offset = VkOffset2D{ rect.offset.x, rect.offset.y };
		renderPassBI.renderArea.extent = VkExtent2D{ rect.extent.x, rect.extent.y };

		renderPassBI.clearValueCount = std::size(surface.clearValues);
		renderPassBI.pClearValues = surface.clearValues;

		vkCmdBeginRenderPass(commands, &renderPassBI, VK_SUBPASS_CONTENTS_INLINE);
	}

	SurfaceRenderPass::ScopedRecord::~ScopedRecord() {
		vkCmdEndRenderPass(cachedCommands);
	}

	void SurfaceRenderPass::DestroyFramebuffers(VulkanLogicalDevice const& logical, Framebuffers& framebuffers) {
		for (const Framebuffer& framebuffer : framebuffers) {
			vkDestroyFramebuffer(logical.device, framebuffer.internal, nullptr);
		}
		framebuffers.clear();
	}

	bool SurfaceRenderPass::Create(VulkanLogicalDevice const& logical, VkFormat format) {
		//Attachments
		std::array<VkAttachmentDescription, EAttachments::MAX> descriptions;
		std::memset(&descriptions, 0, sizeof(descriptions));
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

			clearValues[EAttachments::Color].color = VkClearColorValue{{0.0f, 0.0f, 0.0f, 1.0f}};
		}

		//Subpasses
		std::array<VkSubpassDescription, ESubpasses::MAX> subpasses;
		std::memset(&subpasses, 0, sizeof(subpasses));
		{
			VkSubpassDescription& subpass = subpasses[ESubpasses::Opaque];

			//Attachment References
			struct EReferences { enum : uint8_t {
				Color,
				MAX
			};};

			std::array<VkAttachmentReference, EReferences::MAX> references;
			std::memset(&references, 0, sizeof(references));
			{
				VkAttachmentReference& reference = references[EReferences::Color];
				reference.attachment = EAttachments::Color;
				reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}

			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = references.size();
			subpass.pColorAttachments = references.data();
		}

		//Subpass Dependencies
		std::array<VkSubpassDependency, EDependencies::MAX> dependencies;
		std::memset(&dependencies, 0, sizeof(dependencies));
		{
			VkSubpassDependency& dependency = dependencies[EDependencies::ExternalToOpaque];
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = ESubpasses::Opaque;
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

		assert(!pass);
		if (vkCreateRenderPass(logical.device, &passCI, nullptr, &pass) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to create main render pass");
			return false;
		}

		return true;
	}

	void SurfaceRenderPass::Destroy(VulkanLogicalDevice const& logical) {
		vkDestroyRenderPass(logical.device, pass, nullptr);
		pass = nullptr;
	}

	bool SurfaceRenderPass::CreateFramebuffers(VulkanLogicalDevice const& logical, AttachmentImageViews const& sharedImageViews, TArrayView<VkImageView> colorImageViews, glm::u32vec2 const& size, Framebuffers& framebuffers) const {
		AttachmentImageViews attachments;
		//Copy the shared images into the attachments

		const size_t numImageViews = colorImageViews.size();
		framebuffers.resize(numImageViews);
		for (size_t index = 0; index < numImageViews; ++index) {
			attachments[EAttachments::Color] = colorImageViews[index];

			//Create the framebuffer
			VkFramebufferCreateInfo framebufferCI{};
			framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferCI.renderPass = pass;
			framebufferCI.attachmentCount = std::size(attachments);
			framebufferCI.pAttachments = attachments;
			framebufferCI.width = size.x;
			framebufferCI.height = size.y;
			framebufferCI.layers = 1;

			assert(!framebuffers[index].internal);
			if (vkCreateFramebuffer(logical.device, &framebufferCI, nullptr, &framebuffers[index].internal) != VK_SUCCESS) {
				LOGF(Vulkan, Error, "Failed to create frambuffer %i", index);
				return false;
			}
		}
		return true;
	}

	bool VulkanRenderPasses::Create(VulkanLogicalDevice const& logical, VkFormat format) {
		if (!surface.Create(logical, format)) return false;
		return true;
	}

	void VulkanRenderPasses::Destroy(VulkanLogicalDevice const& logical) {
		surface.Destroy(logical);
	}

	void VulkanFramebuffers::Destroy(VulkanLogicalDevice const& logical) {
		SurfaceRenderPass::DestroyFramebuffers(logical, surface);
	}
}
