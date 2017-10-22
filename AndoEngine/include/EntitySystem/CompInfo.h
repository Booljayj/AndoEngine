// Copyright © 2017 Justin Bool. All rights reserved.

#pragma once

#include <cassert>
#include <iostream>
using namespace std;

#include "General.h"
#include "CompManager.h"

struct CompInfo
{
public:
	CompInfo( const CompTypeID& InID, const char* InName, CompManager* InManager )
		: ID( InID ), Name( InName ), Manager( InManager )
	{
		assert( Manager != nullptr );
	}
	virtual ~CompInfo() {}

protected:
	CompTypeID ID;
	string Name;
	CompManager* Manager;

public:
	CompTypeID GetID() const { return ID; }
	string GetName() const { return Name; }
	CompManager* GetManager() const { return Manager; }
};

inline ostream& operator<<( ostream& Stream, const CompInfo& Info )
{
	Stream << "[ComponentInfo]{ ID: " << Info.GetID() << ", Name: " << Info.GetName();
	Stream << ", Used: " << Info.GetManager()->CountUsed() << "/" << Info.GetManager()->CountTotal() << " }\n";
	return Stream;
}

template< typename TDATA >
struct TCompInfo : public CompInfo
{
	TCompInfo( CompTypeID InID, const char* InName, TCompManager<TDATA>* InManager )
		: CompInfo( InID, InName, InManager )
	{}
	virtual ~TCompInfo() override {}

	TCompManager<TDATA>* GetTypedManager() const { return static_cast<TCompManager<TDATA>*>( CompInfo::GetManager() ); }
};
