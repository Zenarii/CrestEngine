#define HEX_OUTER_RADIUS 1.f
#define HEX_OUTER_DIAMETER HEX_OUTER_RADIUS * 2.f
//Note(Zen): sqrt(3)/2
#define HEX_INNER_RADIUS 0.866025404f
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
#define HEX_CHUNK_WIDTH 10
#define HEX_CHUNK_HEIGHT 10

typedef struct hex_mesh hex_mesh;
struct hex_mesh {
    C3DVertex Vertices[MAX_HEX_VERTICES];
};

typedef struct hex_cell hex_cell;
struct hex_cell {
    v3 Position;
};



typedef struct hex_coordinates hex_coordinates;
struct hex_coordinates {
    int x;
    int y;
    int z;
};


//Collisions
#define MAX_COLLISION_TRIANLGES

typedef struct collision_triangle collision_triangle;
struct collision_triangle {
    v3 Vertex0;
    v3 Vertex1;
    v3 Vertex2;
};

typedef struct collision_mesh collision_mesh;
struct collision_mesh {
    u32 TriangleCount;
    collision_triangle Triangles[MAX_COLLISION_TRIANLGES];
};

typedef struct hex_grid hex_grid;
struct hex_grid {
    hex_cell Cells[HEX_CHUNK_WIDTH * HEX_CHUNK_HEIGHT];
    hex_mesh HexMesh;
    collision_mesh CollisionMesh;
};
