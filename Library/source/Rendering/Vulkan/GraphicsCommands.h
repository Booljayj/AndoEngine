#pragma once
#include "Engine/Core.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** Defines what is inherited by a secondardy command buffer when it is used within a primary command buffer */
	struct CommandInheritance {
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
		VkFramebuffer framebuffer = nullptr;
		
		bool occlusionQueryEnable = false;
		VkQueryControlFlags queryFlags = 0;
		
		VkQueryPipelineStatisticFlags pipelineStatistics = 0;
	};

	struct GraphicsCommandWriter {
		template<typename T, size_t Size>
		using Span = std::span<T const, Size>;

		static constexpr size_t Dynamic = std::dynamic_extent;

		GraphicsCommandWriter(VkCommandBuffer commands);
		GraphicsCommandWriter(VkCommandBuffer commands, CommandInheritance const& inheritance);

		GraphicsCommandWriter(GraphicsCommandWriter const&) = delete;
		GraphicsCommandWriter(GraphicsCommandWriter&&) = delete;
		~GraphicsCommandWriter();

		inline explicit operator VkCommandBuffer() const { return commands; }

		template<size_t Size = Dynamic>
		inline void ExecuteCommands(Span<VkCommandBuffer, Size> secondary_commands) const {
			vkCmdExecuteCommands(commands, CastSize(secondary_commands.size()), secondary_commands.data());
		}
		
		template<size_t Size = Dynamic>
		inline void SetViewports(uint32_t viewport_offset, Span<VkViewport, Size> viewports) const {
			vkCmdSetViewport(commands, viewport_offset, CastSize(viewports.size()), viewports.data());
		}

		template<size_t Size = Dynamic>
		inline void SetScissors(uint32_t scissor_offset, Span<VkRect2D, Size> scissors) const {
			vkCmdSetScissor(commands, scissor_offset, CastSize(scissors.size()), scissors.data());
		}

		inline void BindGraphicsPipeline(VkPipeline pipeline) const {
			vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		}
		
		template<size_t SetsSize = Dynamic, size_t DynamicOffsetsSize = Dynamic>
		inline void BindGraphicsDescriptorSets(VkPipelineLayout layout, uint32_t set_offset, Span<VkDescriptorSet, SetsSize> sets, Span<uint32_t, DynamicOffsetsSize> dynamic_offsets) const {
			vkCmdBindDescriptorSets(
				commands, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
				set_offset, CastSize(sets.size()), sets.data(), CastSize(dynamic_offsets.size()), dynamic_offsets.data()
			);
		}

		template<size_t Size = Dynamic>
		inline void BindVertexBuffers(uint32_t binding_offset, Span<VkBuffer, Size> buffers, Span<VkDeviceSize, Size> buffer_offsets) const {
			if constexpr (Size == Dynamic) if (buffers.size() != buffer_offsets.size()) throw std::range_error{ "buffers and buffer_offsets must be the same size" };
			vkCmdBindVertexBuffers(commands, binding_offset, CastSize(buffers.size()), buffers.data(), buffer_offsets.data());
		}

		inline void BindIndexBuffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType type) const {
			vkCmdBindIndexBuffer(commands, buffer, offset, type);
		}

		inline void DrawIndexed(uint32_t index_offset, uint32_t index_count, uint32_t instance_offset, uint32_t instance_count) const {
			constexpr uint32_t vertex_offset = 0;
			vkCmdDrawIndexed(commands, index_count, instance_count, index_offset, vertex_offset, instance_offset);
		}

	private:
		VkCommandBuffer commands;

		static inline uint32_t CastSize(size_t size) { return static_cast<uint32_t>(size); }
	};
}
