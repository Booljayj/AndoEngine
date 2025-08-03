#include "Engine/Reflection/TypeInfoReference.h"
#include "Engine/Logging.h"
#include "Engine/Ranges.h"

namespace Reflection {
	std::deque<TypeInfo const*> TypeInfoReference::infos;

	TypeInfoReference::Registered::Registered(TypeInfo const& info) : cached(&info) {
		TypeInfoReference::infos.push_back(cached);
	}

	TypeInfoReference::Registered::~Registered() {
		std::deque<TypeInfo const*>& infos = TypeInfoReference::infos;
		
		const auto iter = ranges::find(infos, cached);
		if (iter != infos.end()) {
			//Perform a swap-remove to improve performance when many types are registered.
			const auto last = infos.end() - 1;

			if (iter == last) infos.pop_back();
			else {
				std::iter_swap(iter, last);
				infos.pop_back();
			}
		}
	}

	TypeInfo const* TypeInfoReference::Resolve() const {
		const auto hash_iter = ranges::find_if(infos, [this](TypeInfo const* info) { return info->id == id; });
		if (hash_iter != infos.end()) return *hash_iter;

		if constexpr (LogConfig::IsCompiled(ELogVerbosity::Error)) {
			Logger::Get().Push(LogTemp, ELogVerbosity::Error, LogUtility::GetSourceLocation(), "Unable to resolve type '{}' with id {}. This type may have been removed or changed since a reference to it was created.", name, id);
		};
		return nullptr;
	}
}

namespace Archive {
	void Serializer<Reflection::TypeInfoReference>::Write(Output& archive, Reflection::TypeInfoReference const& value) {
		archive << value.name << value.id;
	}

	void Serializer<Reflection::TypeInfoReference>::Read(Input& archive, Reflection::TypeInfoReference& value) {
		archive >> value.name >> value.id;
	}
}

namespace YAML {
	Node convert<Reflection::TypeInfoReference>::encode(Reflection::TypeInfoReference const& value) {
		Node node{ NodeType::Sequence };
		node.push_back(value.name);
		node.push_back(value.id);
		return node;
	}

	bool convert<Reflection::TypeInfoReference>::decode(Node const& node, Reflection::TypeInfoReference& value) {
		if (!node.IsSequence() || node.size() != 2) return false;

		value.name = node[0].as<std::u16string>();
		value.id = node[1].as<Hash128>();
		return true;
	}
}
