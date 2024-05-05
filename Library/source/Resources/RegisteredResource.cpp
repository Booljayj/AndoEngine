#include "Resources/RegisteredResource.h"

namespace Resources {
	RegisteredResource::Utilities const* RegisteredResource::FindUtilities(Hash128 id) {
		auto& utilities = GetUtilities();
		auto const iter = utilities.find(id);
		if (iter != utilities.end()) return iter->second.get();
		else return nullptr;
	}

	std::unordered_map<Hash128, std::unique_ptr<RegisteredResource::Utilities>>& RegisteredResource::GetUtilities() {
		static std::unordered_map<Hash128, std::unique_ptr<Utilities>> utilities;
		return utilities;
	}
}
