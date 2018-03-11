#pragma once
//General macro utilities for engine functions

struct Context;

/** Macro used to define a function that requires a context. Must appear as the first parameter in the function */
#define CTX_ARG Context const& CTX

#define TEST_BIT( Mask, Bit ) (Mask & (Bit))
#define SET_BIT( Mask, Bit ) (Mask |= (Bit))
#define CLEAR_BIT( Mask, Bit ) (Mask &= (~Bit))
