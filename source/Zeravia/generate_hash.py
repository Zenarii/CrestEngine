#
#   Zenarii 2020
#   A simple script to generate the hash grid for random noise
#   in the CrestEngine.
#
from random import random, seed

def DrawHashGrid(rand_seed, size):
    hash_grid = []
    seed(rand_seed)
    for i in range(size*size):
        hash_grid.append(random())
    with open("NoiseHashGrid.inc", "w") as f:
        print("//Generated with generate_hash.py!\n#define HASH_GRID_SIZE " + str(size) + "\nglobal const float HashNoiseGrid[] = {", ", ".join(map(str, hash_grid)) + "};", file=f)
    

def main():
    seed = 0
    size = 256
    DrawHashGrid(seed, size)

if __name__ == "__main__":
    main()
