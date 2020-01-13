#pragma once
#include <memory>
#include <vulkan/vulkan.hpp>
#include <ostream>
#include "Engine/Context.h"
#include "Engine/Logging/Logger.h"
#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/EntityFilter.h"

struct TransformComponent;
struct MeshRendererComponent;
struct EntityCollectionSystem;

struct VulkanVersion {
	const uint32_t patch : 12;
	const uint32_t minor : 10;
	const uint32_t major : 10;

	VulkanVersion(const uint32_t& value)
	: patch(0)
	, minor(0)
	, major(0)
	{
		static_assert(sizeof(VulkanVersion) == sizeof(uint32_t), "Bits of VulkanVersion must be directly copyable from a uint32_t");
		memcpy(this, &value, sizeof(uint32_t));
	}
};
inline std::ostream& operator<<(std::ostream&, VulkanVersion const&);

struct VulkanPhysicalDeviceInfo {
	/** The physical device handle */
	vk::PhysicalDevice device;
	/** The properties of this physical device (such as device limits) */
	vk::PhysicalDeviceProperties deviceProperties;
	/** The features available on this physical device */
	vk::PhysicalDeviceFeatures deviceFeatures;
	/** The memory properties of this physical device */
	vk::PhysicalDeviceMemoryProperties deviceMemoryProperties;

	static VulkanPhysicalDeviceInfo Extract(vk::PhysicalDevice const& device);
	/** Write a description of this device */
	void Write(std::ostream& stream) const;
};

class RenderingSystem {
private:
	static constexpr size_t FILTER_SIZE = 2;
	std::shared_ptr<EntityFilter<FILTER_SIZE>> Filter;
	TComponentHandle<TransformComponent> TransformHandle;
	TComponentHandle<MeshRendererComponent> MeshRendererHandle;

	// Vulkan instance, stores all per-application states
	vk::Instance instance;
	/** The available physical devices which can be used */
	std::vector<vk::PhysicalDevice> availablePhysicalDevices;
	/** The properties of the currently selected physical device */
	VulkanPhysicalDeviceInfo selectedPhysicalDeviceInfo;

public:
	bool Startup(
		CTX_ARG,
		EntityCollectionSystem* EntityCollection,
		TComponentInfo<TransformComponent>* Transform,
		TComponentInfo<MeshRendererComponent>* MeshRenderer
	);
	bool Shutdown(CTX_ARG);

	void RenderFrame(float InterpolationAlpha) const;
	static void RenderComponent(MeshRendererComponent const* MeshRenderer);
};
