//
//  OwnedComponent.h
//  AndoEngine
//
//  Created by Justin Bool on 7/6/17.
//
//

#pragma once

struct OwnedComponent
{
	CompTypeID TypeID;
	raw_ptr CompPtr;

	bool operator==( const CompTypeID& InTypeID ) const { return TypeID == InTypeID; }
};
