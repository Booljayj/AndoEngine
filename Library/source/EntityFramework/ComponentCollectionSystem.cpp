#include "EntityFramework/ComponentCollectionSystem.h"
#include <cstring>
#include "Engine/Context.h"
#include "Engine/LogCommands.h"
#include "Engine/LinearContainers.h"
#include "Engine/LinearStrings.h"
#include "Engine/ScopedTempBlock.h"
#include "EntityFramework/ComponentInfo.h"

ComponentCollectionSystem::Searcher::Searcher(ComponentCollectionSystem const& inCollection)
: collection(inCollection)
, currentIter(collection.registeredTypeIDs.begin())
{}

ComponentInfo const* ComponentCollectionSystem::Searcher::Get() const {
	size_t const index = std::distance(collection.registeredTypeIDs.begin(), currentIter);
	return collection.registeredInfos[index];
}

bool ComponentCollectionSystem::Searcher::Next(ComponentTypeID typeID) {
	if (currentIter == collection.registeredTypeIDs.end()) {
		return false;
	} else if (*currentIter == typeID) {
		return true;
	} else {
		currentIter = std::lower_bound(currentIter, collection.registeredTypeIDs.end(), typeID);
		return currentIter != collection.registeredTypeIDs.end();
	}
}

void ComponentCollectionSystem::Searcher::Reset() {
	currentIter = collection.registeredTypeIDs.begin();
}

bool ComponentCollectionSystem::Startup(CTX_ARG, ComponentInfo const* const* infos, size_t count) {
	registeredInfos.reserve(count);
	registeredTypeIDs.reserve(count);
	registeredNames.reserve(count);

	//Add all the information about the registered components, making sure it's all sorted by ID
	for (size_t index = 0; index < count; ++index) {
		registeredInfos.push_back(infos[index]);
	}
	std::sort(registeredInfos.begin(), registeredInfos.end(), ComponentInfo::Compare);
	for (ComponentInfo const* info : registeredInfos) {
		registeredTypeIDs.push_back(info->GetID());
		registeredNames.push_back(info->GetName());
	}

	//Ensure we have no duplicates
	bool componentsAreUnique = true;

	auto const duplicateInfoIter = std::adjacent_find(registeredInfos.begin(), registeredInfos.end());
	if (duplicateInfoIter != registeredInfos.end()) {
		LOGF(Component, Error, "ComponentCollection has duplicate info: %p", (void*)*duplicateInfoIter);
		componentsAreUnique = false;
	}

	auto const duplicateIDIter = std::adjacent_find( registeredTypeIDs.begin(), registeredTypeIDs.end() );
	if (duplicateIDIter != registeredTypeIDs.end()) {
		LOGF(Component, Error, "ComponentCollection has duplicate ID: %i", *duplicateIDIter);
		componentsAreUnique = false;
	}

	auto const strEq = [](std::string_view const& a, std::string_view const& b) { return a == b; };
	auto const duplicateNameIter = std::adjacent_find(registeredNames.begin(), registeredNames.end(), strEq);
	if (duplicateNameIter != registeredNames.end()) {
		LOGF(Component, Error, "ComponentCollection has duplicate name: %s", *duplicateNameIter);
		componentsAreUnique = false;
	}

	//If we failed the duplicate check, it's no longer safe to keep the registered components
	if (!componentsAreUnique) {
		registeredInfos.clear();
		registeredTypeIDs.clear();
		registeredNames.clear();
		return false;
	}

	//Start up the component managers if we still have infos registered
	bool managerStartupWasSuccessful = true;
	for (ComponentInfo const* info : registeredInfos) {
		if (!info->GetManager()->Startup(CTX)) {
			LOGF(Component, Error, "%s manager startup failed", info->GetName());
			managerStartupWasSuccessful = false;
		}
	}
	return managerStartupWasSuccessful;
}

bool ComponentCollectionSystem::Shutdown(CTX_ARG) {
	//Shut down the component managers we currently have registered
	bool managerShutdownWasSuccessful = true;
	for (auto const* info : registeredInfos) {
		if (!info->GetManager()->Shutdown(CTX)) {
			LOGF(Component, Error, "%s manager shutdown failed", info->GetName());
			managerShutdownWasSuccessful = false;
		}
	}
	return managerShutdownWasSuccessful;
}

bool ComponentCollectionSystem::ContainsComponentInfos(CTX_ARG, ComponentInfo const* const* infos, size_t count) const {
	for (size_t index = 0; index < count; ++index) {
		auto const foundIter = std::find(registeredInfos.begin(), registeredInfos.end(), infos[index]);
		if (foundIter == registeredInfos.end()) {
			return false;
		}
	}
	return true;
}

ComponentInfo const* ComponentCollectionSystem::GetComponentInfo(CTX_ARG, ComponentTypeID typeID) const {
	auto const foundIter = std::lower_bound(registeredTypeIDs.begin(), registeredTypeIDs.end(), typeID);
	if (foundIter != registeredTypeIDs.end()) {
		size_t const index = std::distance(registeredTypeIDs.begin(), foundIter);
		return registeredInfos[index];

	} else {
		LOGF(Component, Warning, "Unknown component type id %i, cannot find info", typeID);
		return nullptr;
	}
}

ComponentInfo const* ComponentCollectionSystem::GetComponentInfo( CTX_ARG, std::string_view name) const {
	auto const foundIter = std::find(registeredNames.begin(), registeredNames.end(), name);

	if (foundIter != registeredNames.end()) {
		size_t const index = std::distance(registeredNames.begin(), foundIter);
		return registeredInfos[index];

	} else {
		LOGF(Component, Warning, "Unknown component name %s, cannot find info", name);
		return nullptr;
	}
}

void ComponentCollectionSystem::GetComponentInfos(CTX_ARG, ComponentTypeID const* typeIDs, ComponentInfo const** outInfos, size_t count) const {
	for (size_t index = 0; index < count; ++index) {
		outInfos[index] = GetComponentInfo(CTX, typeIDs[index]);
	}
}

void ComponentCollectionSystem::GetComponentInfos(CTX_ARG, char const* const* names, ComponentInfo const** outInfos, size_t count) const {
	for (size_t index = 0; index < count; ++index) {
		outInfos[index] = GetComponentInfo(CTX, names[index]);
	}
}
