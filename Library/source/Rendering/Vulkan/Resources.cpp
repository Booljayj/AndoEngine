#include "Rendering/Vulkan/Resources.h"
#include "Engine/EnumArray.h"
#include "Rendering/Vulkan/UniformLayouts.h"

namespace Rendering {
	GraphicsPipelineResources::GraphicsPipelineResources(VkDevice device, ShaderModules const& modules, UniformLayouts const& uniforms, VertexInformationViews const& vertex, VkRenderPass pass)
		: device(device)
	{
		//Create the descriptor set for per-material parameters
		//@todo Create the descriptor set based on information from the material, possibly obtained from reflection

		//Create the pipeline layout
		{
			EnumArray<VkDescriptorSetLayout, EGraphicsLayouts> setLayouts;
			setLayouts[EGraphicsLayouts::Global] = uniforms.global;
			setLayouts[EGraphicsLayouts::Object] = uniforms.object;
			//setLayouts[ELayouts::Material] = layouts.set;

			VkPipelineLayoutCreateInfo const layoutCI{
				.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
				.setLayoutCount = setLayouts.size(),
				.pSetLayouts = setLayouts.data(),
				.pushConstantRangeCount = 0, // Optional
				.pPushConstantRanges = nullptr, // Optional
			};
			
			if (vkCreatePipelineLayout(device, &layoutCI, nullptr, &layouts.pipeline) != VK_SUCCESS || !layouts.pipeline) {
				throw std::runtime_error{ "Failed to create pipeline layout" };
			}
		}

		//Vertex Input function
		VkPipelineVertexInputStateCreateInfo const vertexInputCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex.bindings.size()),
			.pVertexBindingDescriptions = vertex.bindings.data(),
			.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex.attributes.size()),
			.pVertexAttributeDescriptions = vertex.attributes.data(),
		};

		//Input assembly function
		VkPipelineInputAssemblyStateCreateInfo const inputAssemblyCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE,
		};

		//Viewport
		VkPipelineViewportStateCreateInfo const viewportStateCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.pViewports = nullptr, //provided dynamically
			.scissorCount = 1,
			.pScissors = nullptr, //provided dynamically
		};

		//Rasterizer
		VkPipelineRasterizationStateCreateInfo const rasterizerCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_BACK_BIT,
			.frontFace = VK_FRONT_FACE_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.0f,
			.depthBiasClamp = 0.0f,
			.depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f,
		};
		
		//Multisampling
		//Disabled for now, requires a GPU feature to enable
		VkPipelineMultisampleStateCreateInfo const multisamplingCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE,
			.minSampleShading = 1.0f,
			.pSampleMask = nullptr,
			.alphaToCoverageEnable = VK_FALSE,
			.alphaToOneEnable = VK_FALSE,
		};

		//Color blending (alpha blending setup)
		VkPipelineColorBlendAttachmentState const colorBlendAttachment{
			.blendEnable = VK_TRUE,			
			.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
			.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.colorBlendOp = VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp = VK_BLEND_OP_ADD,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		};

		VkPipelineColorBlendStateCreateInfo const colorBlendingCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.logicOp = VK_LOGIC_OP_COPY, // Optional
			.attachmentCount = 1,
			.pAttachments = &colorBlendAttachment,
			.blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f }, //Optional
		};

		//Dynamic state
		VkDynamicState const dynamicStates[2] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo const dynamicStateCI{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = 2,
			.pDynamicStates = dynamicStates,
		};
		
		//Shader stages
		enum class EShaderStage : uint8_t { Vertex, Fragment, MAX };
		EnumArray<VkPipelineShaderStageCreateInfo, EShaderStage> stages;
		
		stages[EShaderStage::Vertex] = VkPipelineShaderStageCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_VERTEX_BIT,
			.module = modules.vertex,
			.pName = "main",
		};
		stages[EShaderStage::Fragment] = VkPipelineShaderStageCreateInfo{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
			.module = modules.fragment,
			.pName = "main",
		};

		//Final pipeline creation
		VkGraphicsPipelineCreateInfo const pipelineCI{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			//Programmable stages
			.stageCount = stages.size(),
			.pStages = stages.data(),
			//Fixed states
			.pVertexInputState = &vertexInputCI,
			.pInputAssemblyState = &inputAssemblyCI,
			.pViewportState = &viewportStateCI,
			.pRasterizationState = &rasterizerCI,
			.pMultisampleState = &multisamplingCI,
			.pDepthStencilState = nullptr, // Optional
			.pColorBlendState = &colorBlendingCI,
			.pDynamicState = &dynamicStateCI,
			//Additional data
			.layout = layouts.pipeline,
			.renderPass = pass,
			.subpass = 0,
			//Parent pipelines, if this pipeline derives from another
			.basePipelineHandle = VK_NULL_HANDLE, // Optional
			.basePipelineIndex = -1, // Optional
		};
		
		if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, nullptr, &pipeline) != VK_SUCCESS || !pipeline) {
			throw std::runtime_error{ "Failed to create pipeline" };
		}
	}

	GraphicsPipelineResources::~GraphicsPipelineResources() {
		if (device) {
			vkDestroyPipeline(device, pipeline, nullptr);
			vkDestroyPipelineLayout(device, layouts.pipeline, nullptr);
			//vkDestroyDescriptorSetLayout(device, layouts.set, nullptr);
		}
	}

	MeshResources::MeshResources(VmaAllocator allocator, size_t capacity)
		: buffer(allocator, capacity, (BufferUsage::Vertex | BufferUsage::Index | BufferUsage::TransferDst), MemoryUsage::GPU_Only)
	{}
}
