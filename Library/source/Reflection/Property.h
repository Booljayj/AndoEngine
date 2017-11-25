// Copyright © 2017 Justin Bool. All rights reserved.

#ifndef property_h
#define property_h

#include "basic_property.h"

template< typename TType >
struct property : public base_property
{
	property( const string&& in_name, const TType&& in_default_value )
	: base_property( std::forward<const string>( in_name ) ), default_value( in_default_value )
	{}

protected:
	const TType default_value;
};

#endif /* property_h */
