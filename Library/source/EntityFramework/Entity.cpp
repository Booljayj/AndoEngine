#include <cassert>
#include <tuple>
#include "EntityFramework/Entity.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

DEFINE_LOG_CATEGORY(Entity, Debug);

void Entity::Reserve(size_t componentCount) {
	owned.reserve(componentCount);
}

void Entity::Add(ComponentTypeID typeID, ptr_t component) {
	EntityOwnedComponent const newOwnedComponent{component, typeID};
	auto const iter = std::upper_bound(owned.begin(), owned.end(), newOwnedComponent);
	owned.insert(iter, newOwnedComponent);
}

void Entity::Reset(std::vector<EntityOwnedComponent>& outComponents) {
	outComponents.reserve(outComponents.size() + owned.size());
	outComponents.insert(outComponents.end(), owned.begin(), owned.end());
	owned.clear();
}

bool Entity::Has(ComponentTypeID typeID) const {
	auto const iter = std::lower_bound(owned.begin(), owned.end(), typeID);
	return (iter != owned.end()) && (iter->typeID == typeID);
}

bool Entity::HasAll(ComponentTypeID const* typeIDs, size_t count) const {
	for (size_t index = 0; index < count; ++index) {
		if (!Has(typeIDs[index])) return false;
	}
	return true;
}

ptr_t Entity::Get(ComponentTypeID typeID) const {
	auto foundIter = std::find(owned.begin(), owned.end(), typeID);
	return foundIter != owned.end() ? foundIter->compPtr : nullptr;
}
