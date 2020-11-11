#include "Engine/Context.h"
#include "Engine/LinearStrings.h"
#include "Engine/LogCommands.h"
#include "Rendering/MaterialComponent.h"
#include "Rendering/RenderingSystem.h"

namespace Rendering {
	namespace MaterialComponentUtility {
		VkShaderModule CreateShaderModule(CTX_ARG, char const* name, VulkanLogicalDevice const& logical) {
			TEMP_ALLOCATOR_MARK();

			std::string_view const filename = l_printf(CTX.temp, "data/shaders/%s.spv", name);
			std::ifstream file(filename.data(), std::ios::ate | std::ios::binary);

			if (!file.is_open()) return nullptr;

			size_t const bufferSize = (size_t) file.tellg();
			char* const buffer = static_cast<char*>(CTX.temp.Request(bufferSize, sizeof(char), alignof(uint32_t)));

			file.seekg(0);
			file.read(buffer, bufferSize);
			file.close();

			VkShaderModuleCreateInfo createInfo{};
			createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			createInfo.codeSize = bufferSize;
			createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer);

			VkShaderModule module = nullptr;
			if (vkCreateShaderModule(logical.device, &createInfo, nullptr, &module) != VK_SUCCESS) {
				LOGF(Temp, Error, "Failed to create shader module for shader %s", name);
			}
			return module;
		}

		void DestroyShaderModule(VkShaderModule module, VulkanLogicalDevice const& logical) {
    		vkDestroyShaderModule(logical.device, module, nullptr);
		}
	}

	void MaterialComponent::OnCreate(entt::registry& registry, entt::entity entity) {
		using namespace MaterialComponentUtility;

		Context& CTX = *registry.ctx<Context*>();
		RenderingSystem const& rendering = *registry.ctx<RenderingSystem*>();
		VulkanLogicalDevice const& logical = rendering.logical;
		MaterialComponent& material = registry.get<MaterialComponent>(entity);

		//@todo This is a very fixed method of creating the pipeline. Ideally, this should be configurable
		//      using values from the material component. Ideally this wouldn't require storing a string
		//      that defines the pipeline setup, but that is an option (like Unity material files).

		VkShaderModule vertShaderModule = CreateShaderModule(CTX, material.vertex.c_str(), logical);
		if (!vertShaderModule) return;

		VkShaderModule fragShaderModule = CreateShaderModule(CTX, material.fragment.c_str(), logical);
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
		viewport.width = (float)rendering.swapchain.extent.width;
		viewport.height = (float)rendering.swapchain.extent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.offset = {0, 0};
		scissor.extent = rendering.swapchain.extent;

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

		//Disabled for now, requires a GPU feature to enable
		// //Multisampling
		// VkPipelineMultisampleStateCreateInfo multisampling{};
		// multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		// multisampling.sampleShadingEnable = VK_FALSE;
		// multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		// multisampling.minSampleShading = 1.0f;
		// multisampling.pSampleMask = nullptr;
		// multisampling.alphaToCoverageEnable = VK_FALSE;
		// multisampling.alphaToOneEnable = VK_FALSE;

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

		//Create the pipeline layout
		VkPipelineLayoutCreateInfo layoutCI{};
		layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		layoutCI.setLayoutCount = 0; // Optional
		layoutCI.pSetLayouts = nullptr; // Optional
		layoutCI.pushConstantRangeCount = 0; // Optional
		layoutCI.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(logical.device, &layoutCI, nullptr, &material.layout) != VK_SUCCESS) {
			LOGF(Temp, Error, "Failed to create pipeline layout for material %s", entity);
			DestroyShaderModule(vertShaderModule, logical);
			DestroyShaderModule(fragShaderModule, logical);
			return;
		}

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
		pipelineCI.pMultisampleState = nullptr; //&multisampling;
		pipelineCI.pDepthStencilState = nullptr; // Optional
		pipelineCI.pColorBlendState = &colorBlendingCI;
		pipelineCI.pDynamicState = nullptr; // Optional
		//Additional data
		pipelineCI.layout = material.layout;
		pipelineCI.renderPass = rendering.passes.mainRenderPass;
		pipelineCI.subpass = 0;
		//Parent pipelines, if this pipeline derives from another
		pipelineCI.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineCI.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(logical.device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &material.pipeline) != VK_SUCCESS) {
			LOGF(Temp, Error, "Failed to create pipeline for material %i", entity);

			vkDestroyPipelineLayout(logical.device, material.layout, nullptr);
			material.layout = nullptr;
		}

		DestroyShaderModule(vertShaderModule, logical);
		DestroyShaderModule(fragShaderModule, logical);
	}

	void MaterialComponent::OnDestroy(entt::registry& registry, entt::entity entity) {
		RenderingSystem const& rendering = *registry.ctx<RenderingSystem*>();
		MaterialComponent& material = registry.get<MaterialComponent>(entity);

		if (material.pipeline) vkDestroyPipeline(rendering.logical.device, material.pipeline, nullptr);
		if (material.layout) vkDestroyPipelineLayout(rendering.logical.device, material.layout, nullptr);

		material.pipeline = nullptr;
		material.layout = nullptr;
	}
}
