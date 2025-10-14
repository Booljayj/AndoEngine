#include "Rendering/RenderingSystem.h"
#include "Engine/Utility.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/Material.h"
#include "Rendering/MeshRenderer.h"
#include "Rendering/Shader.h"
#include "Rendering/StaticMesh.h"
#include "Rendering/Vulkan/Buffers.h"
#include "Rendering/Vulkan/QueueSelection.h"
#include "Rendering/Vulkan/TransferCommands.h"
#include "Rendering/Vulkan/RenderPasses.h"
#include "Resources/Database.h"
#include "Resources/Cache.h"

DEFINE_LOG_CATEGORY(Rendering, Info);

namespace Rendering {
	bool RenderingSystem::Startup(HAL::WindowingSystem& windowing, Resources::Database& database) {
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
		database.FindOrCreateCache<Material>()->AddObserver(*this);
		database.FindOrCreateCache<StaticMesh>()->AddObserver(*this);
		
		//@todo Create a group for renderable entities, once we have more than one component to include in the group (i.e. renderer and transform).

		return true;
	}

	bool RenderingSystem::Shutdown(Resources::Database& database) {
		if (device) {
			//Wait until all current rendering operations finish
			vkDeviceWaitIdle(*device);

			//Destroy the surfaces
			surfaces.clear();
			//Destroy objects in all active resources
			auto const materials = database.FindOrCreateCache<Material>();
			materials->RemoveObserver(*this);
			materials->ForEachResource([](Material& material) { material.objects.reset(); return true; });
			auto const staticMeshes = database.FindOrCreateCache<StaticMesh>();
			staticMeshes->RemoveObserver(*this);
			staticMeshes->ForEachResource([](StaticMesh& mesh) { mesh.objects.reset(); return true; });
			//Finish any cleanup process and destroy any stale resources that haven't been cleaned up yet.
			cleanup_thread.reset();
			stale_collection.clear();
			//Destroy objects owned by the system
			transferCommandPool.reset();
			uniformLayouts.reset();
			passes.reset();
			
			//Destroy the device itself
			device.reset();
		}

		framework.reset();
		return true;
	}

	bool RenderingSystem::Render(entt::registry& registry) {
		//If we're still destroying resources that were used on the previous frame, wait for that to finish now.
		cleanup_thread.reset();

		//Recreate swapchains if necessary. This happens periodically if the rendering parameters have changed significantly.
		for (auto const& surface : surfaces) {
			if (surface->IsSwapchainDirty()) {
				LOG(Rendering, Info, "Recreating swapchain");
				vkDeviceWaitIdle(*device);
				surface->RecreateSwapchain(*device, GetPhysicalDevice(), *passes, *uniformLayouts);
			}
		}

		//Rebuild any resources, creating new ones and marking dirty ones as stale.
		RebuildResources();

		//Perform the render on every surface.
		bool success = true;
		for (auto const& surface : surfaces) {
			success &= surface->Render(*passes, registry, stale_collection);
		}

		if (!stale_collection.empty()) {
			//Create a worker thread that will attempt to destroy unused resources in parallel with the new rendering process.
			std::swap(stale_collection, cleaning_collection);

			cleanup_thread = std::make_unique<std::jthread>(
				[](std::stop_token, ResourcesCollection* collection) {
					ThreadBuffer buffer{ 20'000 };
					collection->clear();
				},
				&cleaning_collection
			);
		}

		return success;
	}

	void RenderingSystem::RebuildResources() {
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
			if (index >= framework->GetPhysicalDevices().size()) throw FormatType<std::out_of_range>("Physical device index {} out of range", index);
			
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
				LOG(Vulkan, Warning, "Physical device {} is unable to present to surfaces", physical.properties.deviceName);
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
			queues = device->queues.Resolve(shared);

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
			transferCommandPool.emplace(*device, queues->transfers[0]);

			//Let the surfaces know about the new rendering objects so they can prepare for rendering
			for (auto const& surface : surfaces) surface->InitializeRendering(*device, GetPhysicalDevice(), *passes, *uniformLayouts);
			
			VulkanVersion const version = VulkanVersion{ physical.properties.driverVersion };
			LOG(Rendering, Info, "Selected device {} ({})", physical.properties.deviceName, version);
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
			LOG(Rendering, Warning, "Unable to destroy surface with id {}", id);
		}
	}

	std::tuple<QueueRequests, SharedQueues::References> RenderingSystem::GetQueueRequests(PhysicalDeviceDescription const& physical, VkSurfaceKHR surface) {
		struct {
			std::optional<SurfaceQueues::References> surface;
			std::optional<SharedQueues::References> shared;
		} found;
	
		//Default mode - we're creating queues that will work for the first surface, and assuming other surfaces should also be able to use those queues
		QueueFamilySelectors selectors{ physical.GetSurfaceFamilies(surface) };

		found.surface = selectors.SelectSurfaceQueues();
		if (!found.surface) throw FormatType<std::runtime_error>("Physical device %s does not contain required surface queues", physical.properties.deviceName);

		found.shared = selectors.SelectSharedQueues(*found.surface);
		if (!found.shared) throw FormatType<std::runtime_error>("Physical device %s does not contain required shared queues", physical.properties.deviceName);
		
		QueueRequests requests;
		requests << *found.surface;
		requests << *found.shared;

		return std::make_tuple(requests, *found.shared);
	}

	std::tuple<QueueRequests, SharedQueues::References> RenderingSystem::GetHeadlessQueueRequests(PhysicalDeviceDescription const& physical) {
		QueueFamilySelectors selectors{ physical.families };

		const auto shared = selectors.SelectSharedQueues();
		if (!shared) throw FormatType<std::runtime_error>("Physical device %s does not contain required queues", physical.properties.deviceName);

		QueueRequests requests;
		requests << *shared;

		return std::make_tuple(requests, *shared);
	}

	void RenderingSystem::OnDestroyingWindow(HAL::Window::IdType id) {
		if (id == HAL::Window::Invalid) {
			if (device) vkDeviceWaitIdle(*device);
			surfaces.clear();
		} else {
			DestroySurface(id);
		}
	}

	void RenderingSystem::OnCreated(Resources::Handle<Material> const& material) {
		MarkMaterialDirty(material);
	}
	
	void RenderingSystem::OnDestroyed(Resources::Handle<Material> const& material) {
		MarkMaterialStale(material);
	}

	void RenderingSystem::OnCreated(Resources::Handle<StaticMesh> const& mesh) {
		MarkStaticMeshDirty(mesh);
	}

	void RenderingSystem::OnDestroyed(Resources::Handle<StaticMesh> const& mesh) {
		MarkStaticMeshStale(mesh);
	}

	void RenderingSystem::RefreshMaterials() {
		//The library of shader modules that will stay loaded as long as we need to continue creating pipelines
		PipelineCreationHelper helper{ *device };

		for (Resources::Handle<Material> const& material : dirtyMaterials) {
			//Existing resources become stale
			if (material->objects) stale_collection << material->objects;
			//Create new resources
			material->objects = CreateGraphicsPipeline(*material, helper);
		}
		dirtyMaterials.clear();
	}

	void RenderingSystem::MarkMaterialDirty(Resources::Handle<Material> const& material) {
		//Keep track of the dirty material so we can refresh it during the next render
		dirtyMaterials.emplace_back(material);
	}

	void RenderingSystem::MarkMaterialStale(Resources::Handle<Material> const& material) {
		//Steal the resources from the material so we can destroy them later when they're no longer being used
		stale_collection << material->objects;
	}

	void RenderingSystem::RefreshStaticMeshes() {
		MeshCreationHelper helper{ *device, queues->transfers[0], *transferCommandPool };

		for (const Resources::Handle<StaticMesh>& mesh : dirtyStaticMeshes) {
			//Existing resources become stale
			if (mesh->objects) stale_collection << mesh->objects;
			//Create new resources
			mesh->objects = CreateMesh(*mesh, *transferCommandPool, helper);
		}

		dirtyStaticMeshes.clear();
	}

	void RenderingSystem::MarkStaticMeshDirty(Resources::Handle<StaticMesh> const& mesh) {
		//Keep track of the dirty resources so we can destroy them during the next render
		dirtyStaticMeshes.emplace_back(mesh);
	}

	void RenderingSystem::MarkStaticMeshStale(Resources::Handle<StaticMesh> const& mesh) {
		//Keep track of the dirty resources so we can destroy them during the next render
		stale_collection << mesh->objects;
	}

	std::shared_ptr<GraphicsPipelineResources> RenderingSystem::CreateGraphicsPipeline(Material const& material, PipelineCreationHelper& helper) {
		GraphicsPipelineResources::ShaderModules modules;
		modules.vertex = helper.GetModule(material.shaders.vertex);
		modules.fragment = helper.GetModule(material.shaders.fragment);

		auto const binding = Vertex_Simple::GetBindingDescription();
		auto const attributes = Vertex_Simple::GetAttributeDescriptions();
		VertexInformationViews vertex;
		vertex.bindings = std::span{ &binding, 1 };
		vertex.attributes = attributes;

		return std::make_shared<GraphicsPipelineResources>(*device, modules, *uniformLayouts, vertex, passes->surface);
	}

	std::shared_ptr<MeshResources> RenderingSystem::CreateMesh(StaticMesh const& mesh, CommandPool& pool, MeshCreationHelper& helper) {
		//Calculate byte size values for the input mesh
		const auto BufferSizeVisitor = [](auto const& v) { return v.size() * sizeof(typename std::remove_reference_t<decltype(v)>::value_type); };
		size_t const vertexBytes = std::visit(BufferSizeVisitor, mesh.vertices);
		size_t const indexBytes = std::visit(BufferSizeVisitor, mesh.indices);
		size_t const totalBytes = vertexBytes + indexBytes;

		size_t const vertexOffset = 0;
		size_t const indexOffset = vertexBytes;

		//Create the staging buffer used to upload data to the GPU-only buffer
		MappedBuffer staging{ *device, totalBytes, BufferUsage::TransferSrc, MemoryUsage::CPU_Only };

		//Fill the staging buffer with the source data
		std::visit([&](auto const& v) { staging.WriteMultiple(std::span{ v }, vertexOffset); }, mesh.vertices);
		std::visit([&](auto const& v) { staging.WriteMultiple(std::span{ v }, indexOffset); }, mesh.indices);

		//Create the final resources that will be used for this mesh
		std::shared_ptr<MeshResources> const resources = std::make_shared<MeshResources>(*device, totalBytes);

		const VkCommandBuffer buffer = helper.CreateBuffer();
		{
			TransferCommandWriter commands{ buffer };

			//Record the command to transfer from the staging buffer to the target buffer
			VkBufferCopy const region{
				.srcOffset = 0, // Optional
				.dstOffset = 0, // Optional
				.size = totalBytes,
			};
			commands.Copy(staging, resources->buffer, MakeSpan(region));
		}

		//Submit the buffer and commands. We've successfully created everything.
		helper.Submit(buffer, std::move(staging));

		//Record the size and offset information. This is the primary way we'll know that the resources are valid
		const auto CountVisitor = [](const auto& v) { return static_cast<uint32_t>(v.size()); };
		resources->offset.vertex = 0;
		resources->offset.index = vertexBytes;
		resources->size.vertices = std::visit(CountVisitor, mesh.vertices);
		resources->size.indices = std::visit(CountVisitor, mesh.indices);
		
		struct IndexTypeVisitor {
			VkIndexType operator()(Indices_Short const&) { return VK_INDEX_TYPE_UINT16; }
			VkIndexType operator()(Indices_Long const&) { return VK_INDEX_TYPE_UINT32; }
		};
		resources->indexType = std::visit(IndexTypeVisitor{}, mesh.indices);

		return resources;
	}
}
