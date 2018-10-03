#include <cassert>
#include <tuple>
#include "EntityFramework/Entity.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

Entity::Entity()
{}

void Entity::Reserve( size_t ComponentCount )
{
	Owned.reserve( ComponentCount );
}

void Entity::Add( ComponentTypeID TypeID, ptr_t Component )
{
	EntityOwnedComponent NewOwnedComponent{ Component, TypeID };
	const auto Iter = std::upper_bound( Owned.begin(), Owned.end(), NewOwnedComponent );
	Owned.insert( Iter, NewOwnedComponent );
}

void Entity::Reset( std::vector<EntityOwnedComponent>& OutComponents )
{
	OutComponents.reserve( OutComponents.size() + Owned.size() );
	OutComponents.insert( OutComponents.end(), Owned.begin(), Owned.end() );
	Owned.clear();
}

bool Entity::Has( ComponentTypeID TypeID ) const
{
	auto const Iter = std::lower_bound( Owned.begin(), Owned.end(), TypeID );
	return ( Iter != Owned.end() ) && ( Iter->TypeID == TypeID );
}

bool Entity::HasAll( ComponentTypeID const* TypeIDs, size_t Count ) const
{
	for( size_t Index = 0; Index < Count; ++Index ) {
		if( !Has( TypeIDs[Index] ) ) {
			return false;
		}
	}
	return true;
}

ptr_t Entity::Get( ComponentTypeID TypeID ) const
{
	auto FoundIter = std::find( Owned.begin(), Owned.end(), TypeID );
	return FoundIter != Owned.end() ? FoundIter->CompPtr : nullptr;
}
