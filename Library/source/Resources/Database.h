#pragma once
#include "Engine/Events.h"
#include "Engine/StandardTypes.h"
#include "Engine/TupleUtility.h"
#include "Resources/Cache.h"

namespace Resources {
	/**
	 * Databases keep track of a collection of resources, and have utility methods to create, load, and find them.
	 * Some resources are predefined and will alawys have caches with inline accessors. Others are created
	 * on-demand and accessed in O(1) time.
	 */
	template<typename... ResourceTypes>
	struct Database {
	public:
		template<typename ResourceType>
		Cache<ResourceType>& GetCache() {
			using Tuple = std::tuple<ResourceTypes...>;
			constexpr size_t TypeIndex = TupleUtility::Index<ResourceType, Tuple>();
			if constexpr (TypeIndex < std::tuple_size_v<Tuple>) {
				return std::get<TypeIndex>(defined_caches);

			} else {
				using CacheType = Cache<ResourceType>;
				size_t key = GetKey<ResourceType>();
				CacheMap::const_iterator const iter = generic_caches.find(key);
				if (iter != generic_caches.end()) return static_cast<CacheType*>(iter->second);
				else return static_cast<CacheType*>(generic_caches.emplace(std::make_pair(key, new CacheType())).first);
			}
		}

	protected:
		using CacheTuple = std::tuple<Cache<ResourceTypes>...>;
		using CacheMap = std::unordered_map<size_t, ICache*>;

		CacheTuple defined_caches;
		CacheMap generic_caches;

		/** Generates a runtime unique key for a type */
		template<typename ResourceType>
		static size_t GetKey() {
			static const uint8_t key = 0b01010101;
			return static_cast<size_t>(reinterpret_cast<uintptr_t>(&key));
		}
	};
}
