#include "beaver.h"
#include "handmade_intrinsics.h"

#define CHUNK_DIM 16
#define CHUNK_MASK 4

#define TILE_SIDE_PIXELS 60.0f
#define TILE_SIDE_METERS 1.4f

#define METERS_TO_PIXELS (TILE_SIDE_PIXELS / TILE_SIDE_METERS)
#define PLAYER_HEIGHT TILE_SIDE_METERS
#define PLAYER_WIDTH (TILE_SIDE_METERS * 0.75f)

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
    s32 MinY = RoundReal32ToS32(RealMinY);
    s32 MaxY = RoundReal32ToS32(RealMaxY);
    s32 MinX = RoundReal32ToS32(RealMinX);
    s32 MaxX = RoundReal32ToS32(RealMaxX);

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

inline void
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

world_position
GetCanonicalPosition(tile_map *TileMapPointer, world_position OldPosition)
{
    world_position NewPosition = OldPosition;

    // Y
    RecanonicalizeCoordinate(TileMapPointer, 
                             &NewPosition.AbsTileY, &NewPosition.TileOffsetY, 
                             (TILES_IN_WORLD_Y));

    // X
    RecanonicalizeCoordinate(TileMapPointer, 
                             &NewPosition.AbsTileX, &NewPosition.TileOffsetX, 
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
GetAbsTileFromRoomCoords(u32 Room, u32 TileInRoom, u32 TilesPerRoomInThisDimension)
{
    u32 ToReturn = Room * TilesPerRoomInThisDimension + TileInRoom;
    return(ToReturn);
}

inline room_coordinates
GetRoomCoordsFromAbsTiles(tile_map *TileMapPointer, u32 AbsTileY, u32 AbsTileX)
{
    room_coordinates ToReturn;
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
    Assert(AbsTileY < TILES_IN_WORLD_Y);
    Assert(AbsTileX < TILES_IN_WORLD_X);

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
DrawSimpleRoom(memory_arena *ArenaPointer, tile_map *TileMapPointer, u32 RoomY, u32 RoomX,
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
                TileValue = TILE_BLOCK;
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
        u32 DoorNorthAbsTileY = GetAbsTileFromRoomCoords(RoomY, 0, TileMapPointer->TilesPerRoomY); 
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
        u32 DoorSouthAbsTileY = GetAbsTileFromRoomCoords(RoomY, (TileMapPointer->TilesPerRoomY - 1), TileMapPointer->TilesPerRoomY); 
        u32 DoorSouthAbsTileX = GetAbsTileFromRoomCoords(RoomX, (TileMapPointer->TilesPerRoomX / 2), TileMapPointer->TilesPerRoomX); 
        SetTileValue(ArenaPointer, TileMapPointer, DoorSouthAbsTileY, DoorSouthAbsTileX, TILE_WATER);
    }
}

bool32
IsTileAccessible(memory_arena *ArenaPointer, tile_map *TileMapPointer, u32 AbsTileY, u32 AbsTileX)
{
    bool32 IsAccessible = false;
    tile_value TileValue = GetTileValue(ArenaPointer, TileMapPointer, AbsTileY, AbsTileX);
    IsAccessible = ( (TileValue != TILE_INVALID) && (TileValue != TILE_BLOCK) );
    return(IsAccessible);
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

        TileMapPointer->ChunksInMapY = ((TileMapPointer->RoomsInMapY * TileMapPointer->TilesPerRoomY) / TileMapPointer->ChunkDim) + 1;
        TileMapPointer->ChunksInMapX = ((TileMapPointer->RoomsInMapX * TileMapPointer->TilesPerRoomX) / TileMapPointer->ChunkDim) + 1;

        TileMapPointer->TileChunksArray = PushArray(&GameState->WorldArena, tile_chunk, 
                                                    TileMapPointer->ChunksInMapY * TileMapPointer->ChunksInMapX);

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

        u32 CenterRoomYCoord = 10;
        u32 CenterRoomXCoord = 10;
        DrawSimpleRoom(&GameState->WorldArena, TileMapPointer, CenterRoomYCoord, CenterRoomXCoord, true, true, true, true);
        for(int RoomIdx = 1;
            RoomIdx < 11;
            ++RoomIdx)
        {
            DrawSimpleRoom(&GameState->WorldArena, TileMapPointer, (CenterRoomYCoord - RoomIdx), CenterRoomXCoord, true, true, true, true);
        }

        GameState->PlayerPosition.AbsTileY = GetAbsTileFromRoomCoords(CenterRoomYCoord, 4, TileMapPointer->TilesPerRoomY);
        GameState->PlayerPosition.AbsTileX = GetAbsTileFromRoomCoords(CenterRoomXCoord, 4, TileMapPointer->TilesPerRoomX);
        GameState->PlayerPosition.TileOffsetY = 0.6f;
        GameState->PlayerPosition.TileOffsetX = -0.4f;

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

            world_position TestPlayerBase = GameState->PlayerPosition;
            TestPlayerBase.TileOffsetY += dPlayerY * MovementFactor;
            TestPlayerBase.TileOffsetX += dPlayerX * MovementFactor;
            TestPlayerBase = GetCanonicalPosition(TileMapPointer, TestPlayerBase);

            world_position TestPlayerLeft = TestPlayerBase;
            TestPlayerLeft.TileOffsetX -= (PLAYER_WIDTH * 0.5f);
            TestPlayerLeft = GetCanonicalPosition(TileMapPointer, TestPlayerLeft);

            world_position TestPlayerRight = TestPlayerBase;
            TestPlayerRight.TileOffsetX += (PLAYER_WIDTH * 0.5f);
            TestPlayerRight = GetCanonicalPosition(TileMapPointer, TestPlayerRight);

            if(IsTileAccessible(&GameState->WorldArena, TileMapPointer, TestPlayerBase.AbsTileY, TestPlayerBase.AbsTileX) &&
               IsTileAccessible(&GameState->WorldArena, TileMapPointer, TestPlayerLeft.AbsTileY, TestPlayerLeft.AbsTileX) &&
               IsTileAccessible(&GameState->WorldArena, TileMapPointer, TestPlayerRight.AbsTileY, TestPlayerRight.AbsTileX))
            {
                GameState->PlayerPosition = TestPlayerBase;
            }
        }
    }

    // Draw underlayer
    DrawRectangle(Buffer,
                  0, Buffer->Height,
                  0, Buffer->Width,
                  0.75f, 0.25f, 0.5f);

    real32 ScreenCoordZeroY = Buffer->Height;
    real32 ScreenCoordZeroX = -((real32)TileMapPointer->TileSideInPixels * 0.5f);
    room_coordinates PlayerRoom = GetRoomCoordsFromAbsTiles(TileMapPointer, 
                                                             GameState->PlayerPosition.AbsTileY,
                                                             GameState->PlayerPosition.AbsTileX);
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

            u32 ThisTileAbsY = GetAbsTileFromRoomCoords(PlayerRoom.RoomY, RelRow, TileMapPointer->TilesPerRoomY);
            u32 ThisTileAbsX = GetAbsTileFromRoomCoords(PlayerRoom.RoomX, RelCol, TileMapPointer->TilesPerRoomX);
            tile_value TileValue = GetTileValue(&GameState->WorldArena, TileMapPointer, ThisTileAbsY, ThisTileAbsX);
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

    // Draw player
    real32 PlayerBottom = ScreenCoordZeroY - 
                            (PlayerRoom.TileInRoomY * TILE_SIDE_PIXELS) - 
                            (GameState->PlayerPosition.TileOffsetY * METERS_TO_PIXELS);
    real32 PlayerTop    = PlayerBottom - 
                            (PLAYER_HEIGHT * METERS_TO_PIXELS);
    real32 PlayerLeft   = ScreenCoordZeroX +
                            (PlayerRoom.TileInRoomX * TILE_SIDE_PIXELS) +
                            (GameState->PlayerPosition.TileOffsetX * METERS_TO_PIXELS) -
                            (PLAYER_WIDTH * 0.5f * METERS_TO_PIXELS);
    real32 PlayerRight  = PlayerLeft +
                            (PLAYER_WIDTH * METERS_TO_PIXELS);
    DrawRectangle(Buffer,
                  PlayerTop, PlayerBottom,
                  PlayerLeft, PlayerRight,
                  PLAYER_R, PLAYER_G, PLAYER_B);
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
