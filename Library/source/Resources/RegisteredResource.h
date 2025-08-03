#pragma once
#include "Engine/Reflection/StructTypeInfo.h"
#include "Engine/Core.h"
#include "Engine/SmartPointers.h"
#include "Resources/Cache.h"

namespace Resources {
	/** Registers globally-accessible type-erased utilities for a given resource type */
	struct RegisteredResource {
		/** Type-erased utility methods for a specific resource type */
		struct Utilities {
			virtual Reflection::StructTypeInfo const& GetType() const = 0;
			virtual std::shared_ptr<Cache> CreateCache() const = 0;
		};

		template<Concepts::DerivedFromResource T>
		inline RegisteredResource(std::in_place_type_t<T> type) : id(Reflect<T>::ID) {
			auto& utilities = GetUtilities();
			if (utilities.contains(id)) throw FormatType<std::runtime_error>("Multiple registrations found for resource {}", Reflect<T>::Get().name);
			utilities[id] = std::make_unique<TUtilities<T>>();
		}

		~RegisteredResource() {
			GetUtilities().erase(id);
		}

		/** Find the utilities for a type with the given id */
		static Utilities const* FindUtilities(Hash128 id);

	private:
		template<Concepts::DerivedFromResource T>
		struct TUtilities : public Utilities {
			virtual Reflection::StructTypeInfo const& GetType() const final { return Reflect<T>::Get(); }
			virtual std::shared_ptr<Cache> CreateCache() const final { return std::make_shared<TCache<T>>(); }
		};

		static std::unordered_map<Hash128, std::unique_ptr<Utilities>>& GetUtilities();

		Hash128 id;
	};
}

/** Registers a resource type, providing type-erased utility methods for it. */
#define REGISTER_RESOURCE(NAMESPACE, TYPE) Resources::RegisteredResource Registered_ ## TYPE ## __COUNTER__ { std::in_place_type<NAMESPACE::TYPE> };
