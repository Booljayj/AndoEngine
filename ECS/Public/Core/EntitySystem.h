/*
 *  ECS.hpp
 *  ECS
 *
 *  Created by Justin Bool on 3/23/17.
 *  Copyright © 2017 Justin Bool. All rights reserved.
 *
 */

#pragma once

#include <iostream>
#include <vector>
#include <deque>
#include <unordered_map>
using namespace std;

#include "General.h"
#include "OwnedComponent.h"
#include "Entity.h"
#include "CompInfo.h"

/* The classes below are exported */
#pragma GCC visibility push(default)

constexpr EntityID Entity_Null = 0;
constexpr EntityID Entity_Root = 1;

namespace S
{
	struct EntitySystem
	{
		EntitySystem( const vector<CompInfo*>& ComponentInfos );

		bool Initialize();
		bool Deinitialize();

		// Entity creation
		void Create( const EntityID& NewID );
		void Create( const EntityID& NewID, const vector<CompInfo*>& ComponentInfos, const vector<ByteStream>& ComponentDatas = {} );
		void Create( const EntityID& NewID, const vector<CompTypeID>& ComponentTypeIDs, const vector<ByteStream>& ComponentDatas = {} );

		// Entity destruction
		bool Destroy( Entity* );
		bool Destroy( const EntityID& ID );

		// Entity queries
		bool Exists( const EntityID& ID ) const noexcept;
		Entity* Find( const EntityID& ID ) const noexcept;
		Entity& Get( const EntityID& ID ) const;

		void Flush();

		template< typename TStream >
		friend TStream& operator <<( TStream& Stream, const EntitySystem& ECS )
		{
			Stream << "[EntitySystem:" << endl;
			Stream << "\tComponents: (" << ECS.ComponentInfoMap.size() << ")" << endl;
			for( auto& ComponentInfoPair : ECS.ComponentInfoMap )
			{
				Stream << "\t" << *ComponentInfoPair.second;
			}
			Stream << "\tEntities: (" << ECS.EntityIDs.size() << ")" << endl;
			for( size_t Index = 0; Index < ECS.EntityIDs.size(); ++Index )
			{
				Stream << "\t" << ECS.EntityIDs[Index] << ": " << ECS.Entities[Index] << endl;
			}
			return Stream << "]" << endl;
		}

	protected:
		unordered_map<CompTypeID, CompInfo*> ComponentInfoMap;
		vector<CompInfo*> ComponentInfoBuffer;
		vector<OwnedComponent> ReclaimedComponentBuffer;

		// TODO: break entity groups into 'catalogues'. Each catalogue has poly methods for Exists, Find, and Create.
		// Primary catalogue is gameplay one, which holds runtime entities. Other catalogues can hold asset entities which may be loaded from disk.
		// IDs are only unique to each catalogue, not globally.
		// NOTE: 2 different kinds of entities exist: runtime and asset. Asset can be further subdivided, but have the same behavior.
		// Breaking into two different groups and handling separately makes more sense, as Asset entities have async loading.
		// NOTE2: Catalogue needs an index and settings file. These should be in a common directory, project root, but can specify a subfolder from which all paths derive.
		// Multiple indexes can be supplied to override asset paths and add new entities (modding).
		vector<Entity> Entities;
		vector<EntityID> EntityIDs;

		vector<EntityID> Created;
		vector<EntityID> Destroyed;

		Entity& InsertNew( const EntityID& NewID );
		const vector<CompInfo*>& GetComponentInfos( const vector<CompTypeID>& ComponentTypeIDs );
		size_t FindPositionByEntityID( const EntityID& ID ) const noexcept;
		size_t FindPositionByEntity( const Entity& EntityRef ) const noexcept;
		void ReleaseReclaimedComponents();
	};
}

#pragma GCC visibility pop
