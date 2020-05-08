typedef struct ray_cast ray_cast;
struct ray_cast {
    v3 Origin;
    v3 Direction;
};

typedef struct collision_result collision_result;
struct collision_result {
    b32 DidIntersect;
    v3 IntersectionPoint;
};

#define MAX_COLLISIONS_PER_RAYCAST 8

typedef struct collision_result_set collision_result_set;
struct collision_result_set {
    i32 Count;
    collision_result Results[MAX_COLLISIONS_PER_RAYCAST];
};
