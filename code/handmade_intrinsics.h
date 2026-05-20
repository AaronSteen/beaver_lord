#if !defined(HANDMADE_INTRINSICS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

//
// TODO(casey): Convert all of these to platform-efficient versions
// and remove math.h
//

#include "math.h"

inline s32
RoundReal32ToS32(real32 Real32)
{
    s32 Result = (s32)roundf(Real32);
    return(Result);
}

inline u32
RoundReal32ToU32(real32 Real32)
{
    u32 Result = (u32)roundf(Real32);
    return(Result);
}

inline s32 
FloorReal32ToS32(real32 Real32)
{
    s32 Result = (s32)floorf(Real32);
    return(Result);
}

inline u32
FloorReal32ToU32(real32 Real32)
{
    u32 Result = (u32)floorf(Real32);
    return(Result);
}

inline s32
TruncateReal32ToInt32(real32 Real32)
{
    s32 Result = (s32)Real32;
    return(Result);
}

inline real32
Sin(real32 Angle)
{
    real32 Result = sinf(Angle);
    return(Result);
}

inline real32
Cos(real32 Angle)
{
    real32 Result = cosf(Angle);
    return(Result);
}

inline real32
ATan2(real32 Y, real32 X)
{
    real32 Result = atan2f(Y, X);
    return(Result);
}

#define HANDMADE_INTRINSICS_H
#endif
