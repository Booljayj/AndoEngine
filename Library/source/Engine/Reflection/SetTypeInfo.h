#pragma once
#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	struct SetTypeInfo : public TypeInfo {
		using TypeInfo::TypeInfo;
		static constexpr ETypeClassification Classification = ETypeClassification::Set;

		/** The type of the values in the set */
		TypeInfo const* valueType = nullptr;

		virtual ~SetTypeInfo() = default;

		/** Get the number of values that are in the set */
		virtual size_t GetCount(void const* instance) const = 0;

		/** Get a vector of all the values in the set */
		virtual void GetValues(void const* instance, std::vector<void const*>& outValues) const = 0;

		/** Returns true if an element with an equal value is contained in the set */
		virtual bool Contains(void const* instance, void const* value) const = 0;

		/** Remove all values from the set */
		virtual void Clear(void* instance) const = 0;
		/** Adds the value to the set */
		virtual bool Add(void* instance, void const* value) const = 0;
		/** Remove the value from the set */
		virtual bool Remove(void* instance, void const* value) const = 0;
	};

	//============================================================
	// Templates

	template<typename SetType, typename ValueType>
	struct TSetTypeInfo : public ImplementedTypeInfo<SetType, SetTypeInfo> {
		using ImplementedTypeInfo<SetType, SetTypeInfo>::Cast;
		using SetTypeInfo::valueType;

		TSetTypeInfo(std::string_view inName) : ImplementedTypeInfo<SetType, SetTypeInfo>(Reflect<SetType>::ID, inName) {
			valueType = Reflect<ValueType>::Get();
		}

		static constexpr ValueType const& CastValue(void const* value) { return *static_cast<ValueType const*>(value); }

		virtual size_t GetCount(void const* instance) const final { return Cast(instance).size(); }

		virtual void GetValues(void const* instance, std::vector<void const*>& outValues) const final {
			outValues.clear();
			for (ValueType const& value : Cast(instance)) {
				outValues.push_back(&value);
			}
		}

		virtual bool Contains(void const* instance, void const* value) const final {
			return Cast(instance).find(CastValue(value)) != Cast(instance).cend();
		}

		virtual void Clear(void* instance) const final { Cast(instance).clear(); }
		virtual bool Add(void* instance, void const* value) const final { return Cast(instance).insert(CastValue(value)).second; }
		virtual bool Remove(void* instance, void const* value) const final { return Cast(instance).erase(CastValue(value)) > 0; }

		TYPEINFO_BUILDER_METHODS(SetType)
	};
}

TYPEINFO_REFLECT(Set);

//============================================================
// Standard set reflection

#define L_REFLECT_SET(SetTemplate, DescriptionString)\
template<typename ValueType>\
struct Reflect<SetTemplate<ValueType>> {\
	using ThisTypeInfo = ::Reflection::TSetTypeInfo<SetTemplate<ValueType>, ValueType>;\
	static ThisTypeInfo const info;\
	static ::Reflection::SetTypeInfo const* Get() { return &info; }\
	static constexpr Hash128 ID = Hash128{ #SetTemplate } + Reflect<ValueType>::ID;\
};\
template<typename ValueType>\
typename Reflect<SetTemplate<ValueType>>::ThisTypeInfo const Reflect<SetTemplate<ValueType>>::info = Reflect<SetTemplate<ValueType>>::ThisTypeInfo{ #SetTemplate }\
	.Description(DescriptionString)


L_REFLECT_SET(std::set, "ordered set");
L_REFLECT_SET(std::unordered_set, "unordered set");

#undef L_REFLECT_SET
