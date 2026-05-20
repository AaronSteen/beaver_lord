#if !defined(BEAVER_H)
#include "beaver_platform.h"

#define internal static 
#define local_persist static 
#define global_variable static

#define Pi32 3.14159265359f

#if HANDMADE_SLOW
// TODO(casey): Complete assertion macro - don't worry everyone!
#define Assert(Expression) if(!(Expression)) {*(volatile int *)0 = 0;}
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
// TODO(casey): swap, min, max ... macros???

inline u32
SafeTruncateUInt64(u64 Value)
{
    // TODO(casey): Defines for maximum values
    Assert(Value <= 0xFFFFFFFF);
    u32 Result = (u32)Value;
    return(Result);
}

inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(Input->Controllers));
    
    game_controller_input *Result = &Input->Controllers[ControllerIndex];
    return(Result);
}

enum tile_value
{
    TILE_INVALID,
    TILE_WATER,
    TILE_BLOCK
};

struct memory_arena
{
    u8 *Base;
    memory_index Size;
    memory_index Used;
};

struct tile_chunk_position
{
    u32 ChunkX;
    u32 ChunkY;

    u32 TileInChunkX;
    u32 TileInChunkY;
};

struct world_position
{
    /* TODO(casey):

       Take the tile map x and y
       and the tile x and y

       and pack them into single 32-bit values for x and y
       where there is some low bits for the tile index
       and the high bits are the tile "page"

       (NOTE we can eliminate the need for floor!)
    */
    u32 AbsTileX;
    u32 AbsTileY;

    // TODO(casey): Should these be from the center of a tile?
    // TODO(casey): Rename to offset X and Y
    real32 TileRelX;
    real32 TileRelY;
};

struct tile_chunk
{
    u32 *TilesArray;
};


struct tile_map
{
    u32 ChunkMask;
    u32 ChunkDim;

    real32 TileSideInMeters;
    s32 TileSideInPixels;
    real32 MetersToPixels;

    u32 RoomsInMapY;
    u32 RoomsInMapX;
    u32 TilesPerRoomY;
    u32 TilesPerRoomX;

    u32 ChunksInMapY;
    u32 ChunksInMapX;

    u32 TilesInWorldY;
    u32 TilesInWorldX;

    tile_chunk *TileChunksArray;
};

struct world
{
    tile_map *TileMapPointer;
};

struct game_state
{
    world_position PlayerPosition;
    memory_arena WorldArena;
    world *WorldPointer;
};


#define BEAVER_H
#endif
