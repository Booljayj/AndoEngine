#pragma once
#include "EntityFramework/EntityInternals.h"
#include "EntityFramework/EntityTypes.h"

/**
 * A handle to an Entity.
 * An Entity is an identifiable object in the engine. It can own components, which hold data.
 * A handle can be used to manipulate and query a specific entity. It is the primary means of doing so.
 */
struct EntityHandle {
public:
	EntityHandle() = delete;
	EntityHandle(const EntityHandle&) = default;
	EntityHandle(entt::registry& inRegistry, entt::entity inID) : registry(&inRegistry), id(inID) {}

	inline bool IsNull() const { return registry->valid(id); }

	/** Returns true if this entity has all the component types */
	template<typename... T>
	inline bool Has() const { return registry->has<T...>(id); }
	/** Returns the component types from this entity. Undefined if the entity does not own the component types. */
	template<typename... T>
	inline decltype(auto) Get() const { return registry->get<T...>(id); }

	/** Add a component type to this entity. If the entity already has a component of the same type, it will be replaced */
	template<typename T, typename... ArgTypes>
	inline T& Add(ArgTypes... args) { return registry->emplace_or_replace<T>(id, std::forward<ArgTypes>(args)...); }
	/** Remove the component types from this entity */
	template<typename... T>
	inline void Rem() { return registry->remove<T...>(id); }

private:
	entt::registry* registry;
	entt::entity id;

	friend struct EntityConstHandle;
};

/** Similar to an EntityHandle, but can only be used to query information about a specific entity. */
struct EntityConstHandle {
public:
	EntityConstHandle() = delete;
	EntityConstHandle(const EntityConstHandle&) = default;
	EntityConstHandle(entt::registry const& inRegistry, entt::entity inID) : registry(&inRegistry), id(inID) {}
	EntityConstHandle(const EntityHandle& other) : registry(other.registry), id(other.id) {}

	inline EntityConstHandle& operator=(const EntityHandle& other) { registry = other.registry; id = other.id; return *this; }

	inline bool IsNull() const { return registry->valid(id); }

	/** Returns true if this entity has all the component types */
	template<typename... T>
	inline bool Has() const { return registry->has<T...>(id); }
	/** Returns the component types from this entity. Undefined if the entity does not own the component types. */
	template<typename... T>
	inline decltype(auto) Get() const { return registry->get<std::add_const<T>...>(id); }

private:
	entt::registry const* registry;
	entt::entity id;
};
