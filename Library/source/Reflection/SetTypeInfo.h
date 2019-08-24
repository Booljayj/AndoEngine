#pragma once
#include <vector>
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct SetTypeInfo : public TypeInfo {
		static constexpr ETypeClassification CLASSIFICATION = ETypeClassification::Set;

		/** The type of the values in the set */
		TypeInfo const* ValueTypeInfo = nullptr;

		SetTypeInfo() = delete;
		SetTypeInfo(
			Hash128 InUniqueID, CompilerDefinition InDefinition,
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer,
			TypeInfo const* InValueTypeInfo
		);
		virtual ~SetTypeInfo() = default;

		/** Get the number of values that are in the set*/
		virtual size_t GetCount(void const* Instance) const = 0;

		/** Get a vector of all the values in the set */
		virtual void GetValues(void const* Instance, std::vector<void const*>& OutValues) const = 0;

		/** Returns true if an element with an equal value is contained in the set */
		virtual bool Contains(void const* Instance, void const* Value) const = 0;

		/** Remove all values from the set */
		virtual void Clear(void* Instance) const = 0;
		/** Adds the value to the set */
		virtual bool Add(void* Instance, void const* Value) const = 0;
		/** Remove the value from the set */
		virtual bool Remove(void* Instance, void const* Value) const = 0;
	};

	//============================================================
	// Templates

	template<typename ValueType, typename SetType>
	struct TSetTypeInfo : public SetTypeInfo {
		TSetTypeInfo(
			std::string_view InDescription, FTypeFlags InFlags, Serialization::ISerializer* InSerializer)
		: SetTypeInfo(
			TypeResolver<SetType>::GetID(), GetCompilerDefinition<SetType>(),
			InDescription, InFlags, InSerializer,
			TypeResolver<ValueType>::Get())
		{}

		STANDARD_TYPEINFO_METHODS(SetType)

		static constexpr ValueType const& CastValue(void const* Value) { return *static_cast<ValueType const*>(Value); }

		virtual size_t GetCount(void const* Instance) const final { return Cast(Instance).size(); }

		virtual void GetValues(void const* Instance, std::vector<void const*>& OutValues) const final {
			OutValues.clear();
			for (ValueType const& Value : Cast(Instance)) {
				OutValues.push_back(&Value);
			}
		}

		virtual bool Contains(void const* Instance, void const* Value) const final {
			return Cast(Instance).find(CastValue(Value)) != Cast(Instance).cend();
		}

		virtual void Clear(void* Instance) const final { Cast(Instance).clear(); }
		virtual bool Add(void* Instance, void const* Value) const final { return Cast(Instance).insert(CastValue(Value)).second; }
		virtual bool Remove(void* Instance, void const* Value) const final { return Cast(Instance).erase(CastValue(Value)) > 0; }
	};
}
