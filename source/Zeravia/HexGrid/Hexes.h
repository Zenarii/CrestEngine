#define HEX_OUTER_RADIUS 1.f
#define HEX_OUTER_DIAMETER HEX_OUTER_RADIUS * 2.f
//Note(Zen): sqrt(3)/2 * outer radius
#define HEX_INNER_RADIUS 0.866025404f * HEX_OUTER_RADIUS
#define HEX_INNER_DIAMETER HEX_INNER_RADIUS * 2.f

const global v3 HexCorners[] = {
    {0.f, 0.f, HEX_OUTER_RADIUS},
    {HEX_INNER_RADIUS, 0.f, 0.5f * HEX_OUTER_RADIUS},
    {HEX_INNER_RADIUS, 0.f, -0.5f * HEX_OUTER_RADIUS},
    {0.f, 0.f, -HEX_OUTER_RADIUS},
    {-HEX_INNER_RADIUS, 0.f, -0.5f * HEX_OUTER_RADIUS},
    {-HEX_INNER_RADIUS, 0.f, 0.5f * HEX_OUTER_RADIUS},
    {0.f, 0.f, HEX_OUTER_RADIUS},
};

#define MAX_HEX_VERTICES (8192 * 2)

#define HEX_CHUNK_WIDTH 7
#define HEX_CHUNK_HEIGHT 5

#define HEX_SOLID_FACTOR 0.7f
#define HEX_BLEND_FACTOR (1.f - HEX_SOLID_FACTOR)
#define HEX_NUDGE_STRENGTH 0.2f
#define HEX_ELEVATION_NUDGE_STRENGTH 0.15f
#define HEX_NOISE_SCALE 0.15f

#define HEX_ELEVATION_STEP 0.3f
#define HEX_MAX_ELEVATION 4
#define HEX_TERRACES 2
#define HEX_TERRACE_STEPS (HEX_TERRACES * 2 + 1)
#define HEX_HORIZONTAL_TERRACE_SIZE (1.f/(r32)HEX_TERRACE_STEPS)
#define HEX_VERTICAL_TERRACE_SIZE (1.f/(r32)(HEX_TERRACES + 1))

typedef enum hex_edge_type {
    HEX_EDGE_FLAT,
    HEX_EDGE_TERRACE,
    HEX_EDGE_CLIFF
} hex_edge_type;

internal hex_edge_type
GetHexEdgeType(i32 Elevation1, i32 Elevation2) {
    hex_edge_type Result = HEX_EDGE_CLIFF;
    i32 Diff = Elevation1 - Elevation2;
    if(Diff == 0) {
        Result = HEX_EDGE_FLAT;
    }
    else if((Diff == 1) || (Diff == -1)) {
        Result = HEX_EDGE_TERRACE;
    }
    return Result;
}



typedef enum hex_direction {
    HEX_DIRECTION_SE,
    HEX_DIRECTION_E,
    HEX_DIRECTION_NE,
    HEX_DIRECTION_NW,
    HEX_DIRECTION_W,
    HEX_DIRECTION_SW,

    HEX_DIRECTION_COUNT
} hex_direction;

internal hex_direction
HexGetOppositeDirection(hex_direction Dir) {
    return (Dir + 3) % 6;
}

typedef struct hex_edge_vertices hex_edge_vertices;
struct hex_edge_vertices {
    v3 p0;
    v3 p1;
    v3 p2;
    v3 p3;
};

typedef struct hex_mesh_vertex hex_mesh_vertex;
struct hex_mesh_vertex {
    v3 Position;
    v3 Normal;
    v3 Colour;
    v2 TextureCoord;
    r32 TextureID;
};
internal hex_mesh_vertex
HexMeshVertexInit(v3 P, v3 N, v3 C, v2 TC, r32 TID) {
    hex_mesh_vertex Result = {P, N, C, TC, TID};
    return Result;
};
#define HexMeshVertex(P, N, C, TC, TID) HexMeshVertexInit(P, N, C, TC, TID)


typedef struct hex_mesh hex_mesh;
struct hex_mesh {
    u32 VAO, VBO, Shader;
    u32 VerticesCount;
    hex_mesh_vertex Vertices[MAX_HEX_VERTICES];
};

typedef struct hex_coordinates hex_coordinates;
struct hex_coordinates {
    int x;
    int y;
    int z;
};

typedef struct hex_cell hex_cell;
struct hex_cell {
    i32 Index; //Position in hex_grid's cell array
    i32 Elevation;
    v3 Position;
    v3 Colour;
    hex_cell * Neighbours[HEX_DIRECTION_COUNT];
};


//Collisions
#define MAX_COLLISION_TRIANGLES 4096

typedef struct ray_cast ray_cast;
struct ray_cast {
    v3 Origin;
    v3 Direction;
};

typedef struct collision_triangle collision_triangle;
struct collision_triangle {
    v3 Vertex0;
    v3 Vertex1;
    v3 Vertex2;
};

typedef struct collision_mesh collision_mesh;
struct collision_mesh {
    u32 TriangleCount;
    collision_triangle Triangles[MAX_COLLISION_TRIANGLES];
};

//Used to see if a chunk has been edited before using the finer chunks
typedef struct large_collision_mesh large_collision_mesh;
struct large_collision_mesh {
    collision_triangle Triangles[10]; //6 sides of a cube but don't need the bottom face
};

/*
    World Storage
*/

typedef struct hex_grid_chunk hex_grid_chunk;
struct hex_grid_chunk {
    i32 X, Z;
    hex_mesh HexMesh;
    collision_mesh CollisionMesh;
    large_collision_mesh LargeCollisionMesh;
};


#define HEX_MAX_CHUNKS_WIDE 4
#define HEX_MAX_WIDTH_IN_CELLS (HEX_MAX_CHUNKS_WIDE * HEX_CHUNK_WIDTH)
#define HEX_MAX_CHUNKS_HIGH 4
#define HEX_MAX_CHUNKS (HEX_MAX_CHUNKS_WIDE * HEX_MAX_CHUNKS_HIGH)

typedef struct hex_grid hex_grid;
struct hex_grid {
    u32 MeshShader, MeshTexture;
    i32 Width, Height; //In cells
    hex_cell Cells[HEX_MAX_CHUNKS * HEX_CHUNK_WIDTH * HEX_CHUNK_HEIGHT];
    hex_grid_chunk Chunks[HEX_MAX_CHUNKS];
};
