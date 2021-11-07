#include "EntityFramework/ComponentRegistry.h"

l_vector<Reflection::TypeInfo const*> ComponentRegistry::GetTypes(CTX_ARG) {
	l_vector<Reflection::TypeInfo const*> result;
	result.reserve(registry.size());
	for (const auto& entry : registry) {
		result.push_back(entry.first);
	}
	return result;
}

IComponent const* ComponentRegistry::Find(Reflection::TypeInfo const* type) const {
	const auto found = registry.find(type);
	if (found != registry.end()) return found->second.get();
	else return nullptr;
}

l_vector<std::tuple<Reflection::TypeInfo const*, void*>> ComponentRegistry::GetComponents(CTX_ARG, const EntityHandle& handle) const {
	l_vector<std::tuple<Reflection::TypeInfo const*, void*>> result;
	for (const auto& entry : registry) {
		if (entry.second->Has(handle)) {
			const std::tuple<Reflection::TypeInfo const*, void*> info{ entry.first, entry.second->Get(handle) };
			result.push_back(info);
		}
	}
	return result;
}
