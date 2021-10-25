#include "Geometry/GLM.h"
#include "Reflection/StandardResolvers.h"
#include "Reflection/StructTypeInfo.h"

namespace Reflection {
	const std::string_view name_x = "x"sv;
	const std::string_view name_y = "y"sv;
	const std::string_view name_z = "z"sv;
	const std::string_view name_w = "w"sv;

	const TStructTypeInfo<glm::vec2> info_vec2 = TStructTypeInfo<glm::vec2>{}
		.Description("2D vector")
		.Variables({
			VariableInfo{ &glm::vec2::x, name_x, nullptr, FVariableFlags::None },
			VariableInfo{ &glm::vec2::x, name_y, nullptr, FVariableFlags::None }
		});
	const TStructTypeInfo<glm::vec3> info_vec3 = TStructTypeInfo<glm::vec3>{}
		.Description("3D vector")
		.Variables({
			VariableInfo{ &glm::vec3::x, name_x, nullptr, FVariableFlags::None },
			VariableInfo{ &glm::vec3::y, name_y, nullptr, FVariableFlags::None },
			VariableInfo{ &glm::vec3::z, name_z, nullptr, FVariableFlags::None }
		});
	const TStructTypeInfo<glm::vec4> info_vec4 = TStructTypeInfo<glm::vec4>{}
		.Description("4D vector")
		.Variables({
			VariableInfo{ &glm::vec4::x, name_x, nullptr, FVariableFlags::None },
			VariableInfo{ &glm::vec4::y, name_y, nullptr, FVariableFlags::None },
			VariableInfo{ &glm::vec4::z, name_z, nullptr, FVariableFlags::None },
			VariableInfo{ &glm::vec4::w, name_w, nullptr, FVariableFlags::None }
		});

	const TStructTypeInfo<glm::u32vec2> info_u32vec2 = TStructTypeInfo<glm::u32vec2>{}
		.Description("2D integer vector")
		.Variables({
			VariableInfo{ &glm::u32vec2::x, name_x, nullptr, FVariableFlags::None },
			VariableInfo{ &glm::u32vec2::x, name_y, nullptr, FVariableFlags::None }
		});
	const TStructTypeInfo<glm::uvec3> info_u32vec3 = TStructTypeInfo<glm::u32vec3>{}
		.Description("3D integer vector")
		.Variables({
			VariableInfo{ &glm::u32vec3::x, name_x, nullptr, FVariableFlags::None },
			VariableInfo{ &glm::u32vec3::y, name_y, nullptr, FVariableFlags::None },
			VariableInfo{ &glm::u32vec3::z, name_z, nullptr, FVariableFlags::None }
		});

	const TStructTypeInfo<glm::i32vec2> info_i32vec2 = TStructTypeInfo<glm::i32vec2>{}
		.Description("2D integer vector")
		.Variables({
			VariableInfo{ &glm::i32vec2::x, name_x, nullptr, FVariableFlags::None },
			VariableInfo{ &glm::i32vec2::x, name_y, nullptr, FVariableFlags::None }
		});
	const TStructTypeInfo<glm::ivec3> info_i32vec3 = TStructTypeInfo<glm::i32vec3>{}
		.Description("3D integer vector")
		.Variables({
			VariableInfo{ &glm::i32vec3::x, name_x, nullptr, FVariableFlags::None },
			VariableInfo{ &glm::i32vec3::y, name_y, nullptr, FVariableFlags::None },
			VariableInfo{ &glm::i32vec3::z, name_z, nullptr, FVariableFlags::None }
		});

	const std::string_view name_column0 = "column0"sv;
	const std::string_view name_column1 = "column1"sv;
	const std::string_view name_column2 = "column2"sv;
	const std::string_view name_column3 = "column3"sv;

	template<typename Matrix>
	VariableInfo CreateMatrixVariableInfo(size_t index, std::string_view name) {
		return VariableInfo{ list<Matrix, typename Matrix::col_type>, index, name, nullptr, FVariableFlags::None };
	}

	const TStructTypeInfo<glm::mat2x2> info_mat2x2 = TStructTypeInfo<glm::mat2x2>{}
		.Description("2x2 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat2x2>(0, name_column0),
			CreateMatrixVariableInfo<glm::mat2x2>(1, name_column1)
		});
	const TStructTypeInfo<glm::mat2x3> info_mat2x3 = TStructTypeInfo<glm::mat2x3>{}
		.Description("2x3 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat2x3>(0, name_column0),
			CreateMatrixVariableInfo<glm::mat2x3>(1, name_column1)
		});
	const TStructTypeInfo<glm::mat2x4> info_mat2x4 = TStructTypeInfo<glm::mat2x4>{}
		.Description("2x4 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat2x4>(0, name_column0),
			CreateMatrixVariableInfo<glm::mat2x4>(1, name_column1)
		});

	const TStructTypeInfo<glm::mat3x2> info_mat3x2 = TStructTypeInfo<glm::mat3x2>{}
		.Description("3x2 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat3x2>(0, name_column0),
			CreateMatrixVariableInfo<glm::mat3x2>(1, name_column1),
			CreateMatrixVariableInfo<glm::mat3x2>(2, name_column2)
		});
	const TStructTypeInfo<glm::mat3x3> info_mat3x3 = TStructTypeInfo<glm::mat3x3>{}
		.Description("3x3 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat3x3>(0, name_column0),
			CreateMatrixVariableInfo<glm::mat3x3>(1, name_column1),
			CreateMatrixVariableInfo<glm::mat3x3>(2, name_column2)
		});
	const TStructTypeInfo<glm::mat3x4> info_mat3x4 = TStructTypeInfo<glm::mat3x4>{}
		.Description("3x4 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat3x4>(0, name_column0),
			CreateMatrixVariableInfo<glm::mat3x4>(1, name_column1),
			CreateMatrixVariableInfo<glm::mat3x4>(2, name_column2)
		});

	const TStructTypeInfo<glm::mat4x2> info_mat4x2 = TStructTypeInfo<glm::mat4x2>{}
		.Description("4x2 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat4x2>(0, name_column0),
			CreateMatrixVariableInfo<glm::mat4x2>(1, name_column1),
			CreateMatrixVariableInfo<glm::mat4x2>(2, name_column2),
			CreateMatrixVariableInfo<glm::mat4x2>(3, name_column3)
		});
	const TStructTypeInfo<glm::mat4x3> info_mat4x3 = TStructTypeInfo<glm::mat4x3>{}
		.Description("4x3 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat4x3>(0, name_column0),
			CreateMatrixVariableInfo<glm::mat4x3>(1, name_column1),
			CreateMatrixVariableInfo<glm::mat4x3>(2, name_column2),
			CreateMatrixVariableInfo<glm::mat4x3>(3, name_column3)
		});
	const TStructTypeInfo<glm::mat4x4> info_mat4x4 = TStructTypeInfo<glm::mat4x4>{}
		.Description("4x4 matrix"sv)
		.Variables({
			CreateMatrixVariableInfo<glm::mat4x4>(0, name_column0),
			CreateMatrixVariableInfo<glm::mat4x4>(1, name_column1),
			CreateMatrixVariableInfo<glm::mat4x4>(2, name_column2),
			CreateMatrixVariableInfo<glm::mat4x4>(3, name_column3)
		});

	namespace Internal {
		DEFINE_RESOLVER(glm::vec2, vec2);
		DEFINE_RESOLVER(glm::vec3, vec3);
		DEFINE_RESOLVER(glm::vec4, vec4);

		DEFINE_RESOLVER(glm::u32vec2, u32vec2);
		DEFINE_RESOLVER(glm::u32vec3, u32vec3);

		DEFINE_RESOLVER(glm::i32vec2, i32vec2);
		DEFINE_RESOLVER(glm::i32vec3, i32vec3);

		DEFINE_RESOLVER(glm::mat2x2, mat2x2);
		DEFINE_RESOLVER(glm::mat2x3, mat2x3);
		DEFINE_RESOLVER(glm::mat2x4, mat2x4);

		DEFINE_RESOLVER(glm::mat3x2, mat3x2);
		DEFINE_RESOLVER(glm::mat3x3, mat3x3);
		DEFINE_RESOLVER(glm::mat3x4, mat3x4);

		DEFINE_RESOLVER(glm::mat4x2, mat4x2);
		DEFINE_RESOLVER(glm::mat4x3, mat4x3);
		DEFINE_RESOLVER(glm::mat4x4, mat4x4);
	}
}
