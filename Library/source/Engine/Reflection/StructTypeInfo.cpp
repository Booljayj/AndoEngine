#include "Engine/Reflection/StructTypeInfo.h"
#include "Engine/Reflection/PrimitiveTypeInfo.h"

namespace Reflection {
	StructVariableRange::Iterator::Iterator(StructTypeInfo const& type) : current(&type) {
		while (current && current->variables.size() == 0) current = current->base;
	}

	StructVariableRange::Iterator& StructVariableRange::Iterator::operator++() {
		++index;

		//Move up the hierarchy if we've run out of variables in the current struct
		if (index > current->variables.size()) {
			//Seek to the next base class that has new variables. If we don't find one, current will be set to nullptr.
			current = current->base;
			while (current && current->variables.size() == 0) current = current->base;

			//Reset the index
			index = 0;
		}

		return *this;
	}

	VariableInfo const& StructVariableRange::Iterator::operator*() const { return current->variables[index]; }
	VariableInfo const* StructVariableRange::Iterator::operator->() const { return &current->variables[index]; }

	VariableInfo const* StructVariableRange::FindVariable(std::string_view name) const {
		const auto iter = ranges::find(*this, name, [](VariableInfo const& info) { return info.name; });
		if (iter != end()) return &(*iter);
		return nullptr;
	}

	VariableInfo const* StructVariableRange::FindVariable(Hash32 id) const {
		const auto iter = ranges::find(*this, id, [](VariableInfo const& info) { return info.id; });
		if (iter != end()) return &(*iter);
		return nullptr;
	}

	void StructTypeInfo::SerializeVariables(StructTypeInfo const& type, Archive::Output& archive, void const* instance) {
		std::vector<std::byte> buffer;
		
		//Serialize each variable as a name-buffer pair
		for (VariableInfo const& variable : type.GetVariableRange()) {
			if (variable.CanSerialize()) {
				Archive::Output subarchive{ buffer };
				variable.type->Serialize(subarchive, variable.GetImmutable(&instance));

				//If data was actually serialized for this variable, then write it to the output.
				if (buffer.size() > 0) {
					archive << type.name;
					archive << buffer;

					buffer.clear();
				}
			}
		}

		//Serialize an "empty" variable as a sentinel value to indicate the end of the variables.
		archive << ""sv;
		archive << buffer;
	}
	void StructTypeInfo::DeserializeVariables(StructTypeInfo const& type, Archive::Input& archive, void* instance) {
		std::string name;
		std::span<std::byte const> buffer;

		auto const variables = type.GetVariableRange();

		while (true) {
			//First read the information from the archive, then interpret it. This ensures we read all the information that was originally written.
			archive >> name;
			archive >> buffer;

			//If this is the sentinel value that indicates the end of the variables, then we can stop reading
			if (name.size() == 0 || buffer.size() == 0) break;

			VariableInfo const* variable = variables.FindVariable(name);
			if (variable && variable->CanDeserialize()) {
				Archive::Input subarchive{ buffer };
				variable->type->Deserialize(subarchive, variable->GetMutable(instance));
			}
		}
	}

	void StructTypeInfo::SerializeVariables(StructTypeInfo const& type, YAML::Node& node, void const* instance) {
		for (VariableInfo const& variable : type.GetVariableRange()) {
			if (variable.CanSerialize()) {
				YAML::Node const value = variable.type->Serialize(variable.GetImmutable(&instance));

				//If we've serialized any contents for this node, add it to the map using the name of the variable.
				if (value) node[variable.name] = value;
			}
		}
	}
	void StructTypeInfo::DeserializeVariables(StructTypeInfo const& type, YAML::Node const& node, void* instance) {
		auto const variables = type.GetVariableRange();

		for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
			std::string const name = it->first.as<std::string>();

			Reflection::VariableInfo const* variable = variables.FindVariable(name);
			if (variable && variable->CanDeserialize()) {
				variable->type->Deserialize(it->second, variable->GetMutable(&instance));
			}
		}
	}

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
