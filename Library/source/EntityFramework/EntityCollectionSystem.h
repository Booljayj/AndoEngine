#pragma once
#include <initializer_list>
#include <memory>
#include <vector>
#include "Engine/UtilityMacros.h"
#include "Engine/LinearContainers.h"
#include "Engine/Print.h"
#include "EntityFramework/ComponentCollectionSystem.h"
#include "EntityFramework/EntityFilter.h"
#include "EntityFramework/Types.h"

struct Entity;
struct EntityOwnedComponent;
struct ComponentInfo;

constexpr EntityID Entity_Null = 0;
constexpr EntityID Entity_Root = 1;

struct EntityCollectionSystem
{
	CAN_DESCRIBE( EntityCollectionSystem );

public:
	bool Startup( CTX_ARG, ComponentCollectionSystem const* InComponentCollection, size_t InitialCount );
	bool Shutdown( CTX_ARG );

	/// Entity creation/destruction
	/** Create an entity that contains the components with optional loaded data */
	Entity const* Create( CTX_ARG, EntityID const& NewID, ComponentInfo const* const* Infos, ByteStream const* ByteStreams, size_t Count );
	Entity const* Create( CTX_ARG, EntityID const& NewID, std::initializer_list<ComponentInfo const*> const& Infos );

	template<size_t SIZE>
	std::shared_ptr<EntityFilter<SIZE>> MakeFilter( ComponentInfo const* (&Infos)[SIZE] );

	/** Update the entity filters with any new entities that have been created or entities which were destroyed. */
	void UpdateFilters();

	/** Destroy the entity with the provided ID */
	bool Destroy( EntityID const& DeletedID );
	/** Recycle any components that were being used by destroyed entities */
	void RecycleGarbage( CTX_ARG );

	/// Entity queries
	/** Returns true if there is an entity with the provided ID */
	bool Exists( EntityID const& ID ) const noexcept;
	/** Returns a pointer to the entity that is using the provided ID, or null if no entity has this ID */
	Entity const* Find( EntityID const& ID ) const noexcept;
	/** The number of entities in this collection */
	size_t Count() const noexcept { return EntityIDs.size(); }

private:
	ComponentCollectionSystem const* ComponentCollection;

	// TODO: break entity groups into 'catalogues'. Each catalogue has poly methods for Exists, Find, and Create.
	// Primary catalogue is gameplay one, which holds runtime entities. Other catalogues can hold asset entities which may be loaded from disk.
	// IDs are only unique to each catalogue, not globally.
	// NOTE: 2 different kinds of entities exist: runtime and asset. Asset can be further subdivided, but have the same behavior.
	// Breaking into two different groups and handling separately makes more sense, as Asset entities have async loading.
	// NOTE2: Catalogue needs an index and settings file. These should be in a common directory, project root, but can specify a subfolder from which all paths derive.
	// Multiple indexes can be supplied to override asset paths and add new entities (modding).
	std::vector<Entity> Entities;
	std::vector<EntityID> EntityIDs;

	std::vector<EntityID> AddedEntities;
	std::vector<EntityID> RemovedEntities;

	std::vector<std::shared_ptr<EntityFilterBase>> Filters;

	std::vector<EntityOwnedComponent> ReclaimedComponentBuffer;

	/** Insert a new entity with the specified ID */
	Entity* InsertNew( CTX_ARG, EntityID const& NewID );
	/** Find the index of an ID in the master ID list */
	size_t FindPositionByEntityID( EntityID const& ID ) const noexcept;
	/** Find the index of an entity in the master entity list */
	size_t FindPositionByEntity( Entity const& EntityRef ) const noexcept;
};

DESCRIPTION( EntityCollectionSystem );

template<size_t SIZE>
std::shared_ptr<EntityFilter<SIZE>> EntityCollectionSystem::MakeFilter( ComponentInfo const* (&Infos)[SIZE] )
{
	std::shared_ptr<EntityFilter<SIZE>> NewSharedFilter = std::make_shared<EntityFilter<SIZE>>( Infos );
	std::shared_ptr<EntityFilterBase> NewSharedFilterBase =  std::static_pointer_cast<EntityFilterBase>( NewSharedFilter );
	Filters.push_back( NewSharedFilterBase );
	return NewSharedFilter;
}
