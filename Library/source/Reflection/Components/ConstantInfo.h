#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct TypeInfo;

	enum FConstantFlags : uint8_t {
		None = 0,
		Hidden = 1 << 0,
	};

	/** Info that describes a constant value */
	struct ConstantInfo {
		ConstantInfo() = delete;
		ConstantInfo(std::string_view InName, TypeInfo const* InType, std::string_view InDescription, FConstantFlags InFlags)
		: Name(InName)
		, NameHash(InName)
		, Type(InType)
		, Description(InDescription)
		, Flags(InFlags)
		{}
		virtual ~ConstantInfo() = default;

		std::string Name;
		Hash32 NameHash = Hash32{};
		TypeInfo const* Type = nullptr;

		std::string Description;
		FConstantFlags Flags = FConstantFlags::None;

		virtual void const* GetValuePointer(void const* Instance) const = 0;
	};

	/** Info that describes a constant value that is global */
	template<typename ValueType>
	struct TStaticConstantInfo : public ConstantInfo {
		TStaticConstantInfo() = delete;
		TStaticConstantInfo(std::string_view InName, std::string_view InDescription, ValueType* InStaticPointer)
		: ConstantInfo(InName, TypeResolver<typename std::decay<ValueType>::type>::Get(), InDescription, FConstantFlags::None)
		, StaticPointer(InStaticPointer)
		{}

		ValueType const* StaticPointer = nullptr;

		void const* GetValuePointer(void const* Instance) const final { return StaticPointer; }
	};

	/** Info that describes a constant value that is contained within a struct instance */
	template<typename StructType, typename ValueType>
	struct TMemberConstantInfo : public ConstantInfo {
		TMemberConstantInfo() = delete;
		TMemberConstantInfo(std::string_view InName, std::string_view InDescription, ValueType const StructType::* InMemberPointer)
		: ConstantInfo(InName, TypeResolver<typename std::decay<ValueType>::type>::Get(), InDescription, FConstantFlags::None)
		, MemberPointer(InMemberPointer)
		{}

		ValueType const StructType::* MemberPointer = nullptr;

		void const* GetValuePointer(void const* Instance) const final { return &(((StructType const*)Instance)->*MemberPointer); }
	};
}
