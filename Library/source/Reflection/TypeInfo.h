// Copyright © 2017 Justin Bool. All rights reserved.

#ifndef type_info_h
#define type_info_h

enum class type_info_tag : uint8_t
{
	value, //a value
	structure, //a group of values
	member, //a value that exists in a group
	container, //a collection of values
};

enum class type_info_flags : uint32_t
{
	readonly =	1 << 0,
	hidden =	1 << 1,
};

struct type_info
{
	const type_info_tag tag;
	const type_info_flags flags;
};

template< TType >
struct type_info_value : type_info
{
	TType* const vptr;
	TType& get() { return *vptr; }
};

template< TStructureType >
struct type_info_structure : type_info
{
	template< TType >
	struct type_info_member : type_info
	{
		const TType TStructure::* m_vptr;
		const TType& get( TStructure* ctx ) { return ctx.*m_vptr; }
	}

	const type_info[] members;
	const size_t member_count;
};

template< TContainerType, TElementType, TIterator >
struct type_info_container : type_info
{
	TIterator get_iterator( TContainerType& c );
	TElementType& get_element( TContainerType& c, const TIterator& it );
	bool add_element( TContainerType& c, const TIterator& it );
	bool del_element( TConatinerType& c, const TIterator& it );
};

#endif /* type_info_h */
