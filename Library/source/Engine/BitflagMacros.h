#pragma once
//General macro utilities for engine functions

#define TEST_BIT( Mask, Bit ) (Mask & (Bit))
#define SET_BIT( Mask, Bit ) (Mask |= (Bit))
#define CLEAR_BIT( Mask, Bit ) (Mask &= (~Bit))
