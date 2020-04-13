//Note(Zen): Based on Fisher-Yates Shuffle
//shuffles the array in place
internal void
CrestShuffleArray(i32 * Array, i32 Size) {
    for(i32 i = Size-1; i > 0; --i) {
        i32 j = rand()%(i+1);
        i32 Temp = Array[i];
        Array[i] = Array[j];
        Array[j] = Temp;
    }
}
