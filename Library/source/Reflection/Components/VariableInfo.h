#pragma once
#include "Engine/Flags.h"
#include "Engine/Hash.h"
#include "Engine/STL.h"
#include "Engine/TypeTraits.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct TypeInfo;

	/** flags to describe aspects of a particular variable */
	enum class EVariableFlags : uint32_t {
		Const,
		Static,
		Hidden,
		NonSerialized,
	};
	using FVariableFlags = TFlags<EVariableFlags>;

	/** Info that describes a variable value */
	struct VariableInfo {
	public:
		union StorageType {
			VariablePointerStorage variable;
			FunctionPointerStorage function;
		};

		using ImmutableVariableGetterFunc = void const*(*)(StorageType const&, void const*);
		using MutableVariableGetterFunc = void*(*)(StorageType const&, void*);

	private:
		StorageType storage;
		ImmutableVariableGetterFunc immutableGetter;
		MutableVariableGetterFunc mutableGetter;

	public:
		TypeInfo const* type;
		std::string_view name;
		std::string_view description;
		FVariableFlags flags;
		Hash32 id;

		VariableInfo() = delete;
		VariableInfo(VariableInfo&& other) = default;

		/** Construct variable info for a static variable */
		template<typename ValueType>
		VariableInfo(ValueType* pointer, std::string_view inName, std::string_view inDescription, FVariableFlags inFlags)
		: type(TypeResolver<std::decay_t<ValueType>>::Get()), name(inName), description(inDescription), flags(inFlags + EVariableFlags::Static), id(inName)
		{
			using PointerType = decltype(pointer);
			CastAlignedUnion<PointerType>(storage.variable) = pointer;
			immutableGetter = [](StorageType const& storage, void const* instance) -> void const* { return CastAlignedUnion<PointerType>(storage.variable); };
			mutableGetter = [](StorageType const& storage, void* instance) -> void* { return CastAlignedUnion<PointerType>(storage.variable); };
		}
		/** Construct variable info for a const static variable */
		template<typename ValueType>
		VariableInfo(const ValueType* pointer, std::string_view inName, std::string_view inDescription, FVariableFlags inFlags)
		: type(TypeResolver<std::decay_t<ValueType>>::Get()), name(inName), description(inDescription), flags(inFlags + EVariableFlags::Const + EVariableFlags::Static), id(inName)
		{
			using PointerType = decltype(pointer);
			CastAlignedUnion<PointerType>(storage.variable) = pointer;
			immutableGetter = [](StorageType const& storage, void const* instance) -> void const* { return CastAlignedUnion<PointerType>(storage.variable); };
			mutableGetter = [](StorageType const& storage, void* instance) -> void* { return nullptr; };
		}

		/** Construct variable info for a member variable */
		template<typename ClassType, typename ValueType>
		VariableInfo(ValueType ClassType::* pointer, std::string_view inName, std::string_view inDescription, FVariableFlags inFlags)
		: type(TypeResolver<std::decay_t<ValueType>>::Get()), name(inName), description(inDescription), flags(inFlags), id(inName)
		{
			using PointerType = decltype(pointer);
			CastAlignedUnion<PointerType>(static_cast<VariablePointerStorage&>(storage.variable)) = pointer;
			immutableGetter = [](StorageType const& storage, void const* instance) -> void const* {
				PointerType pointer = CastAlignedUnion<PointerType>(storage.variable);
				return &(static_cast<ClassType const*>(instance)->*pointer);
			};
			mutableGetter = [](StorageType const& storage, void* instance) -> void* {
				PointerType pointer = CastAlignedUnion<PointerType>(storage.variable);
				return &(static_cast<ClassType*>(instance)->*pointer);
			};
		}
		/** Construct variable info for a const member variable */
		template<typename ClassType, typename ValueType>
		VariableInfo(const ValueType ClassType::* pointer, std::string_view inName, std::string_view inDescription, FVariableFlags inFlags)
		: type(TypeResolver<std::decay_t<ValueType>>::Get()), name(inName), description(inDescription), flags(inFlags + EVariableFlags::Const), id(inName)
		{
			using PointerType = decltype(pointer);
			CastAlignedUnion<PointerType>(storage.variable) = pointer;
			immutableGetter = [](StorageType const& storage, void const* instance) -> void const* {
				PointerType pointer = CastAlignedUnion<PointerType>(storage.variable);
				return &(static_cast<ClassType const*>(instance)->*pointer);
			};
			mutableGetter = [](StorageType const& storage, void* instance) -> void* { return nullptr; };
		}

		/** Construct variable info for a variable that is accessed using an indexing operator at a specific index */
		template<typename ClassType, typename ReturnType>
		VariableInfo(TTypeList<ClassType, ReturnType>, size_t index, std::string_view inName, std::string_view inDescription, FVariableFlags inFlags)
		: type(TypeResolver<std::decay_t<ReturnType>>::Get()), name(inName), description(inDescription), flags(inFlags)
		{
			CastAlignedUnion<size_t>(storage.variable) = index;
			immutableGetter = [](StorageType const& storage, void const* instance) -> void const* {
				const size_t index = CastAlignedUnion<size_t>(storage.variable);
				ClassType const& object = *(static_cast<ClassType const*>(instance));
				return &(object[index]);
			};
			mutableGetter = [](StorageType const& storage, void* instance) -> void* {
				const size_t index = CastAlignedUnion<size_t>(storage.variable);
				ClassType& object = *(static_cast<ClassType*>(instance));
				return &(object[index]);
			};
		}

		/** Get a pointer to this variable on a particular instance (instance is ignored for static variables) */
		void const* GetValuePointer(void const* instance) const { return immutableGetter(storage, instance); }
		void* GetValuePointer(void* instance) const { return mutableGetter(storage, instance); }
	};
}
