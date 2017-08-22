//
//  istorage.h
//  ECS
//
//  Created by Justin Bool on 4/15/17.
//  Copyright © 2017 Justin Bool. All rights reserved.
//

#pragma once

#include <unordered_map>

#include "General.h"

struct CompManager
{
public:
	virtual bool Initialize() { return true; }
	virtual bool Deinitialize() { return true; }

	virtual raw_ptr Retain() = 0;
	virtual void Release( raw_ptr ) = 0;
	virtual void Flush() = 0;

	virtual size_t CountTotal() const = 0;
	virtual size_t CountFree() const = 0;
	virtual size_t CountUsed() const = 0;
	
	virtual void Save( const raw_ptr, ByteStream& ) = 0;
	virtual void Load( raw_ptr, const ByteStream& ) = 0;
	virtual void Copy( const raw_ptr, raw_ptr ) = 0;

	const vector<raw_ptr>& GetRetained() { return Retained; }
	const vector<raw_ptr>& GetReleased() { return Released; }

protected:
	vector<raw_ptr> Retained;
	vector<raw_ptr> Released;
};
