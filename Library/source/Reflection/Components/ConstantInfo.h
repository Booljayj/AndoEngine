#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include "Engine/Flags.h"
#include "Engine/Hash.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct TypeInfo;

	enum EConstantFlags : uint8_t {
		Hidden,
	};
	using FConstantFlags = TFlags<EConstantFlags>;

	/** Info that describes a constant value */
	struct ConstantInfo {
		ConstantInfo() = delete;
		ConstantInfo(std::string_view inName, TypeInfo const* inType, std::string_view inDescription, FConstantFlags inFlags)
		: name(inName)
		, id(inName)
		, type(inType)
		, description(inDescription)
		, flags(inFlags)
		{}
		virtual ~ConstantInfo() = default;

		std::string name;
		Hash32 id = Hash32{};
		TypeInfo const* type = nullptr;

		std::string description;
		FConstantFlags flags = FConstantFlags::None;

		virtual void const* GetValuePointer(void const* instance) const = 0;
	};

	/** Info that describes a constant value that is global */
	template<typename ValueType>
	struct TStaticConstantInfo : public ConstantInfo {
		TStaticConstantInfo() = delete;
		TStaticConstantInfo(std::string_view inName, std::string_view inDescription, ValueType* inStaticPointer)
		: ConstantInfo(inName, TypeResolver<typename std::decay<ValueType>::type>::Get(), inDescription, FConstantFlags::None)
		, staticPointer(inStaticPointer)
		{}

		ValueType const* staticPointer = nullptr;

		void const* GetValuePointer(void const* instance) const final { return staticPointer; }
	};

	/** Info that describes a constant value that is contained within a struct instance */
	template<typename StructType, typename ValueType>
	struct TMemberConstantInfo : public ConstantInfo {
		TMemberConstantInfo() = delete;
		TMemberConstantInfo(std::string_view inName, std::string_view inDescription, ValueType const StructType::* inMemberPointer)
		: ConstantInfo(inName, TypeResolver<typename std::decay<ValueType>::type>::Get(), inDescription, FConstantFlags::None)
		, memberPointer(inMemberPointer)
		{}

		ValueType const StructType::* memberPointer = nullptr;

		void const* GetValuePointer(void const* instance) const final { return &(((StructType const*)instance)->*memberPointer); }
	};
}
