#include "EntityFramework/ComponentCollectionSystem.h"
#include <cstring>
#include "Engine/Context.h"
#include "Engine/LogCommands.h"
#include "Engine/LinearContainers.h"
#include "Engine/LinearStrings.h"
#include "Engine/ScopedTempBlock.h"
#include "EntityFramework/ComponentInfo.h"

ComponentCollectionSystem::Searcher::Searcher( ComponentCollectionSystem const& InCollection )
: Collection( InCollection )
, CurrentIter( Collection.RegisteredTypeIDs.begin() )
{}

ComponentInfo const* ComponentCollectionSystem::Searcher::Get() const
{
	size_t const Index = std::distance( Collection.RegisteredTypeIDs.begin(),  CurrentIter );
	return Collection.RegisteredInfos[Index];
}

bool ComponentCollectionSystem::Searcher::Next( ComponentTypeID TypeID )
{
	if( CurrentIter == Collection.RegisteredTypeIDs.end() ) {
		return false;
	} else if( *CurrentIter == TypeID ) {
		return true;
	} else {
		CurrentIter = std::lower_bound( CurrentIter, Collection.RegisteredTypeIDs.end(), TypeID );
		return CurrentIter != Collection.RegisteredTypeIDs.end();
	}
}

void ComponentCollectionSystem::Searcher::Reset()
{
	CurrentIter = Collection.RegisteredTypeIDs.begin();
}

bool ComponentCollectionSystem::Startup( CTX_ARG, ComponentInfo const* const* Infos, size_t Count )
{
	RegisteredInfos.reserve( Count );
	RegisteredTypeIDs.reserve( Count );
	RegisteredNames.reserve( Count );

	//Add all the information about the registered components, making sure it's all sorted by ID
	for( size_t Index = 0; Index < Count; ++Index ) {
		RegisteredInfos.push_back( Infos[Index] );
	}
	std::sort( RegisteredInfos.begin(), RegisteredInfos.end(), ComponentInfo::Compare );
	for( ComponentInfo const* Info : RegisteredInfos ) {
		RegisteredTypeIDs.push_back( Info->GetID() );
		RegisteredNames.push_back( Info->GetName() );
	}

	//Ensure we have no duplicates
	bool ComponentsAreUnique = true;

	auto const DuplicateInfoIter = std::adjacent_find( RegisteredInfos.begin(), RegisteredInfos.end() );
	if( DuplicateInfoIter != RegisteredInfos.end() ) {
		LOGF( LogComponent, Error, "ComponentCollection has duplicate info: %p", (void*)*DuplicateInfoIter );
		ComponentsAreUnique = false;
	}

	auto const DuplicateIDIter = std::adjacent_find( RegisteredTypeIDs.begin(), RegisteredTypeIDs.end() );
	if( DuplicateIDIter != RegisteredTypeIDs.end() ) {
		LOGF( LogComponent, Error, "ComponentCollection has duplicate ID: %i", *DuplicateIDIter );
		ComponentsAreUnique = false;
	}

	auto const StrEq = []( char const* A, char const* B ){ return std::strcmp( A, B ) == 0; };
	auto const DuplicateNameIter = std::adjacent_find( RegisteredNames.begin(), RegisteredNames.end(), StrEq );
	if( DuplicateNameIter != RegisteredNames.end() ) {
		LOGF( LogComponent, Error, "ComponentCollection has duplicate name: %s", *DuplicateNameIter );
		ComponentsAreUnique = false;
	}

	//If we failed the duplicate check, it's no longer safe to keep the registered components
	if( !ComponentsAreUnique )
	{
		RegisteredInfos.clear();
		RegisteredTypeIDs.clear();
		RegisteredNames.clear();
		return false;
	}

	//Start up the component managers if we still have infos registered
	bool ManagerStartupWasSuccessful = true;
	for( ComponentInfo const* Info : RegisteredInfos ) {
		if( !Info->GetManager()->Startup( CTX ) ) {
			LOGF( LogComponent, Error, "%s manager startup failed", Info->GetName() );
			ManagerStartupWasSuccessful = false;
		}
	}
	return ManagerStartupWasSuccessful;
}

bool ComponentCollectionSystem::Shutdown( CTX_ARG )
{
	//Shut down the component managers we currently have registered
	bool ManagerShutdownWasSuccessful = true;
	for( auto const* Info : RegisteredInfos ) {
		if( !Info->GetManager()->Shutdown( CTX ) ) {
			LOGF( LogComponent, Error, "%s manager shutdown failed", Info->GetName() );
			ManagerShutdownWasSuccessful = false;
		}
	}
	return ManagerShutdownWasSuccessful;
}

bool ComponentCollectionSystem::ContainsComponentInfos( CTX_ARG, ComponentInfo const* const* Infos, size_t Count ) const
{
	for( size_t Index = 0; Index < Count; ++Index ) {
		auto const FoundIter = std::find( RegisteredInfos.begin(), RegisteredInfos.end(), Infos[Index] );
		if( FoundIter == RegisteredInfos.end() ) {
			return false;
		}
	}
	return true;
}

ComponentInfo const* ComponentCollectionSystem::GetComponentInfo( CTX_ARG, ComponentTypeID TypeID ) const
{
	auto const FoundIter = std::lower_bound( RegisteredTypeIDs.begin(), RegisteredTypeIDs.end(), TypeID );
	if( FoundIter != RegisteredTypeIDs.end() ) {
		size_t const Index = std::distance( RegisteredTypeIDs.begin(), FoundIter );
		return RegisteredInfos[Index];

	} else {
		LOGF( LogComponent, Warning, "Unknown component type id %i, cannot find info", TypeID );
		return nullptr;
	}
}

ComponentInfo const* ComponentCollectionSystem::GetComponentInfo( CTX_ARG, char const* Name ) const
{
	auto const Compare = [=]( char const* Other ){ return ( Name == Other ) || ( std::strcmp( Name, Other ) == 0 ); };
	auto const FoundIter = std::find_if( RegisteredNames.begin(), RegisteredNames.end(), Compare );

	if( FoundIter != RegisteredNames.end() ) {
		size_t const Index = std::distance( RegisteredNames.begin(), FoundIter );
		return RegisteredInfos[Index];

	} else {
		LOGF( LogComponent, Warning, "Unknown component name %s, cannot find info", Name );
		return nullptr;
	}
}

void ComponentCollectionSystem::GetComponentInfos( CTX_ARG, ComponentTypeID const* TypeIDs, ComponentInfo const** OutInfos, size_t Count ) const
{
	for( size_t Index = 0; Index < Count; ++Index ) {
		OutInfos[Index] = GetComponentInfo( CTX, TypeIDs[Index] );
	}
}

void ComponentCollectionSystem::GetComponentInfos( CTX_ARG, char const* const* Names, ComponentInfo const** OutInfos, size_t Count ) const
{
	for( size_t Index = 0; Index < Count; ++Index ) {
		OutInfos[Index] = GetComponentInfo( CTX, Names[Index] );
	}
}
