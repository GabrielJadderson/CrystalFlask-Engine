#if !defined(CRYSTALFLASK_H)

#include "crystalflask_platform.h"

#include "crystalflask_math.h"
#include "crystalflask_intrinsics.h"

struct world
{
    
};

struct game_state
{
    memory_arena WorldArena;
    world *World;
    
    v2 CameraPosition;
    
};

#define CRYSTALFLASK_H
#endif
