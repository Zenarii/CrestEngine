//POLISH(Zen): This can be reduced when the max movement speed has been decided
#define MAX_REACHABLE_CELLS HEX_CELL_COUNT

typedef struct hex_bfs_info hex_bfs_info;
struct hex_bfs_info {
    b32 Visited;
    i32 CameFrom;
};

internal hex_bfs_info
HexBFSInfo(b32 Visited, i32 CameFrom) {
    hex_bfs_info Result = {Visited, CameFrom};
    return Result;
}

typedef struct bfs_path bfs_path;
struct bfs_path {
    u32 Count;
    i32 Indices[64];
};


internal bfs_path
HexPathingBFS(hex_grid * Grid, hex_cell StartCell, hex_cell EndCell) {
    i32 Frontier[MAX_REACHABLE_CELLS] = {0};
    i32 NextCursor = 0;
    i32 BackCursor = 0;
    hex_bfs_info Visited[HEX_CELL_COUNT] = {0};

    Frontier[BackCursor++] = StartCell.Index;

    Visited[StartCell.Index] = HexBFSInfo(1, 0);

    while(NextCursor != BackCursor) {
        i32 CurrentIndex = Frontier[NextCursor++];

        if(CurrentIndex == EndCell.Index) break;

        hex_cell * CurrentCell = &Grid->Cells[CurrentIndex];

        for(hex_direction Direction = 0; Direction < HEX_DIRECTION_COUNT; ++Direction) {
            if(!CurrentCell->Neighbours[Direction]) continue;


            i32 NeighbourIndex = CurrentCell->Neighbours[Direction]->Index;
            if(!Visited[NeighbourIndex].Visited) {
                Frontier[BackCursor++] = NeighbourIndex;
                Visited[NeighbourIndex] = HexBFSInfo(1, CurrentIndex);
            }
        }
    }

    //reconstruct path
    i32 PathIndex = EndCell.Index;

    bfs_path Result = {0};
    while(PathIndex != StartCell.Index) {
        Result.Indices[Result.Count++] = PathIndex;
        PathIndex = Visited[PathIndex].CameFrom;
    }
    return Result;
}
