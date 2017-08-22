//
//  class_property_editor.h
//  ECS
//
//  Created by Justin Bool on 5/31/17.
//  Copyright © 2017 Justin Bool. All rights reserved.
//

#ifndef class_property_editor_h
#define class_property_editor_h

template< class TClass, typename TType >
struct member_property_editor
{
	member_property_editor( TClass* in_ctx_obj, const member_property<TClass, TType>* const in_property )
	: ctx_obj( in_ctx_obj ), property( in_property )
	{}
	
	TType get_value() const
	{
		return property->get( ctx_obj );
	}
	void set_value( const TType& new_value ) const
	{
		property->get( ctx_obj ) = new_value;
	}
	void reset_value() const
	{
		property->get( ctx_obj ) = property->default_value;
	}
	
protected:
	TClass* const ctx_obj;
	const member_property<TClass, TType>* const property;
};

#endif /* class_property_editor_h */
