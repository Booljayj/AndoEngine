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

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

		//Vertex Input function
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;

		//Input assembly function
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

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

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		//Rasterizer
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f;
		rasterizer.depthBiasClamp = 0.0f;
		rasterizer.depthBiasSlopeFactor = 0.0f;

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

		VkPipelineColorBlendStateCreateInfo colorBlending{};
		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable = VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0] = 0.0f; // Optional
		colorBlending.blendConstants[1] = 0.0f; // Optional
		colorBlending.blendConstants[2] = 0.0f; // Optional
		colorBlending.blendConstants[3] = 0.0f; // Optional

		//Dynamic state
		VkDynamicState dynamicStates[2] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_LINE_WIDTH};

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = dynamicStates;

		//Create the pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		if (vkCreatePipelineLayout(logical.device, &pipelineLayoutInfo, nullptr, &material.pipelineLayout) != VK_SUCCESS) {
			LOGF(Temp, Error, "Failed to create pipeline layout for material %s", entity);
			DestroyShaderModule(vertShaderModule, logical);
			DestroyShaderModule(fragShaderModule, logical);
			return;
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		//Programmable stages
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		//Fixed states
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pViewportState = &viewportState;
		pipelineInfo.pRasterizationState = &rasterizer;
		pipelineInfo.pMultisampleState = nullptr; //&multisampling;
		pipelineInfo.pDepthStencilState = nullptr; // Optional
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = nullptr; // Optional
		//Additional data
		pipelineInfo.layout = material.pipelineLayout;
		pipelineInfo.renderPass = rendering.passes.mainRenderPass;
		pipelineInfo.subpass = 0;
		//Parent pipelines, if this pipeline derives from another
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineInfo.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(logical.device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &material.pipeline) != VK_SUCCESS) {
			LOGF(Temp, Error, "Failed to create pipeline for material %i", entity);

			vkDestroyPipelineLayout(logical.device, material.pipelineLayout, nullptr);
			material.pipelineLayout = nullptr;
		}

		DestroyShaderModule(vertShaderModule, logical);
		DestroyShaderModule(fragShaderModule, logical);
	}

	void MaterialComponent::OnDestroy(entt::registry& registry, entt::entity entity) {
		RenderingSystem const& rendering = *registry.ctx<RenderingSystem*>();
		MaterialComponent& material = registry.get<MaterialComponent>(entity);

		if (material.pipelineLayout) {
			vkDestroyPipelineLayout(rendering.logical.device, material.pipelineLayout, nullptr);
			material.pipelineLayout = nullptr;
		}
	}
}
