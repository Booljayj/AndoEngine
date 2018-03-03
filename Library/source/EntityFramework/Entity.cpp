#include <cassert>
#include "EntityFramework/Entity.h"
#include "Engine/Context.h"
#include "Engine/LinearStrings.h"

Entity::Entity()
{}

void Entity::Setup( const std::vector<const ComponentInfo*>& ComponentInfos, const std::vector<ByteStream>& ComponentDatas )
{
	if( Owned.size() > 0 ) { return; }
	if( ComponentInfos.size() != ComponentDatas.size() ) { return; }
	Owned.reserve( ComponentInfos.size() );

	for( size_t Index = 0; Index < ComponentInfos.size(); ++Index )
	{
		const ComponentInfo* const ComponentInfo = ComponentInfos[Index];
		ptr_t const NewOwnedComponent = ComponentInfo->GetManager()->Retain();
		Owned.push_back( EntityOwnedComponent{ ComponentInfo->GetID(), NewOwnedComponent } );

		const ByteStream& ComponentData = ComponentDatas[Index];
		ComponentInfo->GetManager()->Load( NewOwnedComponent, ComponentData );
	}

	for( size_t OwnedIndex = 0; OwnedIndex < Owned.size(); ++OwnedIndex )
	{
		ComponentInfos[OwnedIndex]->GetManager()->Setup( *this, Owned[OwnedIndex].CompPtr );
	}
}

void Entity::Setup( const std::vector<const ComponentInfo*>& ComponentInfos )
{
	if( Owned.size() > 0 ) { return; }
	Owned.reserve( ComponentInfos.size() );

	for( size_t Index = 0; Index < ComponentInfos.size(); ++Index )
	{
		const ComponentInfo* const ComponentInfo = ComponentInfos[Index];
		ptr_t const NewOwnedComponent = ComponentInfo->GetManager()->Retain();
		Owned.push_back( EntityOwnedComponent{ ComponentInfo->GetID(), NewOwnedComponent } );
	}

	for( size_t OwnedIndex = 0; OwnedIndex < Owned.size(); ++OwnedIndex )
	{
		ComponentInfos[OwnedIndex]->GetManager()->Setup( *this, Owned[OwnedIndex].CompPtr );
	}
}

void Entity::Reset( std::vector<EntityOwnedComponent>& OutOwnedComponents )
{
	OutOwnedComponents.insert( OutOwnedComponents.end(), Owned.begin(), Owned.end() );
	Owned.clear();
}

bool Entity::Has( const ComponentTypeID TypeID ) const
{
	return std::find( Owned.begin(), Owned.end(), TypeID ) != Owned.end();
}

bool Entity::HasAll( const std::vector<ComponentTypeID>& TypeIDs ) const
{
	//@todo: finish this later when sorting is established. Loop through each TypeID and increment the owned iterator for each one.
	//		If we encounter an owned ID that is higher than the TypeID or reach the end of the owned components, the test fails.
	//		If we encounter the correct TypeID, we increment the TypeID iterator and keep the same owned iterator.

	// vector<ComponentTypeID>::iterator TypeIDIterator = TypeIDs.begin();
	// vector<OwnedEntityComponent>::iterator OwnedIterator = Owned.begin();
	// for(; TypeIDIterator < TypeIDs.end(); ++TypeIDIterator )
	// {
	// 	bool FoundComponentType = false;
	// 	for(; OwnedIterator < Owned.end(); ++OwnedIterator )
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
