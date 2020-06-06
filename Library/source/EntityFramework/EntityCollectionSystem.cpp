#include <algorithm>
#include "EntityFramework/EntityCollectionSystem.h"
#include "Engine/Context.h"
#include "Engine/LinearContainers.h"
#include "Engine/LinearStrings.h"
#include "Engine/LogCommands.h"
#include "Engine/ScopedTempBlock.h"
#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/Entity.h"

bool EntityCollectionSystem::Startup(CTX_ARG, ComponentCollectionSystem const* inComponentCollection, size_t initialCount) {
	componentCollection = inComponentCollection;
	entities.reserve(initialCount);
	entityIDs.reserve(initialCount);
	return !!componentCollection;
}

bool EntityCollectionSystem::Shutdown(CTX_ARG) {
	return true;
}

Entity const* EntityCollectionSystem::Create(CTX_ARG, EntityID const& newID, ComponentInfo const* const* infos, ByteStream const* byteStreams, size_t count) {
	TEMP_SCOPE;
	Entity* newEntity = InsertNew(CTX, newID);
	if (newEntity) {
		newEntity->Reserve(count);

		for (size_t index = 0; index < count; ++index) {
			if (ComponentInfo const* const info = infos[index]) {
				if (ptr_t newComponentPtr = info->GetManager()->Retain()) {
					//Either load the predefined data into the component, or wipe it to a default state.
					if (byteStreams) {
						ByteStream const& byteStream = byteStreams[index];
						info->GetManager()->Load(newComponentPtr, byteStream);
					} else {
						info->GetManager()->Wipe(newComponentPtr);
					}

					newEntity->Add(info->GetID(), newComponentPtr);

				} else {
					LOGF(Entity, Warning, "Failed to retain new component of type %i", info->GetID());
					//CTX.Log->Warning( l_printf( CTX.Temp, "Failed to retain new component of type %i", info->GetID() ) );
				}
			} else {
				LOG(Entity, Error, "Attempted to create an entity with a null component");
			}
		}

		//Perform final setup on the entity's new components
		for (size_t index = 0; index < count; ++index) {
			if (ComponentInfo const* const info = infos[index]) {
				info->GetManager()->Setup(*newEntity, newEntity->Get(info->GetID()));
			}
		}

		//Record the new entity so that filters can be updated
		addedEntities.push_back(newID);
	}
	return newEntity;
}

Entity const* EntityCollectionSystem::Create(CTX_ARG, EntityID const& newID, std::initializer_list<const ComponentInfo*> const& componentInfos) {
	return Create(CTX, newID, componentInfos.begin(), nullptr, componentInfos.size());
}

void EntityCollectionSystem::UpdateFilters() {
	for (std::shared_ptr<EntityFilterBase>& filter : filters) {
		for (EntityID const& removedEntity : removedEntities) {
			filter->Remove(removedEntity);
		}
		for (EntityID const& addedEntity : addedEntities) {
			size_t const EntityIndex = FindPositionByEntityID(addedEntity);
			filter->Add(addedEntity, entities[EntityIndex]);
		}
	}
	addedEntities.clear();
	removedEntities.clear();
}

bool EntityCollectionSystem::Destroy(EntityID const& id) {
	size_t destroyedEntityIndex = FindPositionByEntityID(id);
	if (destroyedEntityIndex < entityIDs.size()) {
		size_t lastEntityIndex = entityIDs.size() - 1;

		entities[destroyedEntityIndex].Reset(reclaimedComponentBuffer);

		//Last entity and destroyed entity switch places entirely. Destroyed entity is now last.
		std::swap(entities[destroyedEntityIndex], entities[lastEntityIndex]); //Swap actual entities
		std::swap(entityIDs[destroyedEntityIndex], entityIDs[lastEntityIndex]); //Swap entity IDs
		std::swap(destroyedEntityIndex, lastEntityIndex); //Swap our current indexes for both entities

		entities.pop_back();
		entityIDs.pop_back();

		//Record the destroyed entity so that filters can be updated
		removedEntities.push_back(id);
		return true;

	} else {
		return false;
	}
}

void EntityCollectionSystem::RecycleGarbage(CTX_ARG) {
	if (reclaimedComponentBuffer.size() > 0) {
		std::sort(reclaimedComponentBuffer.begin(), reclaimedComponentBuffer.end());
		ComponentCollectionSystem::Searcher searcher{*componentCollection};

		for (EntityOwnedComponent const& reclaimedComponent : reclaimedComponentBuffer) {
			if (searcher.Next(reclaimedComponent.typeID)) {
				searcher.Get()->GetManager()->Release(reclaimedComponent.compPtr);

			} else {
				LOGF(Entity, Error, "Cannot find component type %i, garbage for this component will not be released!", reclaimedComponent.typeID);
				searcher.Reset();
			}
		}

		reclaimedComponentBuffer.clear();
	}
}

bool EntityCollectionSystem::Exists(EntityID const& id) const noexcept {
	return std::find(entityIDs.begin(), entityIDs.end(), id) != entityIDs.end();
}

Entity const* EntityCollectionSystem::Find(EntityID const& id) const noexcept {
	size_t const foundEntityIndex = FindPositionByEntityID(id);
	if (foundEntityIndex < entityIDs.size()) {
		return &entities[foundEntityIndex];
	} else {
		return nullptr;
	}
}

Entity* EntityCollectionSystem::InsertNew(CTX_ARG, EntityID const& newID) {
	if (std::find(entityIDs.begin(), entityIDs.end(), newID) != entityIDs.end()) {
		LOGF(Entity, Error, "Cannot create new entity with id '%i', that id already exists", newID);
		return nullptr;
	}

	entities.push_back(Entity{});
	entityIDs.push_back(newID);
	return &entities.back();
}

size_t EntityCollectionSystem::FindPositionByEntityID(EntityID const& id) const noexcept {
	//Search in reverse so that newer entities are higher priority
	auto const rIter = std::find(entityIDs.rbegin(), entityIDs.rend(), id);
	return std::distance(entityIDs.begin(), (rIter.base() - 1));
}

size_t EntityCollectionSystem::FindPositionByEntity(Entity const& entityRef) const noexcept {
	//The referenced entity should be in our internal array, so calculate the position with the address
	return &entityRef - &(*entities.begin());
}
