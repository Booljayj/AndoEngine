#pragma once
#include <vector>
#include <tuple>
#include "Engine/Logging/LogCategory.h"
#include "Engine/LinearContainers.h"
#include "EntityFramework/Types.h"
#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/ComponentManager.h"

DECLARE_LOG_CATEGORY( LogEntity );

struct EntityOwnedComponent
{
	ptr_t CompPtr;
	ComponentTypeID TypeID;

	bool operator==( const ComponentTypeID& OtherTypeID ) const { return TypeID == OtherTypeID; }
	bool operator<( const ComponentTypeID& OtherTypeID ) const { return TypeID < OtherTypeID; }
	bool operator<( const EntityOwnedComponent& Other ) const { return TypeID < Other.TypeID; }
};

/** An Entity is an identifiable object in the game. It can own components, which define different sets of data */
struct Entity
{
public:
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
	bool HasAll( ComponentTypeID const* TypeIDs, size_t Count ) const;
	template< typename TALLOC >
	bool HasAll( std::vector<ComponentTypeID, TALLOC> const& TypeIDs ) const { return HasAll( TypeIDs.data(), TypeIDs.size() ); }

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
