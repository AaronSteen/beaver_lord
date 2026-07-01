
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

