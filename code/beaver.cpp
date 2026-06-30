#include "beaver.h"
#include "handmade_intrinsics.h"

#define CHUNK_DIM 16
#define CHUNK_MASK 4

#define PLAYER_HEIGHT 1.05f
#define PLAYER_WIDTH 1.05f

#define TILES_IN_WORLD_Y (TileMapPointer->RoomsInMapY * TileMapPointer->TilesPerRoomY)
#define TILES_IN_WORLD_X (TileMapPointer->RoomsInMapX * TileMapPointer->TilesPerRoomX)

// Water
#define WATER_R 0.61f
#define WATER_G 0.86f
#define WATER_B 0.99f

// Blocks
#define BLOCK_R 0.59f
#define BLOCK_G 0.29f
#define BLOCK_B 0

// Player
#define PLAYER_R 0.35f 
#define PLAYER_G 0.47f
#define PLAYER_B 0.37f

// Pi
#define PI 3.14159265358f

internal void
GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
    s16 ToneVolume = 3000;
    int WavePeriod = SoundBuffer->SamplesPerSecond/ToneHz;

    s16 *SampleOut = SoundBuffer->Samples;
    for(int SampleIndex = 0;
        SampleIndex < SoundBuffer->SampleCount;
        ++SampleIndex)
    {
#if 0
        real32 SineValue = sinf(GameState->tSine);
        s16 SampleValue = (s16)(SineValue * ToneVolume);
#else
        s16 SampleValue = 0;
#endif
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;

#if 0
        GameState->tSine += 2.0f*Pi32*1.0f/(real32)WavePeriod;

        if(GameState->tSine > 2.0f*Pi32)
        {
            GameState->tSine -= 2.0f*Pi32;
        }
#endif
    }
}

void
DrawRectangle(game_offscreen_buffer *Buffer,
              vector2 Min, vector2 Max,
              real32 R, real32 G, real32 B)
{
    s32 MinX = RoundReal32ToS32(Min.X);
    s32 MinY = RoundReal32ToS32(Min.Y);
    s32 MaxX = RoundReal32ToS32(Max.X);
    s32 MaxY = RoundReal32ToS32(Max.Y);

    if(MinY < 0)
    {
        MinY = 0;
    }
    if(MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }
    if(MinX < 0)
    {
        MinX = 0;
    }
    if(MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }

    u32 Color = ((RoundReal32ToU32(R * 255.0f) << 16) |
                 (RoundReal32ToU32(G * 255.0f) <<  8) |
                 (RoundReal32ToU32(B * 255.0f) <<  0));

    u8 *Row = (u8 *)Buffer->Memory + (MinY * Buffer->Pitch) + (MinX * Buffer->BytesPerPixel);
    for(int Y = MinY;
        Y < MaxY;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(int X = MinX;
            X < MaxX;
            ++X)
        {
            *Pixel++ = Color;
        }
        Row += Buffer->Pitch;
    }
}

void
DrawTileWithOutline(game_offscreen_buffer *Buffer,
                    vector2 Min, vector2 Max,
                    real32 R, real32 G, real32 B)
{
s32 MinX = RoundReal32ToS32(Min.X);
    s32 MinY = RoundReal32ToS32(Min.Y);
    s32 MaxX = RoundReal32ToS32(Max.X);
    s32 MaxY = RoundReal32ToS32(Max.Y);

    if(MinY < 0)
    {
        MinY = 0;
    }
    if(MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }
    if(MinX < 0)
    {
        MinX = 0;
    }
    if(MaxX > Buffer->Width)
{
        MaxX = Buffer->Width;
    }

    u32 Color = ((RoundReal32ToU32(R * 255.0f) << 16) |
                 (RoundReal32ToU32(G * 255.0f) <<  8) |
                 (RoundReal32ToU32(B * 255.0f) <<  0));

    u8 *Row = (u8 *)Buffer->Memory + (MinY * Buffer->Pitch) + (MinX * Buffer->BytesPerPixel);
    for(int Y = MinY;
        Y < MaxY;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(int X = MinX;
            X < MaxX;
            ++X)
        {
            if(Y == MinY)
            {
                *Pixel++ = 0;
            }
            else if(X == MinX)
            {
                *Pixel++ = 0;
            }
            else
            {
                *Pixel++ = Color;
            }
        }
        Row += Buffer->Pitch;
    }

}

void
BlitBitmap(game_offscreen_buffer *Buffer, vector2 Min, vector2 Max, bitmap *Bitmap)
{
    s32 ScreenMinX = RoundReal32ToS32(Min.X);
    s32 ScreenMinY = RoundReal32ToS32(Min.Y);
    s32 ScreenMaxX = RoundReal32ToS32(Max.X);
    s32 ScreenMaxY = RoundReal32ToS32(Max.Y);
    
    u32 BitmapMinX = 0;
    u32 BitmapMinY = 0;
    u32 BitmapMaxX = Bitmap->Width;
    u32 BitmapMaxY = Bitmap->Height;

    if(ScreenMinX < 0)
    {
        u32 PixelsToMoveRight = abs(ScreenMinX);
        ScreenMinX += PixelsToMoveRight;
        BitmapMinX += PixelsToMoveRight;
    }
    if(ScreenMinY < 0)
    {
        u32 PixelsToMoveDown = abs(ScreenMinY);
        ScreenMinY += PixelsToMoveDown;
        BitmapMaxY -= PixelsToMoveDown;
    }
    if(ScreenMaxX > Buffer->Width)
    {
        u32 PixelsToMoveLeft = ScreenMaxX - Buffer->Width;
        ScreenMaxX -= PixelsToMoveLeft;
        BitmapMaxX -= PixelsToMoveLeft;
    }
    if(ScreenMaxY > Buffer->Height)
    {
        u32 PixelsToMoveUp = ScreenMaxY - Buffer->Height;
        ScreenMaxY -= PixelsToMoveUp;
        BitmapMinY += PixelsToMoveUp;
    }

    u32 *ScreenRow = (u32 *)Buffer->Memory + (ScreenMinY * Buffer->Width) + ScreenMinX;
    u32 *BitmapRow = (u32 *)Bitmap->Pixels + (BitmapMaxY * Bitmap->Width) + BitmapMinX;
    for(int Y = ScreenMinY;
        Y < ScreenMaxY;
        ++Y)
    {
        u32 *Dest = ScreenRow;
        u32 *Source = BitmapRow;
        for(int X = ScreenMinX;
            X < ScreenMaxX;
            ++X)
        {
            *Dest++ = *Source++;
        }
        ScreenRow += Buffer->Width;
        BitmapRow -= Bitmap->Width;
    }
}

u32
LerpWithUInts(u32 A, u32 B, real32 T)
{
    u32 ToReturn = A * (1 - T) + (T * B);
    return(ToReturn);
}

real32
LerpWithReal32s(real32 A, real32 B, real32 T)
{
    real32 ToReturn = A * (1.0f - T) + (T * B);
    return(ToReturn);
}

void
BlitBitmapAndBlend(game_offscreen_buffer *Buffer, vector2 Min, vector2 Max, bitmap *Bitmap)
{
    s32 ScreenMinX = RoundReal32ToS32(Min.X);
    s32 ScreenMinY = RoundReal32ToS32(Min.Y);
    s32 ScreenMaxX = RoundReal32ToS32(Max.X);
    s32 ScreenMaxY = RoundReal32ToS32(Max.Y);

    u32 BitmapMinX = 0;
    u32 BitmapMinY = 0;
    u32 BitmapMaxX = Bitmap->Width;
    u32 BitmapMaxY = Bitmap->Height;

    if(ScreenMinX < 0)
    {
        u32 PixelsToMoveRight = abs(ScreenMinX);
        ScreenMinX += PixelsToMoveRight;
        BitmapMinX += PixelsToMoveRight;
    }
    if(ScreenMinY < 0)
    {
        u32 PixelsToMoveDown = abs(ScreenMinY);
        ScreenMinY += PixelsToMoveDown;
        BitmapMaxY -= PixelsToMoveDown;
    }
    if(ScreenMaxX > Buffer->Width)
    {
        u32 PixelsToMoveLeft = ScreenMaxX - Buffer->Width;
        ScreenMaxX -= PixelsToMoveLeft;
        BitmapMaxX -= PixelsToMoveLeft;
    }
    if(ScreenMaxY > Buffer->Height)
    {
        u32 PixelsToMoveUp = ScreenMaxY - Buffer->Height;
        ScreenMaxY -= PixelsToMoveUp;
        BitmapMinY += PixelsToMoveUp;
    }

    u32 *ScreenRow = (u32 *)Buffer->Memory + (ScreenMinY * Buffer->Width) + ScreenMinX;
    u32 *BitmapRow = (u32 *)Bitmap->Pixels + (BitmapMaxY * Bitmap->Width) + BitmapMinX;
    for(int Y = ScreenMinY;
        Y < ScreenMaxY;
        ++Y)
    {
        u32 *Dest = ScreenRow;
        u32 *Source = BitmapRow;
        for(int X = ScreenMinX;
            X < ScreenMaxX;
            ++X)
        {
            u32 SourceAlpha = *((u8 *)Source + 3);
            u32 SourceRed = *((u8 *)Source + 2);
            u32 SourceGreen = *((u8 *)Source + 1);
            u32 SourceBlue = *((u8 *)Source + 0);

            u32 DestRed = *((u8 *)Dest + 2);
            u32 DestGreen = *((u8 *)Dest + 1);
            u32 DestBlue = *((u8 *)Dest + 0);

            real32 RealAlpha = (real32)SourceAlpha / 255.0f;

            u32 LerpRed = LerpWithUInts(DestRed, SourceRed, RealAlpha);
            u32 LerpGreen = LerpWithUInts(DestGreen, SourceGreen, RealAlpha);
            u32 LerpBlue = LerpWithUInts(DestBlue, SourceBlue, RealAlpha);

            u32 Color = ((LerpRed << 16) | (LerpGreen << 8) | (LerpBlue << 0));

            *Dest = Color;
            ++Dest;
            ++Source;
        }
        ScreenRow += Buffer->Width;
        BitmapRow -= Bitmap->Width;
    }
}

bitmap
DEBUGLoadBitmap(thread_context *Thread, debug_platform_read_entire_file *ReadEntireFile, char *Filename)
{
    bitmap ToReturn = {};
    debug_read_file_result ReadResult = ReadEntireFile(Thread, Filename);
    if(ReadResult.ContentsSize == 0)
    {
        return(ToReturn);
    }
    bitmap_header *BitmapHeader = (bitmap_header *)ReadResult.Contents;
    ToReturn.Size = BitmapHeader->Size;
    ToReturn.Width = BitmapHeader->Width;
    ToReturn.Height = BitmapHeader->Height;
    ToReturn.AlphaMask = (0xFF << 24);
    ToReturn.RedMask = BitmapHeader->RedMask;
    ToReturn.GreenMask = BitmapHeader->GreenMask;
    ToReturn.BlueMask = BitmapHeader->BlueMask;
    ToReturn.Pixels = ((u8 *)ReadResult.Contents + BitmapHeader->DataOffset);

    return(ToReturn);
}

void
RecanonicalizeCoordinate(tile_map *TileMapPointer, u32 *AbsTilePointer, real32 *TileOffsetPointer, u32 TilesInThisDimension)
{
    // e.g., player is on AbsTile 274, starts at center of tile, and moves 0.9 meters to the left.
    //      TileOffset = 0 - 0.9 = -0.9.
    //      HowManyTilesMoved = Round(-0.9 / 1.4f) = Round(-0.6428571429) = -1
    //      AbsTilePointer = 274 + (-1) = 273
    //
    //      *TileOffsetPointer = *TileOffsetPointer - (HowManyTilesMoved * TileSideInMeters)
    //      *TileOffsetPointer = -0.9 - (-1 * 1.4) = -0.9 - (-1.4) = -0.9 + 1.4 = 0.5
    
    s32 HowManyTilesMoved = RoundReal32ToS32(*TileOffsetPointer / TileMapPointer->TileSideInMeters);
    *AbsTilePointer += HowManyTilesMoved;
    *TileOffsetPointer -= (HowManyTilesMoved * TileMapPointer->TileSideInMeters);

    Assert(*AbsTilePointer < TilesInThisDimension);
    Assert(*TileOffsetPointer >= -(TileMapPointer->TileSideInMeters / 2));
    Assert(*TileOffsetPointer <= (TileMapPointer->TileSideInMeters / 2));
}

tile_map_position
GetCanonicalPosition(tile_map *TileMapPointer, tile_map_position OldPosition)
{
    tile_map_position NewPosition = OldPosition;

    // Y
    RecanonicalizeCoordinate(TileMapPointer, 
                             &NewPosition.AbsTileY, &NewPosition.TileOffset.Y, 
                             (TILES_IN_WORLD_Y));

    // X
    RecanonicalizeCoordinate(TileMapPointer, 
                             &NewPosition.AbsTileX, &NewPosition.TileOffset.X, 
                             (TILES_IN_WORLD_X));

    return(NewPosition);
}


void
InitializeArena(memory_arena *ArenaPointer, u8 *Base, memory_index Size)
{
    ArenaPointer->Base =Base;
    ArenaPointer->Size = Size;
    ArenaPointer->Used = 0;
}

void *
PushToArena_(memory_arena *ArenaPointer, memory_index NumBytes)
{
    Assert(ArenaPointer->Used + NumBytes <= ArenaPointer->Size);
    void *ToReturn = ArenaPointer->Base + ArenaPointer->Used;
    ArenaPointer->Used += NumBytes;
    return(ToReturn);
}

#define PushStruct(ArenaPointer, Type) (Type *)PushToArena_((ArenaPointer), sizeof(Type))
#define PushArray(ArenaPointer, Type, HowMany) (Type *)PushToArena_((ArenaPointer), sizeof(Type) * (HowMany))

inline u32
GetAbsTileFromRoomCoords(u32 Room, u32 OffsetFromBottomOfRoom, u32 TilesPerRoomInThisDimension)
{
    u32 ToReturn = Room * TilesPerRoomInThisDimension + OffsetFromBottomOfRoom;
    return(ToReturn);
}

inline room_position
GetRoomCoordsFromAbsTiles(tile_map *TileMapPointer, u32 AbsTileY, u32 AbsTileX)
{
    room_position ToReturn;
    ToReturn.RoomY = FloorReal32ToU32((real32)AbsTileY / (real32)TileMapPointer->TilesPerRoomY);
    ToReturn.RoomX = FloorReal32ToU32((real32)AbsTileX / (real32)TileMapPointer->TilesPerRoomX);
    ToReturn.TileInRoomY = AbsTileY - (ToReturn.RoomY * TileMapPointer->TilesPerRoomY);
    ToReturn.TileInRoomX = AbsTileX - (ToReturn.RoomX * TileMapPointer->TilesPerRoomX);
    return(ToReturn);
}

inline tile_chunk_position
GetChunkPositionFromAbsTiles(tile_map *TileMapPointer, u32 AbsTileY, u32 AbsTileX)
{
    tile_chunk_position ToReturn;
    ToReturn.ChunkY = AbsTileY >> TileMapPointer->ChunkMask;
    ToReturn.ChunkX = AbsTileX >> TileMapPointer->ChunkMask;
    ToReturn.TileInChunkY = AbsTileY & ((1 << TileMapPointer->ChunkMask) - 1);
    ToReturn.TileInChunkX = AbsTileX & ((1 << TileMapPointer->ChunkMask) - 1);
    return(ToReturn);
}

tile_chunk *
GetTileChunkPointerUnchecked(tile_map *TileMapPointer, u32 ChunkY, u32 ChunkX)
{
    tile_chunk *ToReturn = TileMapPointer->TileChunksArray + 
                            ChunkY * TileMapPointer->ChunksInMapX + 
                            ChunkX;
    return(ToReturn);
}

void
SetTileValue(memory_arena *ArenaPointer, tile_map *TileMapPointer, u32 AbsTileY, u32 AbsTileX, tile_value TileValue)
{
    Assert(AbsTileY < TILES_IN_WORLD_Y);
    Assert(AbsTileX < TILES_IN_WORLD_X);

    tile_chunk_position ChunkPosition = GetChunkPositionFromAbsTiles(TileMapPointer, AbsTileY, AbsTileX);
    Assert(ChunkPosition.ChunkY < TileMapPointer->ChunksInMapY);
    Assert(ChunkPosition.ChunkX < TileMapPointer->ChunksInMapX);
    Assert(ChunkPosition.TileInChunkY < TileMapPointer->ChunkDim);
    Assert(ChunkPosition.TileInChunkX < TileMapPointer->ChunkDim);

    tile_chunk *TileChunkPointer = GetTileChunkPointerUnchecked(TileMapPointer, ChunkPosition.ChunkY, ChunkPosition.ChunkX);
    if(!TileChunkPointer->TilesArray)
    {
        TileChunkPointer->TilesArray = PushArray(ArenaPointer, u32, TileMapPointer->ChunkDim * TileMapPointer->ChunkDim);
    }

    TileChunkPointer->TilesArray[ChunkPosition.TileInChunkY * 
        TileMapPointer->ChunkDim + 
        ChunkPosition.TileInChunkX] = (u32)TileValue;
}

tile_value
GetTileValue(memory_arena *ArenaPointer, tile_map *TileMapPointer, u32 AbsTileY, u32 AbsTileX)
{
    tile_value ToReturn = TILE_INVALID;
    Assert(AbsTileY < TILES_IN_WORLD_Y);
    Assert(AbsTileX < TILES_IN_WORLD_X);

    tile_chunk_position ChunkPosition = GetChunkPositionFromAbsTiles(TileMapPointer, AbsTileY, AbsTileX);
    Assert(ChunkPosition.ChunkY < TileMapPointer->ChunksInMapY);
    Assert(ChunkPosition.ChunkX < TileMapPointer->ChunksInMapX);
    Assert(ChunkPosition.TileInChunkY < TileMapPointer->ChunkDim);
    Assert(ChunkPosition.TileInChunkX < TileMapPointer->ChunkDim);

    tile_chunk *TileChunkPointer = GetTileChunkPointerUnchecked(TileMapPointer, ChunkPosition.ChunkY, ChunkPosition.ChunkX);
    if(!TileChunkPointer->TilesArray)
    {
        return(ToReturn);
    }
    else
    {
        ToReturn = (tile_value)TileChunkPointer->TilesArray[ChunkPosition.TileInChunkY * 
                                                            TileMapPointer->ChunkDim + 
                                                            ChunkPosition.TileInChunkX];
        return(ToReturn);
    }
}

void
MakeSimpleRoom(memory_arena *ArenaPointer, tile_map *TileMapPointer, u32 RoomY, u32 RoomX,
                bool32 DoorEast, bool32 DoorNorth, bool32 DoorWest, bool32 DoorSouth)
{
    for(u32 RelRow = 0;
        RelRow < TileMapPointer->TilesPerRoomY;
        ++RelRow)
    {
        for(u32 RelCol = 0;
            RelCol < TileMapPointer->TilesPerRoomX;
            ++RelCol)
        {
            // Set every tile to water unless:
            //      - top row
            //      - bottom row
            //      - left column
            //      - right column
            //  then go back over and set doors.
            tile_value TileValue = TILE_WATER;
            if( (RelRow == 0) || (RelRow == TileMapPointer->TilesPerRoomY - 1) ||
                (RelCol == 0) || (RelCol == TileMapPointer->TilesPerRoomX - 1) )
            {
                TileValue = TILE_TREE;
            }

            u32 ThisTileAbsY = GetAbsTileFromRoomCoords(RoomY, RelRow, TileMapPointer->TilesPerRoomY);
            u32 ThisTileAbsX = GetAbsTileFromRoomCoords(RoomX, RelCol, TileMapPointer->TilesPerRoomX);
            SetTileValue(ArenaPointer, TileMapPointer, ThisTileAbsY, ThisTileAbsX, TileValue);
        }
    }
    if(DoorEast)
    {
        u32 DoorEastAbsTileY = GetAbsTileFromRoomCoords(RoomY, (TileMapPointer->TilesPerRoomY / 2), TileMapPointer->TilesPerRoomY); 
        u32 DoorEastAbsTileX = GetAbsTileFromRoomCoords(RoomX, (TileMapPointer->TilesPerRoomX - 1), TileMapPointer->TilesPerRoomX); 
        SetTileValue(ArenaPointer, TileMapPointer, DoorEastAbsTileY, DoorEastAbsTileX, TILE_WATER);
    }
    if(DoorNorth)
    {
        // Note for DoorNorth and DoorSouth:
        //      Our addressing convention defines increasing Y values to be further away from the "bottom" of the world;
        //          as the player moves "up" on the screen, their AbsTileY value increases and they get further from
        //          the 0th address of the tile address space.
        //      Rooms "start" in their bottom row; as AbsTile values increase, we move further up in the room.
        //      Therefore, DoorNorth has as its Y coordinate:
        //          DoorNorthYCoord = RoomY * TilesPerRoomY + (TilesPerRoomY - 1)
        //          e.g., for room 11,
        //          DoorNorthYCoord = 11 * 9 + (9 - 1)
        //                          = 11 * 9 + 8 = 99 + 8 = 107
        u32 DoorNorthAbsTileY = GetAbsTileFromRoomCoords(RoomY, (TileMapPointer->TilesPerRoomY - 1), TileMapPointer->TilesPerRoomY); 
        u32 DoorNorthAbsTileX = GetAbsTileFromRoomCoords(RoomX, (TileMapPointer->TilesPerRoomX / 2), TileMapPointer->TilesPerRoomX); 
        SetTileValue(ArenaPointer, TileMapPointer, DoorNorthAbsTileY, DoorNorthAbsTileX, TILE_WATER);
    }
    if(DoorWest)
    {
        u32 DoorWestAbsTileY = GetAbsTileFromRoomCoords(RoomY, (TileMapPointer->TilesPerRoomY / 2), TileMapPointer->TilesPerRoomY); 
        u32 DoorWestAbsTileX = GetAbsTileFromRoomCoords(RoomX, 0, TileMapPointer->TilesPerRoomX); 
        SetTileValue(ArenaPointer, TileMapPointer, DoorWestAbsTileY, DoorWestAbsTileX, TILE_WATER);
    }
    if(DoorSouth)
    {
        u32 DoorSouthAbsTileY = GetAbsTileFromRoomCoords(RoomY, 0, TileMapPointer->TilesPerRoomY); 
        u32 DoorSouthAbsTileX = GetAbsTileFromRoomCoords(RoomX, (TileMapPointer->TilesPerRoomX / 2), TileMapPointer->TilesPerRoomX); 
        SetTileValue(ArenaPointer, TileMapPointer, DoorSouthAbsTileY, DoorSouthAbsTileX, TILE_WATER);
    }
}

bool32
IsTileAccessible(memory_arena *ArenaPointer, tile_map *TileMapPointer, u32 AbsTileY, u32 AbsTileX)
{
    tile_value TileValue = GetTileValue(ArenaPointer, TileMapPointer, AbsTileY, AbsTileX);
    // e.g., TileValue == 0, TILE_INVALID
    //      IsAccessible = ( (TileValue != TILE_INVALID) && (TileValue != TILE_TREE) )
    //                   = ( (TILE_INVALID != TILE_INVALID) && (TILE_INVALID != TILE_TREE) )
    //                   = ( 0 && 1)
    //                   = 0, false
    //
    // e.g., TileValue == TILE_WATER, 2
    //      IsAccessible = ( (TileValue != TILE_INVALID) && (TileValue != TILE_TREE) )
    //                   = ( (TILE_WATER != TILE_INVALID) && (TILE_WATER != TILE_TREE) )
    //                   = ( 1 && 1)
    //                   = 1, true
    bool IsAccessible = ( (TileValue != TILE_INVALID) && (TileValue != TILE_TREE) );
    return(IsAccessible);
}

u32
GetRandomNumber(u32 *GameStateRandomSeed, u32 LowerBound, u32 UpperBound)
{
    Assert(UpperBound > LowerBound);
    *GameStateRandomSeed ^= *GameStateRandomSeed << 13;
    *GameStateRandomSeed ^= *GameStateRandomSeed >> 17;
    *GameStateRandomSeed ^= *GameStateRandomSeed << 5;

    // UpperBound - LowerBound + 1 = the count of possible random numbers.
    //      e.g., LowerBound = 2, UpperBound = 7 means we want a random number
    //          between 2 and 7, inclusive of both 2 and 7.
    //      7 - 2 + 1 = 6; we want a random number that is one of 6 possible values:
    //          {2, 3, 4, 5, 6, 7}.

    u32 NumberOfValidResults = UpperBound - LowerBound + 1;
    u32 ToReturn = *GameStateRandomSeed % NumberOfValidResults;
    ToReturn += LowerBound;
    return(ToReturn);
}

real32
GetRandomReal(u32 *GameStateRandomSeed, real32 LowerBound, real32 UpperBound)
{
    Assert(UpperBound > LowerBound);
    // We're basically just adapting the C standard library's conventions for random numbers:
    //      apparently rand() generates random values in the range of 0 to the #define RAND_MAX,
    //      which is 0x7FFFFFFF (max value for a signed 32-bit int).
    //
    //      (Per this blog post: https://c-for-dummies.com/blog/?p=1458)
    //
    // But since we're only working with unsigned ints in our generator, we use 0xFFFFFFFF instead of 0x7FFFFFFF.
    // This is the maximum random number possible. Dividing our generated random number, cast to a real32, by this maximum
    //      random value gives us a value in the range of 0 to 1. We then use it to Lerp between the Lower and
    //      Upper bounds.

    u32 MaxRandomNumber = 0xFFFFFFFF;

    *GameStateRandomSeed ^= *GameStateRandomSeed << 13;
    *GameStateRandomSeed ^= *GameStateRandomSeed >> 17;
    *GameStateRandomSeed ^= *GameStateRandomSeed << 5;

    real32 TParameterForLerp = (real32)*GameStateRandomSeed / (real32)MaxRandomNumber;

    real32 ToReturn = LerpWithReal32s(LowerBound, UpperBound, TParameterForLerp);

    return(ToReturn);
}

vector2
GetVector2FromPolarCoordinates(real32 Argument, real32 Modulus)
{
    real32 X = Modulus * Cos(Argument);
    real32 Y = Modulus * Sin(Argument);

    vector2 Result = {X, Y};
    
    return(Result);
}

enum door
{
    DOOR_INVALID = 0,
    DOOR_EAST = 1,
    DOOR_NORTH = 2,
    DOOR_WEST = 3,
    DOOR_SOUTH = 4
};

enum door_status
{
    DOOR_STATUS_INVALID = 0,
    ADJACENT_ROOM_INVALID = 1,
    ADJACENT_ROOM_OCCUPIED = 2,
    ADJACENT_ROOM_AVAILABLE = 3
};

tile_value
GetTileValueFromRoomCoords(memory_arena *WorldArenaPointer, tile_map *TileMapPointer, room_position RoomPosition)
{
    u32 AbsTileY = GetAbsTileFromRoomCoords(RoomPosition.RoomY, 0, TileMapPointer->TilesPerRoomY);
    u32 AbsTileX = GetAbsTileFromRoomCoords(RoomPosition.RoomX, 0, TileMapPointer->TilesPerRoomX);
    tile_value TileValue = GetTileValue(WorldArenaPointer, TileMapPointer, AbsTileY, AbsTileX);
    return(TileValue);
}

inline bool32
IsRoomPositionInsideTileMap(tile_map *TileMapPointer, room_position RoomPosition)
{
    bool32 IsInsideTileMap = (
                              (RoomPosition.RoomY >= 0) &&
                              (RoomPosition.RoomY < TileMapPointer->RoomsInMapY) &&
                              (RoomPosition.RoomX >= 0) &&
                              (RoomPosition.RoomX < TileMapPointer->RoomsInMapX)
                             );
    return(IsInsideTileMap);
}


void
GetDoorStatuses(memory_arena *WorldArenaPointer, tile_map *TileMapPointer, u32 *RandomSeedPointer, 
         door_status *DoorStatusArray, room_position ThisRoomPosition)
{
    // if a door must be set because there is an adjacent room, set that door's value to 1.
    // if a door must not be set because it would be beyond the tile map, set that door's value to -1.
    // if a door is available for 

    room_position EastRoomPosition = ThisRoomPosition;
    EastRoomPosition.RoomX += 1;
    if(IsRoomPositionInsideTileMap(TileMapPointer, EastRoomPosition) == false)
    {
        DoorStatusArray[DOOR_EAST] = ADJACENT_ROOM_INVALID;
    }
    else
    {
        tile_value EastRoomBottomLeftTile = GetTileValueFromRoomCoords(WorldArenaPointer, TileMapPointer, EastRoomPosition);
        if(EastRoomBottomLeftTile == TILE_TREE)
        {
            DoorStatusArray[DOOR_EAST] = ADJACENT_ROOM_OCCUPIED;
        }
        else
        {
            DoorStatusArray[DOOR_EAST] = ADJACENT_ROOM_AVAILABLE;
        }
    }

    room_position NorthRoomPosition = ThisRoomPosition;
    NorthRoomPosition.RoomY += 1;
    if(IsRoomPositionInsideTileMap(TileMapPointer, NorthRoomPosition) == false)
    {
        DoorStatusArray[DOOR_NORTH] = ADJACENT_ROOM_INVALID;
    }
    else
    {
        tile_value NorthRoomBottomLeftTile = GetTileValueFromRoomCoords(WorldArenaPointer, TileMapPointer, NorthRoomPosition);
        if(NorthRoomBottomLeftTile == TILE_TREE)
        {
            DoorStatusArray[DOOR_NORTH] = ADJACENT_ROOM_OCCUPIED;
        }
        else
        {
            DoorStatusArray[DOOR_NORTH] = ADJACENT_ROOM_AVAILABLE;
        }
    }

    room_position WestRoomPosition = ThisRoomPosition;
    WestRoomPosition.RoomX -= 1;
    if(IsRoomPositionInsideTileMap(TileMapPointer, WestRoomPosition) == false)
    {
        DoorStatusArray[DOOR_WEST] = ADJACENT_ROOM_INVALID;
    }
    else
    {
        tile_value WestRoomBottomLeftTile = GetTileValueFromRoomCoords(WorldArenaPointer, TileMapPointer, WestRoomPosition);
        if(WestRoomBottomLeftTile == TILE_TREE)
        {
            DoorStatusArray[DOOR_WEST] = ADJACENT_ROOM_OCCUPIED;
        }
        else
        {
            DoorStatusArray[DOOR_WEST] = ADJACENT_ROOM_AVAILABLE;
        }
    }   

    room_position SouthRoomPosition = ThisRoomPosition;
    SouthRoomPosition.RoomY -= 1;
    if(IsRoomPositionInsideTileMap(TileMapPointer, SouthRoomPosition) == false)
    {
        DoorStatusArray[DOOR_SOUTH] = ADJACENT_ROOM_INVALID;
    }
    else
    {
        tile_value SouthRoomBottomLeftTile = GetTileValueFromRoomCoords(WorldArenaPointer, TileMapPointer, SouthRoomPosition);
        if(SouthRoomBottomLeftTile == TILE_TREE)
        {
            DoorStatusArray[DOOR_SOUTH] = ADJACENT_ROOM_OCCUPIED;
        }
        else
        {
            DoorStatusArray[DOOR_SOUTH] = ADJACENT_ROOM_AVAILABLE;
        }
    }   
}

u32
MakePathOfNRooms(memory_arena *WorldArenaPointer, u32 *RandomSeedPointer, tile_map *TileMapPointer, 
                 u32 RoomsRemaining, u32 FirstRoomY, u32 FirstRoomX)
{
    --RoomsRemaining;
    if(RoomsRemaining == 0)
    {
        return(0);
    }

    // We only need to set Room X and Y coordinates here because we initialize the struct
    //      to zero and we want TileInRoom X and Y fields to both be zero
    room_position RoomPosition = {0};
    RoomPosition.RoomY = FirstRoomY;
    RoomPosition.RoomX = FirstRoomX;

    door_status DoorStatusArray[5] = {(door_status)0};
    GetDoorStatuses(WorldArenaPointer, TileMapPointer, RandomSeedPointer, 
                         (door_status *)DoorStatusArray, RoomPosition);

    
    // Now set values for the doors in this room for walls which abut the edge of the
    //      tilemap and for walls whose doors should connect to an adjacent room
    bool32 DoorValues[5] = {0};
    for(door Door = DOOR_EAST;
        Door <= DOOR_SOUTH;
        Door = (door)(Door + 1))
    {
        if(DoorStatusArray[Door] == ADJACENT_ROOM_OCCUPIED)
        {
            DoorValues[Door] = true;
        }
    }

    // Check to see if every wall either abuts tilemap edge or has a door
    //      connecting to an adjacent room. If so, we have hit a dead end
    //      and cannot make any more rooms on this pass. Make the current
    //      room and return remaining rooms.
    bool32 CanMakeNextRoom = false;
    for(door Door = DOOR_EAST;
        Door <= DOOR_SOUTH;
        Door = (door)(Door + 1))
    {
        if(DoorStatusArray[Door] == ADJACENT_ROOM_AVAILABLE)
        {
            CanMakeNextRoom = true;
            break;
        }
    }
    if(CanMakeNextRoom == false)
    {
        MakeSimpleRoom(WorldArenaPointer, TileMapPointer, FirstRoomY, FirstRoomX,
                       DoorValues[DOOR_EAST], DoorValues[DOOR_NORTH], DoorValues[DOOR_WEST], DoorValues[DOOR_SOUTH]);
        return(RoomsRemaining);
    }

    // Otherwise there is at least one door with a nonadjacent room; pick one of these to make the next room off of.
    //      Keep generating random indices into the doors array until we find a door that hasn't been set yet.
    door DoorToNextRoom = (door)GetRandomNumber(RandomSeedPointer, 1, 4);
    while( (DoorStatusArray[DoorToNextRoom] == ADJACENT_ROOM_INVALID) || 
           (DoorStatusArray[DoorToNextRoom] == ADJACENT_ROOM_OCCUPIED) )
    {
        DoorToNextRoom = (door)GetRandomNumber(RandomSeedPointer, 1, 4); 
    }

    // Set the door to the next room to be true and make the current room
    DoorValues[DoorToNextRoom] = true;
    MakeSimpleRoom(WorldArenaPointer, TileMapPointer, FirstRoomY, FirstRoomX,
                   DoorValues[DOOR_EAST], DoorValues[DOOR_NORTH], DoorValues[DOOR_WEST], DoorValues[DOOR_SOUTH]);

    // Set the room coordinates for the next room to create and call the function again
    if(DoorToNextRoom == DOOR_EAST)
    {
        FirstRoomX += 1;
    }
    if(DoorToNextRoom == DOOR_NORTH)
    {
        FirstRoomY += 1;
    }
    if(DoorToNextRoom == DOOR_WEST)
    {
        FirstRoomX -= 1;
    }
    if(DoorToNextRoom == DOOR_SOUTH)
    {
        FirstRoomY -= 1;
    }
    u32 RoomsMade = MakePathOfNRooms(WorldArenaPointer, RandomSeedPointer, TileMapPointer, 
                                         RoomsRemaining, FirstRoomY, FirstRoomX);
    return(RoomsMade);
}

tile_screen_coordinates
GetScreenCoordinatesForRoomView(vector2 Origin, s32 RelTileX, s32 RelTileY, real32 TileSideInPixels)
{
    tile_screen_coordinates Result;

    Result.Min.X = Origin.X + RelTileX * TileSideInPixels;
    Result.Max.Y = Origin.Y - RelTileY * TileSideInPixels;
    Result.Max.X = Result.Min.X + TileSideInPixels;
    Result.Min.Y = Result.Max.Y - TileSideInPixels;

    return(Result);
}

tile_screen_coordinates
GetScreenCoordinatesForBirdsEyeView(u32 ScreenCenterX, u32 ScreenCenterY, s32 RelTileX, s32 RelTileY, real32 TileSideInPixels)
{
    tile_screen_coordinates Result;
    
    Result.Min.X = ScreenCenterX - (TileSideInPixels * 0.5f) + (RelTileX * TileSideInPixels);
    Result.Max.X = Result.Min.X + TileSideInPixels;

    Result.Max.Y = ScreenCenterY + (TileSideInPixels * 0.5f) - (RelTileY * TileSideInPixels);
    Result.Min.Y = Result.Max.Y - TileSideInPixels;

    return(Result);
}

tile_screen_coordinates
GetScreenCoordinatesForRelTile(vector2 Origin, s32 RelTileX, s32 RelTileY, real32 TileSideInPixels)
{
    tile_screen_coordinates Result;

    Result.Min.X = Origin.X + (RelTileX * TileSideInPixels);
    Result.Max.Y = Origin.Y - (RelTileY * TileSideInPixels);
    Result.Max.X = Result.Min.X + TileSideInPixels;
    Result.Min.Y = Result.Max.Y - TileSideInPixels;
    
    return(Result);
}

void
BlitBitmapWithNearestNeighborAndBlend(game_offscreen_buffer *Buffer, vector2 Min, vector2 Max, bitmap *Bitmap) 
{

    s32 ScreenMinX = RoundReal32ToS32(Min.X);
    s32 ScreenMinY = RoundReal32ToS32(Min.Y);
    s32 ScreenMaxX = RoundReal32ToS32(Max.X);
    s32 ScreenMaxY = RoundReal32ToS32(Max.Y);

    s32 ClippedScreenMinX = ScreenMinX;
    s32 ClippedScreenMinY = ScreenMinY;
    s32 ClippedScreenMaxX = ScreenMaxX;
    s32 ClippedScreenMaxY = ScreenMaxY;

    u32 BitmapMinX = 0;
    u32 BitmapMinY = 0;
    u32 BitmapMaxX = Bitmap->Width - 1;
    u32 BitmapMaxY = Bitmap->Height - 1;

    if(ScreenMinX < 0)
    {
        s32 PixelsToMoveRight = abs(ScreenMinX);
        ClippedScreenMinX += PixelsToMoveRight;
    }
    if(ScreenMinY < 0)
    {
        s32 PixelsToMoveDown = abs(ScreenMinY);
        ClippedScreenMinY += PixelsToMoveDown;
    }
    if(ScreenMaxX > Buffer->Width)
    {
        s32 PixelsToMoveLeft = ScreenMaxX - Buffer->Width;
        ClippedScreenMaxX -= PixelsToMoveLeft;
    }
    if(ScreenMaxY > Buffer->Height)
    {
        s32 PixelsToMoveUp = ScreenMaxY - Buffer->Height;
        ClippedScreenMaxY -= PixelsToMoveUp;
    }

    u32 *ScreenRow = (u32 *)Buffer->Memory + (ClippedScreenMinY * Buffer->Width) + ClippedScreenMinX;
    for(int Y = ClippedScreenMinY;
        Y < ClippedScreenMaxY;
        ++Y)
    {
        u32 *Dest = ScreenRow;
        real32 NormalizedYCoord = ((real32)Y - ScreenMinY) / ((real32)ScreenMaxY - ScreenMinY);
        u32 NormalizedYCoordMappedToBitmap = BitmapMaxY - RoundReal32ToS32(((real32)BitmapMaxY * NormalizedYCoord));
        Assert(NormalizedYCoordMappedToBitmap >= BitmapMinY);
        Assert(NormalizedYCoordMappedToBitmap <= BitmapMaxY);
        for(int X = ClippedScreenMinX;
            X < ClippedScreenMaxX;
            ++X)
        {
            real32 NormalizedXCoord = ((real32)X - ScreenMinX) / ((real32)ScreenMaxX - ScreenMinX);
            u32 NormalizedXCoordMappedToBitmap = RoundReal32ToS32(((real32)BitmapMaxX * NormalizedXCoord));
            Assert(NormalizedXCoordMappedToBitmap >= BitmapMinX);
            Assert(NormalizedXCoordMappedToBitmap <= BitmapMaxX);
            u32 *SourcePixel = (u32 *)Bitmap->Pixels + (NormalizedYCoordMappedToBitmap * Bitmap->Width) + NormalizedXCoordMappedToBitmap;
        
            u32 SourceAlpha = *((u8 *)SourcePixel + 3);
            u32 SourceRed = *((u8 *)SourcePixel + 2);
            u32 SourceGreen = *((u8 *)SourcePixel + 1);
            u32 SourceBlue = *((u8 *)SourcePixel + 0);

            u32 DestRed = *((u8 *)Dest + 2);
            u32 DestGreen = *((u8 *)Dest + 1);
            u32 DestBlue = *((u8 *)Dest + 0);

            real32 RealAlpha = (real32)SourceAlpha / 255.0f;

            u32 LerpRed = LerpWithUInts(DestRed, SourceRed, RealAlpha);
            u32 LerpGreen = LerpWithUInts(DestGreen, SourceGreen, RealAlpha);
            u32 LerpBlue = LerpWithUInts(DestBlue, SourceBlue, RealAlpha);

            u32 Color = ((LerpRed << 16) | (LerpGreen << 8) | (LerpBlue << 0));

            *Dest = Color;
            ++Dest;
        }
        ScreenRow += Buffer->Width;
    }
}


#if defined __cplusplus
extern "C"
#endif
// #define GAME_UPDATE_AND_RENDER(name) void name(thread_context *Thread, game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if(!Memory->IsInitialized)
    {
        GameState->Tree = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "tree.bmp");
        GameState->TreeSmall = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "tree_small.bmp");
        GameState->Water = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "water.bmp");
        GameState->WaterSmall = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "water_small.bmp");
        GameState->BeaverRight = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "beaver_right.bmp");
        GameState->BeaverUp = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "beaver_up.bmp");
        GameState->BeaverLeft = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "beaver_left.bmp");
        GameState->BeaverDown = DEBUGLoadBitmap(Thread, Memory->DEBUGPlatformReadEntireFile, "beaver_down.bmp");

        GameState->RandomSeed = Memory->RandomSeed;
        GameState->BirdsEye = false;
        InitializeArena(&GameState->WorldArena, 
                        (u8 *)Memory->PermanentStorage + sizeof(game_state),
                        Memory->PermanentStorageSize - sizeof(game_state));
        GameState->WorldPointer = PushStruct(&GameState->WorldArena, world);
        world *WorldPointer = GameState->WorldPointer;
        WorldPointer->TileMapPointer = PushStruct(&GameState->WorldArena, tile_map);
        tile_map *TileMapPointer = WorldPointer->TileMapPointer;
        TileMapPointer->ChunkMask = 4;
        TileMapPointer->ChunkDim = 16;
        TileMapPointer->TileSideInMeters = 1.4f;
        TileMapPointer->TileSideInPixels = 64;
        TileMapPointer->RoomsInMapY = 21;
        TileMapPointer->RoomsInMapX = 21;
        TileMapPointer->TilesPerRoomY = 13;
        TileMapPointer->TilesPerRoomX = 17;

        TileMapPointer->ChunksInMapY = ((TileMapPointer->RoomsInMapY * TileMapPointer->TilesPerRoomY) / TileMapPointer->ChunkDim) + 1;
        TileMapPointer->ChunksInMapX = ((TileMapPointer->RoomsInMapX * TileMapPointer->TilesPerRoomX) / TileMapPointer->ChunkDim) + 1;

        TileMapPointer->TileChunksArray = PushArray(&GameState->WorldArena, tile_chunk, 
                                                    TileMapPointer->ChunksInMapY * TileMapPointer->ChunksInMapX);

        u32 CenterRoomYCoord = 10;
        u32 CenterRoomXCoord = 10;
        u32 RoomsPerPath = 20;
        // Center room at Y = 10, X = 10                                                            E     N     W     S
        MakeSimpleRoom(&GameState->WorldArena, TileMapPointer, CenterRoomYCoord, CenterRoomXCoord, true, true, true, true);

        // Place a random tree on a random tile.
        //      First generate a random angle in the range of 0 to 2 pi, which we call the argument
        //      since we are using the parlance of polar coordinates.
        real32 Argument = GetRandomReal(&GameState->RandomSeed, 0, (PI * 2.0f));
        
        // How many meters are there in the world from left to right?
        real32 MetersInWorldX = TileMapPointer->RoomsInMapX * TileMapPointer->TilesPerRoomX * TileMapPointer->TileSideInMeters;

        // How many meters are there in the world from top to bottom?
        real32 MetersInWorldY = TileMapPointer->RoomsInMapY * TileMapPointer->TilesPerRoomY * TileMapPointer->TileSideInMeters;

        // Since there are fewer meters in the world on the Y axis, let's constrain the distance from the home screen
        //      to a grove center to be three-quarters the distance from the home screen to the top or bottom of the world.
        //      Again, we use modulus since we are thinking in terms of polar coordinates.
        real32 ModulusMax = MetersInWorldY * 0.5f * 0.75f;

        // And let's set the minimum modulus to be at least two screen-heighths from the home screen.
        real32 ModulusMin = TileMapPointer->TilesPerRoomY * TileMapPointer->TileSideInMeters;

        // Get a modulus within those boundaries:
        real32 Modulus = GetRandomReal(&GameState->RandomSeed, ModulusMin, ModulusMax);

        // Convert polar coordinates to X and Y coordinates
        vector2 TreeVector = GetVector2FromPolarCoordinates(Argument, (TileMapPointer->TilesPerRoomY * TileMapPointer->TileSideInMeters));

        // Find the middle tile of the world, or the origin in our coordinate system
        u32 MiddleTileX = TileMapPointer->RoomsInMapX * TileMapPointer->TilesPerRoomX / 2;
        u32 MiddleTileY = TileMapPointer->RoomsInMapY * TileMapPointer->TilesPerRoomY / 2;
        tile_map_position OriginTile = {MiddleTileX, MiddleTileY};

        // Get the tree tile's coordinates in the tilemap by using TreeVector as an offset from the middle
        //      tile, i.e., the origin of the coordinate system
        u32 TreeTileX = MiddleTileX + RoundReal32ToS32(TreeVector.X);
        Assert(TreeTileX < TileMapPointer->RoomsInMapX * TileMapPointer->TilesPerRoomX);
        u32 TreeTileY = MiddleTileY + RoundReal32ToS32(TreeVector.Y);
        Assert(TreeTileY < TileMapPointer->RoomsInMapY * TileMapPointer->TilesPerRoomY);
        tile_map_position TreeTile = {TreeTileX, TreeTileY};

        SetTileValue(&GameState->WorldArena, TileMapPointer, TreeTile.AbsTileY, TreeTile.AbsTileX, TILE_TREE);


        









#if 0
        // Path off of east room
        MakePathOfNRooms(&GameState->WorldArena, &GameState->RandomSeed, TileMapPointer, RoomsPerPath, 10, 11);

        // Path off of north room
        MakePathOfNRooms(&GameState->WorldArena, &GameState->RandomSeed, TileMapPointer, RoomsPerPath, 11, 10);

        // Path off of west room
        MakePathOfNRooms(&GameState->WorldArena, &GameState->RandomSeed, TileMapPointer, RoomsPerPath, 10, 9);
        
        // Path off of south room
        MakePathOfNRooms(&GameState->WorldArena, &GameState->RandomSeed, TileMapPointer, RoomsPerPath, 9, 10);
#endif
        GameState->PlayerPosition.AbsTileY = GetAbsTileFromRoomCoords(CenterRoomYCoord, 4, TileMapPointer->TilesPerRoomY);
        GameState->PlayerPosition.AbsTileX = GetAbsTileFromRoomCoords(CenterRoomXCoord, 4, TileMapPointer->TilesPerRoomX);
        GameState->PlayerPosition.TileOffset.Y = 0.6f;
        GameState->PlayerPosition.TileOffset.X = -0.4f;
        GameState->PlayerFacing = FACING_RIGHT;

        Memory->IsInitialized = true;
    }

    tile_map *TileMapPointer = GameState->WorldPointer->TileMapPointer;

    for(int ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        if(Controller->IsAnalog)
        {
        }
        else
        {
            if( (Controller->ActionDown.EndedDown) && 
                (Controller->ActionDown.HalfTransitionCount == 1) )
            {
                GameState->BirdsEye = !GameState->BirdsEye;
            }

            vector2 ddPlayer = {};

            if(Controller->MoveRight.EndedDown)
            {
                ddPlayer.X += 1.0f;
                GameState->PlayerFacing = FACING_RIGHT;
            }
            if(Controller->MoveUp.EndedDown)
            {
                ddPlayer.Y += 1.0f;
                GameState->PlayerFacing = FACING_UP;
            }
            if(Controller->MoveLeft.EndedDown)
            {
                ddPlayer.X -= 1.0f;
                GameState->PlayerFacing = FACING_LEFT;
            }
            if(Controller->MoveDown.EndedDown)
            {
                ddPlayer.Y -= 1.0f;
                GameState->PlayerFacing = FACING_DOWN;
            }

            if( (ddPlayer.X != 0.0f) && (ddPlayer.Y != 0.0f) )
            {
                ddPlayer *= 0.707106781187f;
            }

            real32 PlayerSpeed = 10.0f;
            if(Controller->Back.EndedDown)
            {
                PlayerSpeed = 50.0f;
            }
            ddPlayer *= PlayerSpeed;

            ddPlayer += -0.9f*GameState->dPlayerP;

            tile_map_position TestPlayerBase = GameState->PlayerPosition;
            TestPlayerBase.TileOffset = (0.5f * ddPlayer * Square(Input->dtForFrame) + // Integrate acceleration with time for change in velocity
                                          GameState->dPlayerP * Input->dtForFrame + // Initial velocity
                                          TestPlayerBase.TileOffset);               // And offset the current position

            GameState->dPlayerP = ddPlayer * Input->dtForFrame + GameState->dPlayerP; // Store player's new velocity as the current velocity
                                                                                    //      for next frame.
            TestPlayerBase = GetCanonicalPosition(TileMapPointer, TestPlayerBase);

            tile_map_position TestPlayerLeft = TestPlayerBase;
            TestPlayerLeft.TileOffset.X -= (PLAYER_WIDTH * 0.5f);
            TestPlayerLeft = GetCanonicalPosition(TileMapPointer, TestPlayerLeft);

            tile_map_position TestPlayerRight = TestPlayerBase;
            TestPlayerRight.TileOffset.X += (PLAYER_WIDTH * 0.5f);
            TestPlayerRight = GetCanonicalPosition(TileMapPointer, TestPlayerRight);

            bool32 Collided = false;
            tile_map_position CollisionPoint;
            if(!IsTileAccessible(&GameState->WorldArena, TileMapPointer, 
                                 TestPlayerBase.AbsTileY, TestPlayerBase.AbsTileX))
            {
                Collided = true;
                CollisionPoint = TestPlayerBase;
            }
            if(!IsTileAccessible(&GameState->WorldArena, TileMapPointer, 
                                 TestPlayerLeft.AbsTileY, TestPlayerLeft.AbsTileX))
            {
                Collided = true;
                CollisionPoint = TestPlayerLeft;
            }
            if(!IsTileAccessible(&GameState->WorldArena, TileMapPointer, 
                                 TestPlayerRight.AbsTileY, TestPlayerRight.AbsTileX))
            {
                Collided = true;
                CollisionPoint = TestPlayerRight;
            }
            if(Collided)
            {
                vector2 NormalFromWall;
                if(CollisionPoint.AbsTileX < GameState->PlayerPosition.AbsTileX)
                {
                    NormalFromWall = {-1, 0};
                }
                if(CollisionPoint.AbsTileX > GameState->PlayerPosition.AbsTileX)
                {
                    NormalFromWall = {1, 0};
                }
                if(CollisionPoint.AbsTileY < GameState->PlayerPosition.AbsTileY)
                {
                    NormalFromWall = {0, 1};
                }
                if(CollisionPoint.AbsTileY > GameState->PlayerPosition.AbsTileY)
                {
                    NormalFromWall = {0, -1};
                }
                GameState->dPlayerP = GameState->dPlayerP - 
                    Inner(GameState->dPlayerP, NormalFromWall) * 
                    NormalFromWall;
            }
            else
            {
                GameState->PlayerPosition = TestPlayerBase;
            }
        }
    }

    // Draw underlayer
    vector2 ScreenMin = {0, 0};
    vector2 ScreenMax = {(real32)Buffer->Width, (real32)Buffer->Height};
    DrawRectangle(Buffer,
                  ScreenMin, ScreenMax,
                  0.75f, 0.25f, 0.5f);

    // Note that for all rendering, we take Y to be a "bottom-up" value. 
    //      e.g., the row of tiles at the extreme south of the tilemap has a Y value of zero.
    //  However, we get a buffer of pixels to draw into that positions the row with a Y value of
    //      zero at the TOP of the screen. i.e., the top-left pixel on the screen has address zero.
    //  So when we go to compute a screen coordinate, we compute an origin (either the center
    //      of the screen or the bottom-left), and then Y values are SUBTRACTED from the origin.
    //  Whereas X values are added to the origin.
    //  e.g., If a tile has a row value of, say, 3, relative to the bottom of the tilemap,
    //      and we say that we have 10 total rows of tiles, that tile's row in
    //      terms of screen coordinates would be 10 - 3 = 7.
    //
    if(GameState->BirdsEye == true)
    {
        // Draw tiles
        TileMapPointer->TileSideInPixels = 16;
        real32 MetersToPixels = (real32)TileMapPointer->TileSideInPixels / TileMapPointer->TileSideInMeters;
        vector2 Origin = {((real32)Buffer->Width * 0.5f), ((real32)Buffer->Height * 0.5f)};

        // How the tiles are rendered when in birdseye view:
        //      Unlike room view (rendering loop below), we draw the player at the center of the screen and never move them.
        //      Instead of the player moving relative to the tilemap, we scroll the tilemap beneath the player.
        //      Therefore, the tile the player is on is drawn at the very center of the screen. 
        //      RelTile Y and X (below) mean "The tiles relative to the center of the screen." e.g.,
        //      RelTile Y = -1, RelTileY = 3 means the tile that is:
        //          In the row below the player's current row
        //          Three columns to the right of the player's current column.

        for(int RelTileY = -40;
            RelTileY < 40;
            ++RelTileY)
        {
            for(int RelTileX = -80;
                RelTileX < 80;
                ++RelTileX)
            {
                tile_map_position *PlayerPosition = &GameState->PlayerPosition;

                // Get tiles starting from the bottom-left relative to the player's position in tile map memory
                u32 TileToDrawX = PlayerPosition->AbsTileX + RelTileX;
                u32 TileToDrawY = PlayerPosition->AbsTileY + RelTileY;
                tile_value TileValue = GetTileValue(&GameState->WorldArena,
                                                    TileMapPointer,
                                                    TileToDrawY,
                                                    TileToDrawX);
                if(TileValue != TILE_INVALID)
                {

                    // Here we determine where to render the tile whose value we just looked up in memory.
                    // Remember: The player's tile is always the one at the center of the screen. So RelTileY = 0, RelTileX = 0
                    //      means the tile that the player is on, and this tile should be drawn at the center of the screen.
                    //      Since we currently render at 1024 by 768, this means an X value of 1024 / 2 = 512 and 
                    //          a Y value of 768 / 2 = 384.
                    //      And since we want that tile to be centered on the screen, we want the center of the tile to be at
                    //          the center of the screen. So for the middle tile, this calculation
                    //
                    //          TileMin.X = Origin.X - (TileMapPointer->TileSideInPixels * 0.5f) + (TileMapPointer->TileSideInPixels * RelTileX);
                    //          would be
                    //          Left edge of tile = 512 - 8 + (16 * 0) = 504
                    //
                    //
                    //          For the tile just to the left of the player, it would be
                    //          Left edge of tile = 512 - 8 + (16 * -1) = 512 - 8 - 16 = 488
                    //
                    //          etc.

                    tile_screen_coordinates TileScreenCoords = GetScreenCoordinatesForBirdsEyeView(Buffer->Width * 0.5f,
                                                                                                   Buffer->Height * 0.5f,
                                                                                                   RelTileX, 
                                                                                                   RelTileY,
                                                                                                   TileMapPointer->TileSideInPixels);


#if 0
                    vector2 TileMin;
                    TileMin.X = Origin.X - (TileMapPointer->TileSideInPixels * 0.5f) + (TileMapPointer->TileSideInPixels * RelTileX);
                    TileMin.Y = Origin.Y - (TileMapPointer->TileSideInPixels * 0.5f) - (TileMapPointer->TileSideInPixels * RelTileY);
                    vector2 TileMax;
                    TileMax.X = TileMin.X + (TileMapPointer->TileSideInPixels);
                    TileMax.Y = TileMin.Y + (TileMapPointer->TileSideInPixels);
#endif

                    // To scroll the screen, we then OFFSET that tile position by how far the player has moved relative to the
                    //      center of the tile they're currently on. e.g., if the player's tile offset is X = 0.3, Y = -0.6,
                    //      (relative to the center of the tile), their location is 0.3 meters to the right of the tile's center,
                    //      and -0.6 meters below the tile's center.
                    //
                    //  If we were moving the player instead of scrolling the tile map, we would just add those offsets to 
                    //      the position at which we are rendering them. But since we are keeping them stationary
                    //      and scrolling the tilemap instead, we need to move the TILE's rendering position by that
                    //      same amount, but in reverse. e.g., if a player was going to move to the right in a tile by
                    //      a half of a meter, we could either add half a meter to their position, or bring that
                    //      point that is a half-meter away TO THE PLAYER, instead, by moving the tile's position
                    //      by half a meter, TOWARD THE PLAYER.
                    //
                    //      Hence below the OffsetX values are subtracted from the tile's position -- we're moving
                    //      the tile TOWARD the player.

                    TileScreenCoords.Min.X -= (PlayerPosition->TileOffset.X * MetersToPixels);
                    TileScreenCoords.Max.X -= (PlayerPosition->TileOffset.X * MetersToPixels);
                    TileScreenCoords.Min.Y += (PlayerPosition->TileOffset.Y * MetersToPixels);
                    TileScreenCoords.Max.Y += (PlayerPosition->TileOffset.Y * MetersToPixels);
                    if( (PlayerPosition->AbsTileY == TileToDrawY) && (PlayerPosition->AbsTileX == TileToDrawX) )
                    {
                        real32 TileR = 0.25f;
                        real32 TileG = 0.25f;
                        real32 TileB = 0.25f;
                        DrawRectangle(Buffer, TileScreenCoords.Min, TileScreenCoords.Max, TileR, TileG, TileB);
                    }
                    else
                    {
                        BlitBitmap(Buffer, TileScreenCoords.Min, TileScreenCoords.Max, &GameState->WaterSmall);
                        if(TileValue == TILE_TREE)
                        {
                            BlitBitmapAndBlend(Buffer, TileScreenCoords.Min, TileScreenCoords.Max, &GameState->TreeSmall);
                        }
                    }
                }
            }
        }

        // Draw player
        real32 PlayerScreenCoordMinY = Origin.Y - (PLAYER_HEIGHT * 0.5f * MetersToPixels);
        real32 PlayerScreenCoordMaxY = PlayerScreenCoordMinY + (PLAYER_HEIGHT * MetersToPixels);
        real32 PlayerScreenCoordMinX = Origin.X - (PLAYER_WIDTH * 0.5f * MetersToPixels);
        real32 PlayerScreenCoordMaxX = PlayerScreenCoordMinX + (PLAYER_WIDTH * MetersToPixels);
        vector2 PlayerMin = {PlayerScreenCoordMinX, PlayerScreenCoordMinY};
        vector2 PlayerMax = {PlayerScreenCoordMaxX, PlayerScreenCoordMaxY};

        bitmap *PlayerSprite;
        switch(GameState->PlayerFacing)
        {
            case(FACING_RIGHT):
                {
                    PlayerSprite = &GameState->BeaverRight;
                } break;

            case(FACING_UP):
                {
                    PlayerSprite = &GameState->BeaverUp;
                } break;

            case(FACING_LEFT):
                {
                    PlayerSprite = &GameState->BeaverLeft;
                } break;

            case(FACING_DOWN):
                {
                    PlayerSprite = &GameState->BeaverDown;
                } break;
        }
        BlitBitmapWithNearestNeighborAndBlend(Buffer, PlayerMin, PlayerMax, PlayerSprite);

    }
    else
    {
        TileMapPointer->TileSideInPixels = 64;
        real32 MetersToPixels = (real32)TileMapPointer->TileSideInPixels / TileMapPointer->TileSideInMeters;

        // Start drawing a half-tile width outside of the screen so that the walls are cut in half and actually
        //      look like walls.
        real32 OffsetPlayfieldXBy = -((real32)TileMapPointer->TileSideInPixels * 0.5f);
        real32 OffsetPlayfieldYBy = (real32)TileMapPointer->TileSideInPixels * 0.5f;
        
        // Draw tiles. Origin is X = 0, Y = 0.
        vector2 Origin = {OffsetPlayfieldXBy, ((real32)Buffer->Height + OffsetPlayfieldYBy)};
        room_position PlayerRoom = GetRoomCoordsFromAbsTiles(TileMapPointer, 
                                                             GameState->PlayerPosition.AbsTileY,
                                                             GameState->PlayerPosition.AbsTileX);
        for(u32 RelTileY = 0;
            RelTileY < TileMapPointer->TilesPerRoomY;
            ++RelTileY)
        {
            for(u32 RelTileX = 0;
                RelTileX < TileMapPointer->TilesPerRoomX;
                ++RelTileX)
            {
                u32 ThisTileAbsY = GetAbsTileFromRoomCoords(PlayerRoom.RoomY, RelTileY, TileMapPointer->TilesPerRoomY);
                u32 ThisTileAbsX = GetAbsTileFromRoomCoords(PlayerRoom.RoomX, RelTileX, TileMapPointer->TilesPerRoomX);

                tile_screen_coordinates TileScreenCoords = GetScreenCoordinatesForRoomView(Origin,
                                                                                           RelTileX, RelTileY,
                                                                                           TileMapPointer->TileSideInPixels);

                tile_value TileValue = GetTileValue(&GameState->WorldArena, TileMapPointer, ThisTileAbsY, ThisTileAbsX);
                if( (RelTileY == PlayerRoom.TileInRoomY) && (RelTileX == PlayerRoom.TileInRoomX) )
                {
                    DrawRectangle(Buffer, TileScreenCoords.Min, TileScreenCoords.Max, 0.25f, 0.25f, 0.25f);
                }
                else
                {
                    // Draw water
                    BlitBitmap(Buffer, TileScreenCoords.Min, TileScreenCoords.Max, &GameState->Water);

                    // DrawRectangle(Buffer, TileScreenCoords.Min, TileScreenCoords.Max, WATER_R, WATER_G, WATER_B);
                    if(TileValue == TILE_TREE)
                    {
                        BlitBitmapAndBlend(Buffer, TileScreenCoords.Min, TileScreenCoords.Max, &GameState->Tree);
                    }
                }
            }
        }

        //  PlayerScreenCoords is computed by first determining what tile the player is on...
        u32 PlayerTileX = PlayerRoom.TileInRoomX;
        u32 PlayerTileY = PlayerRoom.TileInRoomY;
        tile_screen_coordinates PlayerScreenCoords = GetScreenCoordinatesForRoomView(Origin,
                                                                                    PlayerTileX, PlayerTileY,
                                                                                    TileMapPointer->TileSideInPixels);
        PlayerScreenCoords.Min.X += (
                                     (TileMapPointer->TileSideInMeters * 0.5f) +  // Then moving them to the center of the tile ...
                                                                                  
                                     (GameState->PlayerPosition.TileOffset.X) -   // Then offsetting them from the center by their offset.
                                                                                  
                                     (PLAYER_WIDTH * 0.5f)                        // That gives us the center of the bottom of the rectangle
                                                                                  //        that the player is drawn into.

                                    ) * MetersToPixels;                           // And finally, convert the result of that computation
                                                                                  //    from meters to a pixel coordinate for the screen.

        PlayerScreenCoords.Max.X = PlayerScreenCoords.Min.X + (PLAYER_WIDTH * MetersToPixels);  

        PlayerScreenCoords.Max.Y -= ( (TileMapPointer->TileSideInMeters * 0.5f) + // For Y, move to center of the tile
                                      GameState->PlayerPosition.TileOffset.Y) *   // Offset position by offset value
                                        MetersToPixels;                           // And convert from meters to pixel coordinate. 
            
        PlayerScreenCoords.Min.Y = PlayerScreenCoords.Max.Y - (PLAYER_HEIGHT * MetersToPixels);

        bitmap *PlayerSprite;
        switch(GameState->PlayerFacing)
        {
            case(FACING_RIGHT):
            {
                PlayerSprite = &GameState->BeaverRight;
            } break;

            case(FACING_UP):
            {
                PlayerSprite = &GameState->BeaverUp;
            } break;

            case(FACING_LEFT):
            {
                PlayerSprite = &GameState->BeaverLeft;
            } break;

            case(FACING_DOWN):
            {
                PlayerSprite = &GameState->BeaverDown;
            } break;
        }
        BlitBitmapWithNearestNeighborAndBlend(Buffer, PlayerScreenCoords.Min, PlayerScreenCoords.Max, PlayerSprite);
        vector2 LineMin = {PlayerScreenCoords.Min.X, PlayerScreenCoords.Max.Y - 2};
        vector2 LineMax = {PlayerScreenCoords.Max.X, PlayerScreenCoords.Max.Y + 2};
        DrawRectangle(Buffer, LineMin, LineMax, 1.0f, 0, 1.0f);

    }
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    GameOutputSound(GameState, SoundBuffer, 400);
}
