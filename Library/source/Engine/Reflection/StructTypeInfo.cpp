#include "Engine/Reflection/StructTypeInfo.h"
#include "Engine/Archive.h"
#include "Engine/Array.h"
#include "Engine/Ranges.h"
#include "Engine/String.h"
#include "Engine/StringConversion.h"

namespace Reflection {
	static void SerializeVariables_Diff(StructTypeInfo const& type, YAML::Node& node, void const* instance, void const* defaults);
	static void SerializeVariables_NonDiff(StructTypeInfo const& type, YAML::Node& node, void const* instance);

	static void SerializeVariables_Diff(StructTypeInfo const& type, Archive::Output& archive, void const* instance, void const* defaults);
	static void SerializeVariables_NonDiff(StructTypeInfo const& type, Archive::Output& archive, void const* instance);

	void StructSerializationHelpers::SerializeVariables(StructTypeInfo const& type, Archive::Output& archive, void const* instance) {
		void const* defaults = type.GetDefaults();
		if (defaults && type.flags.Has(ETypeFlags::EqualityComparable)) {
			SerializeVariables_Diff(type, archive, instance, defaults);
		} else {
			SerializeVariables_NonDiff(type, archive, instance);
		}
	}

	void StructSerializationHelpers::DeserializeVariables(StructTypeInfo const& type, Archive::Input& archive, void* instance) {
		std::u16string name;
		std::span<std::byte const> buffer;

		const auto FindVariable = [&](std::u16string_view name) -> Reflection::VariableInfo const* {
			for (StructTypeInfo const* current = &type; current; current = current->base) {
				auto const iter = ranges::find_if(current->GetVariables(), [name](std::unique_ptr<VariableInfo const> const& info) { return info->name == name; });
				if (iter != current->GetVariables().end()) return iter->get();
			}
			return nullptr;
		};

		while (true) {
			//First read the information from the archive, then interpret it. This ensures we read all the information that was originally written.
			archive >> name;
			archive >> buffer;

			//If this is the sentinel value that indicates the end of the variables, then we can stop reading
			if (name.size() == 0 || buffer.size() == 0) break;

			if (VariableInfo const* const variable = FindVariable(name)) {
				if (void* pointer = variable->GetMutable(instance)) {
					Archive::Input subarchive{ buffer };
					variable->type->Deserialize(subarchive, pointer);
				}
			}
		}
	}

	void StructSerializationHelpers::SerializeVariables(StructTypeInfo const& type, YAML::Node& node, void const* instance) {
		void const* defaults = type.GetDefaults();
		if (defaults && type.flags.Has(ETypeFlags::EqualityComparable)) {
			SerializeVariables_Diff(type, node, instance, defaults);
		} else {
			SerializeVariables_NonDiff(type, node, instance);
		}
	}

	void StructSerializationHelpers::DeserializeVariables(StructTypeInfo const& type, YAML::Node const& node, void* instance) {
		const auto FindVariable = [&](std::u16string_view name) -> Reflection::VariableInfo const* {
			for (StructTypeInfo const* current = &type; current; current = current->base) {
				auto const iter = ranges::find_if(current->GetVariables(), [name](std::unique_ptr<VariableInfo const> const& info) { return info->name == name; });
				if (iter != current->GetVariables().end()) return iter->get();
			}
			return nullptr;
		};

		std::u16string u16name;

		for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
			std::string_view const name = it->first.as<std::string_view>();

			u16name.clear();
			ConvertString(name, u16name);

			if (Reflection::VariableInfo const* variable = FindVariable(u16name)) {
				if (void* pointer = variable->GetMutable(instance)) {
					variable->type->Deserialize(it->second, pointer);
				}
			}
		}
	}

	void SerializeVariables_Diff(StructTypeInfo const& type, YAML::Node& node, void const* instance, void const* defaults) {
		std::string u8name;

		for (StructTypeInfo const* current = &type; current; current = current->base) {
			for (std::unique_ptr<VariableInfo const> const& variable : current->GetVariables()) {
				if (!variable->flags.Has(Reflection::EVariableFlags::Deprecated) && !variable->type->Equal(variable->GetImmutable(instance), variable->GetImmutable(defaults))) {
					YAML::Node const value = variable->type->Serialize(variable->GetImmutable(instance));

					//If we've serialized any contents for this node, add it to the map using the name of the variable.
					if (value) {
						u8name.clear();
						ConvertString(variable->name, u8name);
						node[u8name] = value;
					}
				}
			}
		}
	}
	void SerializeVariables_NonDiff(StructTypeInfo const& type, YAML::Node& node, void const* instance) {
		std::string u8name;

		for (StructTypeInfo const* current = &type; current; current = current->base) {
			for (std::unique_ptr<VariableInfo const> const& variable : current->GetVariables()) {
				if (!variable->flags.Has(Reflection::EVariableFlags::Deprecated)) {
					YAML::Node const value = variable->type->Serialize(variable->GetImmutable(instance));

					//If we've serialized any contents for this node, add it to the map using the name of the variable.
					if (value) {
						u8name.clear();
						ConvertString(variable->name, u8name);
						node[u8name] = value;
					}
				}
			}
		}
	}

	void SerializeVariables_Diff(StructTypeInfo const& type, Archive::Output& archive, void const* instance, void const* defaults) {
		std::vector<std::byte> buffer;

		//Serialize each variable as a name-buffer pair
		for (StructTypeInfo const* current = &type; current; current = current->base) {
			for (std::unique_ptr<VariableInfo const> const& variable : current->GetVariables()) {
				if (!variable->flags.Has(Reflection::EVariableFlags::Deprecated) && !variable->type->Equal(variable->GetImmutable(instance), variable->GetImmutable(defaults))) {
					Archive::Output subarchive{ buffer };
					variable->type->Serialize(subarchive, variable->GetImmutable(instance));

					//If data was actually serialized for this variable, then write it to the output.
					if (buffer.size() > 0) {
						archive << type.name;
						archive << buffer;

						buffer.clear();
					}
				}
			}
		}

		//Serialize an "empty" variable as a sentinel value to indicate the end of the variables.
		archive << ""sv;
		archive << buffer;
	}
	void SerializeVariables_NonDiff(StructTypeInfo const& type, Archive::Output& archive, void const* instance) {
		std::vector<std::byte> buffer;

		//Serialize each variable as a name-buffer pair
		for (StructTypeInfo const* current = &type; current; current = current->base) {
			for (std::unique_ptr<VariableInfo const> const& variable : current->GetVariables()) {
				if (!variable->flags.Has(Reflection::EVariableFlags::Deprecated)) {
					Archive::Output subarchive{ buffer };
					variable->type->Serialize(subarchive, variable->GetImmutable(instance));

					//If data was actually serialized for this variable, then write it to the output.
					if (buffer.size() > 0) {
						archive << type.name;
						archive << buffer;

						buffer.clear();
					}
				}
			}
		}

		//Serialize an "empty" variable as a sentinel value to indicate the end of the variables.
		archive << ""sv;
		archive << buffer;
	}
}
