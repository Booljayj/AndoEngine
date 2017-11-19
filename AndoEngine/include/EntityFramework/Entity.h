#pragma once
#include <vector>
#include <tuple>
#include "EntityFramework/Types.h"
#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/ComponentManager.h"

struct EntityOwnedComponent
{
	ComponentTypeID TypeID;
	ptr_t CompPtr;

	bool operator==( const ComponentTypeID& InTypeID ) const { return TypeID == InTypeID; }
};

struct Entity
{
	Entity();
	Entity( Entity&& Other ) = default;
	Entity( const Entity& Other ) = delete;

	Entity& operator=( Entity&& Other ) = default;
	Entity& operator=( const Entity& Other ) = delete;

	bool operator==( const Entity& Other ) const { return Owned.data() == Other.Owned.data(); }

	//Entity creation
	void Setup( const std::vector<ComponentInfo*>& InComponentInfos, const std::vector<ByteStream>& InComponentDatas = std::vector<ByteStream>{} );
	void Reset( std::vector<EntityOwnedComponent>& OutOwnedComponents );

	//Component testing
	bool Has( const ComponentTypeID& TypeID ) const;
	template<typename TTData>
	bool Has( const TComponentInfo<TTData>& ComponentInfo ) const { return Has( ComponentInfo.GetID() ); }

	//Component retrieval
	ptr_t Get( const ComponentTypeID& TypeID ) const;
	template<typename TTData>
	TTData* Get( const TComponentInfo<TTData>& ComponentInfo ) { return static_cast<TTData*>( Get( ComponentInfo.GetID() ) ); }

	//Debugging information
	size_t Count() const { return Owned.size(); }
	size_t Capacity() const { return 0; } //used for fixed-size entities

	friend std::ostream& operator<<( std::ostream& Stream, const Entity& Entity );

protected:
	std::vector<EntityOwnedComponent> Owned;

	//Component manipulation
	ptr_t Add( const ComponentTypeID& TypeID, ComponentManager* Manager );
	template<typename TTData>
	TTData* Add( const TComponentInfo<TTData>& ComponentInfo ) { return Add( ComponentInfo.GetID(), ComponentInfo.GetManager() ); }

	void Del( const ComponentTypeID& TypeID, ComponentManager* Manager );
	template<typename TTData>
	void Del( const TComponentInfo<TTData>& ComponentInfo ) { return Del( ComponentInfo.GetID(), ComponentInfo.GetManager() ); }
};
