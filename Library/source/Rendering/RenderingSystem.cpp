#include "Rendering/RenderingSystem.h"
#include "Engine/StandardTypes.h"
#include "Engine/Utility.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/Materials.h"
#include "Rendering/MeshComponent.h"
#include "Rendering/MeshRendererComponent.h"
#include "Rendering/Vulkan/VulkanRenderPasses.h"
#include "Rendering/Uniforms.h"

DEFINE_LOG_CATEGORY(Rendering, Info);

namespace Rendering {
	bool RenderingSystem::Startup(HAL::WindowingSystem& windowing, EntityRegistry& registry, MaterialDatabase& materials) {
		// Vulkan instance
		if (!framework.Create(windowing.GetPrimaryWindow())) return false;

		//Create the primary surface for the primary window
		primarySurface = CreateSurface(windowing.GetPrimaryWindow());
		if (!primarySurface) return false;

		// Collect physical devices and select default one
		{
			t_vector<char const*> const extensionNames = VulkanPhysicalDevice::GetExtensionNames();

			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(framework.instance, &deviceCount, nullptr);
			t_vector<VkPhysicalDevice> devices{ deviceCount };
			vkEnumeratePhysicalDevices(framework.instance, &deviceCount, devices.data());

			for (uint32_t deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
				const VulkanPhysicalDevice physicalDevice = primarySurface->GetPhysicalDevice(devices[deviceIndex]);
				if (IsUsablePhysicalDevice(physicalDevice, extensionNames)) {
					availablePhysicalDevices.push_back(physicalDevice);
				}
			}

			if (availablePhysicalDevices.size() == 0) {
				LOG(Rendering, Error, "Failed to find any vulkan physical devices");
				return false;
			}

			if (!SelectPhysicalDevice(0)) {
				LOG(Rendering, Error, "Failed to select default physical device");
				return false;
			}
		}

		//Create the render passes
		if (!passes.Create(logical, primarySurfaceFormat.format)) return false;
		if (!uniformLayouts.Create(logical)) return false;

		//Create the command pool
		{
			VkCommandPoolCreateInfo poolInfo{};
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.queueFamilyIndex = selectedPhysical->queues.graphics.value().index;
			poolInfo.flags = 0; // Optional

			if (vkCreateCommandPool(logical.device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
				LOG(Vulkan, Error, "Failed to create command pool for rendering system");
				return false;
			}
		}

		//Create the swapchain on the primary surface
		primarySurface->CreateSwapchain(*selectedPhysical, primarySurfaceFormat, passes);

		//Bind this rendering system as a context object callbacks can refer to it.
		registry.Bind(this);
		registry.Callbacks<MeshComponent>()
			.Create<&MeshComponentOperations::OnCreate>()
			.Destroy<&MeshComponentOperations::OnDestroy>()
			.Modify<&MeshComponentOperations::OnModify>();

		//Listen for when rendering-relevant resource types are created or destroyed
		materials.Created.Add(this, &RenderingSystem::MaterialCreated);
		materials.Destroyed.Add(this, &RenderingSystem::MaterialDestroyed);

		//@todo Create a group for renderable entities, once we have more than one component to include in the group (i.e. renderer and transform).

		return true;
	}

	bool RenderingSystem::Shutdown(EntityRegistry& registry, MaterialDatabase& materials) {
		availablePhysicalDevices.clear();

		if (logical) {
			//Wait for any in-progress work to finish before we start cleanup
			for (auto const& surface : surfaces) {
				surface->WaitForCompletion(logical);
			}

			registry.DestroyComponents<MeshComponent>();
			materials.Destroy();
			DestroyStalePipelines();
			DestroyStaleMeshes();

			for (auto const& surface : surfaces) {
				surface->Destroy(framework, logical);
			}

			uniformLayouts.Destroy(logical);
			passes.Destroy(logical);
			vkDestroyCommandPool(logical.device, commandPool, nullptr);
		}
		logical.Destroy();
		framework.Destroy();
		return true;
	}

	bool RenderingSystem::Render(EntityRegistry& registry) {
		//Recreate swapchains if necessary. This happens periodically if the rendering parameters have changed significantly.
		for (auto const& surface : surfaces) {
			if (surface->IsSwapchainDirty()) {
				LOG(Rendering, Info, "Recreating swapchain");
				vkDeviceWaitIdle(logical.device);
				surface->RecreateSwapchain(*selectedPhysical, primarySurfaceFormat, passes);
			}
		}

		//Rebuild any resources, creating new ones and destroying stale ones
		RebuildResources(registry);

		bool success = true;
		for (auto const& surface : surfaces) {
			success &= surface->Render(logical, passes, registry);
		}

		return success;
	}

	void RenderingSystem::RebuildResources(EntityRegistry& registry) {
		const bool hasStaleResources = stalePipelineResources.size() > 0 || staleMeshResources.size() > 0;
		if (hasStaleResources) {
			LOG(Rendering, Info, "Destroying stale resources");
			//Wait until the device is idle so we don't destroy resources that are in use
			vkDeviceWaitIdle(logical.device);

			DestroyStalePipelines();
			DestroyStaleMeshes();
		}

		if (dirtyMaterials.size() > 0) {
			LOG(Rendering, Info, "Creating new pipelines");
			RefreshMaterials();
		}
		if (shouldCreateMeshes) {
			shouldCreateMeshes = false;
			LOG(Rendering, Info, "Creating new meshes");
			CreateMeshes(registry);
		}
	}

	bool RenderingSystem::SelectPhysicalDevice(size_t index) {
		if (index != selectedPhysicalIndex) {
			if (VulkanPhysicalDevice const* newPhysical = GetPhysicalDevice(index)) {
				TArrayView<char const*> const extensions = VulkanPhysicalDevice::GetExtensionNames();

				VulkanLogicalDevice newLogical = VulkanLogicalDevice::Create(framework, *newPhysical, features, extensions);
				if (!newLogical) {
					LOGF(Rendering, Error, "Failed to create logical device for physical device %i", index);
					return false;
				}
				logical = std::move(newLogical);

				selectedPhysical = newPhysical;
				selectedPhysicalIndex = index;
				primarySurfaceFormat = primarySurface->GetPreferredSurfaceFormat(*selectedPhysical);

				VulkanVersion const version = selectedPhysical->GetDriverVersion();
				LOGF(Rendering, Info, "Selected device %s (%i.%i.%i)", selectedPhysical->properties.deviceName, version.major, version.minor, version.patch);
				return true;
			}
		}
		return false;
	}

	Surface* RenderingSystem::CreateSurface(HAL::Window window) {
		//Check if we already have a surface for this window, and return it if we do
		if (Surface* existing = FindSurface(window.id)) return existing;

		//Create the new surface and verify that it is functional
		std::unique_ptr<Surface> surface = std::make_unique<Surface>(*this, window);
		if (!surface->IsValidSurface()) return nullptr;

		//Add the surface to the collection of surfaces, and return a raw pointer to it
		surfaces.emplace_back(std::move(surface));
		return surfaces.back().get();
	}

	Surface* RenderingSystem::FindSurface(uint32_t id) const {
		const auto iter = std::find_if(surfaces.begin(), surfaces.end(), [&](const auto& surface) { return surface->id == id; });
		if (iter != surfaces.end()) return iter->get();
		else return nullptr;
	}

	void RenderingSystem::DestroySurface(uint32_t id) {
		//The primary surface cannot be destroyed through this method, only during shutdown
		if (primarySurface->id != id) {
			const auto iter = std::find_if(surfaces.begin(), surfaces.end(), [&](const auto& surface) { return surface->id == id; });
			if (iter != surfaces.end()) {
				(*iter)->Destroy(framework, logical);
				surfaces.erase(iter);
			} else {
				LOGF(Rendering, Warning, "Unable to destroy surface, no surface found with id %i", id);
			}
		}
	}

	void RenderingSystem::MaterialCreated(Resources::Handle<Material> const& material) {
		MarkMaterialDirty(material);
	}

	void RenderingSystem::MaterialDestroyed(Resources::Handle<Material> const& material) {
		MarkMaterialStale(material);
	}

	void RenderingSystem::MeshComponentOperations::OnCreate(entt::registry& registry, entt::entity entity) {
		RenderingSystem* rendering = registry.ctx().get<RenderingSystem*>();
		rendering->shouldCreateMeshes = true;
	}

	void RenderingSystem::MeshComponentOperations::OnDestroy(entt::registry& registry, entt::entity entity) {
		RenderingSystem* rendering = registry.ctx().get<RenderingSystem*>();
		MeshComponent& mesh = registry.get<MeshComponent>(entity);
		rendering->MarkMeshStale(mesh);
	}

	void RenderingSystem::MeshComponentOperations::OnModify(entt::registry& registry, entt::entity entity) {
		RenderingSystem* rendering = registry.ctx().get<RenderingSystem*>();
		MeshComponent& mesh = registry.get<MeshComponent>(entity);
		rendering->MarkMeshStale(mesh);
		rendering->shouldCreateMeshes = true;
	}

	void RenderingSystem::RefreshMaterials() {
		//The library of shader modules that will stay loaded as long as we need to continue creating pipelines
		VulkanPipelineCreationHelper helper{logical.device};

		for (const Resources::Handle<Material>& material : dirtyMaterials) {
			//Existing resources become stale
			if (material->resources) stalePipelineResources.emplace_back(std::move(material->resources));

			//Create new resources
			VulkanPipelineResources resources = CreatePipeline(*material, helper);
			if (resources) material->resources = std::move(resources);
			else resources.Destroy(logical.device);
		}
		dirtyMaterials.clear();
	}

	void RenderingSystem::MarkMaterialDirty(Resources::Handle<Material> const& material) {
		//Keep track of the dirty material so we can refresh it during the next render
		dirtyMaterials.emplace_back(material);
	}

	void RenderingSystem::MarkMaterialStale(Resources::Handle<Material> const& material) {
		//Steal the resources from the material so we can destroy them later when they're no longer being used
		stalePipelineResources.emplace_back(std::move(material->resources));
	}

	void RenderingSystem::DestroyStalePipelines() {
		for (VulkanPipelineResources const& resources : stalePipelineResources) {
			resources.Destroy(logical.device);
		}
		stalePipelineResources.clear();
	}

	void RenderingSystem::CreateMeshes(EntityRegistry& registry) {
		VulkanMeshCreationHelper helper{logical.device, logical.allocator, logical.queues.graphics, commandPool};

		auto const meshes = registry.GetView<MeshComponent>();
		for (const auto id : meshes) {
			MeshComponent& mesh = meshes.Get<MeshComponent>(id);
			if (!mesh.resources) {
				VulkanMeshResources const resources = CreateMesh(mesh, id, commandPool, helper);
				if (resources) mesh.resources = resources;
				else resources.Destroy(logical.allocator);
			}
		}
		helper.Flush();
	}

	void RenderingSystem::MarkMeshStale(MeshComponent& mesh) {
		staleMeshResources.push_back(mesh.resources);
		mesh.resources = {};
	}

	void RenderingSystem::DestroyStaleMeshes() {
		for (VulkanMeshResources resources : staleMeshResources) {
			resources.Destroy(logical.allocator);
		}
		stalePipelineResources.clear();
	}

	bool RenderingSystem::IsUsablePhysicalDevice(const VulkanPhysicalDevice& physicalDevice, TArrayView<char const*> const& extensionNames) {
		return physicalDevice.HasRequiredQueues() && physicalDevice.HasRequiredExtensions(extensionNames) && physicalDevice.HasSwapchainSupport();
	}

#pragma optimize("", off)
	VulkanPipelineResources RenderingSystem::CreatePipeline(Material const& material, VulkanPipelineCreationHelper& helper) {
		VulkanPipelineResources resources;

		//Create the descriptor set layout
		{
			//@todo Actually create this on a per-material basis
			resources.descriptorSetLayout = nullptr;
		}

		//Create the pipeline layout
		{
			VkDescriptorSetLayout setLayouts[] = {
				uniformLayouts.global,
				//resources.descriptors,
				uniformLayouts.object,
			};

			VkPipelineLayoutCreateInfo layoutCI{};
			layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			layoutCI.setLayoutCount = static_cast<uint32_t>(std::size(setLayouts));
			layoutCI.pSetLayouts = setLayouts;
			layoutCI.pushConstantRangeCount = 0; // Optional
			layoutCI.pPushConstantRanges = nullptr; // Optional

			if (vkCreatePipelineLayout(logical.device, &layoutCI, nullptr, &resources.pipelineLayout) != VK_SUCCESS) {
				LOGF(Temp, Error, "Failed to create pipeline layout for material %i", material.id.ToValue());
				return resources;
			}
		}

		VkShaderModule const vertShaderModule = helper.GetModule(material.vertex);
		VkShaderModule const fragShaderModule = helper.GetModule(material.fragment);
		if (!vertShaderModule || !fragShaderModule) return resources;

		VkPipelineShaderStageCreateInfo vertShaderStageCI{};
		vertShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageCI.module = vertShaderModule;
		vertShaderStageCI.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageCI{};
		fragShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageCI.module = fragShaderModule;
		fragShaderStageCI.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCI, fragShaderStageCI};

		//Vertex Input function
		auto const bindings = Vertex_Simple::GetBindingDescription();
		auto const attributes = Vertex_Simple::GetAttributeDescriptions();

		VkPipelineVertexInputStateCreateInfo vertexInputCI{};
		vertexInputCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCI.vertexBindingDescriptionCount = 1;
		vertexInputCI.pVertexBindingDescriptions = &bindings;
		vertexInputCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
		vertexInputCI.pVertexAttributeDescriptions = attributes.data();

		//Input assembly function
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyCI{};
		inputAssemblyCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyCI.primitiveRestartEnable = VK_FALSE;

		//Viewport
		VkPipelineViewportStateCreateInfo viewportStateCI{};
		viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCI.viewportCount = 1;
		viewportStateCI.pViewports = nullptr; //provided dynamically
		viewportStateCI.scissorCount = 1;
		viewportStateCI.pScissors = nullptr; //provided dynamically

		//Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizerCI{};
		rasterizerCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizerCI.depthClampEnable = VK_FALSE;
		rasterizerCI.rasterizerDiscardEnable = VK_FALSE;
		rasterizerCI.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizerCI.lineWidth = 1.0f;
		rasterizerCI.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizerCI.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizerCI.depthBiasEnable = VK_FALSE;
		rasterizerCI.depthBiasConstantFactor = 0.0f;
		rasterizerCI.depthBiasClamp = 0.0f;
		rasterizerCI.depthBiasSlopeFactor = 0.0f;

		//Multisampling
		//Disabled for now, requires a GPU feature to enable
		VkPipelineMultisampleStateCreateInfo multisamplingCI{};
		multisamplingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisamplingCI.sampleShadingEnable = VK_FALSE;
		multisamplingCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisamplingCI.minSampleShading = 1.0f;
		multisamplingCI.pSampleMask = nullptr;
		multisamplingCI.alphaToCoverageEnable = VK_FALSE;
		multisamplingCI.alphaToOneEnable = VK_FALSE;

		//Color blending (alpha blending setup)
		VkPipelineColorBlendAttachmentState colorBlendAttachment{};
		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_TRUE;
		colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

		VkPipelineColorBlendStateCreateInfo colorBlendingCI{};
		colorBlendingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendingCI.logicOpEnable = VK_FALSE;
		colorBlendingCI.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlendingCI.attachmentCount = 1;
		colorBlendingCI.pAttachments = &colorBlendAttachment;
		colorBlendingCI.blendConstants[0] = 0.0f; // Optional
		colorBlendingCI.blendConstants[1] = 0.0f; // Optional
		colorBlendingCI.blendConstants[2] = 0.0f; // Optional
		colorBlendingCI.blendConstants[3] = 0.0f; // Optional

		//Dynamic state
		VkDynamicState dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicStateCI{};
		dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCI.dynamicStateCount = 2;
		dynamicStateCI.pDynamicStates = dynamicStates;

		//Final pipeline creation
		VkGraphicsPipelineCreateInfo pipelineCI{};
		pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		//Programmable stages
		pipelineCI.stageCount = 2;
		pipelineCI.pStages = shaderStages;
		//Fixed states
		pipelineCI.pVertexInputState = &vertexInputCI;
		pipelineCI.pInputAssemblyState = &inputAssemblyCI;
		pipelineCI.pViewportState = &viewportStateCI;
		pipelineCI.pRasterizationState = &rasterizerCI;
		pipelineCI.pMultisampleState = &multisamplingCI;
		pipelineCI.pDepthStencilState = nullptr; // Optional
		pipelineCI.pColorBlendState = &colorBlendingCI;
		pipelineCI.pDynamicState = &dynamicStateCI;
		//Additional data
		pipelineCI.layout = resources.pipelineLayout;
		pipelineCI.renderPass = passes.surface.pass;
		pipelineCI.subpass = 0;
		//Parent pipelines, if this pipeline derives from another
		pipelineCI.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineCI.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(logical.device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &resources.pipeline) != VK_SUCCESS) {
			LOGF(Temp, Error, "Failed to create pipeline for material %i", material.id.ToValue());
		}

		return resources;
	}
#pragma optimize("", on)

	VulkanMeshResources RenderingSystem::CreateMesh(MeshComponent const& mesh, EntityID id, VkCommandPool pool, VulkanMeshCreationHelper& helper) {
		VulkanMeshResources resources;

		//Calculate byte size values for the input mesh
		size_t const vertexBytes = sizeof(decltype(MeshComponent::vertices)::value_type) * mesh.vertices.size();
		size_t const indexBytes = sizeof(decltype(MeshComponent::indices)::value_type) * mesh.indices.size();
		size_t const totalBytes = vertexBytes + indexBytes;

		size_t const vertexOffset = 0;
		size_t const indexOffset = vertexBytes;

		//Create the staging buffer used to upload data to the GPU-only buffer
		constexpr VkBufferUsageFlags stagingBufferFlags = VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		constexpr VmaMemoryUsage stagingAllocationFlags = VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY;
		VulkanMappedBuffer const staging = CreateMappedBuffer(logical.allocator, totalBytes, stagingBufferFlags, stagingAllocationFlags);
		if (!staging) {
			LOG(Vulkan, Error, "Failed to create staging buffer");
			return resources;
		}

		//Fill the staging buffer with the source data
		staging.WriteArray<Vertex_Simple>(mesh.vertices, vertexOffset);
		staging.WriteArray<uint32_t>(mesh.indices, indexOffset);

		//Create the target buffer that will contain the uploaded data for the GPU to use
		constexpr VkBufferUsageFlags targetBufferFlags =
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		constexpr VmaMemoryUsage targetMemoryFlags = VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
		resources.buffer = CreateBuffer(logical.allocator, totalBytes, targetBufferFlags, targetMemoryFlags);
		if (!resources.buffer) {
			LOG(Vulkan, Error, "Failed to create gpu buffer");
			helper.Submit(staging, nullptr);
			return resources;
		}

		//Allocate a command buffer for the transfer from the staging to the target buffer
		VkCommandBufferAllocateInfo bufferAI{};
		bufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAI.commandPool = pool;
		bufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferAI.commandBufferCount = 1;

		VkCommandBuffer commands;
		if (vkAllocateCommandBuffers(logical.device, &bufferAI, &commands) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to allocate command buffers");
			helper.Submit(staging, nullptr);
			return resources;
		}

		//Begin recording the command buffer for staging commands
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(commands, &beginInfo) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to begin recording command buffer");
			helper.Submit(staging, nullptr);
			return resources;
		}

		//Record the command to transfer from the staging buffer to the target buffer
		VkBufferCopy copy{};
		copy.srcOffset = 0; // Optional
		copy.dstOffset = 0; // Optional
		copy.size = totalBytes;
		vkCmdCopyBuffer(commands, staging.buffer, resources.buffer, 1, &copy);

		//Finish recording the command buffer
		if (vkEndCommandBuffer(commands) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to finish recording command buffer");
			helper.Submit(staging, nullptr);
			return resources;
		}

		//Submit the buffer and commands. We've successfully created everything
		helper.Submit(staging, commands);

		//Record the size and offset information. This is the primary way we'll know that the resources are valid
		resources.offset.vertex = 0;
		resources.offset.index = vertexBytes;
		resources.size.vertices = static_cast<uint32_t>(mesh.vertices.size());
		resources.size.indices = static_cast<uint32_t>(mesh.indices.size());
		return resources;
	}
}
