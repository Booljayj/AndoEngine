#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_ALIGNED_GENTYPES
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/packing.hpp>
#include <glm/gtc/type_aligned.hpp>
#include <glm/gtx/exterior_product.hpp>
#include <glm/matrix.hpp>
#include "Engine/Core.h"
#include "Engine/Reflection/StructTypeInfo.h"

namespace Archive {
	template<size_t N, typename ValueType, glm::qualifier PrecisionType>
	struct Serializer<glm::vec<N, ValueType, PrecisionType>> {
		static void Write(Output& archive, glm::vec<N, ValueType, PrecisionType> const& value) {
			for (size_t index = 0; index < N; ++index) Serializer<ValueType>::Write(archive, value[index]);
		}
		static void Read(Input& archive, glm::vec<N, ValueType, PrecisionType>& value) {
			for (size_t index = 0; index < N; ++index) Serializer<ValueType>::Read(archive, value[index]);
		}
	};

	template<size_t C, size_t R, typename ValueType, glm::qualifier PrecisionType>
	struct Serializer<glm::mat<C, R, ValueType, PrecisionType>> {
		static void Write(Output& archive, glm::mat<C, R, ValueType, PrecisionType> const& value) {
			for (size_t column = 0; column < C; ++column)
				for (size_t row = 0; row < R; ++row)
					Serializer<ValueType>::Write(archive, value[column][row]);
		}
		static void Read(Input& archive, glm::mat<C, R, ValueType, PrecisionType>& value) {
			for (size_t column = 0; column < C; ++column)
				for (size_t row = 0; row < R; ++row)
					Serializer<ValueType>::Read(archive, value[column][row]);
		}
	};
}

REFLECT(glm::vec2, Struct);
REFLECT(glm::vec3, Struct);
REFLECT(glm::vec4, Struct);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::vec2);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::vec3);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::vec4);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::vec2);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::vec3);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::vec4);

REFLECT(glm::u32vec2, Struct);
REFLECT(glm::u32vec3, Struct);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::u32vec2);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::u32vec3);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::u32vec2);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::u32vec3);

REFLECT(glm::i32vec2, Struct);
REFLECT(glm::i32vec3, Struct);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::i32vec2);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::i32vec3);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::i32vec2);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::i32vec3);

REFLECT(glm::u8vec3, Struct);
REFLECT(glm::u8vec4, Struct);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::u8vec3);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::u8vec4);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::u8vec3);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::u8vec4);

REFLECT(glm::mat2x2, Struct);
REFLECT(glm::mat2x3, Struct);
REFLECT(glm::mat2x4, Struct);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::mat2x2);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::mat2x3);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::mat2x4);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::mat2x2);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::mat2x3);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::mat2x4);

REFLECT(glm::mat3x2, Struct);
REFLECT(glm::mat3x3, Struct);
REFLECT(glm::mat3x4, Struct);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::mat3x2);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::mat3x3);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::mat3x4);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::mat3x2);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::mat3x3);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::mat3x4);

REFLECT(glm::mat4x2, Struct);
REFLECT(glm::mat4x3, Struct);
REFLECT(glm::mat4x4, Struct);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::mat4x2);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::mat4x3);
DEFINE_DEFAULT_ARCHIVE_SERIALIZATION(glm::mat4x4);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::mat4x2);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::mat4x3);
DEFINE_DEFAULT_YAML_SERIALIZATION(glm::mat4x4);
