#pragma once
#include <cstdint>
#include <string_view>
#include "Engine/Flags.h"
#include "Engine/Hash.h"
#include "Engine/TypeTraits.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct TypeInfo;

	/** flags to describe aspects of a particular variable */
	enum class EVariableFlags : uint32_t {
		Const,
		Hidden,
		NonSerialized,
	};
	using FVariableFlags = TFlags<EVariableFlags>;

	/** Info that describes a variable value */
	struct VariableInfo {
	private:
		using ImmutableVariableGetterFunc = void const*(*)(VariablePointerStorage const&, void const*);
		using MutableVariableGetterFunc = void*(*)(VariablePointerStorage const&, void*);

		VariablePointerStorage storage;
		ImmutableVariableGetterFunc immutableGetter;
		MutableVariableGetterFunc mutableGetter;

	public:
		TypeInfo const* type;
		std::string_view name;
		std::string_view description;
		FVariableFlags flags;
		Hash32 id;

		VariableInfo() = delete;

		/** Construct variable info for a static variable */
		template<typename ValueType>
		VariableInfo(ValueType* pointer, std::string_view inName, std::string_view inDescription, FVariableFlags inFlags)
		: type(TypeResolver<std::decay_t<ValueType>>::Get()), name(inName), description(inDescription), flags(inFlags), id(inName)
		{
			using PointerType = decltype(pointer);
			Cast<PointerType>(storage) = pointer;
			immutableGetter = [](VariablePointerStorage const& storage, void const* instance) { return Cast<PointerType>(storage); };
			mutableGetter = [](VariablePointerStorage const& storage, void* instance) { return Cast<PointerType>(storage); };
		}
		/** Construct variable info for a const static variable */
		template<typename ValueType>
		VariableInfo(const ValueType* pointer, std::string_view inName, std::string_view inDescription, FVariableFlags inFlags)
		: type(TypeResolver<std::decay_t<ValueType>>::Get()), name(inName), description(inDescription), flags(inFlags + EVariableFlags::Const), id(inName)
		{
			using PointerType = decltype(pointer);
			Cast<PointerType>(storage) = pointer;
			immutableGetter = [](VariablePointerStorage const& storage, void const* instance) { return Cast<PointerType>(storage); };
			mutableGetter = [](VariablePointerStorage const& storage, void* instance) { return nullptr; };
		}

		/** Construct variable info for a member variable */
		template<typename ClassType, typename ValueType>
		VariableInfo(ValueType ClassType::* pointer, std::string_view inName, std::string_view inDescription, FVariableFlags inFlags)
		: type(TypeResolver<std::decay_t<ValueType>>::Get()), name(inName), description(inDescription), flags(inFlags), id(inName)
		{
			using PointerType = decltype(pointer);
			Cast<PointerType>(storage) = pointer;
			immutableGetter = [](VariablePointerStorage const& storage, void const* instance) {
				PointerType pointer = Cast<PointerType>(storage);
				return &(static_cast<ClassType const*>(instance)->pointer);
			};
			mutableGetter = [](VariablePointerStorage const& storage, void* instance) {
				PointerType pointer = Cast<PointerType>(storage);
				return &(static_cast<ClassType*>(instance)->pointer);
			};
		}
		/** Construct variable info for a const member variable */
		template<typename ClassType, typename ValueType>
		VariableInfo(const ValueType ClassType::* pointer, std::string_view inName, std::string_view inDescription, FVariableFlags inFlags)
		: type(TypeResolver<std::decay_t<ValueType>>::Get()), name(inName), description(inDescription), flags(inFlags + EVariableFlags::Const), id(inName)
		{
			using PointerType = decltype(pointer);
			Cast<PointerType>(storage) = pointer;
			immutableGetter = [](VariablePointerStorage const& storage, void const* instance) {
				PointerType pointer = Cast<PointerType>(storage);
				return &(static_cast<ClassType const*>(instance)->pointer);
			};
			mutableGetter = [](VariablePointerStorage const& storage, void* instance) { return nullptr; };
		}

		/** Get a pointer to this variable on a particular instance (instance is ignored for static variables) */
		void const* GetValuePointer(void const* instance) const { return immutableGetter(storage, instance); }
		void* GetValuePointer(void* instance) const { return mutableGetter(storage, instance); }
	};
}
