#pragma once
#include <cassert>
#include <functional>
#include <algorithm>
#include <array>
#include <vector>
#include <bitset>
#include "Engine/Context.h"
#include "EntityFramework/Types.h"

struct Entity;

/** Interface for a class which can manage the lifecycle and assignment of components */
struct ComponentManager {
public:
	virtual ~ComponentManager() {}

	/// Manager Lifetime

	/** Startup the component manager, returns true on success */
	virtual bool Startup(CTX_ARG) { return true; }
	/** Shutdown the component manager, returns true on success */
	virtual bool Shutdown(CTX_ARG) { return true; }

	/// Generic Component Lifetime

	/** Set up an entity that has just been assigned a component */
	virtual void Setup(Entity const& newEntity, ptr_t newComponent) const {}

	/** Retain an instance of a component to be used by a specific entity, returning a pointer to  it */
	virtual ptr_t Retain() = 0;
	/** Release a component that was previously retained */
	virtual void Release(ptr_t) = 0;

	/// Generic Component Manipulation

	/** Serialize the state of a component to a byte stream */
	virtual void Save(cptr_t, ByteStream&) = 0;
	/** Deserialize the state of a component from a byte stream */
	virtual void Load(ptr_t, ByteStream const&) = 0;
	/** Duplicate a component's state */
	virtual void Copy(cptr_t, ptr_t) = 0;
	/** Revert a component back to a default state */
	virtual void Wipe(ptr_t) = 0;

	/// Manager Reporting

	/** Number of component instances that this manager has created */
	virtual size_t CountTotal() const = 0;
	/** Number of component instances that have not been assigned to an entity */
	virtual size_t CountFree() const = 0;
	/** Number of component instances that have been assigned to an entity */
	virtual size_t CountUsed() const = 0;
};
