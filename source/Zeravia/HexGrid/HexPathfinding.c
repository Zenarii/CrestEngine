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
        if(GameState->Enemy.Units[i].CellIndex == From->Index) Accessible = 0;
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
    for(i32 i = 0; i < GameState->Enemy.UnitCount; ++i) {
        Result = (GameState->Enemy.Units[i].CellIndex == Cell->Index) ? 0 : Result;
    }

    return Result;
}

//NOTE(Zen): This is now quite out of date
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

            }

            if(BackCursor >= MAX_REACHABLE_CELLS) BackCursor -= MAX_REACHABLE_CELLS;
            if(NextCursor >= MAX_REACHABLE_CELLS) NextCursor -= MAX_REACHABLE_CELLS;
        }
    }

    //reconstruct path
    i32 PathIndex = EndCell.Index;

    hex_path Result = {0};
    while(PathIndex != StartCell.Index) {
        Result.Indices[Result.Count++] = PathIndex;
        Assert(Visited[PathIndex].Visited);
        Assert(PathIndex != Visited[PathIndex].CameFrom);
        PathIndex = Visited[PathIndex].CameFrom;
    }
    Result.Indices[Result.Count++] = StartCell.Index;
    return Result;
}


typedef struct hex_reachable_cells hex_reachable_cells;
struct hex_reachable_cells {
    u32 Count;
    b32 Empty[MAX_REACHABLE_CELLS];
    i32 Indices[MAX_REACHABLE_CELLS];
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

typedef struct hex_attackable_cells hex_attackable_cells;
struct hex_attackable_cells {
    u32 Count;
    //Could make this an array of structs for cache locality
    i32 From[MAX_REACHABLE_CELLS * 2];
    i32 Indices[MAX_REACHABLE_CELLS * 2];//for now bc i don't stop cells u move to from beign attackable cells >:( (5/20)
};

//TODO(Zen): Rework when dealing with ranged weapons
internal b32
CanAttackCell(hex_grid * Grid, hex_cell Start, hex_cell Target) {
    b32 Attackable = 1;
    if(GetHexEdgeType(Target.Elevation, Start.Elevation) == HEX_EDGE_CLIFF) Attackable = 0;

    return Attackable;
}

//TODO(Zen): Maybe get the djikstra info from GetReachableCells so can make sure not marking
//movement cells as attackable (5/20)
//TODO(Zen): melee/ranged/magic etc. (5/20)
//TODO(Zen): if adjacent to tile when directly attacking enemy unit, player unit shouldn't move (5/20)
internal hex_attackable_cells
HexGetAttackableCells(hex_grid * Grid, hex_reachable_cells * Reachable) {
    hex_attackable_cells Result = {0};
    b32 Visited[HEX_CELL_COUNT] = {0};
    //HARDCODE(Zen) assumes range 1.
    for(i32 i = 0; i < Reachable->Count; ++i) {
        for(hex_direction Direction = 0; Direction < HEX_DIRECTION_COUNT; ++Direction) {
            hex_cell Cell = Grid->Cells[Reachable->Indices[i]];
            if(Cell.Neighbours[Direction]) {
                hex_cell Neighbour = *Cell.Neighbours[Direction];

                b32 VisitedCell = Visited[Cell.Neighbours[Direction]->Index];
                b32 MayAttackCell = CanAttackCell(Grid, Cell, Neighbour);

                if(!VisitedCell && MayAttackCell) {
                    Result.From[Result.Count] = Cell.Index;
                    Result.Indices[Result.Count] = Cell.Neighbours[Direction]->Index;
                    Result.Count++;

                    Visited[Cell.Neighbours[Direction]->Index] = 1;
                }
            }
        }
    }
    Assert(Result.Count < 64 * 4);
    return Result;
}

internal hex_attackable_cells
HexGetAttackableFromCell(hex_grid * Grid, hex_cell Cell) {
    hex_attackable_cells Result = {0};

    for(hex_direction Direction = 0; Direction < HEX_DIRECTION_COUNT; ++Direction) {
        if(Cell.Neighbours[Direction]) {
            hex_cell Neighbour = *Cell.Neighbours[Direction];
            if(CanAttackCell(Grid, Cell, Neighbour)) {

                Result.Indices[Result.Count] = Cell.Neighbours[Direction]->Index;
                Result.Count++;
            }
        }
    }

    return Result;
}

typedef struct hex_attackable_units hex_attackable_units;
struct hex_attackable_units {
    i32 Count;
    i32 Indexes[MAX_ENEMY_UNITS];
};

//TODO(Zen): make it so it works with either enemies or players
internal hex_attackable_units
HexGetAttackableUnits(game_state * GameState, hex_grid * Grid, hex_cell Cell) {
    hex_attackable_units Result = {0};
    hex_attackable_cells AttackableCells = HexGetAttackableFromCell(Grid, Cell);

    for(i32 i = 0; i < AttackableCells.Count; ++i) {
        for(i32 j = 0; j < GameState->Enemy.UnitCount; ++j) {
            if(GameState->Enemy.Units[j].CellIndex == AttackableCells.Indices[i]) {
                Result.Indexes[Result.Count++] = AttackableCells.Indices[i];
            }
        }
    }

    return Result;
}
