#pragma once
#include "Engine/Logging/LogCategory.h"
#include "Engine/TypeTraits.h"
#include "EntityFramework/EntityHandle.h"
#include "EntityFramework/EntityInternals.h"
#include "EntityFramework/EntityTypes.h"

DECLARE_LOG_CATEGORY(Entity);

/** A view that can be used to iterate through entities that share a set of components */
template<typename IncludeTypeList, typename ExcludeTypeList>
struct EntityView {};

template<typename... IncludedTypes, typename... ExcludedTypes>
struct EntityView<TTypeList<IncludedTypes...>, TTypeList<ExcludedTypes...>> {
private:
	using ViewType = entt::basic_view<entt::entity, entt::exclude_t<ExcludedTypes...>, IncludedTypes...>;
	using IteratorType = typename ViewType::iterator;
	ViewType view;

public:
	EntityView(EntityView const&) = default;
	EntityView(ViewType const& inView) : view(inView) {}

	EntityView& operator=(EntityView const&) = default;

	[[nodiscard]] IteratorType begin() const noexcept { return view.begin(); }
	[[nodiscard]] IteratorType end() const noexcept { return view.end(); }
	[[nodiscard]] size_t size() const noexcept { return view.size(); }

	inline bool Contains(EntityID id) const { return view.contains(id); }

	template<typename... ComponentTypes>
    [[nodiscard]] inline decltype(auto) Get(EntityID id) const { return view.template get<ComponentTypes...>(id); }

	template<typename ComponentType>
    [[nodiscard]] inline ComponentType* Find(EntityID id) const {
		if (view.contains(id)) return &view.template get<ComponentType>(id);
		else return nullptr;
	}
};

/** A group which controls the layout of a set of components in memory, and can be used to iterate through them. */
template<typename OwnedTypeList, typename IncludedTypeList, typename ExcludedTypeList>
struct EntityGroup {};

template<typename... OwnedTypes, typename... IncludedTypes, typename... ExcludedTypes>
struct EntityGroup<TTypeList<OwnedTypes...>, TTypeList<IncludedTypes...>, TTypeList<ExcludedTypes...>> {
private:
	using GroupType = entt::basic_group<entt::entity, entt::exclude_t<ExcludedTypes...>, entt::get_t<IncludedTypes...>, OwnedTypes...>;
	using IteratorType = typename GroupType::iterator;
	GroupType group;

public:
	EntityGroup(const EntityGroup&) = default;
	EntityGroup(const GroupType& inGroup) : group(inGroup) {}

	EntityGroup& operator=(const EntityGroup&) = default;

	[[nodiscard]] inline IteratorType begin() const noexcept { return group.begin(); }
	[[nodiscard]] inline IteratorType end() const noexcept { return group.end(); }
	[[nodiscard]] inline size_t size() const noexcept { return group.size(); }

	inline bool Contains(EntityID id) const { return group.contains(id); }

	template<typename... ComponentTypes>
    [[nodiscard]] inline decltype(auto) Get(EntityID id) const { return group.template get<ComponentTypes...>(id); }

	template<typename ComponentType>
    [[nodiscard]] inline ComponentType* Find(EntityID id) const {
		if (group.contains(id)) return &group.template get<ComponentType>(id);
		else return nullptr;
	}
};

/**
 * The registry which contains a set of entities. Entities can be created and retrieved from this registry.
 * It can also provide an object to iterate through entities that contain a specific set of components.
 */
struct EntityRegistry {
public:
	inline EntityHandle Create() { return EntityHandle{registry, registry.create()}; }
	inline void Destroy(EntityID id) { registry.destroy(id); }
	[[nodiscard]] inline EntityConstHandle Find(EntityID id) const { return EntityConstHandle{registry, id}; }
	[[nodiscard]] inline EntityHandle Find(EntityID id) { return EntityHandle{registry, id}; }

	/** Destroy all components of the given types */
	template<typename... ComponentTypes>
	inline void DestroyComponents() { registry.clear<ComponentTypes...>(); }

	/**
	 * Create a view that can be used to iterate through entities with a given set of components.
	 * @param <IncludedTypes...> Entities must have all of these component types to be part of this view
	 * @param excluded Entities must not have any of these components types to be part of this view
	 */
	template<typename... IncludedTypes, typename... ExcludedTypes>
    [[nodiscard]] inline EntityView<TTypeList<IncludedTypes...>, TTypeList<ExcludedTypes...>> GetView(TTypeList<ExcludedTypes...> excluded = {}) const {
		return registry.view<IncludedTypes...>(entt::exclude<ExcludedTypes...>);
	}

	/**
	 * Create a group that controls the layout of a set of entities in memory, and can be used to iterate through them.
	 * @param <OwnedTypes...> The component types that will be striated in memory
	 * @param included Entities must have all of these component types to be part of this group, but their memory layout will not be controlled
	 * @param excluded Entities must not have any of these component types to be part of this group
	 */
	template<typename... OwnedTypes, typename... IncludedTypes, typename... ExcludedTypes>
	inline void CreateGroup(TTypeList<IncludedTypes...> included = {}, TTypeList<ExcludedTypes...> excluded = {}) {
		(void)registry.group<OwnedTypes...>(entt::get<IncludedTypes...>, entt::exclude<ExcludedTypes...>);
	}

	/**
	 * Get a group that was previously created by a call to CreateGroup
	 * @param <OwnedTypes...> The component types that will be striated in memory
	 * @param included Entities must have all of these component types to be part of this group, but their memory layout will not be controlled
	 * @param excluded Entities must not have any of these component types to be part of this group
	 */
	template<typename... OwnedTypes, typename... IncludedTypes, typename... ExcludedTypes>
	[[nodiscard]] inline EntityGroup<TTypeList<OwnedTypes...>, TTypeList<IncludedTypes...>, TTypeList<ExcludedTypes...>> GetGroup(TTypeList<IncludedTypes...> included = {}, TTypeList<ExcludedTypes...> excluded = {}) const {
		return registry.group<OwnedTypes...>(entt::get<IncludedTypes...>, entt::exclude<ExcludedTypes...>);
	}

private:
	entt::registry registry;
};
