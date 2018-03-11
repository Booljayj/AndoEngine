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
	Owned.push_back( EntityOwnedComponent{ TypeID, Component } );
}

void Entity::Reset( std::vector<EntityOwnedComponent>& OutComponents )
{
	OutComponents.reserve( OutComponents.size() + Owned.size() );
	OutComponents.insert( OutComponents.end(), Owned.begin(), Owned.end() );
	Owned.clear();
}

bool Entity::Has( ComponentTypeID TypeID ) const
{
	return std::find( Owned.begin(), Owned.end(), TypeID ) != Owned.end();
}

bool Entity::HasAll( std::vector<ComponentTypeID> const& TypeIDs ) const
{
	//@todo: finish this later when sorting is established. Loop through each TypeID and increment the Owned iterator for each one.
	//		If we encounter an Owned ID that is higher than the TypeID or reach the end of the Owned components, the test fails.
	//		If we encounter the correct TypeID, we increment the TypeID iterator and keep the same Owned iterator.

	// vector<ComponentTypeID>::iterator TypeIDIterator = TypeIDs.begin();
	// vector<OwnedComponentsEntityComponent>::iterator OwnedComponentsIterator = Owned.begin();
	// for(; TypeIDIterator < TypeIDs.end(); ++TypeIDIterator )
	// {
	// 	bool FoundComponentType = false;
	// 	for(; OwnedComponentsIterator < Owned.end(); ++OwnedComponentsIterator )
	// 	{
	// 		if( Owned)
	// 	}
	// }
	return false;
}

ptr_t Entity::Get( ComponentTypeID TypeID ) const
{
	auto FoundIter = std::find( Owned.begin(), Owned.end(), TypeID );
	return FoundIter != Owned.end() ? FoundIter->CompPtr : nullptr;
}

DESCRIPTION( Entity )
{
	return l_printf( CTX.Temp, "[Entity]{ Components: %i }", Value.Owned.size() );
}
