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

void Entity::Add( ComponentTypeID TypeID, void* Component )
{
	Owned.push_back( EntityOwnedComponent{ TypeID, Component } );
}

void Entity::Reset( std::vector<EntityOwnedComponent>& OutComponents )
{
	OutComponents.reserve( OutComponents.size() + Owned.size() );
	OutComponents.insert( OutComponents.end(), Owned.begin(), Owned.end() );
	Owned.clear();
}

bool Entity::Has( const ComponentTypeID TypeID ) const
{
	return std::find( Owned.begin(), Owned.end(), TypeID ) != Owned.end();
}

bool Entity::HasAll( const std::vector<ComponentTypeID>& TypeIDs ) const
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

ptr_t Entity::Get( const ComponentTypeID& TypeID ) const
{
	auto FoundIter = std::find( Owned.begin(), Owned.end(), TypeID );
	return FoundIter != Owned.end() ? FoundIter->CompPtr : nullptr;
}

DESCRIPTION( Entity )
{
	return l_printf( CTX.Temp, "[Entity]{ Components: %i }", Value.Owned.size() );
}
