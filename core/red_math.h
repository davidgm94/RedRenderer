#pragma once

#include "common.h"

inline i32
roundfloatToi32(float real32)
{
    i32 result = (i32)(real32 + 0.5f);
    // TODO(casey): Intrinsic????
    return result;
}

inline u32
roundfloatTou32(float real32)
{
    u32 result = (u32)(real32 + 0.5f);
    // TODO(casey): Intrinsic????
    return result;
}

// TODO(casey): HOW TO IMPLEMENT THESE MATH FUNCTIONS!!!!
#include "math.h"
inline i32
floorfloatToi32(float real32)
{
    i32 result = (i32)floorf(real32);
    return result;
}

inline i32
truncatefloatToi32(float real32)
{
    i32 result = (i32)real32;
    return result;
}