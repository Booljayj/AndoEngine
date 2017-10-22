//
//  general.h
//  ECS
//
//  Created by Justin Bool on 4/13/17.
//  Copyright © 2017 Justin Bool. All rights reserved.
//

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

typedef void* raw_ptr;

typedef uint32_t EntityID;
typedef uint16_t CompTypeID;
typedef size_t CompID;

typedef vector<uint8_t> ByteStream;
