//
//  Entity.cpp
//  ECS
//
//  Created by Justin Bool on 4/9/17.
//  Copyright © 2017 Justin Bool. All rights reserved.
//

#include <vector>
#include <map>
#include <cassert>
using namespace std;

#include "EntitySystem/General.h"
#include "EntitySystem/Entity.h"

Entity::Entity()
{}

void Entity::Setup( const vector<CompInfo*>& InComponentInfos, const vector<ByteStream>& InComponentDatas /* = {} */ )
{
	Owned.reserve( InComponentInfos.size() );

	if( InComponentDatas.empty() )
	{
		for( size_t Index = 0; Index < InComponentInfos.size(); ++Index )
		{
			const auto* ComponentInfo = InComponentInfos[Index];
			Add( ComponentInfo->GetID(), ComponentInfo->GetManager() );
		}
	}
	else
	{
		assert( InComponentInfos.size() == InComponentDatas.size() );
		for( size_t Index = 0; Index < InComponentInfos.size(); ++Index )
		{
			const auto* ComponentInfo = InComponentInfos[Index];
			const auto& ComponentData = InComponentDatas[Index];
			auto* Manager = ComponentInfo->GetManager();

			raw_ptr NewOwnedComp = Add( ComponentInfo->GetID(), Manager );
			Manager->Load( NewOwnedComp, ComponentData );
		}
	}
}

void Entity::Reset( vector<OwnedComponent>& OutOwnedComponents )
{
	OutOwnedComponents = std::move( Owned );
}

bool Entity::Has( const CompTypeID& TypeID ) const
{
	return std::find( Owned.begin(), Owned.end(), TypeID ) != Owned.end();
}

raw_ptr Entity::Add( const CompTypeID& TypeID, CompManager* Manager )
{
	raw_ptr NewlyOwnedComponent = Manager->Retain();
	Owned.push_back( OwnedComponent{ TypeID, NewlyOwnedComponent } );
	return NewlyOwnedComponent;
}

void Entity::Del( const CompTypeID& TypeID, CompManager* Manager )
{
	auto FoundIter = std::find( Owned.begin(), Owned.end(), TypeID );

	if( FoundIter != Owned.end() )
	{
		Manager->Release( FoundIter->CompPtr );
		Owned.erase( FoundIter );
	}
}

raw_ptr Entity::Get( const CompTypeID& TypeID ) const
{
	auto FoundIter = std::find( Owned.begin(), Owned.end(), TypeID );

	return FoundIter != Owned.end() ? FoundIter->CompPtr : nullptr;
}
