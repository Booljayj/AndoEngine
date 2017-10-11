//
//  component.h
//  ECS
//
//  Created by Justin Bool on 5/30/17.
//  Copyright © 2017 Justin Bool. All rights reserved.
//

#pragma once

#include <memory>

#include "General.h"
#include "CompInfo.h"
#include "TCompManager.h"

template< typename TTData >
struct TCompInfo : public CompInfo
{
	TCompInfo( CompTypeID InID, const char* InName, TCompManager<TTData>* InManager )
	: CompInfo( InID, InName, InManager )
	{}
	virtual ~TCompInfo() override {}

	//hides CompManager* CompInfo::GetManager() const;
	TCompManager<TTData>* GetManager() const override final { return static_cast<TCompManager<TTData>*>( CompInfo::GetManager() ); }
};
