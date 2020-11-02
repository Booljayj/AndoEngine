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
, shouldRebuildCommandBuffers(false)
{}

bool RenderingSystem::Startup(CTX_ARG, SDLWindowSystem& windowSystem, EntityRegistry& registry) {
	using namespace Rendering;

	// Vulkan instance
	if (!framework.Create(CTX, windowSystem.GetMainWindow())) {
		LOG(Rendering, Error, "Failed to create the Vulkan framework");
		return false;
	}

	// Collect physical devices and select default one
	{
		TArrayView<char const*> const extensionNames = Rendering::VulkanPhysicalDevice::GetExtensionNames(CTX);

		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(framework.instance, &deviceCount, nullptr);
		VkPhysicalDevice* devices = CTX.temp.Request<VkPhysicalDevice>(deviceCount);
		vkEnumeratePhysicalDevices(framework.instance, &deviceCount, devices);

		for (int32_t deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
			const Rendering::VulkanPhysicalDevice physicalDevice = Rendering::VulkanPhysicalDevice::Get(CTX, devices[deviceIndex], framework.surface);
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
	{
		shouldRecreateSwapchain = false;
		swapchain = Rendering::VulkanSwapchain::Create(CTX, VkExtent2D{1024, 768}, framework.surface, *selectedPhysicalDevice, logicalDevice);
		if (!swapchain) {
			LOG(Rendering, Error, "Failed to create the swapchain");
			return false;
		}
	}

	//Create the command pool
	{
		commands = Rendering::VulkanCommands::Create(CTX, *selectedPhysicalDevice, logicalDevice);
		if (!commands) {
			LOG(Rendering, Error, "Failed to create the command pool");
			return false;
		}
		//Initial command buffers are created during the first Update, when we know what pipelines should be included.
		shouldRebuildCommandBuffers = true;
	}

	//Create the synchronizer
	{
		sync = Rendering::VulkanSynchronizer::Create(CTX, logicalDevice);
		if (!sync) {
			LOG(Rendering, Error, "Failed to create the synchronizer");
			return false;
		}
	}

	//@todo Create a group for renderable entities, once we have more than one component to include in the group (i.e. renderer and transform).

	return true;
}

bool RenderingSystem::Shutdown(CTX_ARG, EntityRegistry& registry) {
	using namespace Rendering;

	//Wait for any in-progress work to finish before we start cleanup
	if (logicalDevice) vkDeviceWaitIdle(logicalDevice.device);

	registry.DestroyComponents<MaterialComponent, MeshRendererComponent>();

	sync.Destroy(logicalDevice);
	commands.Destroy(logicalDevice);
	swapchain.Destroy(logicalDevice);
	logicalDevice.Destroy();
	framework.Destroy();
	return true;
}

bool RenderingSystem::Update(CTX_ARG, Time const& time, EntityRegistry const& registry) {
	//Recreate the swapchain if necessary. This happens periodically if the rendering parameters have changed significantly.
	if (shouldRecreateSwapchain) {
		shouldRecreateSwapchain = false;

		//We need to rebuild command buffers if the swapchain has been modified
		shouldRebuildCommandBuffers = true;

		LOG(Rendering, Info, "Recreating swapchain");
		swapchain = Rendering::VulkanSwapchain::Recreate(CTX, swapchain, VkExtent2D{1024, 768}, framework.surface, *selectedPhysicalDevice, logicalDevice);
		if (!swapchain) {
			LOG(Rendering, Error, "Failed to create a new swapchain");
			return false;
		}
	}

	//Rebuild the command buffers if necessary. This happens when we change what needs to be rendered, or if the rendering parameters
	//have changed significantly.
	if (shouldRebuildCommandBuffers) {
		shouldRebuildCommandBuffers = false;

		size_t const numBuffers = swapchain.images.size();
		commands.ReallocateCommandBuffers(CTX, logicalDevice, numBuffers);

		for (size_t index = 0; index < numBuffers; ++index) {
			if (!RebuildCommandBuffer(CTX, registry, commands.buffers[index], swapchain.images[index])) {
				LOGF(Rendering, Error, "Failed to rebuild command buffer %i", index);
				return false;
			}
		}
	}

	//Render the actual frame
	return sync.RenderFrame(CTX, logicalDevice, swapchain, commands);
}

bool RenderingSystem::SelectPhysicalDevice(CTX_ARG, uint32_t index) {
	using namespace Rendering;

	if (index != selectedPhysicalDeviceIndex) {
		if (VulkanPhysicalDevice const* physicalDevice = GetPhysicalDevice(index)) {
			TArrayView<char const*> const extensions = VulkanPhysicalDevice::GetExtensionNames(CTX);

			VulkanLogicalDevice newLogicalDevice = VulkanLogicalDevice::Create(CTX, *physicalDevice, features, extensions);
			if (!newLogicalDevice) {
				LOGF(Rendering, Error, "Failed to create logical device for physical device %i", index);
				return false;
			}
			logicalDevice = std::move(newLogicalDevice);

			selectedPhysicalDevice = physicalDevice;
			selectedPhysicalDeviceIndex = index;
			shouldRecreateSwapchain = true;

			physicalDevice->WriteDescription(std::cout);
			return true;
		}
	}
	return false;
}

bool RenderingSystem::IsUsablePhysicalDevice(const Rendering::VulkanPhysicalDevice& physicalDevice, TArrayView<char const*> const& extensionNames) {
	return physicalDevice.HasRequiredQueues() && physicalDevice.HasRequiredExtensions(extensionNames) && physicalDevice.HasSwapchainSupport();
}

bool RenderingSystem::RebuildCommandBuffer(CTX_ARG, EntityRegistry const& registry, VkCommandBuffer buffer, Rendering::VulkanSwapchainImageInfo info) {
	using namespace Rendering;

	const auto renderables = registry.GetView<MeshRendererComponent const>();
	const auto materials = registry.GetView<MaterialComponent const>();

	return RecordCommandBuffer(
		CTX, buffer,
		[&](VkCommandBuffer buffer) {
			//Record render pass 0
			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = swapchain.renderPass;
			renderPassInfo.framebuffer = info.framebuffer;
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = swapchain.extent;

			VkClearValue clear;
			clear.color.float32[3] = 1.0f;
			renderPassInfo.clearValueCount = 1;
			renderPassInfo.pClearValues = &clear;

			vkCmdBeginRenderPass(buffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			//@todo This would ideally be done with some acceleration structure that contains a mapping between the pipelines and all of
			//      the geometry that should be drawn with that pipeline, to avoid binding the same pipeline more than once.
			for (const auto id : renderables) {
				auto& renderer = renderables.Get<MeshRendererComponent const>(id);
				if (auto* material = materials.Find<MaterialComponent const>(renderer.material)) {
					//@todo This should actually be binding the real geometry, instead of assuming the shader can draw 3 unspecified vertices
					vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->pipeline);
					vkCmdDraw(buffer, 3, 1, 0, 0);
				}
			}

			vkCmdEndRenderPass(buffer);
		}
	);
}
