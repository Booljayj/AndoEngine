#include "Rendering/Vulkan/Resources.h"
#include "Engine/Temporary.h"
#include "Rendering/Vulkan/UniformLayouts.h"
#include "Rendering/Vulkan/Utility.h"

namespace Rendering {
	GraphicsPipelineResources::GraphicsPipelineResources(VkDevice device, ShaderModules const& modules, UniformLayouts const& uniforms, VertexInformationViews const& vertex, VkRenderPass pass)
		: device(device)
	{
		//Create the descriptor set for per-material parameters
		//@todo Create the descriptor set based on information from the material, possibly obtained from reflection

		//Create the pipeline layout
		{
			EnumBackedContainer<VkDescriptorSetLayout, EGraphicsLayouts> setLayouts;
			{
				setLayouts[EGraphicsLayouts::Global] = uniforms.global;
				setLayouts[EGraphicsLayouts::Object] = uniforms.object;
				//setLayouts[ELayouts::Material] = layouts.set;
			}

			VkPipelineLayoutCreateInfo layoutCI{};
			layoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			layoutCI.setLayoutCount = setLayouts.size();
			layoutCI.pSetLayouts = setLayouts.data();
			layoutCI.pushConstantRangeCount = 0; // Optional
			layoutCI.pPushConstantRanges = nullptr; // Optional
			
			if (vkCreatePipelineLayout(device, &layoutCI, nullptr, &layouts.pipeline) != VK_SUCCESS || !layouts.pipeline) {
				throw std::runtime_error{ "Failed to create pipeline layout" };
			}
		}

		//Vertex Input function
		VkPipelineVertexInputStateCreateInfo vertexInputCI{};
		vertexInputCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputCI.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex.bindings.size());
		vertexInputCI.pVertexBindingDescriptions = vertex.bindings.begin();
		vertexInputCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex.attributes.size());
		vertexInputCI.pVertexAttributeDescriptions = vertex.attributes.begin();

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

		//Shader stages
		enum class EShaderStage : uint8_t { Vertex, Fragment, MAX };
		EnumBackedContainer<VkPipelineShaderStageCreateInfo, EShaderStage> stages;
		{
			VkPipelineShaderStageCreateInfo& vertShaderStageCI = stages[EShaderStage::Vertex];
			vertShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			vertShaderStageCI.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertShaderStageCI.module = modules.vertex;
			vertShaderStageCI.pName = "main";
		}
		{
			VkPipelineShaderStageCreateInfo& fragShaderStageCI = stages[EShaderStage::Fragment];
			fragShaderStageCI.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			fragShaderStageCI.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fragShaderStageCI.module = modules.fragment;
			fragShaderStageCI.pName = "main";
		}

		//Final pipeline creation
		VkGraphicsPipelineCreateInfo pipelineCI{};
		pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		//Programmable stages
		pipelineCI.stageCount = stages.size();
		pipelineCI.pStages = stages.data();
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
		pipelineCI.layout = layouts.pipeline;
		pipelineCI.renderPass = pass;
		pipelineCI.subpass = 0;
		//Parent pipelines, if this pipeline derives from another
		pipelineCI.basePipelineHandle = VK_NULL_HANDLE; // Optional
		pipelineCI.basePipelineIndex = -1; // Optional

		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline) != VK_SUCCESS || !pipeline) {
			throw std::runtime_error{ "Failed to create pipeline" };
		}
	}

	GraphicsPipelineResources::GraphicsPipelineResources(GraphicsPipelineResources&& other) noexcept
		: pipeline(other.pipeline), layouts({ other.layouts.set, other.layouts.pipeline }), device(other.device)
	{
		other.device = nullptr;
	}

	GraphicsPipelineResources::~GraphicsPipelineResources() {
		if (device) {
			vkDestroyPipeline(device, pipeline, nullptr);
			vkDestroyPipelineLayout(device, layouts.pipeline, nullptr);
			//vkDestroyDescriptorSetLayout(device, layouts.set, nullptr);
		}
	}

	MeshResources::MeshResources(VmaAllocator inAllocator, size_t inCapacity)
		: buffer(inAllocator, inCapacity, BufferUsage::Vertex | BufferUsage::Index | BufferUsage::TransferDst, VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY)
	{}

	MeshResources::MeshResources(MeshResources&& other) noexcept
		: buffer(std::move(other.buffer))
		, offset({ other.offset.vertex, other.offset.index })
		, size({ other.size.vertices, other.size.indices })
		, indexType(other.indexType)
	{}
}
