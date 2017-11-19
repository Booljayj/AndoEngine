#pragma once

#include <iostream>
using namespace std;

#include "EntityFramework/Types.h"
#include "EntityFramework/ComponentManager.h"

struct ComponentInfo
{
public:
	ComponentInfo( const ComponentTypeID& InID, const char* InName, ComponentManager* InManager )
		: ID( InID ), Name( InName ), Manager( InManager )
	{}
	virtual ~ComponentInfo() {}

protected:
	ComponentTypeID ID;
	string Name;
	ComponentManager* Manager;

public:
	ComponentTypeID GetID() const { return ID; }
	string GetName() const { return Name; }
	ComponentManager* GetManager() const { return Manager; }

	friend ostream& operator<<( ostream& Stream, const ComponentInfo& Info );
};

template< typename TDATA >
struct TComponentInfo : public ComponentInfo
{
	TComponentInfo( ComponentTypeID InID, const char* InName, TComponentManager<TDATA>* InManager )
		: ComponentInfo( InID, InName, InManager )
	{}
	virtual ~TComponentInfo() override {}

	TComponentManager<TDATA>* GetTypedManager() const { return static_cast<TComponentManager<TDATA>*>( ComponentInfo::GetManager() ); }
};
