#include "Engine/Reflection/StructTypeInfo.h"
#include "Engine/Reflection/PrimitiveTypeInfo.h"

namespace Reflection {
	const std::string_view name_x = "x"sv;
	const std::string_view name_y = "y"sv;
	const std::string_view name_z = "z"sv;
	const std::string_view name_w = "w"sv;

	const std::string_view desc_x = "x coordinate"sv;
	const std::string_view desc_y = "y coordinate"sv;
	const std::string_view desc_z = "z coordinate"sv;
	const std::string_view desc_w = "w coordinate"sv;

	namespace vec2 {
		using Type = glm::vec2;
		auto const info = TStructTypeInfo<Type>{ "glm::vec2"sv }
			.Description("2D vector"sv)
			.Variables({
				{ &Type::x, name_x, desc_x, FVariableFlags::None() },
				{ &Type::y, name_y, desc_y, FVariableFlags::None() },
			});
	}
	namespace vec3 {
		using Type = glm::vec3;
		auto const info = TStructTypeInfo<Type>{ "glm::vec3"sv }
			.Description("3D vector"sv)
			.Variables({
				{ &Type::x, name_x, desc_x, FVariableFlags::None() },
				{ &Type::y, name_y, desc_y, FVariableFlags::None() },
				{ &Type::z, name_z, desc_z, FVariableFlags::None() },
			});
	}
	namespace vec4 {
		using Type = glm::vec4;
		auto const info = TStructTypeInfo<Type>{ "glm::vec4"sv }
			.Description("4D vector"sv)
			.Variables({
				{ &Type::x, name_x, desc_x, FVariableFlags::None() },
				{ &Type::y, name_y, desc_y, FVariableFlags::None() },
				{ &Type::z, name_z, desc_z, FVariableFlags::None() },
				{ &Type::z, name_w, desc_w, FVariableFlags::None() },
			});
	}

	namespace u32vec2 {
		using Type = glm::u32vec2;
		auto const info = TStructTypeInfo<Type>{ "glm::u32vec2"sv }
			.Description("2D unsigned integer vector"sv)
			.Variables({
				{ &Type::x, name_x, desc_x, FVariableFlags::None() },
				{ &Type::y, name_y, desc_y, FVariableFlags::None() },
			});
	}
	namespace u32vec3 {
		using Type = glm::u32vec3;
		auto const info = TStructTypeInfo<Type>{ "glm::u32vec3"sv }
			.Description("3D unsigned integer vector"sv)
			.Variables({
				{ &Type::x, name_x, desc_x, FVariableFlags::None() },
				{ &Type::y, name_y, desc_y, FVariableFlags::None() },
				{ &Type::z, name_z, desc_z, FVariableFlags::None() },
			});
	}

	namespace i32vec2 {
		using Type = glm::i32vec2;
		auto const info = TStructTypeInfo<Type>{ "glm::i32vec2"sv }
			.Description("2D signed integer vector"sv)
			.Variables({
				{ &Type::x, name_x, desc_x, FVariableFlags::None() },
				{ &Type::y, name_y, desc_y, FVariableFlags::None() },
			});
	}
	namespace i32vec3 {
		using Type = glm::i32vec3;
		auto const info = TStructTypeInfo<Type>{ "glm::i32vec3"sv }
			.Description("3D signed integer vector"sv)
			.Variables({
				{ &Type::x, name_x, desc_x, FVariableFlags::None() },
				{ &Type::y, name_y, desc_y, FVariableFlags::None() },
				{ &Type::z, name_z, desc_z, FVariableFlags::None() },
			});
	}

	namespace u8vec3 {
		using Type = glm::u8vec3;
		auto const info = TStructTypeInfo<Type>{ "glm::u8vec3"sv }
			.Description("3D byte vector"sv)
			.Variables({
				{ &Type::x, name_x, desc_x, FVariableFlags::None() },
				{ &Type::y, name_y, desc_y, FVariableFlags::None() },
				{ &Type::z, name_z, desc_z, FVariableFlags::None() },
			});
	}
	namespace u8vec4 {
		using Type = glm::u8vec4;
		auto const info = TStructTypeInfo<Type>{ "glm::u8vec4"sv }
			.Description("4D byte vector"sv)
			.Variables({
				{ &Type::x, name_x, desc_x, FVariableFlags::None() },
				{ &Type::y, name_y, desc_y, FVariableFlags::None() },
				{ &Type::z, name_z, desc_z, FVariableFlags::None() },
				{ &Type::z, name_w, desc_w, FVariableFlags::None() },
			});
	}

	const std::string_view name_column0 = "column0"sv;
	const std::string_view name_column1 = "column1"sv;
	const std::string_view name_column2 = "column2"sv;
	const std::string_view name_column3 = "column3"sv;

	const std::string_view desc_column0 = "matrix column 0"sv;
	const std::string_view desc_column1 = "matrix column 1"sv;
	const std::string_view desc_column2 = "matrix column 2"sv;
	const std::string_view desc_column3 = "matrix column 3"sv;

	namespace mat2x2 {
		using Type = glm::mat2x2;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		auto const info = TStructTypeInfo<Type>{ "glm::mat2x2"sv }
			.Description("2x2 matrix"sv)
			.Variables({
				{ list<Type, ColType, IndexType>, 0, name_column0, desc_column0, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 1, name_column1, desc_column1, FVariableFlags::None() },
			});
	}
	namespace mat2x3 {
		using Type = glm::mat2x3;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		auto const info = TStructTypeInfo<Type>{ "glm::mat2x3"sv }
			.Description("2x3 matrix"sv)
			.Variables({
				{ list<Type, ColType, IndexType>, 0, name_column0, desc_column0, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 1, name_column1, desc_column1, FVariableFlags::None() },
			});
	}
	namespace mat2x4 {
		using Type = glm::mat2x4;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		auto const info = TStructTypeInfo<Type>{ "glm::mat2x4"sv }
			.Description("2x4 matrix"sv)
			.Variables({
				{ list<Type, ColType, IndexType>, 0, name_column0, desc_column0, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 1, name_column1, desc_column1, FVariableFlags::None() },
			});
	}

	namespace mat3x2 {
		using Type = glm::mat3x2;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		auto const info = TStructTypeInfo<Type>{ "glm::mat3x2"sv }
			.Description("3x2 matrix"sv)
			.Variables({
				{ list<Type, ColType, IndexType>, 0, name_column0, desc_column0, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 1, name_column1, desc_column1, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 2, name_column2, desc_column2, FVariableFlags::None() },
			});
	}
	namespace mat3x3 {
		using Type = glm::mat3x3;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		auto const info = TStructTypeInfo<Type>{ "glm::mat3x3"sv }
			.Description("3x3 matrix"sv)
			.Variables({
				{ list<Type, ColType, IndexType>, 0, name_column0, desc_column0, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 1, name_column1, desc_column1, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 2, name_column2, desc_column2, FVariableFlags::None() },
			});
	}
	namespace mat3x4 {
		using Type = glm::mat3x4;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		auto const info = TStructTypeInfo<Type>{ "glm::mat3x4"sv }
			.Description("3x4 matrix"sv)
			.Variables({
				{ list<Type, ColType, IndexType>, 0, name_column0, desc_column0, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 1, name_column1, desc_column1, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 2, name_column2, desc_column2, FVariableFlags::None() },
			});
	}

	namespace mat4x2 {
		using Type = glm::mat4x2;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		auto const info = TStructTypeInfo<Type>{ "glm::mat4x2"sv }
			.Description("4x2 matrix"sv)
			.Variables({
				{ list<Type, ColType, IndexType>, 0, name_column0, desc_column0, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 1, name_column1, desc_column1, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 2, name_column2, desc_column2, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 3, name_column3, desc_column3, FVariableFlags::None() },
			});
	}
	namespace mat4x3 {
		using Type = glm::mat4x3;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		auto const info = TStructTypeInfo<Type>{ "glm::mat4x3"sv }
			.Description("4x3 matrix"sv)
			.Variables({
				{ list<Type, ColType, IndexType>, 0, name_column0, desc_column0, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 1, name_column1, desc_column1, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 2, name_column2, desc_column2, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 3, name_column3, desc_column3, FVariableFlags::None() },
			});
	}
	namespace mat4x4 {
		using Type = glm::mat4x4;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		auto const info = TStructTypeInfo<Type>{ "glm::mat4x4"sv }
			.Description("4x4 matrix"sv)
			.Variables({
				{ list<Type, ColType, IndexType>, 0, name_column0, desc_column0, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 1, name_column1, desc_column1, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 2, name_column2, desc_column2, FVariableFlags::None() },
				{ list<Type, ColType, IndexType>, 3, name_column3, desc_column3, FVariableFlags::None() },
			});
	}
}

DEFINE_REFLECT(glm::vec2, Struct, Reflection::vec2::info);
DEFINE_REFLECT(glm::vec3, Struct, Reflection::vec3::info);
DEFINE_REFLECT(glm::vec4, Struct, Reflection::vec4::info);

DEFINE_REFLECT(glm::u32vec2, Struct, Reflection::u32vec2::info);
DEFINE_REFLECT(glm::u32vec3, Struct, Reflection::u32vec3::info);

DEFINE_REFLECT(glm::i32vec2, Struct, Reflection::i32vec2::info);
DEFINE_REFLECT(glm::i32vec3, Struct, Reflection::i32vec3::info);

DEFINE_REFLECT(glm::u8vec3, Struct, Reflection::u8vec3::info);
DEFINE_REFLECT(glm::u8vec4, Struct, Reflection::u8vec4::info);

DEFINE_REFLECT(glm::mat2x2, Struct, Reflection::mat2x2::info);
DEFINE_REFLECT(glm::mat2x3, Struct, Reflection::mat2x3::info);
DEFINE_REFLECT(glm::mat2x4, Struct, Reflection::mat2x4::info);

DEFINE_REFLECT(glm::mat3x2, Struct, Reflection::mat3x2::info);
DEFINE_REFLECT(glm::mat3x3, Struct, Reflection::mat3x3::info);
DEFINE_REFLECT(glm::mat3x4, Struct, Reflection::mat3x4::info);

DEFINE_REFLECT(glm::mat4x2, Struct, Reflection::mat4x2::info);
DEFINE_REFLECT(glm::mat4x3, Struct, Reflection::mat4x3::info);
DEFINE_REFLECT(glm::mat4x4, Struct, Reflection::mat4x4::info);
