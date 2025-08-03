#pragma once
#include "Engine/Core.h"
#include "Engine/Logging.h"
#include "Engine/Optional.h"
#include "Engine/SmartPointers.h"
#include "Engine/Tuple.h"
#include "Engine/Vector.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/Surface.h"
#include "Rendering/Vulkan/Device.h"
#include "Rendering/Vulkan/Framework.h"
#include "Rendering/Vulkan/Helpers.h"
#include "Rendering/Vulkan/PhysicalDevice.h"
#include "Rendering/Vulkan/RenderPasses.h"
#include "Rendering/Vulkan/Resources.h"
#include "Rendering/Vulkan/UniformLayouts.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Resources/Cache.h"
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
	struct RenderingSystem : public Resources::Observer<Material>, public Resources::Observer<StaticMesh> {
	public:
		/** The maximum number of consecutive times we can fail to render a frame */
		static constexpr uint8_t maxRetryCount = 5;

		/** The enabled features on any physical device that this application uses */
		VkPhysicalDeviceFeatures features = {};

		/** The vulkan framework for this application */
		std::optional<Framework> framework;

		/** The index of the currently selected physical device */
		size_t selectedPhysicalIndex = std::numeric_limits<size_t>::max();

		/** The logical device for the currently selected physical device */
		std::optional<Device> device;
		/** Shared queues used for system operations */
		std::optional<SharedQueues> queues;

		/** The surface format of the primary surface, which is used when rendering to all surfaces */
		VkSurfaceFormatKHR primarySurfaceFormat = {};

		/** The render passes used for scene rendering */
		std::optional<RenderPasses> passes;
		/** Uniform layouts for standard uniforms */
		std::optional<UniformLayouts> uniformLayouts;
		/** Pool for command buffers used in transfer operations */
		std::optional<CommandPool> transferCommandPool;

		/** Flags for tracking rendering behavior and changes */
		uint8_t retryCount = 0;
		
		RenderingSystem() = default;

		bool Startup(HAL::WindowingSystem& windowing, Resources::Database& database);
		bool Shutdown(Resources::Database& database);

		bool Render(entt::registry& registry);
		void RebuildResources();

		PhysicalDeviceDescription const& GetPhysicalDevice() const { return framework->GetPhysicalDevices()[selectedPhysicalIndex]; }
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
		/** Determine which queues to request from the physical device. Queues needed for surface rendering will be avoided if possible. */
		static std::tuple<QueueRequests, SharedQueues::References> GetQueueRequests(PhysicalDeviceDescription const& physical, VkSurfaceKHR surface);
		/** Determine which queues to request from the physical device. Used in headless mode when surface rendering is not available. */
		static std::tuple<QueueRequests, SharedQueues::References> GetHeadlessQueueRequests(PhysicalDeviceDescription const& physical);

		/** Called just before a window is destroyed in the windowing system */
		void OnDestroyingWindow(HAL::Window::IdType id);

		/** Callbacks for when materials are created or destroyed */
		void OnCreated(Resources::Handle<Material> const& material) final;
		void OnDestroyed(Resources::Handle<Material> const& material) final;

		/** Callbacks for when static meshes are created or destroyed */
		void OnCreated(Resources::Handle<StaticMesh> const& mesh) final;
		void OnDestroyed(Resources::Handle<StaticMesh> const& mesh) final;

		/** Dirty resources that need to be rebuilt */
		std::vector<Resources::Handle<Material>> dirtyMaterials;
		std::vector<Resources::Handle<StaticMesh>> dirtyStaticMeshes;

		/** Resources that are pending destruction */
		std::vector<std::shared_ptr<GraphicsPipelineResources>> staleGraphicsPipelineResources;
		std::vector<std::shared_ptr<MeshResources>> staleMeshResources;

		/** Refresh dirty materials so they are no longer dirty */
		void RefreshMaterials();
		/** Mark the pipeline resources on the material as dirty */
		void MarkMaterialDirty(Resources::Handle<Material> const& material);
		/** Mark the pipeline resources on the material as stale */
		void MarkMaterialStale(Resources::Handle<Material> const& material);

		/** Refresh dirty meshes so they are no longer dirty */
		void RefreshStaticMeshes();
		/** Mark the mesh resources on the static mesh as dirty */
		void MarkStaticMeshDirty(Resources::Handle<StaticMesh> const& mesh);
		/** Mark the mesh resources on the material as stale */
		void MarkStaticMeshStale(Resources::Handle<StaticMesh> const& mesh);

	private:
		/** Surfaces used for rendering */
		std::vector<std::unique_ptr<Surface>> surfaces;

		/** Create the pipeline resources for a material */
		std::shared_ptr<GraphicsPipelineResources> CreateGraphicsPipeline(Material const& material, PipelineCreationHelper & helper);
		/** Create the mesh resources for a mesh component */
		std::shared_ptr<MeshResources> CreateMesh(StaticMesh const& mesh, VkCommandPool pool, MeshCreationHelper& helper);

		//InitImGUI(VulkanLogicalDevice& logical, VulkanPhysicalDevice& physical, Surface& surface);
	};
}
