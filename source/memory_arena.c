typedef struct memory_arena memory_arena;
struct memory_arena {
    size_t BufferLength;
    size_t PreviousOffset;
    size_t CurrentOffset;
    unsigned char * Buffer;
};

internal pointer
AlignForward(pointer Pointer, size_t Alignment) {
    pointer p, a, Modulo;
    Assert(IsPowerOfTwo(Alignment));

    p = Pointer;
    a = (pointer)Alignment;

    //Note(Zen): Identical to p % a. Faster since a = 2^k.
    Modulo = p & (a-1);
    if(Modulo != 0) {
        //Note(Zen): If p is unaligned move address forwards
        p += a - Modulo;
    }
    return p;
}

internal void
_ArenaGrow(memory_arena * Arena, size_t NewLength) {
    size_t NewSize = MAX(2 * Arena->BufferLength, NewLength);
    Assert(NewLength <= NewSize);

    Arena->Buffer = realloc(Arena->Buffer, NewSize);
    Arena->BufferLength = NewSize;
}

internal void *
_ArenaAllocAligned(memory_arena * Arena, size_t Size, size_t Alignment) {
    pointer CurrentPointer = (pointer)Arena->Buffer + (pointer)Arena->CurrentOffset;
    pointer Offset = AlignForward(CurrentPointer, Alignment);
    Offset -= (pointer)Arena->Buffer;

    if(Offset + Size <= Arena->BufferLength) {
        void * Pointer = &Arena->Buffer[Offset];
        Arena->PreviousOffset = Offset;
        Arena->CurrentOffset = Offset+Size;

        memset(Pointer, 0, Size);
        return Pointer;
    }
    else {
        _ArenaGrow(Arena, Arena->BufferLength + Size);
        return _ArenaAllocAligned(Arena, Size, Alignment);
    }
}

internal void *
_ArenaResizeAligned(memory_arena * Arena, void * OldMemory, size_t OldSize, size_t NewSize, size_t Alignment) {
    unsigned char * OldMemoryC = (unsigned char *)OldMemory;

    Assert(IsPowerOfTwo(Alignment));

    if(OldMemoryC == 0 || OldMemoryC == NULL) {
        return _ArenaAllocAligned(Arena, NewSize, Alignment);
    }
    else if((Arena->Buffer <= OldMemoryC) && (OldMemoryC < Arena->Buffer + Arena->BufferLength)) {
        if(Arena->Buffer + Arena->PreviousOffset == OldMemoryC) {
            Arena->CurrentOffset = Arena->PreviousOffset + NewSize;
            if(NewSize > OldSize) {
                memset(&Arena->Buffer[Arena->CurrentOffset], 0, NewSize-OldSize);
            }
            return OldMemory;
        }
        else {
            void * NewMemory = _ArenaAllocAligned(Arena, NewSize, Alignment);
            size_t CopySize = (OldSize < NewSize) ? OldSize : NewSize;
            //Copy across the old memory
            memmove(NewMemory, OldMemory, CopySize);
            return NewMemory;
        }
    }
    else {
        Assert(!"Resized memory is out of bounds in arena.\n");
        return 0;
    }

}

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT (2 * sizeof(void *))
#endif
internal void *
ArenaAlloc(memory_arena * Arena, size_t Size) {
    return _ArenaAllocAligned(Arena, Size, DEFAULT_ALIGNMENT);
}


internal void *
ArenaResize(memory_arena * Arena, void * OldMemory, size_t OldSize, size_t NewSize) {
    return _ArenaResizeAligned(Arena, OldMemory, OldSize, NewSize, DEFAULT_ALIGNMENT);
}


internal void
ArenaInit(memory_arena * Arena, void * BackBuffer, size_t BackBufferLength) {
    Arena->Buffer = (unsigned char *) BackBuffer;
    Arena->BufferLength = BackBufferLength;
    Arena->CurrentOffset = 0;
    Arena->PreviousOffset = 0;
}

internal void
ArenaFreeAll(memory_arena * Arena) {
    Arena->CurrentOffset = 0;
    Arena->PreviousOffset = 0;
}
