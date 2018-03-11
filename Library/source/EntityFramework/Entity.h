#pragma once
#include <vector>
#include <tuple>
#include "Engine/Print.h"
#include "Engine/LinearContainers.h"
#include "Engine/UtilityMacros.h"
#include "EntityFramework/Types.h"
#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/ComponentManager.h"

struct EntityOwnedComponent
{
	ComponentTypeID TypeID;
	ptr_t CompPtr;

	bool operator==( const ComponentTypeID& InTypeID ) const { return TypeID == InTypeID; }
	bool operator<( const EntityOwnedComponent& Other ) const { return TypeID < Other.TypeID; }
};

/** An Entity is an identifiable object in the game. It can own components, which define different sets of data */
struct Entity
{
	CAN_DESCRIBE( Entity );

	Entity();
	Entity( Entity&& Other ) = default;
	Entity( const Entity& Other ) = delete;

	Entity& operator=( Entity&& Other ) = default;
	Entity& operator=( const Entity& Other ) = delete;

	bool operator==( const Entity& Other ) const { return Owned.data() == Other.Owned.data(); }

	/// Entity creation

	void Reserve( size_t ComponentCount );
	void Add( ComponentTypeID TypeID, void* Component );
	void Reset( std::vector<EntityOwnedComponent>& OutComponents );

	/// Component manipulation

	/** Returns true if this entity contains a component of the specified type */
	bool Has( ComponentTypeID TypeID ) const;
	/** Returns true if this entity contains all of the component types in the vector */
	bool HasAll( std::vector<ComponentTypeID> const& TypeIDs ) const;

	/** Returns a pointer to a component this entity owns of the specified type */
	ptr_t Get( ComponentTypeID TypeID ) const;
	template<typename TTData>
	TTData* Get( TComponentInfo<TTData> const& ComponentInfo ) const { return static_cast<TTData*>( Get( ComponentInfo.GetID() ) ); }

	//Debugging information
	size_t Size() const { return Owned.size(); }
	size_t Capacity() const { return Owned.capacity(); } //used for fixed-size entities

protected:
	std::vector<EntityOwnedComponent> Owned;
};

DESCRIPTION( Entity );
