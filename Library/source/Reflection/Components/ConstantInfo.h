#pragma once
#include <cstdint>
#include <string>
#include "Engine/StringID.h"
#include "Reflection/Resolver/TypeResolver.h"

namespace Reflection {
	struct TypeInfo;

	enum FConstantFlags : uint8_t {
		None = 0,
		Hidden = 1 << 0,
	};

	/** Info that describes a constant value */
	struct StaticConstantInfo
	{
		StaticConstantInfo() = delete;
		StaticConstantInfo( const char* InName, const char* InDescription )
		: Name( InName )
		, Description( InDescription )
		, NameHash( static_cast<uint16_t>( id( InName ) ) )
		{}
		virtual ~StaticConstantInfo() {};

		std::string Name;
		std::string Description;

		TypeInfo* Type = nullptr;

		uint16_t NameHash = 0;
		FConstantFlags Flags = FConstantFlags::None;

		virtual void const* GetValuePointer() const = 0;
	};

	template<typename TVAR>
	struct TStaticConstantInfo : public StaticConstantInfo
	{
		TStaticConstantInfo() = delete;
		TStaticConstantInfo( const char* InName, const char* InDescription, TVAR* InStaticPointer )
		: StaticConstantInfo( InName, InDescription )
		, StaticPointer( InStaticPointer )
		{
			Type = TypeResolver<typename std::decay<TVAR>::type>::Get();
		}
		virtual ~TStaticConstantInfo() {};

		TVAR const* StaticPointer;

		void const* GetValuePointer() const final { return StaticPointer; }
	};

	/** Info that describes a constant value within a struct */
	struct MemberConstantInfo
	{
		MemberConstantInfo() = delete;
		MemberConstantInfo( const char* InName, const char* InDescription )
		: Name( InName )
		, Description( InDescription )
		, NameHash( static_cast<uint16_t>( id( InName ) ) )
		{}
		virtual ~MemberConstantInfo() {};

		std::string Name;
		std::string Description;

		TypeInfo* Type = nullptr;

		uint16_t NameHash = 0;
		FConstantFlags Flags = FConstantFlags::None;

		virtual void const* GetValuePointer( void const* Instance ) const = 0;
	};

	template<typename TCLASS, typename TVAR>
	struct TMemberConstantInfo : public MemberConstantInfo
	{
		TMemberConstantInfo() = delete;
		TMemberConstantInfo( const char* InName, const char* InDescription, TVAR const TCLASS::* InMemberPointer )
		: MemberConstantInfo( InName, InDescription )
		, MemberPointer( InMemberPointer )
		{
			Type = TypeResolver<typename std::decay<TVAR>::type>::Get();
		}
		virtual ~TMemberConstantInfo() {};

		TVAR const TCLASS::* MemberPointer;

		void const* GetValuePointer( void const* Instance ) const final { return &( ((TCLASS const*)Instance)->*MemberPointer ); }
	};
}
