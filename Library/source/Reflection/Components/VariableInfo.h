#pragma once
#include <cstdint>
#include <string>
#include <type_traits>
#include "Engine/StringID.h"
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
		/** Type used to store name hashes. Smaller than the normal string hash because this value is used in serialization and is more compact */
		using HASH_T = uint16_t;

		VariableInfo() = delete;
		VariableInfo( const char* InName, TypeInfo const* InType, const char* InDescription, FVariableFlags InFlags )
		: Name( InName )
		, NameHash( static_cast<HASH_T>( id( InName ) ) )
		, Type( InType )
		, Description( InDescription )
		, Flags( InFlags )
		{}
		virtual ~VariableInfo() {};

		std::string Name;
		HASH_T NameHash = 0;
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

