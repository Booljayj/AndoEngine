#include <cassert>
#include "EntityFramework/Types.h"
#include "EntityFramework/Entity.h"

Entity::Entity()
{}

void Entity::Setup( const std::vector<ComponentInfo*>& InComponentInfos, const std::vector<ByteStream>& InComponentDatas /* = {} */ )
{
	Owned.reserve( InComponentInfos.size() );

	if( InComponentDatas.empty() )
	{
		for( size_t Index = 0; Index < InComponentInfos.size(); ++Index )
		{
			auto* const ComponentInfo = InComponentInfos[Index];
			ptr_t const NewOwnedComponent = ComponentInfo->GetManager()->Retain();
			Owned.push_back( EntityOwnedComponent{ ComponentInfo->GetID(), NewOwnedComponent } );
		}
	}
	else
	{
		assert( InComponentInfos.size() == InComponentDatas.size() );
		for( size_t Index = 0; Index < InComponentInfos.size(); ++Index )
		{
			auto* const ComponentInfo = InComponentInfos[Index];
			ptr_t const NewOwnedComponent = ComponentInfo->GetManager()->Retain();
			Owned.push_back( EntityOwnedComponent{ ComponentInfo->GetID(), NewOwnedComponent } );

			auto& ComponentData = InComponentDatas[Index];
			ComponentInfo->GetManager()->Load( NewOwnedComponent, ComponentData );
		}
	}

	for( size_t OwnedIndex = 0; OwnedIndex < Owned.size(); ++OwnedIndex )
	{
		InComponentInfos[OwnedIndex]->GetManager()->Setup( *this, Owned[OwnedIndex].CompPtr );
	}
}

void Entity::Reset( std::vector<EntityOwnedComponent>& OutOwnedComponents )
{
	OutOwnedComponents = std::move( Owned );
}

bool Entity::Has( const ComponentTypeID& TypeID ) const
{
	return std::find( Owned.begin(), Owned.end(), TypeID ) != Owned.end();
}

ptr_t Entity::Get( const ComponentTypeID& TypeID ) const
{
	auto FoundIter = std::find( Owned.begin(), Owned.end(), TypeID );

	return FoundIter != Owned.end() ? FoundIter->CompPtr : nullptr;
}

std::ostream& operator<<( std::ostream& Stream, const Entity& Entity )
{
	Stream << "[Entity]: { Components: ";
	for( auto& OwnedComp : Entity.Owned )
	{
		Stream << OwnedComp.TypeID << ", ";
	}
	return Stream << " }";
}

ptr_t Entity::Add( const ComponentTypeID& TypeID, ComponentManager* Manager )
{
	ptr_t NewlyOwnedComponent = Manager->Retain();
	Owned.push_back( EntityOwnedComponent{ TypeID, NewlyOwnedComponent } );
	return NewlyOwnedComponent;
}

void Entity::Del( const ComponentTypeID& TypeID, ComponentManager* Manager )
{
	auto FoundIter = std::find( Owned.begin(), Owned.end(), TypeID );

	if( FoundIter != Owned.end() )
	{
		Manager->Release( FoundIter->CompPtr );
		Owned.erase( FoundIter );
	}
}
