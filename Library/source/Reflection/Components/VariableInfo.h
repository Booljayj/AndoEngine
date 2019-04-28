#pragma once
#include <cstdint>
#include <string>
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
		VariableInfo( const char* InName, TypeInfo const* InType, const char* InDescription, FVariableFlags InFlags )
		: Name( InName )
		, NameHash( InName )
		, Type( InType )
		, Description( InDescription )
		, Flags( InFlags )
		{}
		virtual ~VariableInfo() {};

		std::string Name;
		Hash32 NameHash = Hash32{};
		TypeInfo const* Type = nullptr;

		std::string Description;
		FVariableFlags Flags = FVariableFlags::None;

		virtual void const* GetImmutableValuePointer( void const* Instance ) const = 0;
		virtual void* GetMutableValuePointer( void* Instance ) const = 0;
	};

	template<typename TVAR>
	struct TStaticVariableInfo : public VariableInfo {
		TStaticVariableInfo() = delete;
		TStaticVariableInfo( const char* InName, const char* InDescription, TVAR* InStaticPointer )
		: VariableInfo( InName, TypeResolver<typename std::decay<TVAR>::type>::Get(), InDescription, FVariableFlags::None )
		, StaticPointer( InStaticPointer )
		{}
		virtual ~TStaticVariableInfo() {};

		TVAR* StaticPointer;

		virtual void const* GetImmutableValuePointer( void const* Instance ) const final { return StaticPointer; }
		virtual void* GetMutableValuePointer( void* Instance ) const final { return StaticPointer; }
	};

	/** Info that describes a variable within a struct */
	template<typename TCLASS, typename TVAR>
	struct TMemberVariableInfo : VariableInfo {
		TMemberVariableInfo() = delete;
		TMemberVariableInfo( const char* InName, const char* InDescription, TVAR TCLASS::* InMemberPointer )
		: VariableInfo( InName, TypeResolver<typename std::decay<TVAR>::type>::Get(), InDescription, FVariableFlags::None )
		, MemberPointer( InMemberPointer )
		{}
		virtual ~TMemberVariableInfo() {};

		TVAR TCLASS::* MemberPointer;

		void const* GetImmutableValuePointer( void const* Instance ) const final { return &( ((TCLASS const*)Instance)->*MemberPointer ); }
		void* GetMutableValuePointer( void* Instance ) const final { return &( ((TCLASS*)Instance)->*MemberPointer ); }
	};

	struct VariableView {

	};
}

