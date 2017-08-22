//
//  class_property.h
//  ECS
//
//  Created by Justin Bool on 5/31/17.
//  Copyright © 2017 Justin Bool. All rights reserved.
//

#ifndef class_property_h
#define class_property_h

#include <utility>
#include "property.h"

template< class TClass, typename TType >
struct member_property : public base_property
{
	member_property( const string&& in_name, TType TClass::* const in_value_ptr, const TType&& in_default_value )
	: base_property( std::forward<const string>( in_name ) ), value_ptr( in_value_ptr ), default_value( in_default_value )
	{}
	
	const TType default_value;
	
	TType& get( TClass* ctx ) const { return ctx->*value_ptr; }
	
protected:
	TType TClass::* const value_ptr;
};

#endif /* class_property_h */
