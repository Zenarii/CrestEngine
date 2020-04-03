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
    {-HEX_INNER_RADIUS, 0.f, 0.5f * HEX_OUTER_RADIUS}
};

#define MAX_HEX_VERTICES 4096
#define MAX_HEX_INDICES 2046

#define HEX_CHUNK_WIDTH 7
#define HEX_CHUNK_HEIGHT 5

#define HEX_SOLID_FACTOR 0.75f
#define HEX_BLEND_FACTOR 1.f - HEX_SOLID_FACTOR

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


typedef struct hex_mesh hex_mesh;
struct hex_mesh {
    u32 VAO, VBO, Shader;
    u32 VerticesCount;
    C3DVertex Vertices[MAX_HEX_VERTICES];
    // u32 IndicesCount;
    // u32 Indices[MAX_HEX_INDICES];
};

typedef struct hex_coordinates hex_coordinates;
struct hex_coordinates {
    int x;
    int y;
    int z;
};

typedef struct hex_cell hex_cell;
struct hex_cell {
    v3 Position;
    v3 Colour;
    hex_cell * Neighbours[HEX_DIRECTION_COUNT];
};


//Collisions
#define MAX_COLLISION_TRIANGLES 2048

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

typedef struct hex_grid hex_grid;
struct hex_grid {
    hex_cell Cells[HEX_CHUNK_WIDTH * HEX_CHUNK_HEIGHT];
    hex_mesh HexMesh;
    collision_mesh CollisionMesh;
};
