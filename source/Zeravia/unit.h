#define MAX_PLAYER_UNITS 16
#define MAX_ENEMY_UNITS 32
#define MAX_UNIT_MOVE 8
#define UNIT_SPEED 5.f
#define MAX_ITEMS 5

typedef struct unit unit;
struct unit {
    //Note(Zen): Gameplay Info
    u32 CellIndex;
    b32 HasMoved;
    b32 Exhausted;

    //Note(Zen): Visual Info
    v3 Position;
};

typedef enum item_type item_type;
enum item_type {
    ITEM_TYPE_NONE,
    ITEM_TYPE_WEAPON,
    ITEM_TYPE_USABLE //TODO(Zen): Flesh this out
};

typedef enum unit_stat unit_stat;
enum unit_stat {
    STAT_HEALTH //etc.
};

typedef struct item item;
struct item {
    //TODO(Zen): Icon
    char * Name;
    item_type Type;
    union {
        struct {
            unit_stat AffectedStat;
            i32 Amount;
        };
        struct {
            i32 Damage;
            i32 Range;
            //i32 Weight; etc.
        };
    };
};

typedef struct inventory inventory;
struct inventory {
    u32 ItemCount;
    item Items[MAX_ITEMS];
};
