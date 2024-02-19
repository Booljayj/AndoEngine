#pragma once
#include "Engine/Flags.h"
#include "Engine/Hash.h"
#include "Engine/StandardTypes.h"
#include "Engine/TypeTraits.h"
#include "Engine/Reflection/TypeInfo.h"

namespace Reflection {
	struct TypeInfo;

	/** flags to describe aspects of a particular variable */
	enum class EVariableFlags : uint32_t {
		/** The variable cannot be modified */
		Const,
		/** The variable is global, and does not belong to a specific instance. May be accessed with a null instance pointer. */
		Static,
		/** The variable should not be shown when displaying variables to a user */
		Hidden,
		/** The variable should not be serialized when serializing the containing struct */
		NonSerialized,
		/** The variable is deprecated. Existing values will be loaded during serialization, but new values will not be saved. Using the variable should produce a warning. */
		Deprecated,
	};
	using FVariableFlags = TFlags<EVariableFlags>;

	/** Info that describes a variable value */
	struct VariableInfo {
	public:
		Hash32 id;
		TypeInfo const* type = nullptr;
		std::string_view name;
		std::string_view description;
		FVariableFlags flags;

		VariableInfo() = default;
		VariableInfo(VariableInfo const&) = default;

		/** Construct variable info for a static variable */
		template<typename ValueType>
		VariableInfo(ValueType* pointer, std::string_view inName, std::string_view inDescription, FVariableFlags inFlags)
		: id(inName), type(&Reflect<std::decay_t<ValueType>>::Get()), name(inName), description(inDescription), flags(inFlags + EVariableFlags::Static)
		{
			using PointerType = decltype(pointer);
			CastUntypedStorage<PointerType>(storage) = pointer;
			immutableGetter = [](StorageType const& storage, void const* instance) -> void const* { return CastUntypedStorage<PointerType>(storage); };
			mutableGetter = [](StorageType const& storage, void* instance) -> void* { return CastUntypedStorage<PointerType>(storage); };
		}
		/** Construct variable info for a const static variable */
		template<typename ValueType>
		VariableInfo(const ValueType* pointer, std::string_view inName, std::string_view inDescription, FVariableFlags inFlags)
		: id(inName), type(&Reflect<std::decay_t<ValueType>>::Get()), name(inName), description(inDescription), flags(inFlags + EVariableFlags::Const + EVariableFlags::Static)
		{
			using PointerType = decltype(pointer);
			CastUntypedStorage<PointerType>(storage) = pointer;
			immutableGetter = [](StorageType const& storage, void const* instance) -> void const* { return CastUntypedStorage<PointerType>(storage); };
			mutableGetter = [](StorageType const& storage, void* instance) -> void* { return nullptr; };
		}

		/** Construct variable info for a member variable */
		template<typename ClassType, typename ValueType>
		VariableInfo(ValueType ClassType::* pointer, std::string_view inName, std::string_view inDescription, FVariableFlags inFlags)
		: id(inName), type(&Reflect<std::decay_t<ValueType>>::Get()), name(inName), description(inDescription), flags(inFlags)
		{
			using PointerType = decltype(pointer);
			CastUntypedStorage<PointerType>(storage) = pointer;
			immutableGetter = [](StorageType const& storage, void const* instance) -> void const* {
				PointerType pointer = CastUntypedStorage<PointerType>(storage);
				return &(static_cast<ClassType const*>(instance)->*pointer);
			};
			mutableGetter = [](StorageType const& storage, void* instance) -> void* {
				PointerType pointer = CastUntypedStorage<PointerType>(storage);
				return &(static_cast<ClassType*>(instance)->*pointer);
			};
		}
		/** Construct variable info for a const member variable */
		template<typename ClassType, typename ValueType>
		VariableInfo(const ValueType ClassType::* pointer, std::string_view inName, std::string_view inDescription, FVariableFlags inFlags)
		: id(inName), type(&Reflect<std::decay_t<ValueType>>::Get()), name(inName), description(inDescription), flags(inFlags + EVariableFlags::Const)
		{
			using PointerType = decltype(pointer);
			CastUntypedStorage<PointerType>(storage) = pointer;
			immutableGetter = [](StorageType const& storage, void const* instance) -> void const* {
				PointerType pointer = CastUntypedStorage<PointerType>(storage);
				return &(static_cast<ClassType const*>(instance)->*pointer);
			};
			mutableGetter = [](StorageType const& storage, void* instance) -> void* { return nullptr; };
		}

		/** Construct variable info for a variable that is accessed using an indexing operator at a specific index */
		template<typename ClassType, typename ReturnType, typename IndexType>
		VariableInfo(TTypeList<ClassType, ReturnType, IndexType>, size_t index, std::string_view inName, std::string_view inDescription, FVariableFlags inFlags)
		: id(inName, static_cast<uint32_t>(index)), type(&Reflect<std::decay_t<ReturnType>>::Get()), name(inName), description(inDescription), flags(inFlags)
		{
			CastUntypedStorage<IndexType>(storage) = static_cast<IndexType>(index);
			immutableGetter = [](StorageType const& storage, void const* instance) -> void const* {
				const IndexType index = CastUntypedStorage<IndexType>(storage);
				ClassType const& object = *(static_cast<ClassType const*>(instance));
				return &(object[index]);
			};
			mutableGetter = [](StorageType const& storage, void* instance) -> void* {
				const IndexType index = CastUntypedStorage<IndexType>(storage);
				ClassType& object = *(static_cast<ClassType*>(instance));
				return &(object[index]);
			};
		}

		inline void const* GetImmutable(void const* instance) const { return immutableGetter(storage, instance); }
		inline void* GetMutable(void* instance) const { return mutableGetter(storage, instance); }

	private:
		static constexpr size_t StorageSize = std::max(sizeof(size_t), PointerTraits::VariablePointerSize);
		static constexpr size_t StorageAlign = std::max(alignof(size_t), PointerTraits::VariablePointerAlign);
		using StorageType = std::byte[StorageSize];

		using ImmutableVariableGetterFunc = void const*(*)(StorageType const&, void const*);
		using MutableVariableGetterFunc = void*(*)(StorageType const&, void*);

		alignas(StorageAlign) StorageType storage;
		ImmutableVariableGetterFunc immutableGetter = nullptr;
		MutableVariableGetterFunc mutableGetter = nullptr;
	};
}
