//
//  static_property.h
//  ECS
//
//  Created by Justin Bool on 5/31/17.
//  Copyright © 2017 Justin Bool. All rights reserved.
//

#ifndef static_property_h
#define static_property_h

template< TType >
struct static_property : public base_property
{
	static_property( const string&& in_name, TType* in_value_ptr, const TType& in_default_value )
	: base_property( std::forward<const string>( in_name ) ), value_ptr( in_value_ptr ), default_value( in_default_value )
	{}
	
	const TType default_value;
	
	TType& get() const { return *value_ptr; }
	
protected:
	TType* const value_ptr;
};

#endif /* static_property_h */
