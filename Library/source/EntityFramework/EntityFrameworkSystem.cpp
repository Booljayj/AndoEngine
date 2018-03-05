#include <cassert>
#include "EntityFramework/EntityFrameworkSystem.h"
#include "Engine/Context.h"
#include "Engine/LinearContainers.h"
#include "Engine/LinearStrings.h"
#include "Engine/Logger.h"
#include "Engine/Print.h"
#include "Engine/ScopedTempBlock.h"
#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/Entity.h"

using namespace std;

namespace S
{
	bool EntityFrameworkSystem::Startup( CTX_ARG, const std::initializer_list<const ComponentInfo*>& InComponentInfos )
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
			CTX.Log->Error( "EntityFramework must not have duplicate component infos" );
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
				CTX.Log->Error( "ComponentManager startup failed." );
				return false;
			}
		}

		return true;
	}

	bool EntityFrameworkSystem::Shutdown( CTX_ARG )
	{
		for( auto* Info : RegisteredComponentInfos )
		{
			if( !Info->GetManager()->Shutdown( CTX ) )
			{
				CTX.Log->Error( "ComponentManager shutdown failed." );
				return false;
			}
		}
		return true;
	}

	const Entity* EntityFrameworkSystem::Create( CTX_ARG, const EntityID& NewID, const ComponentInfo* const* ComponentInfoBegin, const ByteStream* ComponentDataBegin, size_t ComponentCount )
	{
		TEMP_SCOPE;

		struct ValidatedComponentData {
			ComponentTypeID TypeID;
			const ComponentInfo* Info;
			void* Component;

			bool operator<( const ValidatedComponentData& Other ) const { return TypeID < Other.TypeID; }
		};

		Entity* NewEntity = InsertNew( CTX, NewID );
		if( NewEntity ) {
			//Build a sorted list of the components to add to the entity
			l_vector<ValidatedComponentData> ValidatedData{ CTX.Temp };
			for( size_t Index = 0; Index < ComponentCount; ++Index ) {
				const ComponentInfo* Info = *( ComponentInfoBegin + Index );

				if( Info ) {
					ptr_t NewRetainedComponent = Info->GetManager()->Retain();

					//Either load the predefined data into the component, or wipe it to a default state.
					if( ComponentDataBegin ) {
						const ByteStream& Data = *( ComponentDataBegin + Index );
						Info->GetManager()->Load( NewRetainedComponent, Data );
					} else {
						Info->GetManager()->Wipe( NewRetainedComponent );
					}

					ValidatedData.push_back( ValidatedComponentData{ Info->GetID(), Info, NewRetainedComponent } );

				} else {
					//@todo Would it be useful to print the index of the null entity here? Depends, maybe add later if it becomes necessary.
					CTX.Log->Warning( "Null component type found when creating an entity, it will be skipped." );
				}
			}
			std::sort( ValidatedData.begin(), ValidatedData.end() );
			const size_t ActualComponentCount = ValidatedData.size(); //The number of components after nullptrs have been removed.

			//Add the new components to the entity
			NewEntity->Reserve( ValidatedData.size() );
			for( size_t Index = 0; Index < ActualComponentCount; ++Index ) {
				const auto& Data = ValidatedData[Index];
				NewEntity->Add( Data.TypeID, Data.Component );
			}

			//Perform final setup on the entity's new components
			for( size_t Index = 0; Index < ActualComponentCount; ++Index ) {
				const auto& Data = ValidatedData[Index];
				Data.Info->GetManager()->Setup( *NewEntity, Data.Component );
			}
		}
		return NewEntity;
	}

	const Entity* EntityFrameworkSystem::Create( CTX_ARG, const EntityID& NewID, const std::initializer_list<const ComponentInfo*>& ComponentInfos )
	{
		return Create( CTX, NewID, ComponentInfos.begin(), nullptr, ComponentInfos.size() );
	}

	bool EntityFrameworkSystem::Destroy( const EntityID& ID )
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

	bool EntityFrameworkSystem::Exists( const EntityID& ID ) const noexcept
	{
		return std::find( EntityIDs.begin(), EntityIDs.end(), ID ) != EntityIDs.end();
	}

	const Entity* EntityFrameworkSystem::Find( const EntityID& ID ) const noexcept
	{
		size_t FoundEntityIndex = FindPositionByEntityID( ID );
		if( FoundEntityIndex < EntityIDs.size() )
		{
			return &Entities[FoundEntityIndex];
		}
		else
		{
			return nullptr;
		}
	}

	const l_vector<const ComponentInfo*> EntityFrameworkSystem::GetComponentInfos( CTX_ARG, const l_vector<ComponentTypeID>& ComponentTypeIDs )
	{
		l_vector<const ComponentInfo*> ReturnBuffer{ CTX.Temp };
		ReturnBuffer.reserve( ComponentTypeIDs.size() );
		for( const auto& ComponentType : ComponentTypeIDs )
		{
			const ComponentInfo* Info = FindComponentInfo( ComponentType );
			ReturnBuffer.push_back( Info );
		}
		return ReturnBuffer;
	}
	const l_vector<const ComponentInfo*> EntityFrameworkSystem::GetComponentInfos( CTX_ARG, const std::initializer_list<ComponentTypeID>& ComponentTypeIDs )
	{
		l_vector<ComponentTypeID> TempBuffer{ ComponentTypeIDs, CTX.Temp };
		return GetComponentInfos( CTX, TempBuffer );
	}

	Entity* EntityFrameworkSystem::InsertNew( CTX_ARG, const EntityID NewID )
	{
		if( std::find( EntityIDs.begin(), EntityIDs.end(), NewID ) != EntityIDs.end() ) {
			CTX.Log->Error( l_printf( CTX.Temp, "Cannot create new entity with ID '%i', that ID already exists", NewID ) );
			return nullptr;
		}

		Entities.push_back( Entity{} );
		EntityIDs.push_back( NewID );
		return &Entities.back();
	}

	size_t EntityFrameworkSystem::FindPositionByEntityID( const EntityID& ID ) const noexcept
	{
		return std::find( EntityIDs.begin(), EntityIDs.end(), ID ) - EntityIDs.begin();
	}

	size_t EntityFrameworkSystem::FindPositionByEntity( const Entity& EntityRef ) const noexcept
	{
		return std::find( Entities.begin(), Entities.end(), EntityRef ) - Entities.begin();
	}

	const ComponentInfo* EntityFrameworkSystem::FindComponentInfo( ComponentTypeID ID ) const noexcept
	{
		size_t ComponentIndex = std::find( RegisteredComponentTypeIDs.begin(), RegisteredComponentTypeIDs.end(), ID ) - RegisteredComponentTypeIDs.begin();
		if( ComponentIndex < RegisteredComponentInfos.size() ) {
			return RegisteredComponentInfos[ComponentIndex];
		}
		return nullptr;
	}

	void EntityFrameworkSystem::ReleaseReclaimedComponents()
	{
		//@todo This can be more efficient because both ReclaimedComponentBuffer and the Registered components array should be sorted,
		// so we can start where we left off during the last iteration.
		for( auto& ReclaimedComponent : ReclaimedComponentBuffer )
		{
			const ComponentInfo* Info = FindComponentInfo( ReclaimedComponent.TypeID );
			assert( Info );//, "Reclaimed a component type that is not registered with the EntityFrameworkSystem" );
			Info->GetManager()->Release( ReclaimedComponent.CompPtr );
		}
	}

	DESCRIPTION( EntityFrameworkSystem )
	{
		return l_printf( CTX.Temp, "[EntityFramework]{ Component Count: %i, Entity Count: %i }", Value.RegisteredComponentTypeIDs.size(), Value.EntityIDs.size() );
	}
}
