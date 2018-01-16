#pragma once
#include <iostream>
#include "Engine/Print.h"
#include "EntityFramework/Types.h"
#include "EntityFramework/ComponentManager.h"

#define CREATE_COMPONENT( __TYPE__, __NAME__, __ID__, __MANAGER_TYPE__ )\
__MANAGER_TYPE__<__TYPE__> __NAME__##Manager{};\
TComponentInfo<__TYPE__> __NAME__{ __ID__, #__NAME__, &__NAME__##Manager }

#define CREATE_STANDARD_COMPONENT( __TYPE__, __NAME__, __ID__ )\
CREATE_COMPONENT( __TYPE__, __NAME__, __ID__, TComponentManager )

/** Represents a component that can be owned by an entity */
struct ComponentInfo
{
	CAN_DESCRIBE( ComponentInfo );

public:
	ComponentInfo( const ComponentTypeID& InID, const char* InName, ComponentManager* InManager )
		: ID( InID ), Name( InName ), Manager( InManager )
	{}
	virtual ~ComponentInfo() {}

protected:
	/** The unique ID of this component. Used to identify a component, so this should never change once it is used */
	ComponentTypeID ID;
	/** The human-readable name of this component. Used in debugging and some types of serialization */
	const char* Name;
	/** The manager which creates and collates components of this type */
	ComponentManager* Manager;

public:
	ComponentTypeID GetID() const { return ID; }
	const char* GetName() const { return Name; }
	ComponentManager* GetManager() const { return Manager; }

	friend std::ostream& operator<<( std::ostream& Stream, const ComponentInfo& Info );
};

/** Template used to provide type information to a ComponentInfo */
template< typename TDATA >
struct TComponentInfo : public ComponentInfo
{
	TComponentInfo( ComponentTypeID InID, const char* InName, TComponentManager<TDATA>* InManager )
		: ComponentInfo( InID, InName, InManager )
	{}
	virtual ~TComponentInfo() override {}

	/** The type of the component */
	using TYPE = TDATA;

	TComponentManager<TDATA>* GetTypedManager() const { return static_cast<TComponentManager<TDATA>*>( ComponentInfo::GetManager() ); }
};

DESCRIPTION( ComponentInfo );
