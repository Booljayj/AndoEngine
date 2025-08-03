#include "Engine/Reflection/StructTypeInfo.h"
#include "Engine/Archive.h"
#include "Engine/Array.h"
#include "Engine/Ranges.h"
#include "Engine/String.h"

namespace Reflection {
	StructVariableRange::Iterator::Iterator(StructTypeInfo const& type) : current(&type) {
		while (current && current->GetVariables().size() == 0) current = current->base;
	}

	StructVariableRange::Iterator& StructVariableRange::Iterator::operator++() {
		++index;

		//Move up the hierarchy if we've run out of variables in the current struct
		const auto variables = current->GetVariables();
		if (index >= variables.size()) {
			//Seek to the next base class that has new variables. If we don't find one, current will be set to nullptr.
			do {
				current = current->base;
			} while (current && variables.size() == 0);

			//Reset the index
			index = 0;
		}

		return *this;
	}

	VariableInfo const* StructVariableRange::Iterator::operator*() const { return current->GetVariables()[index].get(); }
	VariableInfo const* StructVariableRange::Iterator::operator->() const { return current->GetVariables()[index].get(); }

	VariableInfo const* StructVariableRange::FindVariable(std::u16string_view name) const {
		const auto iter = ranges::find_if(*this, [=](VariableInfo const* info) -> bool { return info->name == name; });
		if (iter != end()) return *iter;
		return nullptr;
	}

	VariableInfo const* StructVariableRange::FindVariable(Hash32 id) const {
		const auto iter = ranges::find_if(*this, [=](VariableInfo const* info) -> bool { return info->id == id; });
		if (iter != end()) return *iter;
		return nullptr;
	}

	void StructTypeInfo::SerializeVariables(StructTypeInfo const& type, Archive::Output& archive, void const* instance) {
		void const* defaults = type.GetDefaults();
		if (defaults && type.flags.Has(ETypeFlags::EqualityComparable)) {
			SerializeVariables_Diff(type, archive, instance, defaults);
		} else {
			SerializeVariables_NonDiff(type, archive, instance);
		}
	}
	void StructTypeInfo::DeserializeVariables(StructTypeInfo const& type, Archive::Input& archive, void* instance) {
		std::u16string name;
		std::span<std::byte const> buffer;

		auto const variables = StructVariableRange{ type };

		while (true) {
			//First read the information from the archive, then interpret it. This ensures we read all the information that was originally written.
			archive >> name;
			archive >> buffer;

			//If this is the sentinel value that indicates the end of the variables, then we can stop reading
			if (name.size() == 0 || buffer.size() == 0) break;

			if (VariableInfo const* const variable = variables.FindVariable(name)) {
				if (void* pointer = variable->GetMutable(instance)) {
					Archive::Input subarchive{ buffer };
					variable->type->Deserialize(subarchive, pointer);
				}
			}
		}
	}

	void StructTypeInfo::SerializeVariables(StructTypeInfo const& type, YAML::Node& node, void const* instance) {
		void const* defaults = type.GetDefaults();
		if (defaults && type.flags.Has(ETypeFlags::EqualityComparable)) {
			SerializeVariables_Diff(type, node, instance, defaults);
		} else {
			SerializeVariables_NonDiff(type, node, instance);
		}
	}
	void StructTypeInfo::DeserializeVariables(StructTypeInfo const& type, YAML::Node const& node, void* instance) {
		auto const variables = StructVariableRange{ type };

		for (YAML::const_iterator it = node.begin(); it != node.end(); ++it) {
			std::u16string const name = it->first.as<std::u16string>();

			if (Reflection::VariableInfo const* variable = variables.FindVariable(name)) {
				if (void* pointer = variable->GetMutable(instance)) {
					variable->type->Deserialize(it->second, pointer);
				}
			}
		}
	}

	void StructTypeInfo::SerializeVariables_Diff(StructTypeInfo const& type, YAML::Node& node, void const* instance, void const* defaults) {
		for (VariableInfo const* variable : StructVariableRange{ type }) {
			if (!variable->flags.Has(Reflection::EVariableFlags::Deprecated) && !variable->type->Equal(variable->GetImmutable(instance), variable->GetImmutable(defaults))) {
				YAML::Node const value = variable->type->Serialize(variable->GetImmutable(instance));

				//If we've serialized any contents for this node, add it to the map using the name of the variable.
				if (value) node[variable->name] = value;
			}
		}
	}
	void StructTypeInfo::SerializeVariables_Diff(StructTypeInfo const& type, Archive::Output& archive, void const* instance, void const* defaults) {
		std::vector<std::byte> buffer;

		//Serialize each variable as a name-buffer pair
		for (VariableInfo const* variable : StructVariableRange{ type }) {
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

		//Serialize an "empty" variable as a sentinel value to indicate the end of the variables.
		archive << ""sv;
		archive << buffer;
	}

	void StructTypeInfo::SerializeVariables_NonDiff(StructTypeInfo const& type, YAML::Node& node, void const* instance) {
		for (VariableInfo const* variable : StructVariableRange{ type }) {
			if (!variable->flags.Has(Reflection::EVariableFlags::Deprecated)) {
				YAML::Node const value = variable->type->Serialize(variable->GetImmutable(instance));

				//If we've serialized any contents for this node, add it to the map using the name of the variable.
				if (value) node[variable->name] = value;
			}
		}
	}
	void StructTypeInfo::SerializeVariables_NonDiff(StructTypeInfo const& type, Archive::Output& archive, void const* instance) {
		std::vector<std::byte> buffer;

		//Serialize each variable as a name-buffer pair
		for (VariableInfo const* variable : StructVariableRange{ type }) {
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

		//Serialize an "empty" variable as a sentinel value to indicate the end of the variables.
		archive << ""sv;
		archive << buffer;
	}
}
