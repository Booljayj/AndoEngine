#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <utility>
#include "Engine/MemoryView.h"
#include "Reflection/Components/ArgumentInfo.h"
#include "Reflection/Resolver.h"

namespace Reflection
{
	//@todo Experimental. The main reason to have this is for function invocation, and that seems to be a hacky,
	// non-typesafe mess at the moment. Needs to be thought out more to figure out if there's a better way.

	//@todo It would be FANTASTIC if we could say that arguments must be const, because we don't have to worry about converting to non-const void*
	// That does of course mean that functions may only have a single return value, but that might be good for a lot of other reasons. If you
	// need multiple returns, you return a struct.

	struct TypeInfo;

	enum class FFunctionFlags : uint8_t {
		None = 0,
		Static = 1 << 0,
		Const = 1 << 1,
		Hidden = 1 << 2,
	};

	struct FunctionInfo
	{
		std::string Name;
		std::string Description;

		TypeInfo const* InstanceType = nullptr;
		TypeInfo const* ReturnType = nullptr;
		std::vector<std::unique_ptr<ArgumentInfo>> ArgumentInfos;

		uint16_t NameHash = 0;
		FFunctionFlags Flags = FFunctionFlags::None;

		template<typename TCLASS>
		bool ValidateInstance( TCLASS* Instance ) const
		{
			if( !InstanceType && !Instance ) {
				return true;
			} else if( InstanceType && Instance ) {
				return GetTypeInfo<TCLASS>() == InstanceType;
			} else {
				return false;
			}
		}
		template<typename TRETURN>
		bool ValidateReturn( TRETURN const& Return ) const
		{
			return GetTypeInfo<TRETURN> == ReturnType;
		}
		template<size_t SIZE>
		bool ValidateArguments( std::array<TypeInfo const*, SIZE> const& Args )
		{
			if( SIZE == ArgumentInfos.size() ) {
				for( size_t Index = 0; Index < SIZE; ++Index ) {
					if( ArgumentInfos[Index].Type == Args[Index] ) {
						return false;
					}
				}
				return true;
			}
			return false;
		}

		//Safe invocation function that performs reflected type checking and argument packing
		template<typename TCLASS, typename TRETURN, typename... TARGS>
		bool InvokeMember( TCLASS* Instance, TRETURN& Return, TARGS&... Args ) const
		{
			if( ValidateInstance( Instance ) && ValidateReturn( Return ) ) {
				//@todo Verify that the typeinfo of each argument matches this function's definition, and that the number of arguments is also correct
				std::array<TypeInfo const*, sizeof...( TARGS )> ArgTypes{ { GetTypeInfo<TARGS>()... } };
				if( ValidateArguments( ArgTypes ) ) {
					std::array<void*, sizeof...( TARGS )> ArgPtrs{ { static_cast<void*>( &Args )... } };
					return InternalInvoke( Instance, &Return, ArgPtrs.data() );
				}
			}
			return false;
		}

		//Safe invocation function that performs reflected type checking and argument packing
		template<typename TCLASS, typename... TARGS>
		bool Invoke( TCLASS* Instance, TARGS&... Args ) const
		{
			if( ReturnType == GetTypeInfo<void>() && ValidateInstance( Instance ) ) {
				//@todo Verify that the typeinfo of each argument matches this function's definition, and that the number of arguments is also correct
				std::array<TypeInfo const*, sizeof...( TARGS )> ArgTypes{ { GetTypeInfo<TARGS>()... } };
				if( ValidateArguments( ArgTypes ) ) {
					std::array<void*, sizeof...( TARGS )> ArgPtrs{ { static_cast<void*>( &Args )... } };
					return InternalInvoke( Instance, nullptr, ArgPtrs.data() );
				}
			}
			return false;
		}

	protected:
		/** Unsafe internal invocation function. Assumes that the arguments were typechecked! */
		virtual bool InternalInvoke( void* Instance, void* Return, void** Args ) const = 0;
	};

	template<typename TCLASS, typename TRETURN, typename... TARGS>
	struct MemberFunctionInfo : public FunctionInfo
	{
		MemberFunctionInfo( TRETURN (TCLASS::* InMember)( TARGS... ) )
		: Member( InMember )
		, InstanceType( GetTypeInfo<TCLASS>() )
		, ReturnType( GetTypeInfo<TRETURN>() )
		{}

		TRETURN (TCLASS::* Member)( TARGS... );

		template<size_t... Is>
		void InternalInvokeMember_Impl( TCLASS& Instance, TRETURN& Return, void** Args, std::index_sequence<Is...> ) const
		{
			Return = (Instance.*Member)( *static_cast<TARGS*>( Args[Is] )... );
		}

	protected:
		bool InternalInvoke( void* Instance, void* Return, void** Args ) const final
		{
			if( Instance ) {
				InternalInvokeMember_Impl( *static_cast<TCLASS*>( Instance ), *static_cast<TRETURN*>( Return ), Args, std::index_sequence_for<TARGS...>{} );
				return true;
			} else {
				return false;
			}
		}
	};
}
