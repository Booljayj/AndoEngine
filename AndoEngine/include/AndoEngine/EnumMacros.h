// Copyright © 2017 Justin Bool. All rights reserved.

#pragma once

#include <type_traits>
#include <cstring>
#include <vector>
#include <boost/preprocessor.hpp>
using namespace std;

#define ENUM_VALUE_STRINGIFY( R, DATA, ELEMENT ) #ELEMENT

//A smart enum that comes packed with various reflection functions
#define DeclareEnumeration( NAME, SIZE_TYPE, VALUES_TUPLE )\
namespace NAME {\
	using TYPE = SIZE_TYPE;\
	constexpr TYPE __Count__ = BOOST_PP_TUPLE_SIZE( VALUES_TUPLE );\
	enum ENUM : TYPE { BOOST_PP_TUPLE_ENUM( VALUES_TUPLE ) };\
	constexpr const char* const __Names__[__Count__ + 1] = {\
		BOOST_PP_SEQ_ENUM( BOOST_PP_SEQ_TRANSFORM( ENUM_VALUE_STRINGIFY, NAME, BOOST_PP_TUPLE_TO_SEQ( VALUES_TUPLE ) ) ),\
		"Invalid"\
	};\
\
	constexpr TYPE Count() { return __Count__; }\
	bool IsValid( const TYPE& Value );\
	const char* Name( const TYPE& Value );\
	TYPE Value( const char* Name );\
	ENUM Cast( const TYPE& Value );\
}\


#define DefineEnumeration( NAME )\
namespace NAME {\
	bool IsValid( const TYPE& Value ) {\
		return Value < __Count__;\
	}\
	const char* Name( const TYPE& Value ) {\
		return __Names__[Value > __Count__ ? __Count__ : Value];\
	}\
	TYPE Value( const char* Name ) {\
		for( TYPE Index = 0; Index < __Count__; ++Index ) {\
			if( strcmp( __Names__[Index], Name ) == 0 ) return Index;\
		}\
		return __Count__;\
	}\
	ENUM Cast( const TYPE& Value ) {\
		return static_cast<ENUM>( Value );\
	}\
}\


#define ENUM_TUPLE_PAIR_FIRST( R, DATA, ELEMENT ) BOOST_PP_TUPLE_ELEM( 0, ELEMENT )
#define ENUM_TUPLE_PAIR_SECOND( R, DATA, ELEMENT ) BOOST_PP_TUPLE_ELEM( 1, ELEMENT )

//A smart enum that converts a scoped enum into global enum values, the kind typically used by large old libraries like OpenGL
#define DeclareEnumerationConverter( NAME, TYPE_PAIRS_TUPLE, VALUE_PAIRS_TUPLE )\
DeclareEnumeration( NAME, BOOST_PP_TUPLE_ELEM( 0, TYPE_PAIRS_TUPLE ), BOOST_PP_SEQ_TO_TUPLE( BOOST_PP_SEQ_TRANSFORM( ENUM_TUPLE_PAIR_FIRST,, BOOST_PP_TUPLE_TO_SEQ( VALUE_PAIRS_TUPLE ) ) ) );\
namespace NAME {\
	using GTYPE = BOOST_PP_TUPLE_ELEM( 1, TYPE_PAIRS_TUPLE );\
	constexpr const GTYPE __Globals__[ __Count__ ] = { BOOST_PP_SEQ_ENUM( BOOST_PP_SEQ_TRANSFORM( ENUM_TUPLE_PAIR_SECOND,, BOOST_PP_TUPLE_TO_SEQ( VALUE_PAIRS_TUPLE ) ) ) };\
\
	GTYPE ToGlobal( const TYPE& Value );\
	TYPE FromGlobal( const GTYPE& Global );\
	const char* GlobalName( const GTYPE& Global );\
}\


#define DefineEnumerationConverter( NAME )\
DefineEnumeration( NAME );\
namespace NAME {\
	GTYPE ToGlobal( const TYPE& Value ) {\
		if( IsValid( Value ) ) return __Globals__[Value];\
		else return 0;\
	}\
	TYPE FromGlobal( const GTYPE& Global ) {\
		for( TYPE Index = 0; Index < __Count__; ++Index ) {\
			if( __Globals__[Index] == Global ) return Index;\
		}\
		return __Count__;\
	}\
	const char* GlobalName( const GTYPE& Global ) {\
		return Name( FromGlobal( Global ) );\
	}\
}\

//Experiments
#if 0

template< typename TENUM_INTERNAL, const std::vector<const char*>* TNAMES_INTERNAL >
struct Enum
{
	using TYPE = typename std::underlying_type<TENUM_INTERNAL>::type;

	static constexpr inline const char* Name( const TYPE& Value )
	{
		return TNAMES_INTERNAL->at( Value );
	}
};

enum class EInternal : uint8_t
{
	FirstThing,
	SecondThing
};

const vector<const char*> EInternalNames = { "FirstThing", "SecondThing" };

struct ESmart : Enum<EInternal, &EInternalNames>
{
};

const char* SecondName = ESmart::Name( 1 );

//Or, the alternative, just use global template functions

template< typename TEnum >
const char* EnumName( typename std::underlying_type<TEnum>::type& Value );

template<>
const char* EnumName<EInternal>( std::underlying_type<EInternal>& Value )
{
	return "Something";
}

template< typename TENUM >
class Enum
{
	using ENUM = TENUM;

	static constexpr const size_t COUNT = 0;
	static constexpr const char* Name( TENUM Value );
	static constexpr TENUM Value( const char* Name );
};

enum class ETest : int
{
	Value1,
	Value2,
	Value3
};

template<>
class Enum<ETest>
{
	using ENUM = ETest;
	static constexpr const char* NAMES[3] = { "Value 1", "Value 2", "Value 3" };

	static constexpr const size_t COUNT = 3;
	static constexpr const char* Name( ETest Value ) { return NAMES[static_cast<size_t>( Value )]; }
};

#endif
