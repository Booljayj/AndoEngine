#pragma once
#include <iostream>
#include <vector>
#include <deque>
#include <unordered_map>
#include "EntityFramework/Types.h"
#include "EntityFramework/Entity.h"
#include "EntityFramework/ComponentInfo.h"

constexpr EntityID Entity_Null = 0;
constexpr EntityID Entity_Root = 1;

namespace S
{
	struct EntitySystem
	{
		EntitySystem( const std::vector<ComponentInfo*>& ComponentInfos );

		bool Initialize();
		bool Deinitialize();

		// Entity creation
		void Create( const EntityID& NewID );
		void Create( const EntityID& NewID, const std::vector<ComponentInfo*>& ComponentInfos, const std::vector<ByteStream>& ComponentDatas = {} );
		void Create( const EntityID& NewID, const std::vector<ComponentTypeID>& ComponentTypeIDs, const std::vector<ByteStream>& ComponentDatas = {} );

		// Entity destruction
		bool Destroy( Entity* );
		bool Destroy( const EntityID& ID );

		// Entity queries
		bool Exists( const EntityID& ID ) const noexcept;
		Entity* Find( const EntityID& ID ) const noexcept;
		Entity& Get( const EntityID& ID ) const;

		void Flush();

		friend inline std::ostream& operator <<( std::ostream& Stream, const EntitySystem& ECS )
		{
			Stream << "[EntitySystem]: {" << std::endl;
			Stream << "\tComponents: " << ECS.ComponentInfoMap.size() << std::endl;
			for( auto& ComponentInfoPair : ECS.ComponentInfoMap )
			{
				Stream << "\t" << *ComponentInfoPair.second;
			}
			Stream << "\tEntities: " << ECS.EntityIDs.size() << std::endl;
			for( size_t Index = 0; Index < ECS.EntityIDs.size(); ++Index )
			{
				Stream << "\t" << ECS.EntityIDs[Index] << ": " << ECS.Entities[Index] << std::endl;
			}
			return Stream << "}" << std::endl;
		}

	protected:
		std::unordered_map<ComponentTypeID, ComponentInfo*> ComponentInfoMap;
		std::vector<ComponentInfo*> ComponentInfoBuffer;
		std::vector<EntityOwnedComponent> ReclaimedComponentBuffer;

		// TODO: break entity groups into 'catalogues'. Each catalogue has poly methods for Exists, Find, and Create.
		// Primary catalogue is gameplay one, which holds runtime entities. Other catalogues can hold asset entities which may be loaded from disk.
		// IDs are only unique to each catalogue, not globally.
		// NOTE: 2 different kinds of entities exist: runtime and asset. Asset can be further subdivided, but have the same behavior.
		// Breaking into two different groups and handling separately makes more sense, as Asset entities have async loading.
		// NOTE2: Catalogue needs an index and settings file. These should be in a common directory, project root, but can specify a subfolder from which all paths derive.
		// Multiple indexes can be supplied to override asset paths and add new entities (modding).
		std::vector<Entity> Entities;
		std::vector<EntityID> EntityIDs;

		std::vector<EntityID> Created;
		std::vector<EntityID> Destroyed;

		Entity& InsertNew( const EntityID& NewID );
		const std::vector<ComponentInfo*>& GetComponentInfos( const std::vector<ComponentTypeID>& ComponentTypeIDs );
		size_t FindPositionByEntityID( const EntityID& ID ) const noexcept;
		size_t FindPositionByEntity( const Entity& EntityRef ) const noexcept;
		void ReleaseReclaimedComponents();
	};
}
