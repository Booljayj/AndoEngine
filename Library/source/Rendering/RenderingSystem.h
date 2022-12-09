#pragma once
#include "Engine/Logging.h"
#include "Engine/StandardTypes.h"
#include "EntityFramework/EntityRegistry.h"
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

namespace HAL {
	struct WindowingSystem;
}
namespace Rendering {
	struct MaterialComponent;
	struct MeshComponent;
}

DECLARE_LOG_CATEGORY(Rendering);

namespace Rendering {
	struct RenderingSystem {
	public:
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

		/** The primary rendering surface */
		Surface* primarySurface = nullptr;
		/** Surfaces used for rendering */
		std::vector<std::unique_ptr<Surface>> surfaces;
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
		bool shouldCreatePipelines = false;
		bool shouldCreateMeshes = false;

		RenderingSystem() = default;

		bool Startup(HAL::WindowingSystem& windowing, EntityRegistry& registry);
		bool Shutdown(EntityRegistry& registry);

		bool Render(EntityRegistry& registry);
		void RebuildResources(EntityRegistry& registry);

		inline size_t NumPhysicalDevices() const { return availablePhysicalDevices.size(); }
		inline const Rendering::VulkanPhysicalDevice* GetPhysicalDevice(size_t Index) const {
			if (Index < NumPhysicalDevices()) return &availablePhysicalDevices[Index];
			else return nullptr;
		}
		bool SelectPhysicalDevice(size_t Index);

		/** Find a surface using its id */
		Surface* FindSurface(uint32_t id) const;
		/** Create a new surface bound to the given window */
		Surface* CreateSurface(HAL::Window window);
		/** Destroy a surface using its id */
		void DestroySurface(uint32_t id);

	protected:
		/** Contains callbacks related to material component operations */
		struct MaterialComponentOperations {
			static void OnCreate(entt::registry& registry, entt::entity entity);
			static void OnDestroy(entt::registry& registry, entt::entity entity);
			static void OnModify(entt::registry& registry, entt::entity entity);
		};
		/** Contains callbacks related to mesh component operations */
		struct MeshComponentOperations {
			static void OnCreate(entt::registry& registry, entt::entity entity);
			static void OnDestroy(entt::registry& registry, entt::entity entity);
			static void OnModify(entt::registry& registry, entt::entity entity);
		};

		/** Resources that are pending destruction */
		std::vector<VulkanPipelineResources> stalePipelineResources;
		std::vector<VulkanMeshResources> staleMeshResources;

		/** Create or destroy all pipeline resources */
		void CreatePipelines(EntityRegistry& registry);
		void DestroyPipelines(EntityRegistry& registry);
		/** Mark the pipeline resources on the material as stale */
		void MarkPipelineStale(MaterialComponent& material);
		/** Destroy any stale pipeline resources */
		void DestroyStalePipelines();

		/** Create or destroy all mesh resources */
		void CreateMeshes(EntityRegistry& registry);
		/** Mark the mesh resources on a mesh component as stale */
		void MarkMeshStale(MeshComponent& mesh);
		/** Destroy any stale pipeline resources */
		void DestroyStaleMeshes();

	private:
		/** Returns true if the physical device can actually be used by this rendering system */
		bool IsUsablePhysicalDevice(const Rendering::VulkanPhysicalDevice& physicalDevice, TArrayView<char const*> const& extensionNames);

		/** Create the pipeline resources for a material */
		VulkanPipelineResources CreatePipeline(MaterialComponent const& material, EntityID id, VulkanPipelineCreationHelper& helper);
		/** Create the mesh resources for a mesh component */
		VulkanMeshResources CreateMesh(MeshComponent const& mesh, EntityID id, VkCommandPool pool, VulkanMeshCreationHelper& helper);
	};
}
