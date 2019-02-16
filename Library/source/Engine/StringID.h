#pragma once
#include <cstddef>
#include <cstdint>

/** The type used when creating string ids. */
using sid_t = uint32_t;

namespace {
	/**
	 * Generates a unique id for a particular string. Once this is in use, this MUST NOT CHANGE.
	 * These ids are often used in serialized data and between program executions
	 */
	constexpr inline sid_t id_internal( char const* Str, size_t Size ) {
		//DJB2 Algorithm
		sid_t Result = 5381;
		for( size_t Index = 0; Index < Size; ++Index, ++Str ) {
			Result = ( Result * 33 ) + *Str;
		}
		return Result;
	}
}

constexpr inline sid_t operator ""_id( char const* Str, size_t Size ) { return id_internal( Str, Size ); }

constexpr inline sid_t id( char const* Str ) {
	if( Str != nullptr ) {
		return id_internal( Str, strlen( Str ) );
	} else {
		return 0;
	}
}

constexpr inline sid_t id( std::string_view Str ) {
	if( Str.data() != nullptr ) {
		return id_internal( Str.data(), Str.length() );
	} else {
		return 0;
	}
}

/** Combine two string ids together to create an id that represents the unique combination (not the same as the id of the concatenated strings). */
constexpr inline sid_t id_combine( sid_t A, sid_t B ) {
	//Implementation taken from boost::hash_combine
	return B + 0x9e3779b9 + (A << 6) + (A >> 2);
}

/** Combine a sequence of ids together to create an id that represents the unique sequence */
constexpr inline sid_t id_combine( const sid_t* IDs, size_t Size ) {
	sid_t Result = 0;
	for( size_t Index = 0; Index < Size; ++Index ) {
		Result = id_combine( Result, IDs[Index] );
	}
	return Result;
}
