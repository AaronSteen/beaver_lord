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

inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(Input->Controllers));
    
    game_controller_input *Result = &Input->Controllers[ControllerIndex];
    return(Result);
}

#include "beaver_math.h"
inline u32
SafeTruncateUInt64(u64 Value)
{
    // TODO(casey): Defines for maximum values
    Assert(Value <= 0xFFFFFFFF);
    u32 Result = (u32)Value;
    return(Result);
}

enum tile_value
{
    TILE_INVALID,
    TILE_WATER,
    TILE_BLOCK
};

enum player_facing
{
    FACING_RIGHT,
    FACING_UP,
    FACING_LEFT,
    FACING_DOWN
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

struct tile_map_position
{
    u32 AbsTileX;
    u32 AbsTileY;
    vector2 TileOffset;
};

struct room_position
{
    u32 RoomY;
    u32 RoomX;
    u32 TileInRoomY;
    u32 TileInRoomX;
};

struct tile_screen_coordinates
{
    vector2 Min;
    vector2 Max;
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

    u32 RoomsInMapY;
    u32 RoomsInMapX;
    u32 TilesPerRoomY;
    u32 TilesPerRoomX;

    u32 ChunksInMapY;
    u32 ChunksInMapX;

    tile_chunk *TileChunksArray;
};

struct world
{
    tile_map *TileMapPointer;
};

#pragma pack(push, 1)
struct bitmap_header
{
    char Signature[2];
    u32 FileSize;
    u8 Reserved[4];
    u32 DataOffset;
    u32 Size;
    u32 Width;
    s32 Height;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 CompressedImageSize;
    u32 XPixelsPerMeter;
    u32 YPixelsPerMeter;
    u32 ColorsUsed;
    u32 ImportantColors;
    u32 RedMask;
    u32 GreenMask;
    u32 BlueMask;
};
#pragma pack(pop)

struct bitmap
{
    u32 Size;
    u32 Width;
    s32 Height;
    u32 AlphaMask;
    u32 RedMask;
    u32 GreenMask;
    u32 BlueMask;
    u8 *Pixels;
};

struct game_state
{
    memory_arena WorldArena;
    world *WorldPointer;

    tile_map_position PlayerPosition;
    u32 PlayerFacing;
    vector2 dPlayerP;

    bool32 BirdsEye;
    u32 RandomSeed;

    bitmap Tree;
    bitmap TreeSmall;
    bitmap Water;
    bitmap WaterSmall;

    bitmap BeaverRight;
    bitmap BeaverUp;
    bitmap BeaverLeft;
    bitmap BeaverDown;
};

#define BEAVER_H
#endif
