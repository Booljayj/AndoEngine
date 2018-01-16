#pragma once
#include <cassert>
#include <iostream>
#include <iomanip>
#include <vector>
#include <tuple>
#include <deque>
#include <unordered_map>
#include "Engine/Context.h"
#include "Engine/Print.h"
#include "EntityFramework/Types.h"
#include "EntityFramework/Entity.h"
#include "EntityFramework/ComponentInfo.h"

constexpr EntityID Entity_Null = 0;
constexpr EntityID Entity_Root = 1;

namespace S
{
	struct EntitySystem
	{
		CAN_DESCRIBE( EntitySystem );

		EntitySystem() = default;

		template< size_t N >
		bool Startup( CTX_ARG, const ComponentInfo* (&InComponentInfos)[N] )
		{
			std::array<std::tuple<ComponentTypeID, const ComponentInfo*>, N> ComponentInfoPairs;
			for( size_t Index = 0; Index < N; ++Index )
			{
				const ComponentInfo* InInfo = InComponentInfos[Index];
				ComponentInfoPairs[Index] = std::make_tuple( InInfo->GetID(), InInfo );
			}
			std::sort( ComponentInfoPairs.begin(), ComponentInfoPairs.end() );
			if( std::adjacent_find( ComponentInfoPairs.begin(), ComponentInfoPairs.end() ) != ComponentInfoPairs.end() )
			{
				CTX.Log->Error( "EntitySystem must not have duplicate component infos" );
				return false;
			}

			RegisteredComponentTypeIDs.reserve( N );
			RegisteredComponentInfos.reserve( N );

			for( const auto& ComponentInfoPair : ComponentInfoPairs )
			{
				const ComponentInfo* Info = std::get<1>( ComponentInfoPair );
				RegisteredComponentTypeIDs.push_back( Info->GetID() );
				RegisteredComponentInfos.push_back( Info );

				if( !Info->GetManager()->Initialize() )
				{
					CTX.Log->Error( "EntitySystem startup failed." );
					return false;
				}
			}

			CTX.Log->Verbose( "EntitySystem startup complete." );
			return true;
		}

		bool Shutdown( CTX_ARG );

		/// Entity creation
		/** Create a blank entity that contains no components */
		void Create( const EntityID& NewID );
		/** Create an entity that contains the components referenced by the component infos */
		void Create( const EntityID& NewID, const std::vector<const ComponentInfo*>& ComponentInfos, const std::vector<ByteStream>& ComponentDatas = {} );
		/** Create an entity that contains the components referenced by the component IDs */
		void Create( const EntityID& NewID, const std::vector<ComponentTypeID>& ComponentTypeIDs, const std::vector<ByteStream>& ComponentDatas = {} );

		/// Entity destruction
		/** Destroy a particular entity */
		bool Destroy( Entity* );
		/** Destroy the entity with the provided ID */
		bool Destroy( const EntityID& ID );

		/// Entity queries
		/** Returns true if there is an entity with the provided ID */
		bool Exists( const EntityID& ID ) const noexcept;
		/** Returns a pointer to the entity that is using the provided ID, or null if no entity has this ID */
		Entity* Find( const EntityID& ID ) const noexcept;

		const std::vector<const ComponentInfo*>& GetRegisteredComponents() const { return RegisteredComponentInfos; }

	protected:
		std::vector<ComponentTypeID> RegisteredComponentTypeIDs;
		std::vector<const ComponentInfo*> RegisteredComponentInfos;

		std::vector<const ComponentInfo*> ComponentInfoBuffer;
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
		Entity& InsertNew( const EntityID NewID );
		/** Get a list of component infos from the list of component types */
		const std::vector<const ComponentInfo*>& GetComponentInfos( const std::vector<ComponentTypeID>& ComponentTypeIDs );
		/** Find the index of an ID in the master ID list */
		size_t FindPositionByEntityID( const EntityID& ID ) const noexcept;
		/** Find the index of an entity in the master entity list */
		size_t FindPositionByEntity( const Entity& EntityRef ) const noexcept;
		/** Find a registered component info using its TypeID */
		const ComponentInfo* FindComponentInfo( ComponentTypeID ID ) const noexcept;

		/** Release all components in the reclamation buffer */
		void ReleaseReclaimedComponents();
	};

	DESCRIPTION( EntitySystem );
}
