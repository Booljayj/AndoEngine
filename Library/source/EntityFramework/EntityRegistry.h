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
	using ViewType = entt::basic_group<EntityRuntimeID, entt::exclude_t<ExcludedTypes...>, IncludedTypes...>;
	using IteratorType = typename ViewType::iterator;
	ViewType view;

public:
	EntityView() = default;
	EntityView(const EntityView&) = default;
	EntityView(const ViewType& inView) : view(inView) {}

	EntityView& operator=(const EntityView&) = default;

	[[nodiscard]] IteratorType begin() const noexcept { return view.begin(); }
	[[nodiscard]] IteratorType end() const noexcept { return view.end(); }
	[[nodiscard]] size_t size() const noexcept { return view.size(); }

	template<typename... ComponentTypes>
    [[nodiscard]] decltype(auto) Get(const EntityRuntimeID id) const { return view.template get<ComponentTypes...>(id); }
};

/** A group which controls the layout of a set of components in memory, and can be used to iterate through them. */
template<typename OwnedTypeList, typename IncludedTypeList, typename ExcludedTypeList>
struct EntityGroup {};

template<typename... OwnedTypes, typename... IncludedTypes, typename... ExcludedTypes>
struct EntityGroup<TTypeList<OwnedTypes...>, TTypeList<IncludedTypes...>, TTypeList<ExcludedTypes...>> {
private:
	using GroupType = entt::basic_group<EntityRuntimeID, entt::exclude_t<ExcludedTypes...>, entt::get_t<IncludedTypes...>, OwnedTypes...>;
	using IteratorType = typename GroupType::iterator;
	GroupType group;

public:
	EntityGroup() = default;
	EntityGroup(const EntityGroup&) = default;
	EntityGroup(const GroupType& inGroup) : group(inGroup) {}

	EntityGroup& operator=(const EntityGroup&) = default;

	[[nodiscard]] inline IteratorType begin() const noexcept { return group.begin(); }
	[[nodiscard]] inline IteratorType end() const noexcept { return group.end(); }
	[[nodiscard]] inline size_t size() const noexcept { return group.size(); }

	template<typename... ComponentTypes>
    [[nodiscard]] inline decltype(auto) Get(const EntityRuntimeID id) const { return group.template get<ComponentTypes...>(id); }
};

/**
 * The registry which contains a set of entities. Entities can be created and retrieved from this registry.
 * It can also provide an object to iterate through entities that contain a specific set of components.
 */
struct EntityRegistry {
public:
	inline EntityHandle Create() { return EntityHandle{registry, registry.create()}; }
	inline void Destroy(EntityRuntimeID id) { registry.destroy(entt::entity{id}); }
	[[nodiscard]] inline EntityConstHandle Find(EntityRuntimeID id) const { return EntityConstHandle{registry, entt::entity{id}}; }
	[[nodiscard]] inline EntityHandle Find(EntityRuntimeID id) { return EntityHandle{registry, entt::entity{id}}; }

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
	[[nodiscard]] inline EntityGroup<TTypeList<OwnedTypes...>, TTypeList<IncludedTypes...>, TTypeList<ExcludedTypes...>> CreateGroup(TTypeList<IncludedTypes...> included, TTypeList<ExcludedTypes...> excluded = {}) {
		registry.group<OwnedTypes...>(entt::get<IncludedTypes...>, entt::exclude<ExcludedTypes...>);
	}

private:
	entt::registry registry;
};
