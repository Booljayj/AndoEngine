#pragma once
#include "Engine/ArrayView.h"
#include "Engine/StandardTypes.h"
#include "Geometry/ScreenRect.h"
#include "Rendering/Vulkan/Vulkan.h"
#include "Rendering/Vulkan/VulkanLogicalDevice.h"
#include "Rendering/Vulkan/VulkanSwapchain.h"

/**
 * At a high level, each render pass is defined using a type. Static members of that type provide information about the render pass,
 * and instances of the type are equivalent to an instance of the render pass. render-pass-dependent objects such as framebuffers
 * also exist as unique types within the render pass type.
 */

namespace Rendering {
	struct Framebuffer {
	public:
		inline operator VkFramebuffer() const { return framebuffer; }

		Framebuffer(VkDevice inDevice, VkImageView inView, VkFramebuffer inFramebuffer)
			: device(inDevice), view(inView), framebuffer(inFramebuffer)
		{}
		Framebuffer(Framebuffer const&) = delete;
		Framebuffer(Framebuffer&& other) noexcept;
		~Framebuffer();

	private:
		VkDevice device = nullptr;
		VkImageView view = nullptr;
		VkFramebuffer framebuffer = nullptr;
	};

	template<typename ValueType, typename EnumType>
	struct EnumBackedContainer {
	public:
		EnumBackedContainer() { std::memset(&array, 0, sizeof(array)); }

		ValueType& operator[](EnumType attachment) { return array[static_cast<std::underlying_type_t<EnumType>>(attachment)]; }
		ValueType const& operator[](EnumType attachment) const { return array[static_cast<std::underlying_type_t<EnumType>>(attachment)]; }

		uint32_t size() const { return static_cast<uint32_t>(EnumType::MAX); }
		ValueType const* data() const { return array.data(); }

	private:
		std::array<ValueType, static_cast<uint8_t>(EnumType::MAX)> array;
	};

	/** The primary render pass responsible for rendering surfaces on 3D geometry */
	struct SurfaceRenderPass {
		enum struct EAttachments {
			Color,
			//Normal,
			//Depth,
			MAX
		};
		
		enum struct ESharedAttachments {
			//Normal,
			Depth,
			MAX
		};
		using SharedAttachmentImageViews = EnumBackedContainer<VkImageView, ESharedAttachments>;

		struct FramebufferResources {
		public:
			const Framebuffer& operator[](size_t index) const { return framebuffers[index]; }

			FramebufferResources(VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain, SurfaceRenderPass const& pass);
			FramebufferResources(FramebufferResources const&) = delete;
			FramebufferResources(FramebufferResources&& other) noexcept;
			~FramebufferResources();

		private:
			VkDevice device = nullptr;
			SharedAttachmentImageViews sharedImageViews;
			std::vector<Framebuffer> framebuffers;
		};

		/** Created to mark a scope within which commands are recorded for the pass */
		struct ScopedRecord {
		public:
			ScopedRecord(VkCommandBuffer commands, SurfaceRenderPass const& surface, Framebuffer const& framebuffer, Geometry::ScreenRect const& rect);
			~ScopedRecord();
		private:
			VkCommandBuffer cachedCommands;
		};

		inline operator VkRenderPass() const { return pass; }

		SurfaceRenderPass(VulkanLogicalDevice const& logical, VkFormat format);
		SurfaceRenderPass(const SurfaceRenderPass&) = delete;
		SurfaceRenderPass(SurfaceRenderPass&&) noexcept;
		~SurfaceRenderPass();

	private:
		VkDevice device = nullptr;
		VkRenderPass pass = nullptr;
		EnumBackedContainer<VkClearValue, EAttachments> clearValues;
	};

	// struct PostProcessRenderPass {
	// 	struct EAttachments { enum : uint8_t {
	// 		Screen,
	// 		MAX
	// 	};};
	//
	// 	struct ESubpasses { enum : uint8_t {
	// 		Main,
	// 		MAX
	// 	};};
	//
	// 	struct EDependencies { enum : uint8_t {
	// 		ExternalToMain,
	// 		MAX
	// 	};};
	//
	// 	VkRenderPass pass;
	// 	VkClearValue clearValues[EAttachments::MAX];
	//
	// 	bool Create(VulkanLogicalDevice const& logical, VkFormat format);
	// 	void Destroy(VulkanLogicalDevice const& logical);
	// };

	/** Render passes */
	struct VulkanRenderPasses {
		SurfaceRenderPass surface;
		//PostProcessRenderPass postProcess;

		VulkanRenderPasses(VulkanLogicalDevice const& logical, VkFormat format);
		VulkanRenderPasses(const VulkanRenderPasses&) = delete;
		VulkanRenderPasses(VulkanRenderPasses&&) = default;

		inline VulkanRenderPasses& operator=(VulkanRenderPasses&&) = default;
	};

	/** Framebuffers which can be used with different render passes */
	struct VulkanFramebuffers {
		SurfaceRenderPass::FramebufferResources surface;
		//PostProcessRenderPass::Framebuffers postProcess;

		VulkanFramebuffers(VulkanLogicalDevice const& logical, VulkanSwapchain const& swapchain, VulkanRenderPasses const& passes);
		VulkanFramebuffers(const VulkanFramebuffers&) = delete;
		VulkanFramebuffers(VulkanFramebuffers&&) = default;

		inline VulkanFramebuffers& operator=(VulkanFramebuffers&&) = default;
	};
}
