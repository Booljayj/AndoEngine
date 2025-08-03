#include "Engine/GLM.h"
#include "Engine/Reflection/StructTypeInfo.h"

constexpr Hash32 id_x = "x"_h32;
constexpr Hash32 id_y = "y"_h32;
constexpr Hash32 id_z = "z"_h32;
constexpr Hash32 id_w = "w"_h32;

const std::u16string_view name_x = u"x"sv;
const std::u16string_view name_y = u"y"sv;
const std::u16string_view name_z = u"z"sv;
const std::u16string_view name_w = u"w"sv;

const std::u16string_view desc_x = u"x coordinate"sv;
const std::u16string_view desc_y = u"y coordinate"sv;
const std::u16string_view desc_z = u"z coordinate"sv;
const std::u16string_view desc_w = u"w coordinate"sv;

namespace Reflection {
	namespace vec2 {
		using Type = glm::vec2;
		TStructTypeInfo<Type> const info{
			u"glm::vec2"sv, u"2D vector"sv,
			{
				MakeMember(&Type::x, id_x, name_x, desc_x),
				MakeMember(&Type::y, id_y, name_y, desc_y),
			}
		};
	}
	
	namespace vec3 {
		using Type = glm::vec3;
		TStructTypeInfo<Type> const info{
			u"glm::vec3"sv, u"3D vector"sv,
			{
				MakeMember(&Type::x, id_x, name_x, desc_x),
				MakeMember(&Type::y, id_y, name_y, desc_y),
				MakeMember(&Type::z, id_z, name_z, desc_z),
			}
		};
	}
	namespace vec4 {
		using Type = glm::vec4;
		TStructTypeInfo<Type> const info{
			u"glm::vec4"sv, u"4D vector"sv,
			{
				MakeMember(&Type::x, id_x, name_x, desc_x),
				MakeMember(&Type::y, id_y, name_y, desc_y),
				MakeMember(&Type::z, id_z, name_z, desc_z),
				MakeMember(&Type::w, id_w, name_w, desc_w),
			}
		};
	}

	namespace u32vec2 {
		using Type = glm::u32vec2;
		TStructTypeInfo<Type> const info{
			u"glm::u32vec2"sv, u"2D unsigned integer vector"sv,
			{
				MakeMember(&Type::x, id_x, name_x, desc_x),
				MakeMember(&Type::y, id_y, name_y, desc_y),
			}
		};
	}
	namespace u32vec3 {
		using Type = glm::u32vec3;
		TStructTypeInfo<Type> const info{
			u"glm::u32vec3"sv, u"3D unsigned integer vector"sv,
			{
				MakeMember(&Type::x, id_x, name_x, desc_x),
				MakeMember(&Type::y, id_y, name_y, desc_y),
				MakeMember(&Type::z, id_z, name_z, desc_z),
			}
		};
	}

	namespace i32vec2 {
		using Type = glm::i32vec2;
		TStructTypeInfo<Type> const info{
			u"glm::i32vec2"sv, u"2D signed integer vector"sv,
			{
				MakeMember(&Type::x, id_x, name_x, desc_x),
				MakeMember(&Type::y, id_y, name_y, desc_y),
			}
		};
	}
	namespace i32vec3 {
		using Type = glm::i32vec3;
		TStructTypeInfo<Type> const info{
			u"glm::i32vec3"sv, u"3D signed integer vector"sv,
			{
				MakeMember(&Type::x, id_x, name_x, desc_x),
				MakeMember(&Type::y, id_y, name_y, desc_y),
				MakeMember(&Type::z, id_z, name_z, desc_z),
			}
		};
	}

	namespace u8vec3 {
		using Type = glm::u8vec3;
		TStructTypeInfo<Type> const info{
			u"glm::u8vec3"sv, u"3D byte vector"sv,
			{
				MakeMember(&Type::x, id_x, name_x, desc_x),
				MakeMember(&Type::y, id_y, name_y, desc_y),
				MakeMember(&Type::z, id_z, name_z, desc_z),
			}
		};
	}
	namespace u8vec4 {
		using Type = glm::u8vec4;
		TStructTypeInfo<Type> const info{
			u"glm::u8vec4"sv, u"4D byte vector"sv,
			{
				MakeMember(&Type::x, id_x, name_x, desc_x),
				MakeMember(&Type::y, id_y, name_y, desc_y),
				MakeMember(&Type::z, id_z, name_z, desc_z),
				MakeMember(&Type::w, id_w, name_w, desc_w),
			}
		};
	}

	constexpr Hash32 id_column0 = "column0"_h32;
	constexpr Hash32 id_column1 = "column1"_h32;
	constexpr Hash32 id_column2 = "column2"_h32;
	constexpr Hash32 id_column3 = "column3"_h32;

	const std::u16string_view name_column0 = u"column0"sv;
	const std::u16string_view name_column1 = u"column1"sv;
	const std::u16string_view name_column2 = u"column2"sv;
	const std::u16string_view name_column3 = u"column3"sv;

	const std::u16string_view desc_column0 = u"matrix column 0"sv;
	const std::u16string_view desc_column1 = u"matrix column 1"sv;
	const std::u16string_view desc_column2 = u"matrix column 2"sv;
	const std::u16string_view desc_column3 = u"matrix column 3"sv;

	namespace mat2x2 {
		using Type = glm::mat2x2;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		TStructTypeInfo<Type> const info{
			u"glm::mat2x2"sv, u"2x2 matrix"sv,
			{
				MakeIndexed<Type, ColType>(0, id_column0, name_column0, desc_column0),
				MakeIndexed<Type, ColType>(1, id_column1, name_column1, desc_column1)
			}
		};
	}
	namespace mat2x3 {
		using Type = glm::mat2x3;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		TStructTypeInfo<Type> const info{
			u"glm::mat2x3"sv, u"2x3 matrix"sv,
			{
				MakeIndexed<Type, ColType>(0, id_column0, name_column0, desc_column0),
				MakeIndexed<Type, ColType>(1, id_column1, name_column1, desc_column1)
			}
		};
	}
	namespace mat2x4 {
		using Type = glm::mat2x4;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		TStructTypeInfo<Type> const info{
			u"glm::mat2x4"sv, u"2x4 matrix"sv,
			{
				MakeIndexed<Type, ColType>(0, id_column0, name_column0, desc_column0),
				MakeIndexed<Type, ColType>(1, id_column1, name_column1, desc_column1)
			}
		};
	}

	namespace mat3x2 {
		using Type = glm::mat3x2;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		TStructTypeInfo<Type> const info{
			u"glm::mat3x2"sv, u"3x2 matrix"sv,
			{
				MakeIndexed<Type, ColType>(0, id_column0, name_column0, desc_column0),
				MakeIndexed<Type, ColType>(1, id_column1, name_column1, desc_column1),
				MakeIndexed<Type, ColType>(2, id_column2, name_column2, desc_column2)
			}
		};
	}
	namespace mat3x3 {
		using Type = glm::mat3x3;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		TStructTypeInfo<Type> const info{
			u"glm::mat3x3"sv, u"3x3 matrix"sv,
			{
				MakeIndexed<Type, ColType>(0, id_column0, name_column0, desc_column0),
				MakeIndexed<Type, ColType>(1, id_column1, name_column1, desc_column1),
				MakeIndexed<Type, ColType>(2, id_column2, name_column2, desc_column2)
			}
		};
	}
	namespace mat3x4 {
		using Type = glm::mat3x4;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		TStructTypeInfo<Type> const info{
			u"glm::mat3x4"sv, u"3x4 matrix"sv,
			{
				MakeIndexed<Type, ColType>(0, id_column0, name_column0, desc_column0),
				MakeIndexed<Type, ColType>(1, id_column1, name_column1, desc_column1),
				MakeIndexed<Type, ColType>(2, id_column2, name_column2, desc_column2)
			}
		};
	}

	namespace mat4x2 {
		using Type = glm::mat4x2;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		TStructTypeInfo<Type> const info{
			u"glm::mat4x2"sv, u"4x2 matrix"sv,
			{
				MakeIndexed<Type, ColType>(0, id_column0, name_column0, desc_column0),
				MakeIndexed<Type, ColType>(1, id_column1, name_column1, desc_column1),
				MakeIndexed<Type, ColType>(2, id_column2, name_column2, desc_column2),
				MakeIndexed<Type, ColType>(3, id_column3, name_column3, desc_column3)
			}
		};
	}
	namespace mat4x3 {
		using Type = glm::mat4x3;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		TStructTypeInfo<Type> const info{
			u"glm::mat4x3"sv, u"4x3 matrix"sv,
			{
				MakeIndexed<Type, ColType>(0, id_column0, name_column0, desc_column0),
				MakeIndexed<Type, ColType>(1, id_column1, name_column1, desc_column1),
				MakeIndexed<Type, ColType>(2, id_column2, name_column2, desc_column2),
				MakeIndexed<Type, ColType>(3, id_column3, name_column3, desc_column3)
			}
		};
	}
	namespace mat4x4 {
		using Type = glm::mat4x4;
		using ColType = Type::col_type;
		using IndexType = Type::length_type;
		TStructTypeInfo<Type> const info{
			u"glm::mat4x4"sv, u"4x4 matrix"sv,
			{
				MakeIndexed<Type, ColType>(0, id_column0, name_column0, desc_column0),
				MakeIndexed<Type, ColType>(1, id_column1, name_column1, desc_column1),
				MakeIndexed<Type, ColType>(2, id_column2, name_column2, desc_column2),
				MakeIndexed<Type, ColType>(3, id_column3, name_column3, desc_column3)
			}
		};
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
