#pragma once
#include <algorithm>
#include <array>
#include <tuple>
#include <vector>
#include "EntityFramework/ComponentInfo.h"
#include "EntityFramework/Entity.h"

/** handle used to quickly get a specific component type from an array of components using its local index. */
template<typename ComponentType>
struct TComponentHandle {
private:
	size_t componentIndex;

public:
	TComponentHandle() = default;
	TComponentHandle(size_t inComponentIndex) : componentIndex(inComponentIndex) {}
	TComponentHandle<ComponentType>& operator=(TComponentHandle<ComponentType> const& other) = default;

	inline size_t GetIndex() const { return componentIndex; }
};

/** Base for entity filters, used by the EntityCollection to create and update a filter */
struct EntityFilterBase {
	virtual ~EntityFilterBase() = default;
	virtual void Reserve(size_t capacity) = 0;
	virtual bool Add(EntityID const& id, Entity const& entity) = 0;
	virtual bool Remove(EntityID const& id) = 0;
};

/** Entity filter that holds a collection of matching entities. It is managed by the EntityCollection that created it */
template<size_t FilterSize>
struct EntityFilter : EntityFilterBase {
	/** A match collected by the entity filter */
	struct FilterMatch {
		EntityID id;
		std::array<void*, FilterSize> components;

		template<typename ComponentType>
		inline ComponentType* Get(TComponentHandle<ComponentType> const& handle) const { return static_cast<ComponentType*>(components[handle.GetIndex()]); }
	};

private:
	std::array<ComponentTypeID, FilterSize> typeIDs;
	std::vector<FilterMatch> matches;

public:
	EntityFilter(ComponentInfo const* (&infos)[FilterSize]) {
		for (size_t index = 0; index < FilterSize; ++index) {
			typeIDs[index] = infos[index]->GetID();
		}
		std::sort(typeIDs.begin(), typeIDs.end());
	}
	virtual ~EntityFilter() = default;

	void Reserve(size_t capacity) override { matches.reserve(capacity); }

	bool Add(EntityID const& id, Entity const& entity) override {
		bool const hasAllComponents = entity.HasAll(typeIDs.begin(), typeIDs.size());
		if (hasAllComponents) {
			FilterMatch newMatch;
			newMatch.id = id;
			for (size_t index = 0; index < FilterSize; ++index) {
				newMatch.components[index] = entity.Get(typeIDs[index]);
			}
			matches.push_back(newMatch);
			return true;
		}
		return false;
	}

	bool Remove(EntityID const& id) override {
		size_t removedCount = 0;
		for (typename std::vector<FilterMatch>::reverse_iterator iter = matches.rbegin(); iter != matches.rend(); ++iter) {
			if (iter->id == id) {
				std::swap(*iter, matches.back());
				matches.pop_back();
				++removedCount;
			}
		}
		return removedCount > 0;
	}

	template<typename ComponentType>
	TComponentHandle<ComponentType> GetMatchComponentHandle(TComponentInfo<ComponentType> const* info) const {
		size_t const index = std::distance(typeIDs.begin(), std::find(typeIDs.begin(), typeIDs.end(), info->GetID()));
		return TComponentHandle<ComponentType>{index};
	}

	typename std::vector<FilterMatch>::const_iterator begin() const { return matches.begin(); }
	typename std::vector<FilterMatch>::const_iterator end() const { return matches.end(); }
};
