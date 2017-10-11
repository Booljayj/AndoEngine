//
//  icomponent.h
//  ECS
//
//  Created by Justin Bool on 5/30/17.
//  Copyright © 2017 Justin Bool. All rights reserved.
//

#pragma once

#include <cassert>
#include <iostream>
#include <map>
using namespace std;

#include "General.h"
#include "CompManager.h"

struct CompInfo
{
	CompInfo( const CompTypeID& InID, const char* InName, CompManager* InManager )
	: ID( InID ), Name( InName ), Manager( InManager )
	{
		assert( Manager != nullptr );
	}
	virtual ~CompInfo() {}

	const CompTypeID ID;
	const string Name;
	CompManager* const Manager;

	virtual CompManager* GetManager() const { return Manager; }

	template< typename TStream >
	friend TStream& operator <<( TStream& Stream, const CompInfo& Info )
	{
		return Stream << "[CompInfo: " << Info.ID << "\t" << Info.Name << "\tFree: " << Info.Manager->CountFree() << "\tUsed: " << Info.Manager->CountUsed() << "]" << endl;
	}
};
