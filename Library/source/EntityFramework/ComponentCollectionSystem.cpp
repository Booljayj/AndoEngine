#include "EntityFramework/ComponentCollectionSystem.h"
#include <cstring>
#include "Engine/Context.h"
#include "Engine/LinearContainers.h"
#include "Engine/LinearStrings.h"
#include "Engine/ScopedTempBlock.h"
#include "EntityFramework/ComponentInfo.h"

ComponentCollectionSystem::Searcher::Searcher( const ComponentCollectionSystem& InCollection )
: Collection( InCollection )
, CurrentIter( Collection.RegisteredTypeIDs.begin() )
{}

ComponentInfo const* ComponentCollectionSystem::Searcher::Get() const
{
	const size_t Index = ( CurrentIter - Collection.RegisteredTypeIDs.begin() );
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

bool ComponentCollectionSystem::Startup( CTX_ARG, const ComponentInfo* const* Infos, size_t Count )
{
	RegisteredInfos.reserve( Count );
	RegisteredTypeIDs.reserve( Count );
	RegisteredNames.reserve( Count );

	//Add all the information about the registered components, making sure it's all sorted by ID
	for( size_t Index = 0; Index < Count; ++Index ) {
		RegisteredInfos.push_back( Infos[Index] );
	}
	std::sort( RegisteredInfos.begin(), RegisteredInfos.end(), &ComponentInfo::Compare );
	for( const ComponentInfo* Info : RegisteredInfos ) {
		RegisteredTypeIDs.push_back( Info->GetID() );
		RegisteredNames.push_back( Info->GetName() );
	}

	//Ensure we have no duplicates
	bool ComponentsAreUnique = true;

	const auto DuplicateInfoIter = std::adjacent_find( RegisteredInfos.begin(), RegisteredInfos.end() );
	if( DuplicateInfoIter != RegisteredInfos.end() ) {
		CTX.Log->Error( l_printf( CTX.Temp, "ComponentCollection has duplicate info: %p", (void*)*DuplicateInfoIter ) );
		ComponentsAreUnique = false;
	}

	const auto DuplicateIDIter = std::adjacent_find( RegisteredTypeIDs.begin(), RegisteredTypeIDs.end() );
	if( DuplicateIDIter != RegisteredTypeIDs.end() ) {
		CTX.Log->Error( l_printf( CTX.Temp, "ComponentCollection has duplicate ID: %i", *DuplicateIDIter ) );
		ComponentsAreUnique = false;
	}

	const auto StrEq = []( const char* A, const char* B ){ return std::strcmp( A, B ) == 0; };
	const auto DuplicateNameIter = std::adjacent_find( RegisteredNames.begin(), RegisteredNames.end(), StrEq );
	if( DuplicateNameIter != RegisteredNames.end() ) {
		CTX.Log->Error( l_printf( CTX.Temp, "ComponentCollection has duplicate name: %s", *DuplicateNameIter ) );
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
	for( const ComponentInfo* Info : RegisteredInfos ) {
		if( !Info->GetManager()->Startup( CTX ) ) {
			CTX.Log->Error( l_printf( CTX.Temp, "%s manager startup failed", Info->GetName() ) );
			ManagerStartupWasSuccessful = false;
		}
	}
	return ManagerStartupWasSuccessful;
}

bool ComponentCollectionSystem::Shutdown( CTX_ARG )
{
	//Shut down the component managers we currently have registered
	bool ManagerShutdownWasSuccessful = true;
	for( const auto* Info : RegisteredInfos ) {
		if( !Info->GetManager()->Shutdown( CTX ) ) {
			CTX.Log->Error( l_printf( CTX.Temp, "%s manager shutdown failed", Info->GetName() ) );
			ManagerShutdownWasSuccessful = false;
		}
	}
	return ManagerShutdownWasSuccessful;
}

bool ComponentCollectionSystem::ContainsComponentInfos( CTX_ARG, const ComponentInfo* const* Infos, size_t Count ) const
{
	for( size_t Index = 0; Index < Count; ++Index ) {
		const ComponentInfo* Info = Infos[Index];
		const auto FoundIter = std::find( RegisteredInfos.begin(), RegisteredInfos.end(), Info );
		if( FoundIter == RegisteredInfos.end() ) {
			return false;
		}
	}
	return true;
}

const ComponentInfo* ComponentCollectionSystem::GetComponentInfo( CTX_ARG, ComponentTypeID TypeID ) const
{
	const auto FoundIter = std::lower_bound( RegisteredTypeIDs.begin(), RegisteredTypeIDs.end(), TypeID );

	if( FoundIter != RegisteredTypeIDs.end() ) {
		const size_t Index = FoundIter - RegisteredTypeIDs.begin();
		return RegisteredInfos[Index];

	} else {
		CTX.Log->Warning( l_printf( CTX.Temp, "Unknown component type id %i, cannot find info", TypeID ) );
		return nullptr;
	}
}

const ComponentInfo* ComponentCollectionSystem::GetComponentInfo( CTX_ARG, const char* Name ) const
{
	const auto Compare = [=]( const char* Other ){ return ( Name == Other ) || ( std::strcmp( Name, Other ) == 0 ); };
	const auto FoundIter = std::find_if( RegisteredNames.begin(), RegisteredNames.end(), Compare );

	if( FoundIter != RegisteredNames.end() ) {
		const size_t Index = FoundIter - RegisteredNames.begin();
		return RegisteredInfos[Index];

	} else {
		CTX.Log->Warning( l_printf( CTX.Temp, "Unknown component name %s, cannot find info", Name ) );
		return nullptr;
	}
}

void ComponentCollectionSystem::GetComponentInfos( CTX_ARG, const ComponentTypeID* TypeIDs, const ComponentInfo** OutInfos, size_t Count ) const
{
	for( size_t Index = 0; Index < Count; ++Index ) {
		OutInfos[Index] = GetComponentInfo( CTX, TypeIDs[Index] );
	}
}

void ComponentCollectionSystem::GetComponentInfos( CTX_ARG, const char* const* Names, const ComponentInfo** OutInfos, size_t Count ) const
{
	for( size_t Index = 0; Index < Count; ++Index ) {
		OutInfos[Index] = GetComponentInfo( CTX, Names[Index] );
	}
}

void ComponentCollectionSystem::DescribeComponents( CTX_ARG ) const
{
	TEMP_SCOPE;
	for( ComponentInfo const* Info : RegisteredInfos )
	{
		CTX.Log->Message( l_printf( CTX.Temp, "\t%s", DESC( *Info ) ) );
	}
}

DESCRIPTION( ComponentCollectionSystem )
{
	return l_printf( CTX.Temp, "[ComponentCollection]{ Count: %i }", Value.RegisteredTypeIDs.size() );
}
