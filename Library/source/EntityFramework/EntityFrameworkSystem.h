#pragma once
#include <initializer_list>
#include <vector>
#include "Engine/UtilityMacros.h"
#include "Engine/LinearContainers.h"
#include "Engine/Print.h"
#include "EntityFramework/Types.h"

struct Entity;
struct EntityOwnedComponent;
struct ComponentInfo;

constexpr EntityID Entity_Null = 0;
constexpr EntityID Entity_Root = 1;

namespace S
{
	struct EntityFrameworkSystem
	{
		CAN_DESCRIBE( EntityFrameworkSystem );

		bool Startup( CTX_ARG, const std::initializer_list<const ComponentInfo*>& InComponentInfos );
		bool Shutdown( CTX_ARG );

		/// Entity creation
		/** Create an entity that contains the components with optional loaded data */
		const Entity* Create( CTX_ARG, const EntityID& NewID, const ComponentInfo* const* ComponentInfoBegin, const ByteStream* ComponentDataBegin, size_t ComponentCount );
		const Entity* Create( CTX_ARG, const EntityID& NewID, const std::initializer_list<const ComponentInfo*>& ComponentInfos );

		/// Entity destruction
		/** Destroy a particular entity */
		bool Destroy( Entity* );
		/** Destroy the entity with the provided ID */
		bool Destroy( const EntityID& ID );

		/// Entity queries
		/** Returns true if there is an entity with the provided ID */
		bool Exists( const EntityID& ID ) const noexcept;
		/** Returns a pointer to the entity that is using the provided ID, or null if no entity has this ID */
		const Entity* Find( const EntityID& ID ) const noexcept;

		/** Get a list of all the registered components in this system */
		const std::vector<const ComponentInfo*>& GetRegisteredComponents() const { return RegisteredComponentInfos; }
		/** Get a list of component infos from the list of component types */
		const l_vector<const ComponentInfo*> GetComponentInfos( CTX_ARG, const l_vector<ComponentTypeID>& ComponentTypeIDs );
		const l_vector<const ComponentInfo*> GetComponentInfos( CTX_ARG, const std::initializer_list<ComponentTypeID>& ComponentTypeIDs );

	protected:
		std::vector<ComponentTypeID> RegisteredComponentTypeIDs;
		std::vector<const ComponentInfo*> RegisteredComponentInfos;

		std::vector<EntityOwnedComponent> ReclaimedComponentBuffer;

		// TODO: break entity groups into 'catalogues'. Each catalogue has poly methods for Exists, Find, and Create.
		// Primary catalogue is gameplay one, which holds runtime entities. Other catalogues can hold asset entities which may be loaded from disk.
		// IDs are only unique to each catalogue, not globally.
		// NOTE: 2 different kinds of entities exist: runtime and asset. Asset can be further subdivided, but have the same behavior.
		// Breaking into two different groups and handling separately makes more sense, as Asset entities have async loading.
		// NOTE2: Catalogue needs an index and settings file. These should be in a common directory, project root, but can specify a subfolder from which all paths derive.
		// Multiple indexes can be supplied to override asset paths and add new entities (modding).
		std::vector<Entity> Entities;
		std::vector<EntityID> EntityIDs;

		/** Insert a new entity with the specified ID */
		Entity* InsertNew( CTX_ARG, const EntityID NewID );
		/** Find the index of an ID in the master ID list */
		size_t FindPositionByEntityID( const EntityID& ID ) const noexcept;
		/** Find the index of an entity in the master entity list */
		size_t FindPositionByEntity( const Entity& EntityRef ) const noexcept;
		/** Find a registered component info using its TypeID */
		const ComponentInfo* FindComponentInfo( ComponentTypeID ID ) const noexcept;

		/** Release all components in the reclamation buffer */
		void ReleaseReclaimedComponents();
	};

	DESCRIPTION( EntityFrameworkSystem );
}
