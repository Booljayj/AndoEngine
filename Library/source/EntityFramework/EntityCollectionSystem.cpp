#include <cassert>
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

	struct ValidatedComponentData {
		ComponentTypeID TypeID;
		ComponentInfo const* Info;
		ptr_t Component;

		bool operator<( ValidatedComponentData const& Other ) const { return TypeID < Other.TypeID; }
	};

	Entity* NewEntity = InsertNew( CTX, NewID );
	if( NewEntity ) {
		//Build a sorted list of the components to add to the entity
		l_vector<ValidatedComponentData> ValidatedData{ CTX.Temp };
		ValidatedData.reserve( Count );

		for( size_t Index = 0; Index < Count; ++Index ) {
			if( ComponentInfo const* Info = Infos[Index] ) {
				if( ptr_t NewComponentPtr = Info->GetManager()->Retain() ) {
					//Either load the predefined data into the component, or wipe it to a default state.
					if( ByteStreams ) {
						ByteStream const& ByteStream = ByteStreams[Index];
						Info->GetManager()->Load( NewComponentPtr, ByteStream );

					} else {
						Info->GetManager()->Wipe( NewComponentPtr );
					}

					ValidatedData.push_back( ValidatedComponentData{ Info->GetID(), Info, NewComponentPtr } );

				} else {
					CTX.Log->Warning( l_printf( CTX.Temp, "Failed to retain new component of type %i", Info->GetID() ) );
				}

			} else {
				CTX.Log->Error( "Attempted to create an entity with a null component" );
			}
		}
		std::sort( ValidatedData.begin(), ValidatedData.end() );
		size_t const ActualComponentCount = ValidatedData.size(); //The number of components after nullptrs have been removed.

		//Add the new components to the entity
		NewEntity->Reserve( ValidatedData.size() );
		for( size_t Index = 0; Index < ActualComponentCount; ++Index ) {
			auto const& Data = ValidatedData[Index];
			NewEntity->Add( Data.TypeID, Data.Component );
		}

		//Perform final setup on the entity's new components
		for( size_t Index = 0; Index < ActualComponentCount; ++Index ) {
			auto const& Data = ValidatedData[Index];
			Data.Info->GetManager()->Setup( *NewEntity, Data.Component );
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
	return std::find( EntityIDs.begin(), EntityIDs.end(), ID ) - EntityIDs.begin();
}

size_t EntityCollectionSystem::FindPositionByEntity( Entity const& EntityRef ) const noexcept
{
	return std::find( Entities.begin(), Entities.end(), EntityRef ) - Entities.begin();
}

DESCRIPTION( EntityCollectionSystem )
{
	return l_printf( CTX.Temp, "[EntityFramework]{ Count: %i }", Value.EntityIDs.size() );
}
