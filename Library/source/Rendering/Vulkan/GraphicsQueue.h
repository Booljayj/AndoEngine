#pragma once
#include "Engine/Core.h"
#include "Engine/MoveOnly.h"
#include "Rendering/Vulkan/QueueReference.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	/** Non-owning handle to a specific queue that was created on a device and which can be used for graphics commands */
	struct GraphicsQueue : public QueueReference {
		GraphicsQueue(VkQueue queue, QueueReference reference) : QueueReference(reference), queue(queue) {}

		inline operator VkQueue() const { return queue; }

		/** Submit commands to this queue */
		void Submit(std::span<VkSemaphore const> wait_semaphores, std::span<VkPipelineStageFlags const> wait_stages, std::span<VkCommandBuffer const> command_buffers, std::span<VkSemaphore const> signal_semaphores, VkFence fence) const;

	protected:
		VkQueue queue = nullptr;
	};

	/** A pool that can be used to allocate command buffers that are appropriate for graphics commands */
	struct GraphicsCommandPool {
	public:
		GraphicsCommandPool(VkDevice device, GraphicsQueue graphics);
		GraphicsCommandPool(GraphicsCommandPool const&) = delete;
		GraphicsCommandPool(GraphicsCommandPool&&) noexcept = default;
		~GraphicsCommandPool();

		inline operator VkCommandPool() const { return pool; }

		/** Reset the memory in the pool so it can be used by new command buffers */
		void Reset();

		VkCommandBuffer CreateBuffer(VkCommandBufferLevel level);
		void DestroyBuffer(VkCommandBuffer buffer);

		std::vector<VkCommandBuffer> CreateBuffers(uint32_t numBuffers, VkCommandBufferLevel level);
		void DestroyBuffers(std::span<VkCommandBuffer const> buffers);

	private:
		MoveOnly<VkDevice> device;
		VkCommandPool pool = nullptr;
	};

	/** Defines what is inherited by a secondary command buffer when it is used within a primary command buffer */
	struct CommandInheritance {
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
		VkFramebuffer framebuffer = nullptr;
		
		bool occlusionQueryEnable = false;
		VkQueryControlFlags queryFlags = 0;
		
		VkQueryPipelineStatisticFlags pipelineStatistics = 0;
	};

	struct GraphicsCommandWriter {
		template<typename T>
		using Span = std::span<T const>;

		explicit GraphicsCommandWriter(VkCommandBuffer commands);
		explicit GraphicsCommandWriter(VkCommandBuffer commands, CommandInheritance const& inheritance);

		GraphicsCommandWriter(GraphicsCommandWriter const&) = delete;
		GraphicsCommandWriter(GraphicsCommandWriter&&) = delete;
		~GraphicsCommandWriter();

		inline explicit operator VkCommandBuffer() const { return commands; }

		inline void ExecuteCommands(Span<VkCommandBuffer> secondary_commands) const {
			vkCmdExecuteCommands(commands, CastSize(secondary_commands.size()), secondary_commands.data());
		}
		
		inline void SetViewports(uint32_t viewport_offset, Span<VkViewport> viewports) const {
			vkCmdSetViewport(commands, viewport_offset, CastSize(viewports.size()), viewports.data());
		}

		inline void SetScissors(uint32_t scissor_offset, Span<VkRect2D> scissors) const {
			vkCmdSetScissor(commands, scissor_offset, CastSize(scissors.size()), scissors.data());
		}

		inline void BindGraphicsPipeline(VkPipeline pipeline) const {
			vkCmdBindPipeline(commands, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		}
		
		inline void BindGraphicsDescriptorSets(VkPipelineLayout layout, uint32_t set_offset, Span<VkDescriptorSet> sets, Span<uint32_t> dynamic_offsets) const {
			vkCmdBindDescriptorSets(
				commands, VK_PIPELINE_BIND_POINT_GRAPHICS, layout,
				set_offset, CastSize(sets.size()), sets.data(), CastSize(dynamic_offsets.size()), dynamic_offsets.data()
			);
		}

		inline void BindVertexBuffers(uint32_t binding_offset, Span<VkBuffer> buffers, Span<VkDeviceSize> buffer_offsets) const {
			if (buffers.size() != buffer_offsets.size()) throw std::length_error{ "buffers and buffer_offsets must be the same size" };
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
