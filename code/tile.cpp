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

