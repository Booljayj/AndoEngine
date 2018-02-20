#pragma once
#include <iostream>
#include "Engine/Print.h"
#include "EntityFramework/Types.h"
#include "EntityFramework/ComponentManager.h"

#define CREATE_COMPONENT( __ID__, __NAME__, __TYPE__, __MAN_INIT__ )\
auto __NAME__##Manager = __MAN_INIT__;\
TComponentInfo<__TYPE__> __NAME__{ __ID__, #__NAME__, &__NAME__##Manager }

/** Represents a component that can be owned by an entity */
struct ComponentInfo
{
	CAN_DESCRIBE( ComponentInfo );

public:
	virtual ~ComponentInfo() {}

	ComponentTypeID GetID() const { return ID; }
	const char* GetName() const { return Name; }
	ComponentManager* GetManager() const { return Manager; }

protected:
	/** Hidden explicit construction. Use the derived template to create instances of ComponentInfo. */
	ComponentInfo( const ComponentTypeID& InID, const char* InName, ComponentManager* InManager )
		: ID( InID ), Name( InName ), Manager( InManager )
	{}

	/** The unique ID of this component. Used to identify a component, so this should never change once it is used */
	ComponentTypeID ID;
	/** The human-readable name of this component. Used in debugging and some types of serialization */
	const char* Name;
	/** The manager which creates and collates components of this type */
	ComponentManager* Manager;
};

/** Template used to provide type information to a ComponentInfo in template functions. */
template< typename TDATA >
struct TComponentInfo : public ComponentInfo
{
public:
	TComponentInfo( ComponentTypeID InID, const char* InName, ComponentManager* InManager )
		: ComponentInfo( InID, InName, InManager )
	{}
	virtual ~TComponentInfo() override {}

	/** The type of the component */
	using TYPE = TDATA;
};

DESCRIPTION( ComponentInfo );
