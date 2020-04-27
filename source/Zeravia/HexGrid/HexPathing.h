#define MAX_REACHABLE_CELLS HEX_CELL_COUNT

typedef struct hex_path hex_path;
struct hex_path {
    u32 Count;
    i32 Indices[MAX_UNIT_MOVE + 1]; //Include the starting hex.
};

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

typedef struct hex_djikstra_info hex_djikstra_info;
struct hex_djikstra_info {
    b32 Visited;
    i32 CameFrom;
    i32 Distance;
};
