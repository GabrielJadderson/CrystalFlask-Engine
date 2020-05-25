#pragma once

#include "math.h"

inline s32
RoundReal32ToInt32(r32 Real32)
{
    s32 Result = (s32)roundf(Real32);
    return(Result);
}

inline u32
RoundReal32ToUInt32(r32 Real32)
{
    u32 Result = (u32)roundf(Real32);
    return(Result);
}

inline s32
FloorReal32ToInt32(r32 Real32)
{
    s32 Result = (s32)floorf(Real32);
    return(Result);
}

inline s32
TruncateReal32ToInt32(r32 Real32)
{
    s32 Result = (s32)Real32;
    return(Result);
}

inline r32
Sin(r32 Angle)
{
    r32 Result = sinf(Angle);
    return(Result);
}

inline r32
Cos(r32 Angle)
{
    r32 Result = cosf(Angle);
    return(Result);
}

inline r32
ATan2(r32 Y, r32 X)
{
    r32 Result = atan2f(Y, X);
    return(Result);
}



struct bit_scan_result
{
    b32 Found;
    u32 Index;
};
inline bit_scan_result
FindLeastSignificantSetBit(u32 Value)
{
    bit_scan_result Result = {};
#if COMPILER_MSVC
    Result.Found = _BitScanForward((unsigned long *)&Result.Index, Value);
#else
    for(u32 Test = 0; Test < 32; Test++)
    {
        if (Value & (1 << Test))
        {
            Result.Index = Test;
            Result.Found = true;
            break;
        }
    }
#endif
    return Result;
}
