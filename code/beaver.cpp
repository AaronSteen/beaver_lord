#include "beaver.h"
#include "handmade_intrinsics.h"

#define CHUNK_DIM 16
#define CHUNK_MASK 4

#define TILE_SIDE_PIXELS 60.0f
#define TILE_SIDE_METERS 1.4f

#define METERS_TO_PIXELS (TILE_SIDE_PIXELS / TILE_SIDE_METERS)
#define PLAYER_HEIGHT TILE_SIDE_METERS
#define PLAYER_WIDTH (TILE_SIDE_METERS * 0.75f)

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
              real32 RealMinY, real32 RealMaxY, 
              real32 RealMinX, real32 RealMaxX,
              real32 R, real32 G, real32 B)
{
    s32 MinY = RoundReal32ToInt32(RealMinY);
    s32 MaxY = RoundReal32ToInt32(RealMaxY);
    s32 MinX = RoundReal32ToInt32(RealMinX);
    s32 MaxX = RoundReal32ToInt32(RealMaxX);

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

    u32 Color = ((RoundReal32ToUInt32(R * 255.0f) << 16) |
                 (RoundReal32ToUInt32(G * 255.0f) <<  8) |
                 (RoundReal32ToUInt32(B * 255.0f) <<  0));

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

// inline void
// RecanonicalizeCoordinate(world *World, u32 *AbsTile, real32 *TileOffset, u32 TilesInThisDimension)
// {
//     s32 HowManyTilesMoved = FloorReal32ToInt32(*TileOffset / World->TileSideInMeters);
//     *AbsTile += HowManyTilesMoved;
//     *TileOffset -= (HowManyTilesMoved * World->TileSideInMeters);
//
//     Assert(*AbsTile < TilesInThisDimension);
//     Assert(*TileOffset >= 0);
//     Assert(*TileOffset <= World->TileSideInMeters);
// }

// position
// GetCanonicalPosition(world *World, position OldP)
// {
//     position NewP = OldP;
//
//     // Y
//     RecanonicalizeCoordinate(World, &NewP.AbsTileY, &NewP.TileOffsetY, (World->ChunkDim * World->ChunksInWorldY));
//
//     // X
//     RecanonicalizeCoordinate(World, &NewP.AbsTileX, &NewP.TileOffsetX, (World->ChunkDim * World->ChunksInWorldX));
//
//     return(NewP);
// }
//


// bool32
// IsTileAccessible(world *World, u32 AbsTileY, u32 AbsTileX)
// {
//     bool32 IsAccessible = false;
//     tile_value TileValue = GetTileValue(World, AbsTileY, AbsTileX);
//     IsAccessible = (TILE_EMPTY == TileValue);
//     return(IsAccessible);
// }
//

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
GetAbsTileFromRoomCoords(u32 Room, u32 TileInRoom, u32 TilesPerRoomInThisDimension)
{
    u32 ToReturn = Room * TilesPerRoomInThisDimension + TileInRoom;
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
SetTileValue(memory_arena *ArenaPointer, 
             tile_map *TileMapPointer, 
             u32 RoomY, u32 TileInRoomY, 
             u32 RoomX, u32 TileInRoomX, 
             tile_value TileValue)
{
    u32 AbsTileY = GetAbsTileFromRoomCoords(RoomY, TileInRoomY, TileMapPointer->TilesPerRoomY);
    Assert(AbsTileY < TileMapPointer->TilesInWorldY);
    u32 AbsTileX = GetAbsTileFromRoomCoords(RoomX, TileInRoomX, TileMapPointer->TilesPerRoomX);
    Assert(AbsTileX < TileMapPointer->TilesInWorldX);

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
GetTileValue(memory_arena *ArenaPointer, 
             tile_map *TileMapPointer, 
             u32 RoomY, u32 TileInRoomY, 
             u32 RoomX, u32 TileInRoomX)
{
    u32 AbsTileY = GetAbsTileFromRoomCoords(RoomY, TileInRoomY, TileMapPointer->TilesPerRoomY);
    Assert(AbsTileY < TileMapPointer->TilesInWorldY);
    u32 AbsTileX = GetAbsTileFromRoomCoords(RoomX, TileInRoomX, TileMapPointer->TilesPerRoomX);
    Assert(AbsTileX < TileMapPointer->TilesInWorldX);

    tile_chunk_position ChunkPosition = GetChunkPositionFromAbsTiles(TileMapPointer, AbsTileY, AbsTileX);
    Assert(ChunkPosition.ChunkY < TileMapPointer->ChunksInMapY);
    Assert(ChunkPosition.ChunkX < TileMapPointer->ChunksInMapX);
    Assert(ChunkPosition.TileInChunkY < TileMapPointer->ChunkDim);
    Assert(ChunkPosition.TileInChunkX < TileMapPointer->ChunkDim);

    tile_chunk *TileChunkPointer = GetTileChunkPointerUnchecked(TileMapPointer, ChunkPosition.ChunkY, ChunkPosition.ChunkX);
    Assert(TileChunkPointer->TilesArray);

    tile_value ToReturn = (tile_value)TileChunkPointer->TilesArray[ChunkPosition.TileInChunkY * 
        TileMapPointer->ChunkDim + 
        ChunkPosition.TileInChunkX];
    return(ToReturn);
}

void
DrawSimpleRoom(memory_arena *ArenaPointer, tile_map *TileMapPointer, u32 RoomY, u32 RoomX)
{
    for(u32 RelRow = 0;
        RelRow < TileMapPointer->TilesPerRoomY;
        ++RelRow)
    {
        for(u32 RelCol = 0;
            RelCol < TileMapPointer->TilesPerRoomX;
            ++RelCol)
        {
            tile_value TileValue = TILE_WATER;
            if( (RelRow == 0) && (RelCol != (TileMapPointer->TilesPerRoomX / 2) ) )
            {
                TileValue = TILE_BLOCK;
            }
            if( (RelCol == 0) && (RelRow != (TileMapPointer->TilesPerRoomY / 2) ) )
            {
                TileValue = TILE_BLOCK;
            }
            if( (RelCol == TileMapPointer->TilesPerRoomX - 1) && (RelRow != (TileMapPointer->TilesPerRoomY / 2) ) )
            {
                TileValue = TILE_BLOCK;
            }
            if( (RelRow == TileMapPointer->TilesPerRoomY - 1) && (RelCol != (TileMapPointer->TilesPerRoomX / 2) ) )
            {
                TileValue = TILE_BLOCK;
            }
            SetTileValue(ArenaPointer, TileMapPointer, RoomY, RelRow, RoomX, RelCol, TileValue);
        }
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
        TileMapPointer->TileSideInPixels = 60;
        TileMapPointer->MetersToPixels = (real32)TileMapPointer->TileSideInPixels / TileMapPointer->TileSideInMeters;
        TileMapPointer->RoomsInMapY = 21;
        TileMapPointer->RoomsInMapX = 21;
        TileMapPointer->TilesPerRoomY = 9;
        TileMapPointer->TilesPerRoomX = 17;
        TileMapPointer->TilesInWorldY = TileMapPointer->RoomsInMapY * TileMapPointer->TilesPerRoomY;
        TileMapPointer->TilesInWorldX = TileMapPointer->RoomsInMapX * TileMapPointer->TilesPerRoomX;


        // e.g., Y:
        //      21 rooms * 9 tiles each = 21 * 9 = 210 - 21 = 189.
        //      189 total tiles in Y dimension / 16 tiles per chunk in Y dimension = (uint)189 / 16 = 11
        //      Need to add 1 to account for tiles that aren't covered by the 11th chunk: 11 + 1 = 12

        TileMapPointer->ChunksInMapY = ((TileMapPointer->RoomsInMapY * TileMapPointer->TilesPerRoomY) / TileMapPointer->ChunkDim) + 1;
        TileMapPointer->ChunksInMapX = ((TileMapPointer->RoomsInMapX * TileMapPointer->TilesPerRoomX) / TileMapPointer->ChunkDim) + 1;

        TileMapPointer->TileChunksArray = PushArray(&GameState->WorldArena, tile_chunk, 
                                                    TileMapPointer->ChunksInMapY * TileMapPointer->ChunksInMapX);

        DrawSimpleRoom(&GameState->WorldArena, TileMapPointer, 10, 10);

        // for(u32 RelRow = 0;
        //     RelRow < TileMapPointer->TilesPerRoomY;
        //     ++RelRow)
        // {
        //     for(u32 RelCol = 0;
        //         RelCol < TileMapPointer->TilesPerRoomX;
        //         ++RelCol)
        //     {
        //         tile_value TileValue = TILE_WATER;
        //         if( (RelRow == 0) && (RelCol != (TileMapPointer->TilesPerRoomX / 2) ) )
        //         {
        //             TileValue = TILE_BLOCK;
        //         }
        //         if( (RelCol == 0) && (RelRow != (TileMapPointer->TilesPerRoomY / 2) ) )
        //         {
        //             TileValue = TILE_BLOCK;
        //         }
        //         if( (RelCol == TileMapPointer->TilesPerRoomX - 1) && (RelRow != (TileMapPointer->TilesPerRoomY / 2) ) )
        //         {
        //             TileValue = TILE_BLOCK;
        //         }
        //         if( (RelRow == TileMapPointer->TilesPerRoomY - 1) && (RelCol != (TileMapPointer->TilesPerRoomX / 2) ) )
        //         {
        //             TileValue = TILE_BLOCK;
        //         }
        //         SetTileValue(&GameState->WorldArena, TileMapPointer, RoomY, RelRow, RoomX, RelCol, TileValue);
        //     }
        // }



        // Generate the world like this:
        //      Start in the center room e.g., Room(9, 5)
        //      Generate the room by drawing the tiles with a door on each wall
        //      Iterate through the 4 doors:
        //          For each door, draw a series of 10 connected rooms, deciding at random how many doors each room will have
        //          For the first pass, draw the 10 rooms by picking one door at random from those created to create the
        //              next room until 10 have been drawn.
        //          For a second pass, pop the created 10 rooms off of the stack and draw dead-end rooms for the doors that were not
        //              on the main path.
        //          Requirements for a room:
        //              The code should check that the room is within the bounds of the tilemap
        //              Each room must have a door connecting to the door on the room created just before it
        //              Before drawing a new room, the code should check whether there is already one there by reading
        //                  the tile in the bottom-left corner of the hypothetical room, which is guaranteed to have a block 
        //                  if there is indeed a room already there
        //              If a room is already there, the code should just draw a connecting door and move on to the next door,
        //                  if there is one.
        //

        Memory->IsInitialized = true;
    }

    for(int ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        if(Controller->IsAnalog)
        {
            // NOTE(casey): Use analog movement tuning
        }
        else
        {
            real32 dPlayerY = 0.0f;
            real32 dPlayerX = 0.0f;
            if(Controller->MoveRight.EndedDown)
            {
                dPlayerX += 1.0f;
            }
            if(Controller->MoveUp.EndedDown)
            {
                dPlayerY += 1.0f;
            }
            if(Controller->MoveLeft.EndedDown)
            {
                dPlayerX -= 1.0f;
            }
            if(Controller->MoveDown.EndedDown)
            {
                dPlayerY -= 1.0f;
            }

            real32 PlayerSpeed = 2.0f;
            if(Controller->Back.EndedDown)
            {
                PlayerSpeed *= 3.0f;
            }
            real32 MovementFactor = Input->dtForFrame * PlayerSpeed;

            // position TestPlayerBase = GameState->PlayerP;
            // TestPlayerBase.TileOffsetY += dPlayerY * MovementFactor;
            // TestPlayerBase.TileOffsetX += dPlayerX * MovementFactor;
            // TestPlayerBase = GetCanonicalPosition(GameState->World, TestPlayerBase);
            //
            // position TestPlayerLeft = TestPlayerBase;
            // TestPlayerLeft.TileOffsetX -= (PLAYER_WIDTH * 0.5f);
            // TestPlayerLeft = GetCanonicalPosition(GameState->World, TestPlayerLeft);
            //
            // position TestPlayerRight = TestPlayerBase;
            // TestPlayerRight.TileOffsetX += (PLAYER_WIDTH * 0.5f);
            // TestPlayerRight = GetCanonicalPosition(GameState->World, TestPlayerRight);
            //
            // if(IsTileAccessible(GameState->World, TestPlayerBase.AbsTileY, TestPlayerBase.AbsTileX) &&
            //    IsTileAccessible(GameState->World, TestPlayerLeft.AbsTileY, TestPlayerLeft.AbsTileX) &&
            //    IsTileAccessible(GameState->World, TestPlayerRight.AbsTileY, TestPlayerRight.AbsTileX))
            // {
            //     GameState->PlayerP = TestPlayerBase;
            // }
        }
    }

    tile_map *TileMapPointer = GameState->WorldPointer->TileMapPointer;
    u32 RoomY = 10;
    u32 TileInRoomY = 4;
    u32 RoomX = 10;
    u32 TileInRoomX = 4;

    // Draw underlayer
    DrawRectangle(Buffer,
                  0, Buffer->Height,
                  0, Buffer->Width,
                  0.75f, 0.25f, 0.5f);

    real32 ScreenCoordZeroY = Buffer->Height;
    real32 ScreenCoordZeroX = -((real32)TileMapPointer->TileSideInPixels * 0.5f);

    for(u32 RelRow = 0;
        RelRow < TileMapPointer->TilesPerRoomY;
        ++RelRow)
    {
        for(u32 RelCol = 0;
            RelCol < TileMapPointer->TilesPerRoomX;
            ++RelCol)
        {
            real32 TileR = WATER_R;
            real32 TileG = WATER_G;
            real32 TileB = WATER_B;

            tile_value TileValue = GetTileValue(&GameState->WorldArena, TileMapPointer, RoomY, RelRow, RoomX, RelCol);
            if(TileValue == TILE_BLOCK)
            {
                TileR = BLOCK_R;
                TileG = BLOCK_G;
                TileB = BLOCK_B;
            }
            real32 TileBottom = ScreenCoordZeroY - (RelRow * TileMapPointer->TileSideInPixels);
            real32 TileTop = TileBottom - TileMapPointer->TileSideInPixels;
            real32 TileLeft = ScreenCoordZeroX + (RelCol * TileMapPointer->TileSideInPixels);
            real32 TileRight = TileLeft + TileMapPointer->TileSideInPixels;
            DrawRectangle(Buffer,
                          TileTop, TileBottom,
                          TileLeft, TileRight,
                          TileR, TileG, TileB);
        }

    }

    // // Draw player
    // real32 PlayerBottom = SCREEN_BOTTOM - 
    //                         (GameState->PlayerP.TileY * TILE_SIDE_PIXELS) -
    //                         (GameState->PlayerP.TileOffsetY * METERS_TO_PIXELS);
    // real32 PlayerTop    = PlayerBottom - 
    //                         (PLAYER_HEIGHT * METERS_TO_PIXELS);
    // real32 PlayerLeft   = SCREEN_LEFT +
    //                         (GameState->PlayerP.TileX * TILE_SIDE_PIXELS) +
    //                         (GameState->PlayerP.TileOffsetX * METERS_TO_PIXELS) -
    //                         (PLAYER_WIDTH * 0.5f * METERS_TO_PIXELS);
    // real32 PlayerRight  = PlayerLeft +
    //                         (PLAYER_WIDTH * METERS_TO_PIXELS);
    // DrawRectangle(Buffer,
    //               PlayerTop, PlayerBottom,
    //               PlayerLeft, PlayerRight,
    //               PLAYER_R, PLAYER_G, PLAYER_B);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    GameOutputSound(GameState, SoundBuffer, 400);
}
/*
internal void
RenderWeirdGradient(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
    // TODO(casey): Let's see what the optimizer does

    u8 *Row = (u8 *)Buffer->Memory;    
    for(int Y = 0;
        Y < Buffer->Height;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(int X = 0;
            X < Buffer->Width;
            ++X)
        {
            u8 Blue = (u8)(X + BlueOffset);
            u8 Green = (u8)(Y + GreenOffset);

            *Pixel++ = ((Green << 16) | Blue);
        }
        
        Row += Buffer->Pitch;
    }
}
*/
