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
: retryCount(0)
, shouldRecreateSwapchain(false)
, shouldCreatePipelines(false)
{}

bool RenderingSystem::Startup(CTX_ARG, SDLWindowingSystem& windowing, EntityRegistry& registry) {
	using namespace Rendering;

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

	//@todo Create a group for renderable entities, once we have more than one component to include in the group (i.e. renderer and transform).

	return true;
}

bool RenderingSystem::Shutdown(CTX_ARG, EntityRegistry& registry) {
	using namespace Rendering;

	availablePhysicalDevices.clear();

	//Wait for any in-progress work to finish before we start cleanup
	if (logical) vkDeviceWaitIdle(logical.device);

	DestroyPipelines(registry);
	DestroyRenderPasses(CTX);
	organizer.Destroy(logical);
	swapchain.Destroy(logical);
	logical.Destroy();
	framework.Destroy();
	return true;
}

bool RenderingSystem::Render(CTX_ARG, EntityRegistry& registry) {
	using namespace Rendering;

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

	//If we have stale pipelines, destroy them now
	if (stalePipelineResources.size() > 0) {
		LOG(Rendering, Info, "Destroying stale pipelines");
		vkDeviceWaitIdle(logical.device);
		DestroyStalePipelines();
	}
	//If we need to create any new pipelines, create them now
	if (shouldCreatePipelines) {
		shouldCreatePipelines = false;

		LOG(Rendering, Info, "Creating new pipelines");
		vkDeviceWaitIdle(logical.device);
		CreatePipelines(CTX, registry);
	}

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

	auto const recorder = [&](VkCommandBuffer buffer, size_t index) {
		ScopedRenderPass pass{buffer, main, index, VkOffset2D{0, 0}, swapchain.extent};

		for (const auto id : renderables) {
			auto& renderer = renderables.Get<MeshRendererComponent const>(id);
			if (auto* material = materials.Find<MaterialComponent const>(renderer.material)) {
				if (material->resources) {
					//@todo This should actually be binding the real geometry, instead of assuming the shader can draw 3 unspecified vertices
					vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, material->resources.pipeline);
					vkCmdDraw(buffer, 3, 1, 0, 0);
				}
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

bool RenderingSystem::SelectPhysicalDevice(CTX_ARG, uint32_t index) {
	using namespace Rendering;

	if (index != selectedPhysicalIndex) {
		if (VulkanPhysicalDevice const* newPhysical = GetPhysicalDevice(index)) {
			TArrayView<char const*> const extensions = VulkanPhysicalDevice::GetExtensionNames(CTX);

			VulkanLogicalDevice newLogical = VulkanLogicalDevice::Create(CTX, *newPhysical, features, extensions);
			if (!newLogical) {
				LOGF(Rendering, Error, "Failed to create logical device for physical device %i", index);
				return false;
			}
			logical = std::move(newLogical);

			selectedPhysical = newPhysical;
			selectedPhysicalIndex = index;
			shouldRecreateSwapchain = true;

			selectedPhysical->WriteDescription(std::cout);
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
	using namespace Rendering;
	RenderingSystem* rendering = registry.ctx<RenderingSystem*>();
	MaterialComponent& material = registry.get<MaterialComponent>(entity);
	rendering->MarkPipelineStale(material);
}

void RenderingSystem::MaterialComponentOperations::OnModify(entt::registry& registry, entt::entity entity) {
	using namespace Rendering;
	RenderingSystem* rendering = registry.ctx<RenderingSystem*>();
	MaterialComponent& material = registry.get<MaterialComponent>(entity);
	rendering->MarkPipelineStale(material);
	rendering->shouldCreatePipelines = true;
}

bool RenderingSystem::CreateRenderPasses(CTX_ARG) {
	using namespace Rendering;

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
	using namespace Rendering;

	//The library of shader modules that will stay loaded as long as we need to continue creating pipelines
	Rendering::VulkanShaderModuleLibrary library{ logical.device };

	auto const materials = registry.GetView<MaterialComponent>();
	for (const auto id : materials) {
		MaterialComponent& material = materials.Get<MaterialComponent>(id);
		if (!material.resources) {
			CreatePipeline(CTX, material, id, library);
		}
	}
}

void RenderingSystem::DestroyPipelines(EntityRegistry& registry) {
	using namespace Rendering;
	auto const materials = registry.GetView<MaterialComponent>();
	for (const auto id : materials) {
		MarkPipelineStale(materials.Get<MaterialComponent>(id));
	}
	DestroyStalePipelines();
}

void RenderingSystem::CreatePipeline(CTX_ARG, Rendering::MaterialComponent& material, EntityID id, Rendering::VulkanShaderModuleLibrary& library) {
	using namespace Rendering;

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
	VkPipelineVertexInputStateCreateInfo vertexInputCI{};
	vertexInputCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputCI.vertexBindingDescriptionCount = 0;
	vertexInputCI.pVertexBindingDescriptions = nullptr;
	vertexInputCI.vertexAttributeDescriptionCount = 0;
	vertexInputCI.pVertexAttributeDescriptions = nullptr;

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

void RenderingSystem::MarkPipelineStale(Rendering::MaterialComponent& material) {
	stalePipelineResources.push_back(material.resources);
	material.resources = {};
}

void RenderingSystem::DestroyStalePipelines() {
	using namespace Rendering;

	//Destroy all the stale resources
	for (VulkanPipelineResources resources : stalePipelineResources) {
		if (resources.pipeline) vkDestroyPipeline(logical.device, resources.pipeline, nullptr);
		if (resources.layout) vkDestroyPipelineLayout(logical.device, resources.layout, nullptr);
	}
	stalePipelineResources.clear();
}

bool RenderingSystem::IsUsablePhysicalDevice(const Rendering::VulkanPhysicalDevice& physicalDevice, TArrayView<char const*> const& extensionNames) {
	return physicalDevice.HasRequiredQueues() && physicalDevice.HasRequiredExtensions(extensionNames) && physicalDevice.HasSwapchainSupport();
}
