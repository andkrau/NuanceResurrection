#ifndef UTILITY_H
#define UTILITY_H

#include "basetypes.h"

__forceinline uint32 OnesCount(const uint32 x)
{
    return __popcnt(x);
    /*x -= ((x >> 1) & 0x55555555);
    x = (((x >> 2) & 0x33333333) + (x & 0x33333333));
    x = (((x >> 4) + x) & 0x0f0f0f0f);
    x += (x >> 8);
    x += (x >> 16);
    return(x & 0x0000003f);*/
}

#endif
