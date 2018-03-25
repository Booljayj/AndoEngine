#include <algorithm>
#include "EntityFramework/EntityCollectionSystem.h"
#include "Engine/Context.h"
#include "Engine/LinearContainers.h"
#include "Engine/LinearStrings.h"
#include "Engine/Logger.h"
#include "Engine/Print.h"
#include "Engine/ScopedTempBlock.h"
#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/Entity.h"

bool EntityCollectionSystem::Startup( CTX_ARG, ComponentCollectionSystem const* InComponentCollection, size_t InitialCount )
{
	ComponentCollection = InComponentCollection;
	Entities.reserve( InitialCount );
	EntityIDs.reserve( InitialCount );
	return !!ComponentCollection;
}

bool EntityCollectionSystem::Shutdown( CTX_ARG )
{
	return true;
}

Entity const* EntityCollectionSystem::Create( CTX_ARG, EntityID const& NewID, ComponentInfo const* const* Infos, ByteStream const* ByteStreams, size_t Count )
{
	TEMP_SCOPE;
	Entity* NewEntity = InsertNew( CTX, NewID );
	if( NewEntity ) {
		NewEntity->Reserve( Count );

		for( size_t Index = 0; Index < Count; ++Index ) {
			if( ComponentInfo const* const Info = Infos[Index] ) {
				if( ptr_t NewComponentPtr = Info->GetManager()->Retain() ) {
					//Either load the predefined data into the component, or wipe it to a default state.
					if( ByteStreams ) {
						ByteStream const& ByteStream = ByteStreams[Index];
						Info->GetManager()->Load( NewComponentPtr, ByteStream );
					} else {
						Info->GetManager()->Wipe( NewComponentPtr );
					}

					NewEntity->Add( Info->GetID(), NewComponentPtr );

				} else {
					CTX.Log->Warning( l_printf( CTX.Temp, "Failed to retain new component of type %i", Info->GetID() ) );
				}
			} else {
				CTX.Log->Error( "Attempted to create an entity with a null component" );
			}
		}

		//Perform final setup on the entity's new components
		for( size_t Index = 0; Index < Count; ++Index ) {
			if( ComponentInfo const* const Info = Infos[Index] ) {
				Info->GetManager()->Setup( *NewEntity, NewEntity->Get( Info->GetID() ) );
			}
		}
	}
	return NewEntity;
}

Entity const* EntityCollectionSystem::Create( CTX_ARG, EntityID const& NewID, std::initializer_list<const ComponentInfo*> const& ComponentInfos )
{
	return Create( CTX, NewID, ComponentInfos.begin(), nullptr, ComponentInfos.size() );
}

bool EntityCollectionSystem::Destroy( EntityID const& ID )
{
	size_t DestroyedEntityIndex = FindPositionByEntityID( ID );
	if( DestroyedEntityIndex < EntityIDs.size() ) {
		size_t LastEntityIndex = EntityIDs.size() - 1;

		Entities[DestroyedEntityIndex].Reset( ReclaimedComponentBuffer );

		//Last entity and destroyed entity switch places entirely. Destroyed entity is now last.
		std::swap( Entities[DestroyedEntityIndex], Entities[LastEntityIndex] ); //Swap actual entities
		std::swap( EntityIDs[DestroyedEntityIndex], EntityIDs[LastEntityIndex] ); //Swap entity IDs
		std::swap( DestroyedEntityIndex, LastEntityIndex ); //Swap our current indexes for both entities

		Entities.pop_back();
		EntityIDs.pop_back();
		return true;

	} else {
		return false;
	}
}

void EntityCollectionSystem::RecycleGarbage( CTX_ARG )
{
	if( ReclaimedComponentBuffer.size() > 0 ) {
		std::sort( ReclaimedComponentBuffer.begin(), ReclaimedComponentBuffer.end() );
		ComponentCollectionSystem::Searcher Searcher{ *ComponentCollection };

		for( EntityOwnedComponent const& ReclaimedComponent : ReclaimedComponentBuffer ) {
			if( Searcher.Next( ReclaimedComponent.TypeID ) ) {
				Searcher.Get()->GetManager()->Release( ReclaimedComponent.CompPtr );

			} else {
				CTX.Log->Error( l_printf( CTX.Temp, "Cannot find component type %i, garbage for this component will not be released!", ReclaimedComponent.TypeID ) );
				Searcher.Reset();
			}
		}

		ReclaimedComponentBuffer.clear();
	}
}

bool EntityCollectionSystem::Exists( EntityID const& ID ) const noexcept
{
	return std::find( EntityIDs.begin(), EntityIDs.end(), ID ) != EntityIDs.end();
}

Entity const* EntityCollectionSystem::Find( EntityID const& ID ) const noexcept
{
	size_t const FoundEntityIndex = FindPositionByEntityID( ID );
	if( FoundEntityIndex < EntityIDs.size() ) {
		return &Entities[FoundEntityIndex];
	} else {
		return nullptr;
	}
}

Entity* EntityCollectionSystem::InsertNew( CTX_ARG, EntityID const& NewID )
{
	if( std::find( EntityIDs.begin(), EntityIDs.end(), NewID ) != EntityIDs.end() ) {
		CTX.Log->Error( l_printf( CTX.Temp, "Cannot create new entity with ID '%i', that ID already exists", NewID ) );
		return nullptr;
	}

	Entities.push_back( Entity{} );
	EntityIDs.push_back( NewID );
	return &Entities.back();
}

size_t EntityCollectionSystem::FindPositionByEntityID( EntityID const& ID ) const noexcept
{
	return std::distance( EntityIDs.begin(), std::find( EntityIDs.begin(), EntityIDs.end(), ID ) );
}

size_t EntityCollectionSystem::FindPositionByEntity( Entity const& EntityRef ) const noexcept
{
	return std::distance( Entities.begin(), std::find( Entities.begin(), Entities.end(), EntityRef ) );
}

DESCRIPTION( EntityCollectionSystem )
{
	return l_printf( CTX.Temp, "[EntityFramework]{ Count: %i }", Value.EntityIDs.size() );
}
