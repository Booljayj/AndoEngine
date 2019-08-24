#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct TypeInfo;

	/** Flags to describe aspects of a particular type */
	enum class FVariableFlags : uint8_t {
		None = 0,
		Hidden = 1 << 1,
	};

	/** Info that describes a variable value */
	struct VariableInfo {
		VariableInfo() = delete;
		VariableInfo(std::string_view InName, TypeInfo const* InType, std::string_view InDescription, FVariableFlags InFlags)
		: Name(InName)
		, NameHash(InName)
		, Type(InType)
		, Description(InDescription)
		, Flags(InFlags)
		{}
		virtual ~VariableInfo() = default;

		std::string Name;
		Hash32 NameHash = Hash32{};
		TypeInfo const* Type = nullptr;

		std::string Description;
		FVariableFlags Flags = FVariableFlags::None;

		virtual void const* GetImmutableValuePointer(void const* Instance) const = 0;
		virtual void* GetMutableValuePointer(void* Instance) const = 0;
	};

	template<typename ValueType>
	struct TStaticVariableInfo : public VariableInfo {
		TStaticVariableInfo() = delete;
		TStaticVariableInfo(std::string_view InName, std::string_view InDescription, ValueType* InStaticPointer)
		: VariableInfo(InName, TypeResolver<typename std::decay<ValueType>::type>::Get(), InDescription, FVariableFlags::None)
		, StaticPointer(InStaticPointer)
		{}

		ValueType* StaticPointer = nullptr;

		virtual void const* GetImmutableValuePointer( void const* Instance ) const final { return StaticPointer; }
		virtual void* GetMutableValuePointer( void* Instance ) const final { return StaticPointer; }
	};

	/** Info that describes a variable within a struct */
	template<typename StructType, typename ValueType>
	struct TMemberVariableInfo : VariableInfo {
		TMemberVariableInfo() = delete;
		TMemberVariableInfo(std::string_view InName, std::string_view InDescription, ValueType StructType::* InMemberPointer)
		: VariableInfo(InName, TypeResolver<typename std::decay<ValueType>::type>::Get(), InDescription, FVariableFlags::None)
		, MemberPointer(InMemberPointer)
		{}

		ValueType StructType::* MemberPointer = nullptr;

		void const* GetImmutableValuePointer(void const* Instance) const final { return &(((StructType const*)Instance)->*MemberPointer); }
		void* GetMutableValuePointer(void* Instance) const final { return &(((StructType*)Instance)->*MemberPointer); }
	};
}

