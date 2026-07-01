enum tile_value
{
    TILE_INVALID,
    TILE_WATER,
    TILE_TREE
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

