//
//  basic_property.h
//  ECS
//
//  Created by Justin Bool on 5/31/17.
//  Copyright © 2017 Justin Bool. All rights reserved.
//

#ifndef basic_property_h
#define basic_property_h

struct base_property
{
	base_property( const string&& in_name )
	: name( in_name )
	{}
	
	const string name;
};

#endif /* basic_property_h */
