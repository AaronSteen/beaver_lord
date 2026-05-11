/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include "beaver.h"
#include "handmade_intrinsics.h"

#define TILES_PER_MAP_Y 9
#define TILES_PER_MAP_X 17

#define TILE_SIDE_PIXELS 60.0f
#define TILE_SIDE_METERS 1.4f

#define METERS_TO_PIXELS (TILE_SIDE_PIXELS / TILE_SIDE_METERS)
#define PLAYER_HEIGHT TILE_SIDE_METERS
#define PLAYER_WIDTH (TILE_SIDE_METERS * 0.75f)

#define SCREEN_BOTTOM Buffer->Height
#define SCREEN_LEFT -((real32) TILE_SIDE_PIXELS * 0.5f)

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

inline void
RecanonicalizeCoordinate(world *World, u32 *Tile, real32 *TileOffset, u32 TilesInThisDimension)
{
    s32 HowManyTilesMoved = FloorReal32ToInt32(*TileOffset / World->TileSideInMeters);
    *Tile += HowManyTilesMoved;
    *TileOffset -= (HowManyTilesMoved * World->TileSideInMeters);

    Assert(*Tile < TilesInThisDimension);
    Assert(*TileOffset >= 0);
    Assert(*TileOffset <= World->TileSideInMeters);
}

position
GetCanonicalPosition(world *World, position OldP)
{
    position NewP = OldP;

    // Y
    RecanonicalizeCoordinate(World, &NewP.TileY, &NewP.TileOffsetY, World->TilesPerMapY);

    // X
    RecanonicalizeCoordinate(World, &NewP.TileX, &NewP.TileOffsetX, World->TilesPerMapX);

    return(NewP);
}

inline u32 
GetTileValueUnchecked(world *World, tile_map *TileMap, u32 TileY, u32 TileX)
{
    u32 TileValue = TileMap->Tiles[TileY * World->TilesPerMapX + TileX];
    return(TileValue);
}

s32
GetTileValue(world *World, u32 TileY, u32 TileX)
{
    s32 TileValue = -1;
    tile_map *TileMap = World->TileMap;
    if(TileMap)
    {
        TileValue = GetTileValueUnchecked(World, TileMap, TileY, TileX);
    }
    return(TileValue);
}


bool32
IsTileAccessible(world *World, u32 TileY, u32 TileX)
{
    bool32 IsAccessible = false;
    s32 TileValue = GetTileValue(World, TileY, TileX);

    // Tile value must be either 0 (empty) or a positive number.
    //      GetTileValue returns -1 if no tilemap was found for
    //      the passed coordinates.
    Assert(TileValue >= 0);

    IsAccessible = (0 == TileValue);
    return(IsAccessible);
}

#if defined __cplusplus
extern "C"
#endif
GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons)));
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
    
    // Tiles, blocks
    u32 TileMap[TILES_PER_MAP_Y][TILES_PER_MAP_X] = 
    {
        {1, 1, 1, 1,    1, 1, 1, 1,    0,     1, 1, 1, 1,    1, 1, 1, 1},
        {1, 0, 1, 0,    0, 0, 0, 0,    0,     0, 0, 0, 0,    0, 0, 0, 1},
        {1, 0, 0, 0,    0, 0, 0, 0,    0,     0, 0, 0, 0,    0, 0, 0, 1},
        {1, 0, 1, 0,    0, 0, 0, 0,    0,     0, 0, 0, 0,    0, 0, 0, 1},

        {0, 0, 0, 0,    0, 0, 0, 0,    0,     0, 0, 0, 0,    0, 0, 0, 0},

        {1, 0, 0, 0,    0, 0, 0, 0,    0,     0, 0, 0, 0,    0, 0, 0, 1},
        {1, 0, 0, 0,    0, 0, 0, 0,    0,     0, 0, 0, 0,    0, 0, 0, 1},
        {1, 0, 0, 0,    0, 0, 0, 0,    0,     0, 0, 0, 0,    0, 0, 0, 1},
        {1, 1, 1, 1,    1, 1, 1, 1,    0,     1, 1, 1, 1,    1, 1, 1, 1},
    };

    world World;
    World.TilesPerMapY = TILES_PER_MAP_Y;
    World.TilesPerMapX = TILES_PER_MAP_X;
    World.TileMap->Tiles = (u32 *)TileMap;
    World.TileSideInMeters = TILE_SIDE_METERS;

    game_state *GameState = (game_state *)Memory->PermanentStorage;
    if(!Memory->IsInitialized)
    {
        GameState->PlayerP.TileY = 4;
        GameState->PlayerP.TileX = 4;
        GameState->PlayerP.TileOffsetY = 0.36f;
        GameState->PlayerP.TileOffsetX = 0.79f;
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

            position TestPlayerBase = GameState->PlayerP;
            TestPlayerBase.TileOffsetY += dPlayerY * MovementFactor;
            TestPlayerBase.TileOffsetX += dPlayerX * MovementFactor;
            TestPlayerBase = GetCanonicalPosition(&World, TestPlayerBase);

            position TestPlayerLeft = TestPlayerBase;
            TestPlayerLeft.TileOffsetX -= (PLAYER_WIDTH * 0.5f);
            TestPlayerLeft = GetCanonicalPosition(&World, TestPlayerLeft);

            position TestPlayerRight = TestPlayerBase;
            TestPlayerRight.TileOffsetX += (PLAYER_WIDTH * 0.5f);
            TestPlayerRight = GetCanonicalPosition(&World, TestPlayerRight);

            if(IsTileAccessible(&World, TestPlayerBase.TileY, TestPlayerBase.TileX) &&
               IsTileAccessible(&World, TestPlayerLeft.TileY, TestPlayerLeft.TileX) &&
               IsTileAccessible(&World, TestPlayerRight.TileY, TestPlayerRight.TileX))
            {
                GameState->PlayerP = TestPlayerBase;
            }
        }
    }

    // Draw underlayer
    DrawRectangle(Buffer,
                  0, Buffer->Height,
                  0, Buffer->Width,
                  0.75f, 0.25f, 0.5f);

    // Draw tiles
    for(int Y = 0;
        Y < TILES_PER_MAP_Y;
        ++Y)
    {
        for(int X = 0;
            X < TILES_PER_MAP_X;
            ++X)
        {
            real32 TileR = WATER_R;
            real32 TileG = WATER_G;
            real32 TileB = WATER_B;
            if(TileMap[Y][X])
            {
                TileR = BLOCK_R;
                TileG = BLOCK_G;
                TileB = BLOCK_B;
            }
                real32 TileBottom = SCREEN_BOTTOM - (TILE_SIDE_PIXELS * Y);
                real32 TileTop = TileBottom - TILE_SIDE_PIXELS;
                real32 TileLeft = SCREEN_LEFT + (X * TILE_SIDE_PIXELS);
                real32 TileRight = TileLeft + TILE_SIDE_PIXELS;
                DrawRectangle(Buffer,
                              TileTop, TileBottom,
                              TileLeft, TileRight,
                              TileR, TileG, TileB);
        }
    }

    // Draw player
    real32 PlayerBottom = SCREEN_BOTTOM - 
                            (GameState->PlayerP.TileY * TILE_SIDE_PIXELS) -
                            (GameState->PlayerP.TileOffsetY * METERS_TO_PIXELS);
    real32 PlayerTop    = PlayerBottom - 
                            (PLAYER_HEIGHT * METERS_TO_PIXELS);
    real32 PlayerLeft   = SCREEN_LEFT +
                            (GameState->PlayerP.TileX * TILE_SIDE_PIXELS) +
                            (GameState->PlayerP.TileOffsetX * METERS_TO_PIXELS) -
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
