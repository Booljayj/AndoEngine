#include "Engine/Context.h"
#include "Engine/LinearContainers.h"
#include "EntityFramework/EntityHandle.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

/** A generic interface for manipulating a specific component type on any entity */
struct IComponent {
	virtual bool Has(const EntityHandle& handle) const = 0;
	virtual void Add(const EntityHandle& handle) const = 0;
	virtual void Rem(const EntityHandle& handle) const = 0;
	virtual void* Get(const EntityHandle& handle) const = 0;
};

/**
 * The component registry keeps track of a set of registered components using their TypeInfo, and can provide interfaces
 * and information to manipulate those components on entities. This is intended to be an editor resource.
 */
struct ComponentRegistry {
private:
	template<typename Type>
	struct Component : public IComponent {
		bool Has(const EntityHandle& handle) const final { return handle.Has<Type>(); };
		void Add(const EntityHandle& handle) const final { return handle.Add<Type>(); };
		void Rem(const EntityHandle& handle) const final { return handle.Rem<Type>(); };
		void* Get(const EntityHandle& handle) const final { return &handle.Get<Type>(); };
	};

	using RegistryType = std::unordered_map<Reflection::TypeInfo const*, std::unique_ptr<const IComponent>>;
	using EntryType = RegistryType::value_type;

	RegistryType registry;

public:
	/** Register a type of component that can be queried from this registry. */
	template<typename Type>
	ComponentRegistry& Register() {
		registry.insert(
			EntryType{ Reflection::TypeResolver<Type>::Get(), std::make_unique<const Component<Type>>() }
		);
		return *this;
	}

	/** Get all component types that have been registered */
	l_vector<Reflection::TypeInfo const*> GetTypes(CTX_ARG);

	/** Find a registered component with the given type */
	IComponent const* Find(Reflection::TypeInfo const* type) const;

	/** Get information about all the registered components on an entity. Returns the TypeInfo and a pointer to each component. */
	l_vector<std::tuple<Reflection::TypeInfo const*, void*>> GetComponents(CTX_ARG, const EntityHandle& handle) const;
};
