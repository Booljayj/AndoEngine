#include "Geometry/GLM.h"

namespace Reflection {
	const std::string_view desc_x = "x coordinate"sv;
	const std::string_view desc_y = "y coordinate"sv;
	const std::string_view desc_z = "z coordinate"sv;
	const std::string_view desc_w = "w coordinate"sv;

	DEFINE_STRUCTTYPEINFO(glm::vec2, info_vec2)
		.Description("2D vector"sv)
		.Variables({
			REFLECT_MVAR(glm::vec2, x, desc_x),
			REFLECT_MVAR(glm::vec2, y, desc_y)
		});
	DEFINE_STRUCTTYPEINFO(glm::vec3, info_vec3)
		.Description("3D vector"sv)
		.Variables({
			REFLECT_MVAR(glm::vec3, x, desc_x),
			REFLECT_MVAR(glm::vec3, y, desc_y),
			REFLECT_MVAR(glm::vec3, z, desc_z)
		});
	DEFINE_STRUCTTYPEINFO(glm::vec4, info_vec4)
		.Description("4D vector"sv)
		.Variables({
			REFLECT_MVAR(glm::vec4, x, desc_x),
			REFLECT_MVAR(glm::vec4, y, desc_y),
			REFLECT_MVAR(glm::vec4, z, desc_z),
			REFLECT_MVAR(glm::vec4, w, desc_w)
		});

	DEFINE_STRUCTTYPEINFO(glm::u32vec2, info_u32vec2)
		.Description("2D unsigned integer vector"sv)
		.Variables({
			REFLECT_MVAR(glm::u32vec2, x, desc_x),
			REFLECT_MVAR(glm::u32vec2, y, desc_y)
		});
	DEFINE_STRUCTTYPEINFO(glm::u32vec3, info_u32vec3)
		.Description("3D unsigned integer vector"sv)
		.Variables({
			REFLECT_MVAR(glm::u32vec3, x, desc_x),
			REFLECT_MVAR(glm::u32vec3, y, desc_y),
			REFLECT_MVAR(glm::u32vec3, z, desc_z)
		});

	DEFINE_STRUCTTYPEINFO(glm::i32vec2, info_i32vec2)
		.Description("2D signed integer vector"sv)
		.Variables({
			REFLECT_MVAR(glm::i32vec2, x, desc_x),
			REFLECT_MVAR(glm::i32vec2, y, desc_y)
		});
	DEFINE_STRUCTTYPEINFO(glm::i32vec3, info_i32vec3)
		.Description("3D signed integer vector"sv)
		.Variables({
			REFLECT_MVAR(glm::i32vec3, x, desc_x),
			REFLECT_MVAR(glm::i32vec3, y, desc_y),
			REFLECT_MVAR(glm::i32vec3, z, desc_z)
		});

	DEFINE_STRUCTTYPEINFO(glm::u8vec3, info_u8vec3)
		.Description("3D byte vector"sv)
		.Variables({
			REFLECT_MVAR(glm::u8vec3, x, desc_x),
			REFLECT_MVAR(glm::u8vec3, y, desc_y),
			REFLECT_MVAR(glm::u8vec3, z, desc_z)
		});
	DEFINE_STRUCTTYPEINFO(glm::u8vec4, info_u8vec4)
		.Description("4D byte vector"sv)
		.Variables({
			REFLECT_MVAR(glm::u8vec4, x, desc_x),
			REFLECT_MVAR(glm::u8vec4, y, desc_y),
			REFLECT_MVAR(glm::u8vec4, z, desc_z),
			REFLECT_MVAR(glm::u8vec4, w, desc_w)
		});

	const std::string_view name_column0 = "column0"sv;
	const std::string_view name_column1 = "column1"sv;
	const std::string_view name_column2 = "column2"sv;
	const std::string_view name_column3 = "column3"sv;

	const std::string_view desc_column0 = "matrix column 0"sv;
	const std::string_view desc_column1 = "matrix column 1"sv;
	const std::string_view desc_column2 = "matrix column 2"sv;
	const std::string_view desc_column3 = "matrix column 3"sv;

	template<typename Matrix>
	VariableInfo CreateMatrixVariableInfo(size_t index, std::string_view name, std::string_view desc) {
		return VariableInfo{ list<Matrix, typename Matrix::col_type>, index, name, desc, FVariableFlags::None };
	}

	DEFINE_STRUCTTYPEINFO(glm::mat2x2, info_mat2x2)
		.Description("2x2 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat2x2>(0, name_column0, desc_column0),
			CreateMatrixVariableInfo<glm::mat2x2>(1, name_column1, desc_column1)
		});
	DEFINE_STRUCTTYPEINFO(glm::mat2x3, info_mat2x3)
		.Description("2x3 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat2x3>(0, name_column0, desc_column0),
			CreateMatrixVariableInfo<glm::mat2x3>(1, name_column1, desc_column1)
		});
	DEFINE_STRUCTTYPEINFO(glm::mat2x4, info_mat2x4)
		.Description("2x4 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat2x4>(0, name_column0, desc_column0),
			CreateMatrixVariableInfo<glm::mat2x4>(1, name_column1, desc_column1)
		});

	DEFINE_STRUCTTYPEINFO(glm::mat3x2, info_mat3x2)
		.Description("3x2 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat3x2>(0, name_column0, desc_column0),
			CreateMatrixVariableInfo<glm::mat3x2>(1, name_column1, desc_column1),
			CreateMatrixVariableInfo<glm::mat3x2>(2, name_column2, desc_column2)
		});
	DEFINE_STRUCTTYPEINFO(glm::mat3x3, info_mat3x3)
		.Description("3x3 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat3x3>(0, name_column0, desc_column0),
			CreateMatrixVariableInfo<glm::mat3x3>(1, name_column1, desc_column1),
			CreateMatrixVariableInfo<glm::mat3x3>(2, name_column2, desc_column2)
		});
	DEFINE_STRUCTTYPEINFO(glm::mat3x4, info_mat3x4)
		.Description("3x4 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat3x4>(0, name_column0, desc_column0),
			CreateMatrixVariableInfo<glm::mat3x4>(1, name_column1, desc_column1),
			CreateMatrixVariableInfo<glm::mat3x4>(2, name_column2, desc_column2)
		});

	DEFINE_STRUCTTYPEINFO(glm::mat4x2, info_mat4x2)
		.Description("4x2 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat4x2>(0, name_column0, desc_column0),
			CreateMatrixVariableInfo<glm::mat4x2>(1, name_column1, desc_column1),
			CreateMatrixVariableInfo<glm::mat4x2>(2, name_column2, desc_column2),
			CreateMatrixVariableInfo<glm::mat4x2>(3, name_column3, desc_column3)
		});
	DEFINE_STRUCTTYPEINFO(glm::mat4x3, info_mat4x3)
		.Description("4x3 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat4x3>(0, name_column0, desc_column0),
			CreateMatrixVariableInfo<glm::mat4x3>(1, name_column1, desc_column1),
			CreateMatrixVariableInfo<glm::mat4x3>(2, name_column2, desc_column2),
			CreateMatrixVariableInfo<glm::mat4x3>(3, name_column3, desc_column3)
		});
	DEFINE_STRUCTTYPEINFO(glm::mat4x4, info_mat4x4)
		.Description("4x4 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat4x4>(0, name_column0, desc_column0),
			CreateMatrixVariableInfo<glm::mat4x4>(1, name_column1, desc_column1),
			CreateMatrixVariableInfo<glm::mat4x4>(2, name_column2, desc_column2),
			CreateMatrixVariableInfo<glm::mat4x4>(3, name_column3, desc_column3)
		});
}

DEFINE_REFLECT(glm::vec2, Struct, Reflection::info_vec2);
DEFINE_REFLECT(glm::vec3, Struct, Reflection::info_vec3);
DEFINE_REFLECT(glm::vec4, Struct, Reflection::info_vec4);

DEFINE_REFLECT(glm::u32vec2, Struct, Reflection::info_u32vec2);
DEFINE_REFLECT(glm::u32vec3, Struct, Reflection::info_u32vec3);

DEFINE_REFLECT(glm::i32vec2, Struct, Reflection::info_i32vec2);
DEFINE_REFLECT(glm::i32vec3, Struct, Reflection::info_i32vec3);

DEFINE_REFLECT(glm::mat2x2, Struct, Reflection::info_mat2x2);
DEFINE_REFLECT(glm::mat2x3, Struct, Reflection::info_mat2x3);
DEFINE_REFLECT(glm::mat2x4, Struct, Reflection::info_mat2x4);

DEFINE_REFLECT(glm::mat3x2, Struct, Reflection::info_mat3x2);
DEFINE_REFLECT(glm::mat3x3, Struct, Reflection::info_mat3x3);
DEFINE_REFLECT(glm::mat3x4, Struct, Reflection::info_mat3x4);

DEFINE_REFLECT(glm::mat4x2, Struct, Reflection::info_mat4x2);
DEFINE_REFLECT(glm::mat4x3, Struct, Reflection::info_mat4x3);
DEFINE_REFLECT(glm::mat4x4, Struct, Reflection::info_mat4x4);
