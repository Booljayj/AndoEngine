#include "Rendering/RenderingSystem.h"
#include "Engine/StandardTypes.h"
#include "Engine/Utility.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/Material.h"
#include "Rendering/MeshRenderer.h"
#include "Rendering/Shader.h"
#include "Rendering/StaticMesh.h"
#include "Rendering/Vulkan/VulkanRenderPasses.h"
#include "Rendering/Uniforms.h"

DEFINE_LOG_CATEGORY(Rendering, Info);

namespace Rendering {
	bool RenderingSystem::Startup(HAL::WindowingSystem& windowing, Resources::Cache<Material>& materials, Resources::Cache<StaticMesh>& staticMeshes) {
		//The primary window must exist in order to start the rendering system
		HAL::Window& primaryWindow = windowing.GetPrimaryWindow();
		
		// Vulkan instance
		if (!framework.Create(primaryWindow)) return false;

		//Create the primary surface for the primary window
		if (!CreateSurface(primaryWindow)) return false;
		
		Surface& primarySurface = GetPrimarySurface();

		// Collect physical devices and select default one
		{
			t_vector<char const*> const extensionNames = VulkanPhysicalDevice::GetExtensionNames();

			uint32_t deviceCount = 0;
			vkEnumeratePhysicalDevices(framework.instance, &deviceCount, nullptr);
			t_vector<VkPhysicalDevice> devices{ deviceCount };
			vkEnumeratePhysicalDevices(framework.instance, &deviceCount, devices.data());

			for (uint32_t deviceIndex = 0; deviceIndex < deviceCount; ++deviceIndex) {
				const VulkanPhysicalDevice physicalDevice = primarySurface.GetPhysicalDevice(devices[deviceIndex]);
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
		passes.emplace(logical, primarySurfaceFormat.format);
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
		primarySurface.CreateSwapchain(*selectedPhysical, primarySurfaceFormat, *passes);

		//Listen for when rendering-relevant resource types are created or destroyed
		materials.Created.Add(this, &RenderingSystem::MaterialCreated);
		materials.Destroyed.Add(this, &RenderingSystem::MaterialDestroyed);
		staticMeshes.Created.Add(this, &RenderingSystem::StaticMeshCreated);
		staticMeshes.Destroyed.Add(this, &RenderingSystem::StaticMeshDestroyed);

		//@todo Create a group for renderable entities, once we have more than one component to include in the group (i.e. renderer and transform).

		return true;
	}

	bool RenderingSystem::Shutdown(Resources::Cache<Material>& materials, Resources::Cache<StaticMesh>& staticMeshes) {
		availablePhysicalDevices.clear();

		if (logical) {
			vkDeviceWaitIdle(logical.device);

			surfaces.clear();

			materials.Destroy();
			staticMeshes.Destroy();
			DestroyStalePipelines();
			DestroyStaleMeshes();

			uniformLayouts.Destroy(logical);
			passes.reset();
			vkDestroyCommandPool(logical.device, commandPool, nullptr);
		}
		logical.Destroy();
		framework.Destroy();
		return true;
	}

	bool RenderingSystem::Render(entt::registry& registry) {
		//Recreate swapchains if necessary. This happens periodically if the rendering parameters have changed significantly.
		for (auto const& surface : surfaces) {
			if (surface->IsSwapchainDirty()) {
				LOG(Rendering, Info, "Recreating swapchain");
				vkDeviceWaitIdle(logical.device);
				surface->RecreateSwapchain(*selectedPhysical, primarySurfaceFormat, *passes);
			}
		}

		//Rebuild any resources, creating new ones and destroying stale ones
		RebuildResources();

		bool success = true;
		for (auto const& surface : surfaces) {
			success &= surface->Render(logical, *passes, registry);
		}

		return success;
	}

	void RenderingSystem::RebuildResources() {
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
		if (dirtyStaticMeshes.size() > 0) {
			LOG(Rendering, Info, "Creating new meshes");
			RefreshStaticMeshes();
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
				primarySurfaceFormat = GetPrimarySurface().GetPreferredSurfaceFormat(*selectedPhysical);

				VulkanVersion const version = selectedPhysical->GetDriverVersion();
				LOGF(Rendering, Info, "Selected device %s (%i.%i.%i)", selectedPhysical->properties.deviceName, version.major, version.minor, version.patch);
				return true;
			}
		}
		return false;
	}

	Surface* RenderingSystem::CreateSurface(HAL::Window& window) {
		//Check if we already have a surface for this window, and return it if we do
		if (Surface* existing = FindSurface(window.id)) return existing;

		//Create the new surface and verify that it is functional
		std::unique_ptr<Surface> surface = std::make_unique<Surface>(*this, window);
		if (!surface->IsValid()) return nullptr;

		//Add the surface to the collection of surfaces, and return a raw pointer to it
		return surfaces.emplace_back(std::move(surface)).get();
	}

	Surface* RenderingSystem::FindSurface(uint32_t id) const {
		const auto iter = std::find_if(surfaces.begin(), surfaces.end(), [&](const auto& surface) { return surface->GetID() == id; });
		if (iter != surfaces.end()) return iter->get();
		else return nullptr;
	}

	void RenderingSystem::DestroySurface(uint32_t id) {
		const auto iter = std::find_if(surfaces.begin(), surfaces.end(), [=](auto const& surface) { return surface->GetID() == id; });
		if (iter != surfaces.end() && iter != surfaces.begin()) {
			(*iter)->Destroy(framework, logical);
			surfaces.erase(iter);
		} else {
			LOGF(Rendering, Warning, "Unable to destroy surface with id %i", id);
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
		VulkanPipelineCreationHelper helper{logical.device};

		for (Resources::Handle<Material> const& material : dirtyMaterials) {
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

	void RenderingSystem::RefreshStaticMeshes() {
		VulkanMeshCreationHelper helper{ logical.device, logical.queues.graphics, commandPool};

		for (const Resources::Handle<StaticMesh>& mesh : dirtyStaticMeshes) {
			//Existing resources become stale
			if (mesh->gpuResources) staleMeshResources.emplace_back(std::move(*mesh->gpuResources));
			//Create new resources
			mesh->gpuResources.emplace(CreateMesh(*mesh, commandPool, helper));
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
		stalePipelineResources.clear();
	}

	bool RenderingSystem::IsUsablePhysicalDevice(const VulkanPhysicalDevice& physicalDevice, TArrayView<char const*> const& extensionNames) {
		return physicalDevice.HasRequiredQueues() && physicalDevice.HasRequiredExtensions(extensionNames) && physicalDevice.HasSwapchainSupport();
	}

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
				uniformLayouts.object,
				//resources.descriptors,
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
		pipelineCI.renderPass = passes->surface;
		pipelineCI.subpass = 0;
		//Parent pipelines, if this pipeline derives from another
		pipelineCI.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineCI.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(logical.device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &resources.pipeline) != VK_SUCCESS) {
			LOGF(Temp, Error, "Failed to create pipeline for material %i", material.id.ToValue());
		}

		return resources;
	}

	MeshResources RenderingSystem::CreateMesh(StaticMesh const& mesh, VkCommandPool pool, VulkanMeshCreationHelper& helper) {
		//Calculate byte size values for the input mesh
		const auto BufferSizeVisitor = [](auto const& v) { return v.size() * sizeof(typename std::remove_reference_t<decltype(v)>::value_type); };
		size_t const vertexBytes = std::visit(BufferSizeVisitor, mesh.vertices);
		size_t const indexBytes = std::visit(BufferSizeVisitor, mesh.indices);
		size_t const totalBytes = vertexBytes + indexBytes;

		size_t const vertexOffset = 0;
		size_t const indexOffset = vertexBytes;

		//Create the staging buffer used to upload data to the GPU-only buffer
		MappedBuffer staging{ logical.allocator, totalBytes, BufferUsageBits::TransferSrc, VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY };

		//Fill the staging buffer with the source data
		const auto VertexWriteVisitor = [&](auto const& v) { staging.Write(v.data(), v.size() * sizeof(typename std::remove_reference_t<decltype(v)>::value_type), vertexOffset); };
		std::visit(VertexWriteVisitor, mesh.vertices);
		const auto IndexWriteVisitor = [&](auto const& v) { staging.Write(v.data(), v.size() * sizeof(typename std::remove_reference_t<decltype(v)>::value_type), indexOffset); };
		std::visit(IndexWriteVisitor, mesh.indices);

		//Create the final resources that will be used for this mesh
		MeshResources resources{ logical.allocator, totalBytes };

		//Allocate a command buffer for the transfer from the staging to the target buffer
		VkCommandBufferAllocateInfo bufferAI{};
		bufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAI.commandPool = pool;
		bufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferAI.commandBufferCount = 1;

		TUniquePoolHandles<1, &vkFreeCommandBuffers> tempCommands{ logical.device, pool };
		if (vkAllocateCommandBuffers(logical.device, &bufferAI, *tempCommands) != VK_SUCCESS) {
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
