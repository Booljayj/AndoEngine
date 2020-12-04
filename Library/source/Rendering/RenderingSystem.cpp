#include <SDL2/SDL_vulkan.h>
#include "Rendering/RenderingSystem.h"
#include "Engine/LogCommands.h"
#include "Engine/STL.h"
#include "Engine/Utility.h"
#include "Geometry/LinearAlgebra.h"
#include "Rendering/MaterialComponent.h"
#include "Rendering/MeshRendererComponent.h"
#include "Rendering/SDLSystems.h"

DEFINE_LOG_CATEGORY(Rendering, Warning);

RenderingSystem::RenderingSystem()
: shouldRecreateSwapchain(false)
{}

bool RenderingSystem::Startup(CTX_ARG, SDLWindowingSystem& windowing, EntityRegistry& registry) {
	using namespace Rendering;

	// Vulkan instance
	if (!framework.Create(CTX, windowing.GetMainWindow())) return false;

	// Collect physical devices and select default one
	{
		TArrayView<char const*> const extensionNames = VulkanPhysicalDevice::GetExtensionNames(CTX);

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(framework.instance, &deviceCount, nullptr);
		VkPhysicalDevice* devices = CTX.temp.Request<VkPhysicalDevice>(deviceCount);
		vkEnumeratePhysicalDevices(framework.instance, &deviceCount, devices);

		for (int32_t deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
			const VulkanPhysicalDevice physicalDevice = VulkanPhysicalDevice::Get(CTX, devices[deviceIndex], framework.surface);
			if (IsUsablePhysicalDevice(physicalDevice, extensionNames)) {
				availablePhysicalDevices.push_back(physicalDevice);
			}
		}

		if (availablePhysicalDevices.size() == 0) {
			LOG(Rendering, Error, "Failed to find any vulkan physical devices");
			return false;
		}

		if (!SelectPhysicalDevice(CTX, 0)) {
			LOG(Rendering, Error, "Failed to select default physical device");
			return false;
		}
	}

	//Create the initial swapchain
	shouldRecreateSwapchain = false;
	if (!swapchain.Create(CTX, VkExtent2D{1024, 768}, framework.surface, *selectedPhysical, logical)) return false;
	//Create the organizer that will be used for rendering
	if (!organizer.Create(CTX, *selectedPhysical, logical, EBuffering::Double, swapchain.views.size(), 1)) return false;

	//Create the render passes
	if (!CreateRenderPasses(CTX)) return false;

	//@todo Create a group for renderable entities, once we have more than one component to include in the group (i.e. renderer and transform).

	return true;
}

bool RenderingSystem::Shutdown(CTX_ARG, EntityRegistry& registry) {
	using namespace Rendering;

	availablePhysicalDevices.clear();

	//Wait for any in-progress work to finish before we start cleanup
	if (logical) vkDeviceWaitIdle(logical.device);

	registry.DestroyComponents<MaterialComponent, MeshRendererComponent>();

	DestroyRenderPasses(CTX);
	organizer.Destroy(logical);
	swapchain.Destroy(logical);
	logical.Destroy();
	framework.Destroy();
	return true;
}

bool RenderingSystem::Render(CTX_ARG, EntityRegistry const& registry) {
	using namespace Rendering;

	//Recreate the swapchain if necessary. This happens periodically if the rendering parameters have changed significantly.
	if (shouldRecreateSwapchain) {
		shouldRecreateSwapchain = false;

		LOG(Rendering, Info, "Recreating swapchain");

		//Wait for any current rendering operations to finish before continuing. This may cause an expected stutter.
		vkDeviceWaitIdle(logical.device);

		//Destroy resources that depend on the swapchain
		organizer.Destroy(logical);
		DestroyRenderPasses(CTX);

		//Recreate the swapchain and everythign depending on it. If any of this fails, we need to request a shutdown.
		if (!swapchain.Recreate(CTX, VkExtent2D{1024, 768}, framework.surface, *selectedPhysical, logical)) return false;
		if (!organizer.Create(CTX, *selectedPhysical, logical, EBuffering::Double, swapchain.views.size(), 1)) return false;
		if (!CreateRenderPasses(CTX)) return false;
	}

	//Prepare the frame for rendering, which may need to wait for resources
	EPreparationResult const result = organizer.Prepare(CTX, logical, swapchain);
	if (result == EPreparationResult::Retry) {
		if (++retryCount >= maxRetryCount) {
			LOGF(Rendering, Error, "Too many subsequent frames (%i) have failed to render.", maxRetryCount);
			return false;
		}
		return true;
	}
	if (result == EPreparationResult::Error) return false;

	//Lambda to record rendering commands
	//@todo This would ideally be done with some acceleration structure that contains a mapping between the pipelines and all of
	//      the geometry that should be drawn with that pipeline, to avoid binding the same pipeline more than once and to strip
	//      out culled geometry.
	auto const renderables = registry.GetView<MeshRendererComponent const>();
	auto const materials = registry.GetView<MaterialComponent const>();

	auto const recorder = [&](VkCommandBuffer buffer, size_t index) {
		ScopedRenderPass pass{buffer, main, index, VkOffset2D{0, 0}, swapchain.extent};

		for (const auto id : renderables) {
			auto& renderer = renderables.Get<MeshRendererComponent const>(id);
			if (auto* material = materials.Find<MaterialComponent const>(renderer.material)) {
				//@todo This should actually be binding the real geometry, instead of assuming the shader can draw 3 unspecified vertices
				vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipeline);
				vkCmdDraw(buffer, 3, 1, 0, 0);
			}
		}
	};

	//Record rendering commands for this frame
	if (!organizer.Record(CTX, recorder)) return false;

	//Submit the rendering commands for this frame
	if (!organizer.Submit(CTX, logical, swapchain)) return false;

	retryCount = 0;
	return true;
}

bool RenderingSystem::SelectPhysicalDevice(CTX_ARG, uint32_t index) {
	using namespace Rendering;

	if (index != selectedPhysicalIndex) {
		if (VulkanPhysicalDevice const* newPhysical = GetPhysicalDevice(index)) {
			TArrayView<char const*> const extensions = VulkanPhysicalDevice::GetExtensionNames(CTX);

			VulkanLogicalDevice newLogical = VulkanLogicalDevice::Create(CTX, *newPhysical, features, extensions);
			if (!newLogical) {
				LOGF(Rendering, Error, "Failed to create logical device for physical device %i", index);
				return false;
			}
			logical = std::move(newLogical);

			selectedPhysical = newPhysical;
			selectedPhysicalIndex = index;
			shouldRecreateSwapchain = true;

			selectedPhysical->WriteDescription(std::cout);
			return true;
		}
	}
	return false;
}

bool RenderingSystem::CreateRenderPasses(CTX_ARG) {
	using namespace Rendering;

	//Attachments
	enum EAttachment : uint8_t {
		ColorAttachment,
		MAX_ATTACHMENT
	};
	std::array<VkAttachmentDescription, MAX_ATTACHMENT> descriptions;
	std::memset(&descriptions, 0, sizeof(descriptions));
	{
		VkAttachmentDescription& colorDescription = descriptions[ColorAttachment];
		colorDescription.format = swapchain.surfaceFormat.format;
		colorDescription.samples = VK_SAMPLE_COUNT_1_BIT;
		colorDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		colorDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		colorDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		colorDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		colorDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		main.clearValues[ColorAttachment].color = VkClearColorValue{{0.0f, 0.0f, 0.0f, 1.0f}};
	}

	//Subpasses
	enum ESubpass : uint8_t {
		ColorSubpass,
		MAX_SUBPASS
	};
	std::array<VkSubpassDescription, MAX_SUBPASS> subpasses;
	std::memset(&subpasses, 0, sizeof(subpasses));
	{
		VkSubpassDescription& colorSubpass = subpasses[ColorSubpass];

		VkAttachmentReference colorReference;
		colorReference.attachment = ColorAttachment;
		colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		colorSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		colorSubpass.colorAttachmentCount = 1;
		colorSubpass.pColorAttachments = &colorReference;
	}

	//Subpass dependencies
	enum EDependencies : uint8_t {
		ExternalToColorDependency,
		MAX_DEPENDENCY
	};
	std::array<VkSubpassDependency, MAX_DEPENDENCY> dependencies;
	std::memset(&dependencies, 0, sizeof(dependencies));
	{
		VkSubpassDependency& dependency = dependencies[ExternalToColorDependency];
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = ColorSubpass;
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

	assert(!main.pass);
	if (vkCreateRenderPass(logical.device, &passCI, nullptr, &main.pass) != VK_SUCCESS) {
		LOG(Vulkan, Error, "Failed to create main render pass");
		return false;
	}

	//Create one framebuffer for each image in the swapchain, binding the image as one of the attachments
	std::array<VkImageView, MAX_ATTACHMENT> attachments;

	const size_t numImages = swapchain.views.size();
	main.framebuffers.resize(numImages);
	for (size_t index = 0; index < numImages; ++index) {
		attachments[ColorAttachment] = swapchain.views[index];

		//Create the framebuffer
		VkFramebufferCreateInfo framebufferCI{};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = main.pass;
		framebufferCI.attachmentCount = 1;
		framebufferCI.pAttachments = attachments.data();
		framebufferCI.width = swapchain.extent.width;
		framebufferCI.height = swapchain.extent.height;
		framebufferCI.layers = 1;

		assert(!main.framebuffers[index]);
		if (vkCreateFramebuffer(logical.device, &framebufferCI, nullptr, &main.framebuffers[index]) != VK_SUCCESS) {
			LOGF(Vulkan, Error, "Failed to create frambuffer %i", index);
			return false;
		}
	}

	return true;
}

void RenderingSystem::DestroyRenderPasses(CTX_ARG) {
	main.Destroy(logical);
}

bool RenderingSystem::IsUsablePhysicalDevice(const Rendering::VulkanPhysicalDevice& physicalDevice, TArrayView<char const*> const& extensionNames) {
	return physicalDevice.HasRequiredQueues() && physicalDevice.HasRequiredExtensions(extensionNames) && physicalDevice.HasSwapchainSupport();
}
