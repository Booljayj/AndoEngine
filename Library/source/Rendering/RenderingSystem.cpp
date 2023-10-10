#include "Rendering/RenderingSystem.h"
#include "Engine/StandardTypes.h"
#include "Engine/Utility.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/Material.h"
#include "Rendering/MeshRenderer.h"
#include "Rendering/Shader.h"
#include "Rendering/StaticMesh.h"
#include "Rendering/Vulkan/Buffers.h"
#include "Rendering/Vulkan/RenderPasses.h"

DEFINE_LOG_CATEGORY(Rendering, Info);

namespace Rendering {
	bool RenderingSystem::Startup(HAL::WindowingSystem& windowing, Resources::Cache<Material>& materials, Resources::Cache<StaticMesh>& staticMeshes) {
		windowing.destroying.Add(this, &RenderingSystem::OnDestroyingWindow);

		//The primary window must exist in order to start the rendering system
		HAL::Window& primaryWindow = windowing.GetPrimaryWindow();

		framework.emplace(primaryWindow);

		//Create the primary surface for the primary window
		if (!CreateSurface(primaryWindow)) return false;
		
		Surface& primarySurface = GetPrimarySurface();

		//Select a default physical device
		if (!SelectPhysicalDevice(0)) {
			LOG(Rendering, Error, "Failed to select default physical device");
			return false;
		}

		//Listen for when rendering-relevant resource types are created or destroyed
		materials.Created.Add(this, &RenderingSystem::MaterialCreated);
		materials.Destroyed.Add(this, &RenderingSystem::MaterialDestroyed);
		staticMeshes.Created.Add(this, &RenderingSystem::StaticMeshCreated);
		staticMeshes.Destroyed.Add(this, &RenderingSystem::StaticMeshDestroyed);

		//@todo Create a group for renderable entities, once we have more than one component to include in the group (i.e. renderer and transform).

		return true;
	}

	bool RenderingSystem::Shutdown(Resources::Cache<Material>& materials, Resources::Cache<StaticMesh>& staticMeshes) {
		if (device) {
			vkDeviceWaitIdle(*device);

			surfaces.clear();

			materials.Destroy();
			staticMeshes.Destroy();
			DestroyStalePipelines();
			DestroyStaleMeshes();

			transferCommandPool.reset();
			uniformLayouts.reset();
			passes.reset();
			
			device.reset();
		}

		framework.reset();
		return true;
	}

	bool RenderingSystem::Render(entt::registry& registry) {
		//Recreate swapchains if necessary. This happens periodically if the rendering parameters have changed significantly.
		for (auto const& surface : surfaces) {
			if (surface->IsSwapchainDirty()) {
				LOG(Rendering, Info, "Recreating swapchain");
				vkDeviceWaitIdle(*device);
				surface->RecreateSwapchain(*device, GetPhysicalDevice(), *passes, *uniformLayouts);
			}
		}

		//Rebuild any resources, creating new ones and destroying stale ones
		RebuildResources();

		bool success = true;
		for (auto const& surface : surfaces) {
			success &= surface->Render(*passes, registry);
		}

		return success;
	}

	void RenderingSystem::RebuildResources() {
		const bool hasStaleResources = staleGraphicsPipelineResources.size() > 0 || staleMeshResources.size() > 0;
		if (hasStaleResources) {
			LOG(Rendering, Info, "Destroying stale resources");
			//Wait until the device is idle so we don't destroy resources that are in use
			vkDeviceWaitIdle(*device);

			DestroyStalePipelines();
			DestroyStaleMeshes();
		}

		if (dirtyMaterials.size() > 0) {
			LOG(Rendering, Info, "Creating new pipelines");
			RefreshMaterials();
		}
		if (dirtyStaticMeshes.size() > 0) {
			LOG(Rendering, Info, "Creating new meshes");
			RefreshStaticMeshes();
		}
	}

	bool RenderingSystem::SelectPhysicalDevice(size_t index) {
		if (index != selectedPhysicalIndex) {
			if (index >= framework->GetPhysicalDevices().size()) throw std::out_of_range{ t_printf("Physical device index %i out of range", index).data() };
			
			//Clean up the previous device, and everything created for it
			if (device) {
				vkDeviceWaitIdle(*device);
				for (auto const& surface : surfaces) surface->DeinitializeRendering();
				transferCommandPool.reset();
				uniformLayouts.reset();
				passes.reset();
				device.reset();
			}

			PhysicalDeviceDescription const& physical = framework->GetPhysicalDevices()[index];

			//Get information about how to present to this device
			auto const presentation = PhysicalDevicePresentation::GetPresentation(physical, *surfaces[0]);
			if (!presentation) {
				LOGF(Vulkan, Warning, "Physical device %s is unable to present to surfaces", physical.properties.deviceName);
				return false;
			}

			//Determine which extensions to enable on this device
			t_vector<char const*> const extensions = Framework::GetDeviceExtensionNames();

			//Determine which queues to request from this device
			QueueRequests requests;
			SharedQueues::References shared;
			if (surfaces.size() > 0) std::tie(requests, shared) = GetQueueRequests(physical, *surfaces[0]);
			else std::tie(requests, shared) = GetHeadlessQueueRequests(physical);

			//Create the new device, and set up all the required values
			selectedPhysicalIndex = index;
			device.emplace(*framework, physical, features, extensions, requests);
			queues = shared.ResolveFrom(device->queues);

			//Get the surface format that will be used when rendering with this device
			primarySurfaceFormat = [presentation]() -> VkSurfaceFormatKHR {
				for (const auto& surfaceFormat : presentation->surfaceFormats) {
					if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM && surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
						return surfaceFormat;
					}
				}
				return presentation->surfaceFormats[0];
			}();

			passes.emplace(*device, primarySurfaceFormat.format);
			uniformLayouts.emplace(*device);
			transferCommandPool.emplace(*device, queues->transfer.index);

			//Let the surfaces know about the new rendering objects so they can prepare for rendering
			for (auto const& surface : surfaces) surface->InitializeRendering(*device, GetPhysicalDevice(), *passes, *uniformLayouts);
			
			VulkanVersion const version = VulkanVersion{ physical.properties.driverVersion };
			LOGF(Rendering, Info, "Selected device %s (%i.%i.%i)", physical.properties.deviceName, version.major, version.minor, version.patch);
			return true;
		}
		return false;
	}

	Surface* RenderingSystem::CreateSurface(HAL::Window& window) {
		//Check if we already have a surface for this window, and return it if we do
		if (Surface* existing = FindSurface(window.id)) return existing;

		//Create the surface in the collection of surfaces, and return a raw pointer to it
		std::unique_ptr<Surface>& surface = surfaces.emplace_back(std::make_unique<Surface>(*framework, window));

		return surface.get();
	}

	Surface* RenderingSystem::FindSurface(HAL::Window::IdType id) const {
		const auto iter = std::find_if(surfaces.begin(), surfaces.end(), [&](const auto& surface) { return surface->GetID() == id; });
		if (iter != surfaces.end()) return iter->get();
		else return nullptr;
	}

	void RenderingSystem::DestroySurface(HAL::Window::IdType id) {
		const auto iter = std::find_if(surfaces.begin(), surfaces.end(), [=](auto const& surface) { return surface->GetID() == id; });
		if (iter != surfaces.end() && iter != surfaces.begin()) {
			if (device) vkDeviceWaitIdle(*device);
			surfaces.erase(iter);
		} else {
			LOGF(Rendering, Warning, "Unable to destroy surface with id %i", id);
		}
	}

	std::tuple<QueueRequests, SharedQueues::References> RenderingSystem::GetQueueRequests(PhysicalDeviceDescription const& physical, VkSurfaceKHR surface) {
		struct {
			std::optional<SurfaceQueues::References> surface;
			std::optional<SharedQueues::References> shared;
		} found;
	
		//Default mode - we're creating queues that will work for the first surface, and assuming other surfaces should also be able to use those queues
		t_vector<QueueFamilyDescription> const families = physical.GetSurfaceFamilies(surface);

		found.surface = SurfaceQueues::References::Find(families);
		if (!found.surface) throw std::runtime_error{ t_printf("Physical device %s does not contain required queues", physical.properties.deviceName).data() };

		found.shared = SharedQueues::References::Find(families, *found.surface);
		if (!found.shared) throw std::runtime_error{ t_printf("Physical device %s does not contain required queues", physical.properties.deviceName).data() };

		QueueRequests requests;
		requests += *found.surface;
		requests += *found.shared;

		return std::make_tuple(requests, *found.shared);
	}

	std::tuple<QueueRequests, SharedQueues::References> RenderingSystem::GetHeadlessQueueRequests(PhysicalDeviceDescription const& physical) {
		struct {
			std::optional<SharedQueues::References> shared;
		} found;

		found.shared = SharedQueues::References::FindHeadless(physical.families);
		if (!found.shared) throw std::runtime_error{ t_printf("Physical device %s does not contain required queues", physical.properties.deviceName).data() };

		QueueRequests requests;
		requests += *found.shared;

		return std::make_tuple(requests, *found.shared);
	}

	void RenderingSystem::OnDestroyingWindow(HAL::Window::IdType id) {
		if (id == HAL::Window::Invalid) {
			if (device) vkDeviceWaitIdle(*device);
			surfaces.clear();
		} else {
			DestroySurface(id);
		}
	}

	void RenderingSystem::MaterialCreated(Resources::Handle<Material> const& material) {
		MarkMaterialDirty(material);
	}

	void RenderingSystem::MaterialDestroyed(Resources::Handle<Material> const& material) {
		MarkMaterialStale(material);
	}

	void RenderingSystem::StaticMeshCreated(Resources::Handle<StaticMesh> const& mesh) {
		MarkStaticMeshDirty(mesh);
	}

	void RenderingSystem::StaticMeshDestroyed(Resources::Handle<StaticMesh> const& mesh) {
		MarkStaticMeshStale(mesh);
	}

	void RenderingSystem::RefreshMaterials() {
		//The library of shader modules that will stay loaded as long as we need to continue creating pipelines
		PipelineCreationHelper helper{*device};

		for (Resources::Handle<Material> const& material : dirtyMaterials) {
			//Existing resources become stale
			if (material->gpuResources) staleGraphicsPipelineResources.emplace_back(std::move(*material->gpuResources));
			//Create new resources
			material->gpuResources.emplace(CreateGraphicsPipeline(*material, helper));
		}
		dirtyMaterials.clear();
	}

	void RenderingSystem::MarkMaterialDirty(Resources::Handle<Material> const& material) {
		//Keep track of the dirty material so we can refresh it during the next render
		dirtyMaterials.emplace_back(material);
	}

	void RenderingSystem::MarkMaterialStale(Resources::Handle<Material> const& material) {
		//Steal the resources from the material so we can destroy them later when they're no longer being used
		staleGraphicsPipelineResources.emplace_back(std::move(*material->gpuResources));
	}

	void RenderingSystem::DestroyStalePipelines() {
		staleGraphicsPipelineResources.clear();
	}

	void RenderingSystem::RefreshStaticMeshes() {
		MeshCreationHelper helper{ *device, queues->transfer, *transferCommandPool};

		for (const Resources::Handle<StaticMesh>& mesh : dirtyStaticMeshes) {
			//Existing resources become stale
			if (mesh->gpuResources) staleMeshResources.emplace_back(std::move(*mesh->gpuResources));
			//Create new resources
			mesh->gpuResources.emplace(CreateMesh(*mesh, *transferCommandPool, helper));
		}

		dirtyStaticMeshes.clear();
	}

	void RenderingSystem::MarkStaticMeshDirty(Resources::Handle<StaticMesh> const& mesh) {
		//Keep track of the dirty material so we can refresh it during the next render
		dirtyStaticMeshes.emplace_back(mesh);
	}

	void RenderingSystem::MarkStaticMeshStale(Resources::Handle<StaticMesh> const& mesh) {
		staleMeshResources.emplace_back(std::move(*mesh->gpuResources));
	}

	void RenderingSystem::DestroyStaleMeshes() {
		staleMeshResources.clear();
	}

	GraphicsPipelineResources RenderingSystem::CreateGraphicsPipeline(Material const& material, PipelineCreationHelper& helper) {
		GraphicsPipelineResources::ShaderModules modules;
		modules.vertex = helper.GetModule(material.vertex);
		modules.fragment = helper.GetModule(material.fragment);

		auto const binding = Vertex_Simple::GetBindingDescription();
		auto const attributes = Vertex_Simple::GetAttributeDescriptions();
		VertexInformationViews vertex;
		vertex.bindings = TArrayView{ binding };
		vertex.attributes = attributes;

		return GraphicsPipelineResources{ *device, modules, *uniformLayouts, vertex, passes->surface };
	}

	MeshResources RenderingSystem::CreateMesh(StaticMesh const& mesh, VkCommandPool pool, MeshCreationHelper& helper) {
		//Calculate byte size values for the input mesh
		const auto BufferSizeVisitor = [](auto const& v) { return v.size() * sizeof(typename std::remove_reference_t<decltype(v)>::value_type); };
		size_t const vertexBytes = std::visit(BufferSizeVisitor, mesh.vertices);
		size_t const indexBytes = std::visit(BufferSizeVisitor, mesh.indices);
		size_t const totalBytes = vertexBytes + indexBytes;

		size_t const vertexOffset = 0;
		size_t const indexOffset = vertexBytes;

		//Create the staging buffer used to upload data to the GPU-only buffer
		MappedBuffer staging{ *device, totalBytes, BufferUsage::TransferSrc, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY };

		//Fill the staging buffer with the source data
		const auto VertexWriteVisitor = [&](auto const& v) { staging.Write(v.data(), v.size() * sizeof(typename std::remove_reference_t<decltype(v)>::value_type), vertexOffset); };
		std::visit(VertexWriteVisitor, mesh.vertices);
		const auto IndexWriteVisitor = [&](auto const& v) { staging.Write(v.data(), v.size() * sizeof(typename std::remove_reference_t<decltype(v)>::value_type), indexOffset); };
		std::visit(IndexWriteVisitor, mesh.indices);

		//Create the final resources that will be used for this mesh
		MeshResources resources{ *device, totalBytes };

		//Allocate a command buffer for the transfer from the staging to the target buffer
		VkCommandBufferAllocateInfo bufferAI{};
		bufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAI.commandPool = pool;
		bufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferAI.commandBufferCount = 1;

		TUniquePoolHandles<1, &vkFreeCommandBuffers> tempCommands{ *device, pool };
		if (vkAllocateCommandBuffers(*device, &bufferAI, *tempCommands) != VK_SUCCESS) {
			throw std::runtime_error{ "Failed to allocate command buffer for staging commands" };
		}

		{
			ScopedCommands const scope{ tempCommands[0], VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, nullptr };

			//Record the command to transfer from the staging buffer to the target buffer
			VkBufferCopy copy{};
			copy.srcOffset = 0; // Optional
			copy.dstOffset = 0; // Optional
			copy.size = totalBytes;
			vkCmdCopyBuffer(tempCommands[0], staging, resources.buffer, 1, &copy);
		}

		//Submit the buffer and commands. We've successfully created everything
		helper.Submit(tempCommands.Release()[0], std::move(staging));

		//Record the size and offset information. This is the primary way we'll know that the resources are valid
		const auto CountVisitor = [](const auto& v) { return static_cast<uint32_t>(v.size()); };
		resources.offset.vertex = 0;
		resources.offset.index = vertexBytes;
		resources.size.vertices = std::visit(CountVisitor, mesh.vertices);
		resources.size.indices = std::visit(CountVisitor, mesh.indices);
		
		struct IndexTypeVisitor {
			VkIndexType operator()(Indices_Short const&) { return VK_INDEX_TYPE_UINT16; }
			VkIndexType operator()(Indices_Long const&) { return VK_INDEX_TYPE_UINT32; }
		};
		resources.indexType = std::visit(IndexTypeVisitor{}, mesh.indices);

		return resources;
	}
}
