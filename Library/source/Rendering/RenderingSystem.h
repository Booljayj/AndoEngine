#pragma once
#include "Engine/Logging.h"
#include "Engine/StandardTypes.h"
#include "Rendering/Surface.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/VulkanFrameOrganizer.h"
#include "Rendering/Vulkan/VulkanFramework.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanPhysicalDevice.h"
#include "Rendering/Vulkan/VulkanRenderPasses.h"
#include "Rendering/Vulkan/VulkanResources.h"
#include "Rendering/Vulkan/VulkanResourcesHelpers.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"
#include "Rendering/Vulkan/VulkanUniformLayouts.h"
#include "Resources/Database.h"
#include "ThirdParty/EnTT.h"

namespace HAL {
	struct WindowingSystem;
}
namespace Rendering {
	struct Material;
	struct StaticMesh;
}

DECLARE_LOG_CATEGORY(Rendering);

namespace Rendering {
	struct RenderingSystem {
	public:
		using SurfaceContainer = std::vector<std::unique_ptr<Surface>>;

		/** The maximum number of consecutive times we can fail to render a frame */
		static constexpr uint8_t maxRetryCount = 5;

		/** The enabled features on any physical device that this application uses */
		VkPhysicalDeviceFeatures features = {};

		/** The vulkan framework for this application */
		VulkanFramework framework;

		/** The available physical devices which can be used */
		std::vector<VulkanPhysicalDevice> availablePhysicalDevices;
		/** The current selected physical device */
		VulkanPhysicalDevice const* selectedPhysical = nullptr;
		/** The index of the currently selected physical device */
		size_t selectedPhysicalIndex = std::numeric_limits<size_t>::max();

		/** The logical device for the currently selected physical device */
		VulkanLogicalDevice logical;

		/** The surface format of the primary surface, which is used when rendering to all surfaces */
		VkSurfaceFormatKHR primarySurfaceFormat = {};

		/** The render passes used for scene rendering */
		VulkanRenderPasses passes;

		/** Uniform layouts for standard uniforms */
		VulkanUniformLayouts uniformLayouts;

		/** Command recording objects */
		VkCommandPool commandPool = nullptr;

		/** Flags for tracking rendering behavior and changes */
		uint8_t retryCount = 0;

		RenderingSystem() = default;

		bool Startup(HAL::WindowingSystem& windowing, Resources::Cache<Material>& materials, Resources::Cache<StaticMesh>& staticMeshes);
		bool Shutdown(Resources::Cache<Material>& materials, Resources::Cache<StaticMesh>& staticMeshes);

		bool Render(entt::registry& registry);
		void RebuildResources();

		inline size_t NumPhysicalDevices() const { return availablePhysicalDevices.size(); }
		inline const Rendering::VulkanPhysicalDevice* GetPhysicalDevice(size_t Index) const {
			if (Index < NumPhysicalDevices()) return &availablePhysicalDevices[Index];
			else return nullptr;
		}
		bool SelectPhysicalDevice(size_t Index);

		/** Get the primary window, which is the first window created on startup */
		inline Surface& GetPrimarySurface() const { return *surfaces[0].get(); }

		/** Create a new surface bound to the given window */
		Surface* CreateSurface(HAL::Window& window);
		/** Find a surface using its id */
		Surface* FindSurface(HAL::Window::IdType id) const;
		/** Destroy a surface using its id */
		void DestroySurface(HAL::Window::IdType id);

	protected:
		/** Callbacks for when materials are created or destroyed */
		void MaterialCreated(Resources::Handle<Material> const& material);
		void MaterialDestroyed(Resources::Handle<Material> const& material);

		/** Callbacks for when static meshes are created or destroyed */
		void StaticMeshCreated(Resources::Handle<StaticMesh> const& mesh);
		void StaticMeshDestroyed(Resources::Handle<StaticMesh> const& mesh);

		/** Dirty resources that need to be rebuilt */
		std::vector<Resources::Handle<Material>> dirtyMaterials;
		std::vector<Resources::Handle<StaticMesh>> dirtyStaticMeshes;

		/** Resources that are pending destruction */
		std::vector<VulkanPipelineResources> stalePipelineResources;
		std::vector<VulkanMeshResources> staleMeshResources;

		/** Refresh dirty materials so they are no longer dirty */
		void RefreshMaterials();

		/** Mark the pipeline resources on the material as dirty */
		void MarkMaterialDirty(Resources::Handle<Material> const& material);
		/** Mark the pipeline resources on the material as stale */
		void MarkMaterialStale(Resources::Handle<Material> const& material);
		/** Destroy any stale pipeline resources */
		void DestroyStalePipelines();

		/** Refresh dirty meshes so they are no longer dirty */
		void RefreshStaticMeshes();

		/** Mark the mesh resources on the static mesh as dirty */
		void MarkStaticMeshDirty(Resources::Handle<StaticMesh> const& mesh);
		/** Mark the mesh resources on the material as stale */
		void MarkStaticMeshStale(Resources::Handle<StaticMesh> const& mesh);
		/** Destroy any stale pipeline resources */
		void DestroyStaleMeshes();

	private:
		/** Surfaces used for rendering */
		SurfaceContainer surfaces;

		/** Returns true if the physical device can actually be used by this rendering system */
		bool IsUsablePhysicalDevice(const Rendering::VulkanPhysicalDevice& physicalDevice, TArrayView<char const*> const& extensionNames);

		/** Create the pipeline resources for a material */
		VulkanPipelineResources CreatePipeline(Material const& material, VulkanPipelineCreationHelper& helper);
		/** Create the mesh resources for a mesh component */
		VulkanMeshResources CreateMesh(StaticMesh const& mesh, VkCommandPool pool, VulkanMeshCreationHelper& helper);

		//InitImGUI(VulkanLogicalDevice& logical, VulkanPhysicalDevice& physical, Surface& surface);
	};
}
