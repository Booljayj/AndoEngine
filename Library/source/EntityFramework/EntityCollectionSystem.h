#pragma once
#include <initializer_list>
#include <memory>
#include <vector>
#include "Engine/Context.h"
#include "Engine/LinearContainers.h"
#include "EntityFramework/ComponentCollectionSystem.h"
#include "EntityFramework/EntityFilter.h"
#include "EntityFramework/Types.h"

struct Entity;
struct EntityOwnedComponent;
struct ComponentInfo;

constexpr EntityID NullEntity = 0;
constexpr EntityID RootEntity = 1;

struct EntityCollectionSystem {
public:
	bool Startup(CTX_ARG, ComponentCollectionSystem const* inComponentCollection, size_t initialCount);
	bool Shutdown(CTX_ARG);

	/// Entity creation/destruction
	/** Create an entity that contains the components with optional loaded data */
	Entity const* Create(CTX_ARG, EntityID const& newID, ComponentInfo const* const* infos, ByteStream const* byteStreams, size_t count);
	Entity const* Create(CTX_ARG, EntityID const& newID, std::initializer_list<ComponentInfo const*> const& infos);

	template<size_t Size>
	std::shared_ptr<EntityFilter<Size>> MakeFilter(ComponentInfo const* (&infos)[Size]);

	/** Update the entity filters with any new entities that have been created or entities which were destroyed. */
	void UpdateFilters();

	/** Destroy the entity with the provided id */
	bool Destroy(EntityID const& deletedID);
	/** Recycle any components that were being used by destroyed entities */
	void RecycleGarbage(CTX_ARG);

	/// Entity queries
	/** Returns true if there is an entity with the provided id */
	bool Exists(EntityID const& id) const noexcept;
	/** Returns a pointer to the entity that is using the provided id, or null if no entity has this id */
	Entity const* Find(EntityID const& id) const noexcept;
	/** The number of entities in this collection */
	size_t count() const noexcept { return entityIDs.size(); }

private:
	ComponentCollectionSystem const* componentCollection;

	// TODO: break entity groups into 'catalogues'. Each catalogue has poly methods for Exists, Find, and Create.
	// Primary catalogue is gameplay one, which holds runtime entities. Other catalogues can hold asset entities which may be loaded from disk.
	// IDs are only unique to each catalogue, not globally.
	// NOTE: 2 different kinds of entities exist: runtime and asset. Asset can be further subdivided, but have the same behavior.
	// Breaking into two different groups and handling separately makes more sense, as Asset entities have async loading.
	// NOTE2: Catalogue needs an index and settings file. These should be in a common directory, project root, but can specify a subfolder from which all paths derive.
	// Multiple indexes can be supplied to override asset paths and add new entities (modding).
	std::vector<Entity> entities;
	std::vector<EntityID> entityIDs;

	std::vector<EntityID> addedEntities;
	std::vector<EntityID> removedEntities;

	std::vector<std::shared_ptr<EntityFilterBase>> filters;

	std::vector<EntityOwnedComponent> reclaimedComponentBuffer;

	/** Insert a new entity with the specified id */
	Entity* InsertNew(CTX_ARG, EntityID const& newID);
	/** Find the index of an id in the master id list */
	size_t FindPositionByEntityID(EntityID const& id) const noexcept;
	/** Find the index of an entity in the master entity list */
	size_t FindPositionByEntity(Entity const& entityRef) const noexcept;
};

template<size_t Size>
std::shared_ptr<EntityFilter<Size>> EntityCollectionSystem::MakeFilter(ComponentInfo const* (&infos)[Size]) {
	std::shared_ptr<EntityFilter<Size>> const newSharedFilter = std::make_shared<EntityFilter<Size>>(infos);
	filters.push_back(std::static_pointer_cast<EntityFilterBase>(newSharedFilter));
	return newSharedFilter;
}
