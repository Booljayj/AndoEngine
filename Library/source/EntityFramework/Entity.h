#pragma once
#include <vector>
#include <tuple>
#include "Engine/Print.h"
#include "EntityFramework/Types.h"
#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/ComponentManager.h"

struct EntityOwnedComponent
{
	ComponentTypeID TypeID;
	ptr_t CompPtr;

	bool operator==( const ComponentTypeID& InTypeID ) const { return TypeID == InTypeID; }
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

	/** Setup this entity to use a set of components that are loaded from some set of data */
	void Setup( const std::vector<const ComponentInfo*>& InComponentInfos, const std::vector<ByteStream>& InComponentDatas );
	/** Setup this entity to use a set of defaulted components */
	void Setup( const std::vector<const ComponentInfo*>& InComponentInfos );
	/** Reset this component, moving out the list of owned components so they can be cleaned up externally */
	void Reset( std::vector<EntityOwnedComponent>& OutOwnedComponents );

	/// Component manipulation

	/** Returns true if this entity contains a component of the specified type */
	bool Has( const ComponentTypeID TypeID ) const;
	/** Returns true if this entity contains all of the component types in the vector */
	bool HasAll( const std::vector<ComponentTypeID>& TypeIDs ) const;

	/** Returns a pointer to a component this entity owns of the specified type */
	ptr_t Get( const ComponentTypeID& TypeID ) const;
	template<typename TTData>
	TTData* Get( const TComponentInfo<TTData>& ComponentInfo ) { return static_cast<TTData*>( Get( ComponentInfo.GetID() ) ); }

	//Debugging information
	size_t Count() const { return Owned.size(); }
	size_t Capacity() const { return 0; } //used for fixed-size entities

protected:
	std::vector<EntityOwnedComponent> Owned;
};

DESCRIPTION( Entity );
