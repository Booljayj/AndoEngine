#pragma once
#include <cstdint>
#include <string>
#include "Engine/StringID.h"
#include "Reflection/TypeResolver.h"

namespace Reflection {
	struct TypeInfo;

	enum FConstantFlags : uint8_t {
		None = 0,
		Hidden = 1 << 0,
	};

	/** Info that describes a constant value */
	struct ConstantInfo {
		/** Type used to store name hashes */
		using HASH_T = sid_t;

		ConstantInfo() = delete;
		ConstantInfo( const char* InName, TypeInfo const* InType, const char* InDescription, FConstantFlags InFlags )
		: Name( InName )
		, NameHash( id( InName ) )
		, Type( InType )
		, Description( InDescription )
		, Flags( InFlags )
		{}
		virtual ~ConstantInfo() {};

		std::string Name;
		HASH_T NameHash = 0;
		TypeInfo const* Type = nullptr;

		std::string Description;
		FConstantFlags Flags = FConstantFlags::None;

		virtual void const* GetValuePointer( void const* Instance ) const = 0;
	};

	/** Info that describes a constant value that is global */
	template<typename TVAR>
	struct TStaticConstantInfo : public ConstantInfo {
		TStaticConstantInfo() = delete;
		TStaticConstantInfo( const char* InName, const char* InDescription, TVAR* InStaticPointer )
		: ConstantInfo( InName, TypeResolver<typename std::decay<TVAR>::type>::Get(), InDescription, FConstantFlags::None )
		, StaticPointer( InStaticPointer )
		{}
		virtual ~TStaticConstantInfo() {};

		TVAR const* StaticPointer;

		void const* GetValuePointer( void const* Instance ) const final { return StaticPointer; }
	};

	/** Info that describes a constant value that is contained within a struct instance */
	template<typename TCLASS, typename TVAR>
	struct TMemberConstantInfo : public ConstantInfo {
		TMemberConstantInfo() = delete;
		TMemberConstantInfo( const char* InName, const char* InDescription, TVAR const TCLASS::* InMemberPointer )
		: ConstantInfo( InName, TypeResolver<typename std::decay<TVAR>::type>::Get(), InDescription, FConstantFlags::None )
		, MemberPointer( InMemberPointer )
		{}
		virtual ~TMemberConstantInfo() {};

		TVAR const TCLASS::* MemberPointer;

		void const* GetValuePointer( void const* Instance ) const final { return &( ((TCLASS const*)Instance)->*MemberPointer ); }
	};
}
