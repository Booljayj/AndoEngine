#pragma once
#include <cstdint>
#include <string>
#include <type_traits>
#include "Engine/StringID.h"
#include "Reflection/Resolver/TypeResolver.h"

namespace Reflection {
	struct TypeInfo;

	/** Flags to describe aspects of a particular type */
	enum class FVariableFlags : uint8_t {
		None = 0,
		Hidden = 1 << 1,
	};

	/** Info that describes a variable value */
	struct StaticVariableInfo
	{
		StaticVariableInfo() = delete;
		StaticVariableInfo( const char* InName, const char* InDescription )
		: Name( InName )
		, Description( InDescription )
		, NameHash( static_cast<uint16_t>( id( InName ) ) )
		{}
		virtual ~StaticVariableInfo() {};

		std::string Name;
		std::string Description;

		TypeInfo* Type = nullptr;

		uint16_t NameHash = 0;
		FVariableFlags Flags = FVariableFlags::None;

		template<typename TVAR>
		TVAR* GetValuePointer() const {
			if( TypeResolver<TVAR>::Get() == Type ) return GetValuePointer();
			else return nullptr;
		}

	protected:
		virtual void* GetValuePointer() const = 0;
	};

	template<typename TVAR>
	struct TStaticVariableInfo : public StaticVariableInfo
	{
		TStaticVariableInfo() = delete;
		TStaticVariableInfo( const char* InName, const char* InDescription, TVAR* InStaticPointer )
		: StaticVariableInfo( InName, InDescription )
		, StaticPointer( InStaticPointer )
		{
			Type = TypeResolver<typename std::decay<TVAR>::type>::Get();
		}
		virtual ~TStaticVariableInfo() {};

		TVAR* StaticPointer;

	protected:
		void* GetValuePointer() const final { return StaticPointer; }
	};

	/** Info that describes a variable within a struct */
	struct MemberVariableInfo
	{
		MemberVariableInfo() = delete;
		MemberVariableInfo( const char* InName, const char* InDescription )
		: Name( InName )
		, Description( InDescription )
		, NameHash( static_cast<uint16_t>( id( InName ) ) )
		{}
		virtual ~MemberVariableInfo() {};

		std::string Name;
		std::string Description;

		TypeInfo* Type = nullptr;

		uint16_t NameHash = 0;
		FVariableFlags Flags = FVariableFlags::None;

		template<typename TVAR>
		TVAR const* GetValuePointer( void const* Instance ) const {
			if( TypeResolver<TVAR>::Get() == Type ) return GetValuePointer( Instance );
			else return nullptr;
		}
		template<typename TVAR>
		TVAR* GetValuePointer( void* Instance ) const {
			return const_cast<void*>( GetValuePointer( static_cast<void const*>( Instance ) ) );
		}

	protected:
		virtual void const* GetValuePointer( void const* Instance ) const = 0;
	};

	template<typename TCLASS, typename TVAR>
	struct TMemberVariableInfo : MemberVariableInfo
	{
		TMemberVariableInfo() = delete;
		TMemberVariableInfo( const char* InName, const char* InDescription, TVAR TCLASS::* InMemberPointer )
		: MemberVariableInfo( InName, InDescription )
		, MemberPointer( InMemberPointer )
		{
			Type = TypeResolver<typename std::decay<TVAR>::type>::Get();
		}
		virtual ~TMemberVariableInfo() {};

		TVAR TCLASS::* MemberPointer;

	protected:
		void const* GetValuePointer( void const* Instance ) const final { return &( ((TCLASS const*)Instance)->*MemberPointer ); }
	};
}

