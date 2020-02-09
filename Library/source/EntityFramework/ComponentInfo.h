#pragma once
#include <iostream>
#include "Engine/Logging/LogCategory.h"
#include "EntityFramework/Types.h"
#include "EntityFramework/ComponentManager.h"

DECLARE_LOG_CATEGORY(Component);

#define CREATE_COMPONENT(ID, Name, Type, ManagerInit)\
auto Name##manager = ManagerInit; TComponentInfo<Type> Name{ID, #Name, &Name##manager}

/** Represents a component that can be owned by an entity */
struct ComponentInfo {
public:
	virtual ~ComponentInfo() {}

	/** Standard comparison predicate. Sorts ascending by id. */
	static bool Compare(ComponentInfo const* a, ComponentInfo const* b);

	ComponentTypeID GetID() const { return id; }
	std::string_view GetName() const { return name; }
	ComponentManager* GetManager() const { return manager; }

protected:
	/** Hidden explicit construction. Use the derived template to create instances of ComponentInfo. */
	ComponentInfo(ComponentTypeID const& inID, std::string_view inName, ComponentManager* inManager)
		: id(inID), name(inName), manager(inManager)
	{}

	/** The unique id of this component. Used to identify a component, so this should never change once it is used */
	ComponentTypeID id;
	/** The human-readable name of this component. Used in debugging and some types of serialization */
	std::string_view name;
	/** The manager which creates and collates components of this type */
	ComponentManager* manager;
};

/** Template used to provide type information to a ComponentInfo in template functions. */
template<typename DataType_>
struct TComponentInfo : public ComponentInfo {
public:
	TComponentInfo(ComponentTypeID inID, char const* inName, ComponentManager* inManager)
		: ComponentInfo(inID, inName, inManager)
	{}
	virtual ~TComponentInfo() override {}

	/** The type of the component */
	using DataType = DataType_;
};
