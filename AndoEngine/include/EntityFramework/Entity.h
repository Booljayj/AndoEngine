// Copyright © 2017 Justin Bool. All rights reserved.

#pragma once

#include <vector>
#include <tuple>
using namespace std;

#include "EntityFrameworkTypes.h"
#include "ComponentInfo.h"
#include "ComponentManager.h"

struct EntityOwnedComponent
{
	ComponentTypeID TypeID;
	raw_ptr CompPtr;

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
	void Setup( const vector<ComponentInfo*>& InComponentInfos, const vector<ByteStream>& InComponentDatas = vector<ByteStream>{} );
	void Reset( vector<EntityOwnedComponent>& OutOwnedComponents );

	//Component testing
	bool Has( const ComponentTypeID& TypeID ) const;
	template<typename TTData>
	bool Has( const TComponentInfo<TTData>& ComponentInfo ) const { return Has( ComponentInfo.GetID() ); }

	//Component retrieval
	raw_ptr Get( const ComponentTypeID& TypeID ) const;
	template<typename TTData>
	TTData* Get( const TComponentInfo<TTData>& ComponentInfo ) { return static_cast<TTData*>( Get( ComponentInfo.GetID() ) ); }

	//Debugging information
	size_t Count() const { return Owned.size(); }
	size_t Capacity() const { return 0; } //used for fixed-size entities

	friend ostream& operator<<( ostream& Stream, const Entity& Entity );

protected:
	vector<EntityOwnedComponent> Owned;

	//Component manipulation
	raw_ptr Add( const ComponentTypeID& TypeID, ComponentManager* Manager );
	template<typename TTData>
	TTData* Add( const TComponentInfo<TTData>& ComponentInfo ) { return Add( ComponentInfo.GetID(), ComponentInfo.GetManager() ); }

	void Del( const ComponentTypeID& TypeID, ComponentManager* Manager );
	template<typename TTData>
	void Del( const TComponentInfo<TTData>& ComponentInfo ) { return Del( ComponentInfo.GetID(), ComponentInfo.GetManager() ); }
};
