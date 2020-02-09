#pragma once
#include <vector>
#include <tuple>
#include "Engine/Logging/LogCategory.h"
#include "Engine/LinearContainers.h"
#include "EntityFramework/Types.h"
#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/ComponentManager.h"

DECLARE_LOG_CATEGORY( Entity );

struct EntityOwnedComponent {
	ptr_t compPtr;
	ComponentTypeID typeID;

	bool operator==( const ComponentTypeID& otherTypeID ) const { return typeID == otherTypeID; }
	bool operator<( const ComponentTypeID& otherTypeID ) const { return typeID < otherTypeID; }
	bool operator<( const EntityOwnedComponent& other ) const { return typeID < other.typeID; }
};

/** An Entity is an identifiable object in the game. It can own components, which define different sets of data */
struct Entity {
public:
	Entity() = default;
	Entity(Entity&& other) = default;
	Entity(const Entity& other) = delete;

	Entity& operator=(Entity&& other) = default;
	Entity& operator=(const Entity& other) = delete;

	bool operator==(const Entity& other) const { return owned.data() == other.owned.data(); }

	/// Entity creation

	void Reserve(size_t componentCount);
	void Add(ComponentTypeID typeID, void* component);
	void Reset(std::vector<EntityOwnedComponent>& outComponents);

	/// Component manipulation

	/** Returns true if this entity contains a component of the specified type */
	bool Has(ComponentTypeID typeID) const;
	/** Returns true if this entity contains all of the component types in the vector */
	bool HasAll(ComponentTypeID const* typeIDs, size_t count) const;
	template<typename AllocatorType>
	bool HasAll(std::vector<ComponentTypeID, AllocatorType> const& typeIDs) const { return HasAll(typeIDs.data(), typeIDs.size()); }

	/** Returns a pointer to a component this entity owns of the specified type */
	ptr_t Get(ComponentTypeID typeID) const;
	template<typename DataType>
	DataType* Get(TComponentInfo<DataType> const& ComponentInfo) const { return static_cast<DataType*>(Get(ComponentInfo.GetID())); }

	//Debugging information
	size_t Size() const { return owned.size(); }
	size_t Capacity() const { return owned.capacity(); } //used for fixed-size entities

protected:
	std::vector<EntityOwnedComponent> owned;
};
