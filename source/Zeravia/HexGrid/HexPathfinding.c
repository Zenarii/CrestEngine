//POLISH(Zen): This can be reduced when the max movement speed has been decided


internal i32
GetCostToCell(hex_cell * Cell) {
    return 1;
}

internal b32
IsCellAccessible(game_state * GameState, hex_cell * From, hex_cell * To) {
    b32 Accessible = 1;
    if(GetHexEdgeType(From->Elevation, To->Elevation) == HEX_EDGE_CLIFF) Accessible = 0;
    if(To->WaterLevel > To->Elevation) Accessible = 0;
    //HARDCODE(Zen): Assumes only player units using this function
    for(i32 i = 0; i < GameState->Enemy.UnitCount; ++i) {
        if(GameState->Enemy.Units[i].CellIndex == To->Index) Accessible = 0;
    }
    return Accessible;
}

internal b32
IsCellEmpty(game_state * GameState, hex_cell * Cell) {
    b32 Result = 1;
    //Note(Zen): Loop through player units
    for(i32 i = 0; i < GameState->Player.UnitCount; ++i) {
        Result = (GameState->Player.Units[i].CellIndex == Cell->Index) ? 0 : Result;
    }

    //Note(Zen): Loop through enemy units
    for(i32 i = 0; i < GameState->Player.UnitCount; ++i) {
        Result = (GameState->Enemy.Units[0].CellIndex == Cell->Index) ? 0 : Result;
    }

    return Result;
}


internal hex_path
HexPathingBFS(hex_grid * Grid, hex_cell StartCell, hex_cell EndCell) {
    i32 Frontier[MAX_REACHABLE_CELLS] = {0};
    i32 NextCursor = 0;
    i32 BackCursor = 0;
    hex_bfs_info Visited[HEX_CELL_COUNT] = {0};

    Frontier[BackCursor++] = StartCell.Index;

    Visited[StartCell.Index] = HexBFSInfo(1, 0);

    //Note(Zen): Makes paths look nicer
    b32 AlternateDirections = 0;

    while(NextCursor != BackCursor) {
        i32 CurrentIndex = Frontier[NextCursor++];

        if(CurrentIndex == EndCell.Index) break;

        hex_cell * CurrentCell = &Grid->Cells[CurrentIndex];

        if(AlternateDirections) {
            for(hex_direction Direction = 0; Direction < HEX_DIRECTION_COUNT; ++Direction) {
                if(!CurrentCell->Neighbours[Direction]) continue;


                i32 NeighbourIndex = CurrentCell->Neighbours[Direction]->Index;
                if(!Visited[NeighbourIndex].Visited) {
                    Frontier[BackCursor++] = NeighbourIndex;
                    Visited[NeighbourIndex] = HexBFSInfo(1, CurrentIndex);
                }
            }
        }
        else {
            for(hex_direction Direction = HEX_DIRECTION_COUNT - 1; Direction >= 0; --Direction) {
                if(!CurrentCell->Neighbours[Direction]) continue;

                i32 NeighbourIndex = CurrentCell->Neighbours[Direction]->Index;
                if(!Visited[NeighbourIndex].Visited) {
                    Frontier[BackCursor++] = NeighbourIndex;
                    Visited[NeighbourIndex] = HexBFSInfo(1, CurrentIndex);
                }
            }
        }
        AlternateDirections = !AlternateDirections;

    }

    //reconstruct path
    i32 PathIndex = EndCell.Index;

    hex_path Result = {0};
    while(PathIndex != StartCell.Index) {
        Result.Indices[Result.Count++] = PathIndex;
        PathIndex = Visited[PathIndex].CameFrom;
    }
    Result.Indices[Result.Count++] = StartCell.Index;
    return Result;
}


internal hex_djikstra_info
HexDjikstraInfo(b32 Visited, i32 CameFrom, i32 Distance) {
    hex_djikstra_info Result = {Visited, CameFrom, Distance};
    return Result;
}


internal hex_path
HexPathingDjikstra(game_state * GameState, hex_grid * Grid, hex_cell StartCell, hex_cell EndCell) {
    i32 Frontier[MAX_REACHABLE_CELLS] = {0};
    i32 NextCursor = 0;
    i32 BackCursor = 0;
    hex_djikstra_info Visited[HEX_CELL_COUNT] = {0};

    Frontier[BackCursor++] = StartCell.Index;

    Visited[StartCell.Index] = HexDjikstraInfo(1, StartCell.Index, 0);

    while(NextCursor != BackCursor) {
        i32 CurrentIndex = Frontier[NextCursor++];

        hex_cell * CurrentCell = &Grid->Cells[CurrentIndex];

        for(hex_direction Direction = 0; Direction < HEX_DIRECTION_COUNT; ++Direction) {
            if(!CurrentCell->Neighbours[Direction]) continue;
            i32 NeighbourIndex = CurrentCell->Neighbours[Direction]->Index;
            hex_cell * NeighbourCell = &Grid->Cells[NeighbourIndex];

            if(!IsCellAccessible(GameState, CurrentCell, NeighbourCell)) continue;

            i32 Cost = GetCostToCell(NeighbourCell);
            if(!Visited[NeighbourIndex].Visited || (Visited[NeighbourIndex].Distance > Visited[CurrentIndex].Distance + Cost)) {
                Frontier[BackCursor++] = NeighbourIndex;
                Visited[NeighbourIndex] = HexDjikstraInfo(1, CurrentIndex, Visited[CurrentIndex].Distance + Cost);

                if(BackCursor >= MAX_REACHABLE_CELLS) BackCursor -= MAX_REACHABLE_CELLS;
                if(NextCursor >= MAX_REACHABLE_CELLS) NextCursor -= MAX_REACHABLE_CELLS;
            }
        }
    }

    //reconstruct path
    i32 PathIndex = EndCell.Index;

    hex_path Result = {0};
    while(PathIndex != StartCell.Index) {
        Result.Indices[Result.Count++] = PathIndex;
        PathIndex = Visited[PathIndex].CameFrom;
    }
    Result.Indices[Result.Count++] = StartCell.Index;
    return Result;
}


typedef struct hex_reachable_cells hex_reachable_cells;
struct hex_reachable_cells {
    u32 Count;
    b32 Empty[64 * 4];
    i32 Indices[64 * 4]; //HARDCODE(Zen): Max move in each direction
};


internal hex_reachable_cells
HexGetReachableCells(game_state * GameState, hex_grid * Grid, hex_cell StartCell, i32 Distance) {
    i32 Frontier[MAX_REACHABLE_CELLS] = {0};
    i32 NextCursor = 0;
    i32 BackCursor = 0;
    hex_djikstra_info Visited[HEX_CELL_COUNT] = {0};
    hex_reachable_cells Result = {0};
    Frontier[BackCursor++] = StartCell.Index;

    Visited[StartCell.Index] = HexDjikstraInfo(1, StartCell.Index, 0);

    while(NextCursor != BackCursor) {
        i32 CurrentIndex = Frontier[NextCursor++];

        hex_cell * CurrentCell = &Grid->Cells[CurrentIndex];
        IsCellEmpty(GameState, CurrentCell);

        for(hex_direction Direction = 0; Direction < HEX_DIRECTION_COUNT; ++Direction) {
            if(!CurrentCell->Neighbours[Direction]) continue;

            i32 NeighbourIndex = CurrentCell->Neighbours[Direction]->Index;
            hex_cell * NeighbourCell = &Grid->Cells[NeighbourIndex];

            if(!IsCellAccessible(GameState, CurrentCell, NeighbourCell)) continue;



            i32 Cost = GetCostToCell(NeighbourCell);
            if(!Visited[NeighbourIndex].Visited || (Visited[NeighbourIndex].Distance > Visited[CurrentIndex].Distance + Cost)) {
                if(Visited[CurrentIndex].Distance + Cost <= Distance) {
                    Frontier[BackCursor++] = NeighbourIndex;
                    Visited[NeighbourIndex] = HexDjikstraInfo(1, CurrentIndex, Visited[CurrentIndex].Distance + Cost);
                    Result.Indices[Result.Count] = NeighbourIndex;
                    Result.Empty[Result.Count] = IsCellEmpty(GameState, NeighbourCell);
                    Result.Count++;
                }

                if(BackCursor >= MAX_REACHABLE_CELLS) BackCursor -= MAX_REACHABLE_CELLS;
                if(NextCursor >= MAX_REACHABLE_CELLS) NextCursor -= MAX_REACHABLE_CELLS;
            }
        }
    }

    return Result;
}
