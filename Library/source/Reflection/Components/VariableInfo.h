#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <type_traits>
#include "Engine/Flags.h"
#include "Engine/Hash.h"
#include "Reflection/TypeInfo.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct TypeInfo;

	/** flags to describe aspects of a particular type */
	enum class EVariableFlags : uint8_t {
		Hidden,
	};
	using FVariableFlags = TFlags<EVariableFlags>;

	/** Info that describes a variable value */
	struct VariableInfo {
		VariableInfo() = delete;
		VariableInfo(std::string_view inName, TypeInfo const* inType, std::string_view inDescription, FVariableFlags inFlags)
		: name(inName)
		, id(inName)
		, type(inType)
		, description(inDescription)
		, flags(inFlags)
		{}
		virtual ~VariableInfo() = default;

		std::string name;
		Hash32 id = Hash32{};
		TypeInfo const* type = nullptr;

		std::string description;
		FVariableFlags flags = FVariableFlags::None;

		virtual void const* GetImmutableValuePointer(void const* instance) const = 0;
		virtual void* GetMutableValuePointer(void* instance) const = 0;
	};

	template<typename ValueType>
	struct TStaticVariableInfo : public VariableInfo {
		TStaticVariableInfo() = delete;
		TStaticVariableInfo(std::string_view inName, std::string_view inDescription, ValueType* inStaticPointer)
		: VariableInfo(inName, TypeResolver<typename std::decay<ValueType>::type>::Get(), inDescription, FVariableFlags::None)
		, staticPointer(inStaticPointer)
		{}

		ValueType* staticPointer = nullptr;

		virtual void const* GetImmutableValuePointer( void const* instance ) const final { return staticPointer; }
		virtual void* GetMutableValuePointer( void* instance ) const final { return staticPointer; }
	};

	/** Info that describes a variable within a struct */
	template<typename StructType, typename ValueType>
	struct TMemberVariableInfo : VariableInfo {
		TMemberVariableInfo() = delete;
		TMemberVariableInfo(std::string_view inName, std::string_view inDescription, ValueType StructType::* inMemberPointer)
		: VariableInfo(inName, TypeResolver<typename std::decay<ValueType>::type>::Get(), inDescription, FVariableFlags::None)
		, memberPointer(inMemberPointer)
		{}

		ValueType StructType::* memberPointer = nullptr;

		void const* GetImmutableValuePointer(void const* instance) const final { return &(((StructType const*)instance)->*memberPointer); }
		void* GetMutableValuePointer(void* instance) const final { return &(((StructType*)instance)->*memberPointer); }
	};
}

