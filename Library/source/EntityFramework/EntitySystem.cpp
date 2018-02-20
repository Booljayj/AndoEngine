#include <stdio.h>
#include <cassert>
#include "Engine/LinearContainers.h"
#include "EntityFramework/EntitySystem.h"

using namespace std;

namespace S
{
	bool EntitySystem::Startup( CTX_ARG, const l_vector<const ComponentInfo*>& InComponentInfos )
	{
		const size_t Count = InComponentInfos.size();
		l_vector<std::tuple<ComponentTypeID, const ComponentInfo*>> ComponentInfoPairs{ CTX.Temp };
		ComponentInfoPairs.reserve( Count );
		for( const ComponentInfo* Info : InComponentInfos )
		{
			ComponentInfoPairs.push_back( std::make_tuple( Info->GetID(), Info ) );
		}

		std::sort( ComponentInfoPairs.begin(), ComponentInfoPairs.end() );
		if( std::adjacent_find( ComponentInfoPairs.begin(), ComponentInfoPairs.end() ) != ComponentInfoPairs.end() )
		{
			CTX.Log->Error( "EntitySystem must not have duplicate component infos" );
			return false;
		}

		RegisteredComponentTypeIDs.reserve( Count );
		RegisteredComponentInfos.reserve( Count );

		for( const auto& ComponentInfoPair : ComponentInfoPairs )
		{
			const ComponentInfo* Info = std::get<1>( ComponentInfoPair );
			RegisteredComponentTypeIDs.push_back( Info->GetID() );
			RegisteredComponentInfos.push_back( Info );

			if( !Info->GetManager()->Startup( CTX ) )
			{
				CTX.Log->Error( "EntitySystem startup failed." );
				return false;
			}
		}

		CTX.Log->Verbose( "EntitySystem startup complete." );
		return true;
	}

	bool EntitySystem::Shutdown( CTX_ARG )
	{
		for( auto* Info : RegisteredComponentInfos )
		{
			if( !Info->GetManager()->Shutdown( CTX ) )
			{
				CTX.Log->Error( "EntitySystem shutdown failed." );
				return false;
			}
		}
		CTX.Log->Verbose( "EntitySystem shutdown complete." );
		return true;
	}

	void EntitySystem::Create( const EntityID& NewID )
	{
		(void)InsertNew( NewID );
	}

	void EntitySystem::Create( const EntityID& NewID, const vector<const ComponentInfo*>& ComponentInfos, const vector<ByteStream>& ComponentDatas /* = {} */ )
	{
		InsertNew( NewID ).Setup( ComponentInfos, ComponentDatas );
	}

	void EntitySystem::Create( const EntityID& NewID, const vector<ComponentTypeID>& ComponentTypeIDs, const vector<ByteStream>& ComponentDatas /* = {} */ )
	{
		InsertNew( NewID ).Setup( GetComponentInfos( ComponentTypeIDs ), ComponentDatas );
	}

	bool EntitySystem::Destroy( const EntityID& ID )
	{
		size_t DestroyedEntityIndex = FindPositionByEntityID( ID );
		if( DestroyedEntityIndex < EntityIDs.size() )
		{
			size_t LastEntityIndex = EntityIDs.size() - 1;

			//Last entity and destroyed entity switch places entirely. Destroyed entity is now last.
			std::swap( Entities[DestroyedEntityIndex], Entities[LastEntityIndex] ); //Swap actual entities
			std::swap( EntityIDs[DestroyedEntityIndex], EntityIDs[LastEntityIndex] ); //Swap entity IDs
			std::swap( DestroyedEntityIndex, LastEntityIndex ); //Swap our current indexes for both entities

			Entities[DestroyedEntityIndex].Reset( ReclaimedComponentBuffer );
			ReleaseReclaimedComponents();

			Entities.pop_back();
			EntityIDs.pop_back();

			return true;
		}
		else
		{
			return false;
		}
	}

	bool EntitySystem::Exists( const EntityID& ID ) const noexcept
	{
		return std::find( EntityIDs.begin(), EntityIDs.end(), ID ) != EntityIDs.end();
	}

	Entity* EntitySystem::Find( const EntityID& ID ) const noexcept
	{
		size_t FoundEntityIndex = FindPositionByEntityID( ID );
		if( FoundEntityIndex < EntityIDs.size() )
		{
			return const_cast<Entity*>( &Entities[FoundEntityIndex] );
		}
		else
		{
			return nullptr;
		}
	}

	Entity& EntitySystem::InsertNew( const EntityID NewID )
	{
		assert( std::find( EntityIDs.begin(), EntityIDs.end(), NewID ) == EntityIDs.end() );

		Entities.push_back( Entity{} );
		EntityIDs.push_back( NewID );

		return Entities.back();
	}

	const vector<const ComponentInfo*>& EntitySystem::GetComponentInfos( const vector<ComponentTypeID>& ComponentTypes )
	{
		ComponentInfoBuffer.clear();
		for( const auto& ComponentType : ComponentTypes )
		{
			const ComponentInfo* Info = FindComponentInfo( ComponentType );
			assert( Info );//, "Tried to get info for a component type that is not registered" ) ;
			ComponentInfoBuffer.push_back( Info );
		}
		return ComponentInfoBuffer;
	}

	size_t EntitySystem::FindPositionByEntityID( const EntityID& ID ) const noexcept
	{
		return std::find( EntityIDs.begin(), EntityIDs.end(), ID ) - EntityIDs.begin();
	}

	size_t EntitySystem::FindPositionByEntity( const Entity& EntityRef ) const noexcept
	{
		return std::find( Entities.begin(), Entities.end(), EntityRef ) - Entities.begin();
	}

	const ComponentInfo* EntitySystem::FindComponentInfo( ComponentTypeID ID ) const noexcept
	{
		size_t ComponentIndex = std::find( RegisteredComponentTypeIDs.begin(), RegisteredComponentTypeIDs.end(), ID ) - RegisteredComponentTypeIDs.begin();
		if( ComponentIndex < RegisteredComponentInfos.size() )
		{
			return RegisteredComponentInfos[ComponentIndex];
		}
		return nullptr;
	}

	void EntitySystem::ReleaseReclaimedComponents()
	{
		//@todo This can be more efficient because both ReclaimedComponentBuffer and the Registered components array should be sorted,
		// so we can start where we left off during the last iteration.
		for( auto& ReclaimedComponent : ReclaimedComponentBuffer )
		{
			const ComponentInfo* Info = FindComponentInfo( ReclaimedComponent.TypeID );
			assert( Info );//, "Reclaimed a component type that is not registered with the EntitySystem" );
			Info->GetManager()->Release( ReclaimedComponent.CompPtr );
		}
	}

	DESCRIPTION( EntitySystem )
	{
		return l_printf( CTX, "[EntitySystem]{ Components: %i, Entities: %i }", Value.RegisteredComponentTypeIDs.size(), Value.EntityIDs.size() );
	}
}
