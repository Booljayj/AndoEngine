#pragma once
#include "Engine/StandardTypes.h"
#include "Engine/TupleUtility.h"
#include "Rendering/Color.h"
#include "Rendering/CompressedVec.h"
#include "Rendering/Vulkan/Vulkan.h"

namespace Rendering {
	namespace Concepts {
		template<typename T>
		concept VertexType =
			std::is_trivially_copyable_v<T> and std::is_trivially_destructible_v<T> and //Copied and destroyed trivially
			TupleElementSizes<typename T::VariableTypes>::Sum() == sizeof(T) and //Packed tightly, with no unaccounted-for padding space
			(sizeof(T) % sizeof(uint32_t)) == 0 and //Size is a multiple of 4
			alignof(T) == alignof(uint32_t) and //Aligned to 4-byte boundaries
			requires {
				{ T::GetBindingDescription() } -> std::convertible_to<VkVertexInputBindingDescription>;
				{ T::GetAttributeDescriptions() } -> ranges::range;
			};
	}

	/** A simple 3D vertex */
	struct Vertex_Simple {
		glm::packed_vec3 position = { 0, 0, 0 };
		Color color = { 0, 0, 0, 255 };
		CompressedVec3 normal = { 0, 0, 1 };
		CompressedVec2 uv0 = { 0, 0 };
		CompressedVec2 uv1 = { 0, 0 };
		CompressedVec2 uv2 = { 0, 0 };

		enum class VariableIndices : uint8_t {
			Position, Color,
			Normal, UV0, UV1, UV2,
			MAX
		};
		using VariableTypes = std::tuple<
			decltype(position), decltype(color),
			decltype(normal), decltype(uv0), decltype(uv1), decltype(uv2)
		>;
		static constexpr size_t NumAttributes = std::tuple_size_v<VariableTypes>;

		static VkVertexInputBindingDescription GetBindingDescription();
		static std::array<VkVertexInputAttributeDescription, NumAttributes> GetAttributeDescriptions();

		Vertex_Simple() = default;
		Vertex_Simple(glm::vec3 position, Color color, glm::vec3 normal, glm::vec2 uv0, glm::vec2 uv1 = { 0, 0 }, glm::vec2 uv2 = { 0, 0 })
			: position(position), color(color), normal(normal), uv0(uv0), uv1(uv1), uv2(uv2)
		{}
	};
	static_assert(Concepts::VertexType<Vertex_Simple>);
	
	/** A complex 3d vertex, with tangent space vectors */
	struct Vertex_Complex {
		glm::packed_vec3 position = { 0, 0, 0 };
		Color color = { 0, 0, 0, 255 };
		CompressedVec3 normal = { 0, 0, 1 };
		CompressedVec3 tangent = { 1, 0, 0 };
		CompressedVec3 bitangent = { 0, 1, 0 };
		uint32_t userData = 0;
		CompressedVec2 uv0 = { 0, 0 };
		CompressedVec2 uv1 = { 0, 0 };
		CompressedVec2 uv2 = { 0, 0 };
		CompressedVec2 uv3 = { 0, 0 };

		enum class VariableIndices : uint8_t {
			Position, Color,
			Normal, Tangent, Bitangent, UserData,
			UV0, UV1, UV2, UV3,
			MAX
		};
		using VariableTypes = std::tuple<
			decltype(position), decltype(color),
			decltype(normal), decltype(tangent), decltype(bitangent), decltype(userData),
			decltype(uv0), decltype(uv1), decltype(uv2), decltype(uv3)
		>;
		static constexpr size_t NumAttributes = std::tuple_size_v<VariableTypes>;

		static VkVertexInputBindingDescription GetBindingDescription();
		static std::array<VkVertexInputAttributeDescription, NumAttributes> GetAttributeDescriptions();

		Vertex_Complex() = default;
		Vertex_Complex(glm::packed_vec3 position, Color color, glm::vec3 normal, glm::vec3 tangent, glm::vec3 bitangent, uint32_t userData, glm::vec2 uv0, glm::vec2 uv1 = { 0, 0 }, glm::vec2 uv2 = { 0, 0 }, glm::vec2 uv3 = { 0, 0 })
			: position(position), color(color), normal(normal), tangent(tangent), bitangent(bitangent), userData(userData), uv0(uv0), uv1(uv1), uv2(uv2), uv3(uv3)
		{}
	};
	static_assert(Concepts::VertexType<Vertex_Complex>);
}
