//
//  entity.h
//  ECS
//
//  Created by Justin Bool on 4/8/17.
//  Copyright © 2017 Justin Bool. All rights reserved.
//

#pragma once

#include <vector>
#include <tuple>
using namespace std;

#include "General.h"
#include "OwnedComponent.h"
#include "CompInfo.h"
#include "CompManager.h"

struct Entity
{
	Entity();
	Entity( Entity&& Other ) = default;
	Entity( const Entity& Other ) = delete;

	Entity& operator=( Entity&& Other ) = default;
	Entity& operator=( const Entity& Other ) = delete;

	bool operator==( const Entity& Other ) const { return Owned.data() == Other.Owned.data(); }

	//Entity creation
	void Setup( const vector<CompInfo*>& InComponentInfos, const vector<ByteStream>& InComponentDatas = vector<ByteStream>{} );
	void Reset( vector<OwnedComponent>& OutOwnedComponents );

	//Component testing
	bool Has( const CompTypeID& TypeID ) const;
	template<typename TTData>
	bool Has( const TCompInfo<TTData>& CompInfo ) const { return Has( CompInfo.GetID() ); }

	//Component manipulation
	raw_ptr Add( const CompTypeID& TypeID, CompManager* Manager );
	template<typename TTData>
	TTData* Add( const TCompInfo<TTData>& CompInfo ) { return Add( CompInfo.GetID(), CompInfo.GetManager() ); }

	void Del( const CompTypeID& TypeID, CompManager* Manager );
	template<typename TTData>
	void Del( const TCompInfo<TTData>& CompInfo ) { return Del( CompInfo.GetID(), CompInfo.GetManager() ); }

	//Component retrieval
	raw_ptr Get( const CompTypeID& TypeID ) const;
	template<typename TTData>
	TTData* Get( const TCompInfo<TTData>& CompInfo ) { return static_cast<TTData*>( Get( CompInfo.GetID() ) ); }

	//Debugging information
	size_t Count() const { return Owned.size(); }
	size_t Capacity() const { return 0; } //used for fixed-size entities

	friend inline ostream& operator<<( ostream& Stream, const Entity& Entity )
	{
		Stream << "[Entity: ";
		for( auto& OwnedComp : Entity.Owned )
		{
			Stream << OwnedComp.TypeID << ", ";
		}
		return Stream << "]";
	}

protected:
	vector<OwnedComponent> Owned;
};
