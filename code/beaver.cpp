#include "beaver.h"
#include "tile.cpp"
#include "procedural.cpp"
#include "stb_easy_font.h"

struct vertex
{
    real32 X;
    real32 Y;
    real32 Z;
    u32 Color;
};

struct quad
{
    vertex TopLeft;
    vertex TopRight;
    vertex BottomRight;
    vertex BottomLeft;
};
#define DEBUG_TEXT_BUFFER_SIZE Megabytes(2)

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
        } ScreenRow += Buffer->Width;
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
DEBUGDrawText(game_offscreen_buffer *GameOffscreenBuffer, 
              real32 X, real32 Y, 
              char *StringText, u8 *RasterizedTextBuffer, 
              real32 R, real32 G, real32 B)
{
    // Using stb parlance. We of course don't have quads, only rectangles
    u32 DebugTextBufferSize = DEBUG_TEXT_BUFFER_SIZE;
    int NumQuads = stb_easy_font_print(0, 0, StringText, NULL, RasterizedTextBuffer, DEBUG_TEXT_BUFFER_SIZE);
    // STOP: this is written but untested; may be totally broken
    for(int QuadIdx = 0;
        QuadIdx < NumQuads;
        ++QuadIdx)
    {
        quad *ThisQuad = (quad *)(RasterizedTextBuffer + QuadIdx * sizeof(quad));
        vector2 Min = {X + ThisQuad->TopLeft.X*2.5f, Y + ThisQuad->TopLeft.Y*2.5f};
        vector2 Max = {X + ThisQuad->BottomRight.X*2.5f, Y + ThisQuad->BottomRight.Y*2.5f};
        DrawRectangle(GameOffscreenBuffer, Min, Max, R, G, B);
    }
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
    // GameMemory.PermanentStorageSize = Megabytes(64);
    // GameMemory.TransientStorageSize = Gigabytes(1);
    //
    // Use PermanentStorage for debug state and game state.
    // debug state:
    //      First 16 mb of permanent storage goes to debug_state struct + DebugArena
    //          (DebugArena is where the stuff in DebugState is stored).
    //      Note debug_state struct right now is 32 bytes (memory_arena struct and a u8 pointer),
    //          so first 32 bytes of DebugState is the struct, and the remaining
    //          ((1024 * 1024 = 1,048,576) - 32 = 1,048,544) bytes represent the DebugArena from which we can allocate
    //          as needed. Right now we're only going to use 2mb for a buffer for storing rasterized debug text.
    //
    // game state:
    //      game state memory starts at the (16 * 1024 * 1024)th byte in GameMemory.PermanentStorage,
    //          and is for storing player position, some bitmaps for now, grove hotspots, etc.
    u32 AmountOfMemoryForDebugState = Megabytes(16);
    u32 AmountOfMemoryForGameState = Megabytes(48);

    Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
           (ArrayCount(Input->Controllers[0].Buttons)));

    Assert(AmountOfMemoryForDebugState + AmountOfMemoryForGameState <= Memory->PermanentStorageSize);

    debug_state *DebugState = (debug_state *)Memory->PermanentStorage;
    game_state *GameState = (game_state *)((u8 *)Memory->PermanentStorage + AmountOfMemoryForDebugState);
    if(!Memory->IsInitialized)
    {
        InitializeArena(&DebugState->DebugArena,
                        (u8 *)Memory->PermanentStorage + sizeof(debug_state),
                        AmountOfMemoryForDebugState - sizeof(debug_state));
        DebugState->DebugText = PushArray(&DebugState->DebugArena, u8, Megabytes(2));

        InitializeArena(&GameState->WorldArena, 
                        (u8 *)Memory->PermanentStorage + AmountOfMemoryForDebugState + sizeof(game_state),
                        AmountOfMemoryForGameState - sizeof(game_state));

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

        u32 TilesInWorldX = TileMapPointer->RoomsInMapX * TileMapPointer->TilesPerRoomX;
        u32 TilesInWorldY = TileMapPointer->RoomsInMapY * TileMapPointer->TilesPerRoomY;

        TileMapPointer->ChunksInMapX = (TilesInWorldX / TileMapPointer->ChunkDim) + 1;
        TileMapPointer->ChunksInMapY = (TilesInWorldY / TileMapPointer->ChunkDim) + 1;

        TileMapPointer->TileChunksArray = PushArray(&GameState->WorldArena, tile_chunk, 
                                                    TileMapPointer->ChunksInMapY * TileMapPointer->ChunksInMapX);

        u32 CenterRoomYCoord = 10;
        u32 CenterRoomXCoord = 10;
        u32 RoomsPerPath = 20;
        // Center room at Y = 10, X = 10                                                            E     N     W     S
        MakeSimpleRoom(&GameState->WorldArena, TileMapPointer, CenterRoomYCoord, CenterRoomXCoord, true, true, true, true);

        // Organize the world around three groves of trees.
        // Procedurally generate three "hotspots" in the world around which the three groves will emanate.
        // First, compute some constants used in the generation procedure.

        // Store this locally so we don't have to dereference it all the time
        real32 TileSideInMeters = TileMapPointer->TileSideInMeters;

        // How many meters are there in the world from left to right? 499.8
        real32 MetersInWorldX = TilesInWorldX * TileSideInMeters;

        // How many meters are there in the world from top to bottom? 382.2
        real32 MetersInWorldY = TilesInWorldY * TileSideInMeters;

        // Since there are fewer meters in the world on the Y axis, let's constrain the distance from the home screen
        //      to the center of a grove to be three-quarters the distance from the home screen to the top or bottom of the world.
        //      We use modulus since we are thinking in terms of polar coordinates.
        real32 ModulusMax = MetersInWorldY * 0.5f * 0.75f;

        // And let's set the minimum modulus to be at least two screen-heighths from the home screen.
        real32 ModulusMin = TileMapPointer->TilesPerRoomY * TileSideInMeters * 2;

        // Find the middle tile of the world, or the origin in our coordinate system
        u32 MiddleTileX = TilesInWorldX / 2;
        u32 MiddleTileY = TilesInWorldY / 2;

        tile_map_position OriginTile = {MiddleTileX, MiddleTileY};

        for(int NewGroveHotspotIdx = 0;
            NewGroveHotspotIdx < 3;
            ++NewGroveHotspotIdx)
        {

            // Need to keep track of the hotspot locations we generate to avoid the ultra-rare but possible case
            //      that we generate two of them on the same spot.
            bool32 AlreadyGenerated = false;
            tile_map_position TileMapPositionForThisGroveHotspot;
            do
            {
                // First generate a random angle in the range of 0 to 2 pi, which we call the argument
                //      since we are using the parlance of polar coordinates.
                real32 Argument = GetRandomReal(&GameState->RandomSeed, 0, (PI * 2.0f));

                // Get a modulus within those boundaries:
                real32 Modulus = GetRandomReal(&GameState->RandomSeed, ModulusMin, ModulusMax);

                // Convert polar coordinates to X and Y coordinates
                vector2 HotspotRawCoordinates = GetVector2FromPolarCoordinates(Argument, Modulus);

                // Now use HotspotRawCoordinates to determine the AbsTileX and AbsTileY values of the hotspot.
                //      HotspotRawCoordinates were generated relative to an origin of (0, 0), but we can
                //      round them and add them to our our MiddleTileX and MiddleTileY coords we computed above this 
                //      for loop to determine which AbsTiles they correspond to in our tilemap. 
                TileMapPositionForThisGroveHotspot.AbsTileX = MiddleTileX + RoundReal32ToS32(HotspotRawCoordinates.X);
                TileMapPositionForThisGroveHotspot.AbsTileY = MiddleTileY + RoundReal32ToS32(HotspotRawCoordinates.Y);
                Assert(TileMapPositionForThisGroveHotspot.AbsTileX < TileMapPointer->RoomsInMapX * TileMapPointer->TilesPerRoomX);
                Assert(TileMapPositionForThisGroveHotspot.AbsTileY < TileMapPointer->RoomsInMapY * TileMapPointer->TilesPerRoomY);

                // Now check that this Grove Hotspot does not match one already generated.
                for(int AlreadyGeneratedIdx = 0;
                    AlreadyGeneratedIdx < (NewGroveHotspotIdx + 1);
                    ++AlreadyGeneratedIdx)
                {
                    tile_map_position GroveHotspotToCheck = GameState->GroveHotspots[AlreadyGeneratedIdx];
                    if(TileMapPositionForThisGroveHotspot.AbsTileX == GroveHotspotToCheck.AbsTileX && 
                       TileMapPositionForThisGroveHotspot.AbsTileY == GroveHotspotToCheck.AbsTileY)
                    {
                        AlreadyGenerated = true;
                        break;
                    }
                }
            } while(AlreadyGenerated == true);

            tile_map_position *WhereToStoreThisNewGroveHotspot = &GameState->GroveHotspots[NewGroveHotspotIdx];
            WhereToStoreThisNewGroveHotspot->AbsTileX = TileMapPositionForThisGroveHotspot.AbsTileX;
            WhereToStoreThisNewGroveHotspot->AbsTileY = TileMapPositionForThisGroveHotspot.AbsTileY;
        }

        // Iterate through every possible tile in the world and compute its distance from one of the hotspots.
        // A tile that is a hotspot has a 100% chance of being a tree. 
        // A tile that is 200 meters or more from a hotspot has a zero percent chance of being a tree.

        vector2 GroveHotspotVectors[3];
        for(int GroveHotspotIdx = 0;
            GroveHotspotIdx < 3;
            ++GroveHotspotIdx)
        {
            tile_map_position ThisGroveHotspot = GameState->GroveHotspots[GroveHotspotIdx];
            GroveHotspotVectors[GroveHotspotIdx].X = ThisGroveHotspot.AbsTileX * 
                                                        TileSideInMeters + 
                                                        TileSideInMeters * 0.5f;
            GroveHotspotVectors[GroveHotspotIdx].Y = ThisGroveHotspot.AbsTileY * 
                                                        TileSideInMeters + 
                                                        TileSideInMeters * 0.5f;
        }

        for(u32 AbsTileY = 0;
            AbsTileY < TilesInWorldY;
            ++AbsTileY)
        {
            for(u32 AbsTileX = 0;
                AbsTileX < TilesInWorldX;
                ++AbsTileX)
            {
                // Set each tile to water
                SetTileValue(&GameState->WorldArena, TileMapPointer, AbsTileY, AbsTileX, TILE_WATER);

                // Now determine whether the tile gets a tree based on its proximity to each hotspot.
                vector2 ThisTileVector = {(AbsTileX * TileSideInMeters + TileSideInMeters * 0.5f),
                                          (AbsTileY * TileSideInMeters + TileSideInMeters * 0.5f)};

                real32 ProbOfNoTreeBasedOnHotspot[3];
                for(int HotspotIdx = 0;
                    HotspotIdx < 3;
                    ++HotspotIdx)
                {
                    vector2 DiffVector = GroveHotspotVectors[HotspotIdx] - ThisTileVector;
                    real32 SquaredDiff = SquaredMagnitude(DiffVector);

                    // Actually not clamped yet
                    real32 Clamped = SquaredDiff / Square(50.0f);

                    // Clamp it
                    if(Clamped > 1) { Clamped = 1; }
                    if(Clamped < 0) { Clamped = 0; }

                    ProbOfNoTreeBasedOnHotspot[HotspotIdx] = Clamped;
                }

                real32 TotalProbOfNoTree = 1;
                for(int ProbBasedOnHotspotIdx = 0;
                    ProbBasedOnHotspotIdx < 3;
                    ++ProbBasedOnHotspotIdx)
                {
                    TotalProbOfNoTree *= ProbOfNoTreeBasedOnHotspot[ProbBasedOnHotspotIdx];
                }

                // STOP: can't check if this worked because we are not rendering trees properly in birdseye.
                //      need to remove tile_hotspot if statement in birdseye rendering loop (should if on TILE_TREE),
                //      and blit if so. 
                //
                // but actually think we should still record where the hotspots are, so we can see
                //      if there is clustering around them as we expect.
                real32 ProbOfTree = 1.0f - TotalProbOfNoTree;
                real32 DiceRoll = GetRandomReal(&GameState->RandomSeed, 0, 1);
                if(DiceRoll <= ProbOfTree)
                {
                    SetTileValue(&GameState->WorldArena, TileMapPointer, AbsTileY, AbsTileX, TILE_TREE);
                }
            }
        }

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
#if 0
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
#else
            GameState->PlayerPosition = TestPlayerBase;
        }
    }
#endif

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
                        real32 PlayerTileR = 0.25f;
                        real32 PlayerTileG = 0.25f;
                        real32 PlayerTileB = 0.25f;
                        DrawRectangle(Buffer, TileScreenCoords.Min, TileScreenCoords.Max, PlayerTileR, PlayerTileG, PlayerTileB);
                    }
                    else
                    {
                        BlitBitmap(Buffer, TileScreenCoords.Min, TileScreenCoords.Max, &GameState->WaterSmall);
                        if(TileValue == TILE_TREE)
                        {
                            BlitBitmapWithNearestNeighborAndBlend(Buffer, TileScreenCoords.Min, TileScreenCoords.Max, &GameState->Tree);
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

    DEBUGDrawText(Buffer, 15, 15, "Frames per second: 33.2", DebugState->DebugText, 1.0f, 0.2f, 0.2f); 
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->PermanentStorage;
    GameOutputSound(GameState, SoundBuffer, 400);
}
