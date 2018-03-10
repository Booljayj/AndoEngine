#pragma once
#include <initializer_list>
#include <vector>
#include "Engine/UtilityMacros.h"
#include "Engine/LinearContainers.h"
#include "Engine/Print.h"
#include "EntityFramework/ComponentCollectionSystem.h"
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
	bool Startup( CTX_ARG, const ComponentCollectionSystem* InComponentCollection, size_t InitialCount );
	bool Shutdown( CTX_ARG );

	/// Entity creation/destruction
	/** Create an entity that contains the components with optional loaded data */
	const Entity* Create( CTX_ARG, const EntityID& NewID, const ComponentInfo* const* Infos, const ByteStream* ByteStreams, size_t Count );
	const Entity* Create( CTX_ARG, const EntityID& NewID, const std::initializer_list<const ComponentInfo*>& Infos );

	/** Destroy the entity with the provided ID */
	bool Destroy( const EntityID& DeletedID );
	/** Recycle any components that were being used by destroyed entities */
	void RecycleGarbage( CTX_ARG );

	/// Entity queries
	/** Returns true if there is an entity with the provided ID */
	bool Exists( const EntityID& ID ) const noexcept;
	/** Returns a pointer to the entity that is using the provided ID, or null if no entity has this ID */
	const Entity* Find( const EntityID& ID ) const noexcept;
	/** The number of entities in this collection */
	size_t Count() const noexcept { return EntityIDs.size(); }

private:
	const ComponentCollectionSystem* ComponentCollection;

	// TODO: break entity groups into 'catalogues'. Each catalogue has poly methods for Exists, Find, and Create.
	// Primary catalogue is gameplay one, which holds runtime entities. Other catalogues can hold asset entities which may be loaded from disk.
	// IDs are only unique to each catalogue, not globally.
	// NOTE: 2 different kinds of entities exist: runtime and asset. Asset can be further subdivided, but have the same behavior.
	// Breaking into two different groups and handling separately makes more sense, as Asset entities have async loading.
	// NOTE2: Catalogue needs an index and settings file. These should be in a common directory, project root, but can specify a subfolder from which all paths derive.
	// Multiple indexes can be supplied to override asset paths and add new entities (modding).
	std::vector<Entity> Entities;
	std::vector<EntityID> EntityIDs;

	std::vector<EntityOwnedComponent> ReclaimedComponentBuffer;

	/** Insert a new entity with the specified ID */
	Entity* InsertNew( CTX_ARG, const EntityID NewID );
	/** Find the index of an ID in the master ID list */
	size_t FindPositionByEntityID( const EntityID& ID ) const noexcept;
	/** Find the index of an entity in the master entity list */
	size_t FindPositionByEntity( const Entity& EntityRef ) const noexcept;
};

DESCRIPTION( EntityCollectionSystem );
