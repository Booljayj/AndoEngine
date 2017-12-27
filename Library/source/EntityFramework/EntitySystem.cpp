#include <stdio.h>
#include <cassert>
#include "EntityFramework/EntitySystem.h"

using namespace std;

namespace S
{
	EntitySystem::EntitySystem( const vector<ComponentInfo*>& ComponentInfos )
	{
		for( auto* ComponentInfo : ComponentInfos )
		{
			ComponentInfoMap.insert( make_pair( ComponentInfo->GetID(), ComponentInfo ) );
		}
	}

	bool EntitySystem::Initialize()
	{
		for( auto& ComponentInfoPair : ComponentInfoMap )
		{
			ComponentInfo* Info = ComponentInfoPair.second;
			if( !Info->GetManager()->Initialize() )
			{
				fprintf( stdout, "Fatal error initializing component manager for %s", Info->GetName() );
				return false;
			}
		}
		return true;
	}

	bool EntitySystem::Deinitialize()
	{
		for( auto& ComponentInfoPair : ComponentInfoMap )
		{
			ComponentInfo* Info = ComponentInfoPair.second;
			if( !Info->GetManager()->Deinitialize() )
			{
				fprintf( stdout, "Fatal error deinitializing component manager for %s", Info->GetName() );
				return false;
			}
		}
		return true;
	}

	void EntitySystem::Create( const EntityID& NewID )
	{
		(void)InsertNew( NewID );
	}

	void EntitySystem::Create( const EntityID& NewID, const vector<ComponentInfo*>& ComponentInfos, const vector<ByteStream>& ComponentDatas /* = {} */ )
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

	const vector<ComponentInfo*>& EntitySystem::GetComponentInfos( const vector<ComponentTypeID>& ComponentTypes )
	{
		ComponentInfoBuffer.clear();
		for( const auto& ComponentType : ComponentTypes )
		{
			ComponentInfoBuffer.push_back( ComponentInfoMap[ComponentType] );
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

	void EntitySystem::ReleaseReclaimedComponents()
	{
		for( auto& ReclaimedComponent : ReclaimedComponentBuffer )
		{
			assert( ComponentInfoMap.find( ReclaimedComponent.TypeID ) != ComponentInfoMap.end() );
			ComponentInfoMap[ReclaimedComponent.TypeID]->GetManager()->Release( ReclaimedComponent.CompPtr );
		}
	}
}
