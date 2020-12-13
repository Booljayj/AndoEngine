#include "Rendering/RenderingSystem.h"
#include "Engine/LogCommands.h"
#include "Engine/STL.h"
#include "Engine/Utility.h"
#include "Geometry/GLM.h"
#include "HAL/WindowingSystem.h"
#include "Rendering/MaterialComponent.h"
#include "Rendering/MeshComponent.h"
#include "Rendering/MeshRendererComponent.h"

DEFINE_LOG_CATEGORY(Rendering, Warning);

namespace Rendering {
	RenderingSystem::RenderingSystem()
	: retryCount(0)
	, shouldRecreateSwapchain(false)
	, shouldCreatePipelines(false)
	, shouldCreateMeshes(false)
	{}

	bool RenderingSystem::Startup(CTX_ARG, HAL::WindowingSystem& windowing, EntityRegistry& registry) {
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

		//Bind this rendering system as a context object callbacks can refer to it.
		registry.Bind(this);
		//Add the callbacks for rendering-related components.
		registry.Callbacks<MaterialComponent>()
			.Create<MaterialComponentOperations::OnCreate>()
			.Destroy<MaterialComponentOperations::OnDestroy>()
			.Modify<MaterialComponentOperations::OnModify>();
		registry.Callbacks<MeshComponent>()
			.Create<MeshComponentOperations::OnCreate>()
			.Destroy<MeshComponentOperations::OnDestroy>()
			.Modify<MaterialComponentOperations::OnModify>();

		//@todo Create a group for renderable entities, once we have more than one component to include in the group (i.e. renderer and transform).

		return true;
	}

	bool RenderingSystem::Shutdown(CTX_ARG, EntityRegistry& registry) {
		availablePhysicalDevices.clear();

		//Wait for any in-progress work to finish before we start cleanup
		if (organizer) organizer.WaitForCompletion(CTX, logical);

		registry.DestroyComponents<MaterialComponent, MeshComponent>();
		DestroyStalePipelines();
		DestroyStaleMeshes();

		DestroyRenderPasses(CTX);
		organizer.Destroy(logical);
		swapchain.Destroy(logical);
		logical.Destroy();
		framework.Destroy();
		return true;
	}

	bool RenderingSystem::Render(CTX_ARG, EntityRegistry& registry) {
		//Recreate the swapchain if necessary. This happens periodically if the rendering parameters have changed significantly.
		if (shouldRecreateSwapchain) {
			shouldRecreateSwapchain = false;
			shouldCreatePipelines = true;

			LOG(Rendering, Info, "Recreating swapchain");
			vkDeviceWaitIdle(logical.device);

			//Destroy resources that depend on the swapchain
			organizer.Destroy(logical);
			DestroyPipelines(registry);
			DestroyRenderPasses(CTX);

			//Recreate the swapchain and everything depending on it. If any of this fails, we need to request a shutdown.
			if (!swapchain.Recreate(CTX, VkExtent2D{1024, 768}, framework.surface, *selectedPhysical, logical)) return false;
			if (!organizer.Create(CTX, *selectedPhysical, logical, EBuffering::Double, swapchain.views.size(), 1)) return false;
			if (!CreateRenderPasses(CTX)) return false;
		}

		//Rebuild any resources, creating new ones and destroying stale ones
		RebuildResources(CTX, registry);

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
		auto const meshes = registry.GetView<MeshComponent const>();

		auto const recorder = [&](VkCommandBuffer commands, size_t index) {
			ScopedRenderPass pass{commands, main, index, VkOffset2D{0, 0}, swapchain.extent};

			for (const auto id : renderables) {
				auto& renderer = renderables.Get<MeshRendererComponent const>(id);

				auto const* material = materials.Find<MaterialComponent const>(renderer.material);
				auto const* mesh = meshes.Find<MeshComponent const>(renderer.mesh);
				if (material && material->resources && mesh && mesh->resources) {
					//Bind the pipeline that will be used for rendering
					vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, material->resources.pipeline);

					//Bind the vetex and index buffers for the mesh
					VkDeviceSize const offset = 0;
					vkCmdBindVertexBuffers(commands, 0, 1, &mesh->resources.vertex.buffer, &offset);
					vkCmdBindIndexBuffer(commands, mesh->resources.index.buffer, 0, VK_INDEX_TYPE_UINT32);

					//Submit the command to draw using the vertex and index buffers
					vkCmdDrawIndexed(commands, static_cast<uint32_t>(mesh->indices.size()), 1, 0, 0, 0);
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

	void RenderingSystem::RebuildResources(CTX_ARG, EntityRegistry& registry) {
		const bool hasStaleResources = stalePipelineResources.size() > 0 || staleMeshResources.size() > 0;
		if (hasStaleResources) {
			LOG(Rendering, Info, "Destroying stale resources");
			//Wait until the device is idle so we don't destroy resources that are in use
			vkDeviceWaitIdle(logical.device);

			DestroyStalePipelines();
			DestroyStaleMeshes();
		}

		if (shouldCreatePipelines) {
			shouldCreatePipelines = false;
			LOG(Rendering, Info, "Creating new pipelines");
			CreatePipelines(CTX, registry);
		}
		if (shouldCreateMeshes) {
			shouldCreateMeshes = false;
			LOG(Rendering, Info, "Creating new meshes");
			CreateMeshes(CTX, registry);
		}
	}

	bool RenderingSystem::SelectPhysicalDevice(CTX_ARG, uint32_t index) {
		if (index != selectedPhysicalIndex) {
			if (VulkanPhysicalDevice const* newPhysical = GetPhysicalDevice(index)) {
				TArrayView<char const*> const extensions = VulkanPhysicalDevice::GetExtensionNames(CTX);

				VulkanLogicalDevice newLogical = VulkanLogicalDevice::Create(CTX, framework, *newPhysical, features, extensions);
				if (!newLogical) {
					LOGF(Rendering, Error, "Failed to create logical device for physical device %i", index);
					return false;
				}
				logical = std::move(newLogical);

				selectedPhysical = newPhysical;
				selectedPhysicalIndex = index;
				shouldRecreateSwapchain = true;

				VulkanVersion const version = selectedPhysical->GetDriverVersion();
				LOGF(Rendering, Info, "Selected device %s (%i.%i.%i)", selectedPhysical->properties.deviceName, version.major, version.minor, version.patch);
				return true;
			}
		}
		return false;
	}

	void RenderingSystem::MaterialComponentOperations::OnCreate(entt::registry& registry, entt::entity entity) {
		RenderingSystem* rendering = registry.ctx<RenderingSystem*>();
		rendering->shouldCreatePipelines = true;
	}

	void RenderingSystem::MaterialComponentOperations::OnDestroy(entt::registry& registry, entt::entity entity) {
		RenderingSystem* rendering = registry.ctx<RenderingSystem*>();
		MaterialComponent& material = registry.get<MaterialComponent>(entity);
		rendering->MarkPipelineStale(material);
	}

	void RenderingSystem::MaterialComponentOperations::OnModify(entt::registry& registry, entt::entity entity) {
		RenderingSystem* rendering = registry.ctx<RenderingSystem*>();
		MaterialComponent& material = registry.get<MaterialComponent>(entity);
		rendering->MarkPipelineStale(material);
		rendering->shouldCreatePipelines = true;
	}

	void RenderingSystem::MeshComponentOperations::OnCreate(entt::registry& registry, entt::entity entity) {
		RenderingSystem* rendering = registry.ctx<RenderingSystem*>();
		rendering->shouldCreateMeshes = true;
	}

	void RenderingSystem::MeshComponentOperations::OnDestroy(entt::registry& registry, entt::entity entity) {
		RenderingSystem* rendering = registry.ctx<RenderingSystem*>();
		MeshComponent& mesh = registry.get<MeshComponent>(entity);
		rendering->MarkMeshStale(mesh);
	}

	void RenderingSystem::MeshComponentOperations::OnModify(entt::registry& registry, entt::entity entity) {
		RenderingSystem* rendering = registry.ctx<RenderingSystem*>();
		MeshComponent& mesh = registry.get<MeshComponent>(entity);
		rendering->MarkMeshStale(mesh);
		rendering->shouldCreateMeshes = true;
	}

	bool RenderingSystem::CreateRenderPasses(CTX_ARG) {
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

	void RenderingSystem::CreatePipelines(CTX_ARG, EntityRegistry& registry) {
		//The library of shader modules that will stay loaded as long as we need to continue creating pipelines
		VulkanShaderModuleLibrary library{ logical.device };

		auto const materials = registry.GetView<MaterialComponent>();
		for (const auto id : materials) {
			MaterialComponent& material = materials.Get<MaterialComponent>(id);
			if (!material.resources) CreatePipeline(CTX, material, id, library);
		}
	}

	void RenderingSystem::DestroyPipelines(EntityRegistry& registry) {
		auto const materials = registry.GetView<MaterialComponent>();
		for (const auto id : materials) {
			MarkPipelineStale(materials.Get<MaterialComponent>(id));
		}
		DestroyStalePipelines();
	}

	void RenderingSystem::MarkPipelineStale(MaterialComponent& material) {
		stalePipelineResources.push_back(material.resources);
		material.resources = {};
	}

	void RenderingSystem::DestroyStalePipelines() {
		for (VulkanPipelineResources resources : stalePipelineResources) {
			resources.Destroy(logical.device);
		}
		stalePipelineResources.clear();
	}

	void RenderingSystem::CreateMeshes(CTX_ARG, EntityRegistry& registry) {
		VulkanMeshCreationHelper helper{logical, organizer.pool};

		auto const meshes = registry.GetView<MeshComponent>();
		for (const auto id : meshes) {
			MeshComponent& mesh = meshes.Get<MeshComponent>(id);
			if (!mesh.resources) {
				VulkanMeshCreationResults const result = CreateMesh(CTX, mesh, id, organizer.pool);
				helper.Submit(result);
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

	void RenderingSystem::CreatePipeline(CTX_ARG, MaterialComponent& material, EntityID id, VulkanShaderModuleLibrary& library) {
		//Create the pipeline layout
		VkPipelineLayoutCreateInfo layoutCI{};
		layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutCI.setLayoutCount = 0; // Optional
		layoutCI.pSetLayouts = nullptr; // Optional
		layoutCI.pushConstantRangeCount = 0; // Optional
		layoutCI.pPushConstantRanges = nullptr; // Optional

		assert(!material.resources.layout);
		if (vkCreatePipelineLayout(logical.device, &layoutCI, nullptr, &material.resources.layout) != VK_SUCCESS) {
			LOGF(Temp, Error, "Failed to create pipeline layout for material %i", id);
			return;
		}

		VkShaderModule const vertShaderModule = library.GetModule(CTX, material.vertex);
		if (!vertShaderModule) return;
		VkShaderModule const fragShaderModule = library.GetModule(CTX, material.fragment);
		if (!fragShaderModule) return;

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
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)swapchain.extent.width;
		viewport.height = (float)swapchain.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = swapchain.extent;

		VkPipelineViewportStateCreateInfo viewportStateCI{};
		viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCI.viewportCount = 1;
		viewportStateCI.pViewports = &viewport;
		viewportStateCI.scissorCount = 1;
		viewportStateCI.pScissors = &scissor;

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

		// //Dynamic state
		// VkDynamicState dynamicStates[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};

		// VkPipelineDynamicStateCreateInfo dynamicStateCI{};
		// dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		// dynamicStateCI.dynamicStateCount = 2;
		// dynamicStateCI.pDynamicStates = dynamicStates;

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
		pipelineCI.pDynamicState = nullptr; // Optional
		//Additional data
		pipelineCI.layout = material.resources.layout;
		pipelineCI.renderPass = main.pass;
		pipelineCI.subpass = 0;
		//Parent pipelines, if this pipeline derives from another
		pipelineCI.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineCI.basePipelineIndex = -1; // Optional

		assert(!material.resources.pipeline);
		if (vkCreateGraphicsPipelines(logical.device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &material.resources.pipeline) != VK_SUCCESS) {
			LOGF(Temp, Error, "Failed to create pipeline for material %i", id);
			//Make sure we destroy the layout as well.
			vkDestroyPipelineLayout(logical.device, material.resources.layout, nullptr);
		}
	}

	VulkanMeshCreationResults RenderingSystem::CreateMesh(CTX_ARG, MeshComponent& mesh, EntityID id, VkCommandPool pool) {
		//Allocate a command buffer for vertex and index buffer creation
		VkCommandBufferAllocateInfo bufferAI{};
		bufferAI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		bufferAI.commandPool = pool;
		bufferAI.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		bufferAI.commandBufferCount = 1;

		VkCommandBuffer commands;
		if (vkAllocateCommandBuffers(logical.device, &bufferAI, &commands) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to allocate command buffers");
			return VulkanMeshCreationResults{};
		}

		//Begin recording the command buffer for staging commands
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(commands, &beginInfo) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to begin recording command buffer");
			return VulkanMeshCreationResults{};
		}

		//Create the vertex buffer
		size_t const vertexSize = sizeof(decltype(MeshComponent::vertices)::value_type) * mesh.vertices.size();
		VulkanBufferCreationResults vertex = CreateBufferWithData(logical, mesh.vertices.data(), vertexSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, commands);
		if (!vertex) {
			LOG(Vulkan, Error, "Failed to create vertex buffer");
			vertex.Destroy(logical.allocator);
			return VulkanMeshCreationResults{};
		}

		//Create the index buffer
		size_t const indexSize = sizeof(decltype(MeshComponent::indices)::value_type) * mesh.indices.size();
		VulkanBufferCreationResults index = CreateBufferWithData(logical, mesh.indices.data(), indexSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, commands);
		if (!index) {
			LOG(Vulkan, Error, "Failed to create index buffer");
			vertex.Destroy(logical.allocator);
			index.Destroy(logical.allocator);
			return VulkanMeshCreationResults{};
		}

		//Finish recording the command buffer
		if (vkEndCommandBuffer(commands) != VK_SUCCESS) {
			LOG(Vulkan, Error, "Failed to finish recording command buffer");
			vertex.Destroy(logical.allocator);
			index.Destroy(logical.allocator);
			return VulkanMeshCreationResults{};
		}

		//We've successfully created the mesh, so copy all the data into the outputs.
		VulkanMeshCreationResults result;
		result.commands = commands;
		result.staging.vertex = vertex.staging;
		result.staging.index = index.staging;

		mesh.resources.vertex = vertex.gpu;
		mesh.resources.index = index.gpu;
		return result;
	}
}
