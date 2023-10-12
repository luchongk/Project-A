/** 
 * TODO: WE MAY NEED A WAY TO MANAGE ALLOCATION CONTEXT.
 * REMEMBER https://www.youtube.com/watch?time_continue=4&v=ciGQCP6HgqI&feature=emb_title
 * AND ALSO Game Engine Architecture, the way Naughty Dog do it
 */


#ifndef MEMORY_H
#define MEMORY_H

#include <cstdlib>
#include <cstring>

#include "types.h"
#include "utils.h"

// Default allocator interface //

typedef void* (*Allocator)(void* old_pointer, u64 old_size, u64 new_size, void* allocator_data);

// malloc allocator //

static void* malloc_allocator(void* old_pointer, u64 old_size, u64 new_size, void* allocator_data) {
    if(old_size == 0) {
        //ALLOC
        assert(new_size);
        return malloc(new_size);
    }
    else if(new_size == 0) {
        //FREE
        assert(old_pointer);
        free(old_pointer);
        return nullptr;
    }
    else {
        //RESIZE
        assert(new_size);
        assert(old_pointer);
        return realloc(old_pointer, new_size);
    }

    //Unreachable
}

// Global default allocator (can be changed by user code) //

Allocator default_allocator = malloc_allocator;
void* default_allocator_data = nullptr;

template<typename T>
T* alloc_(u64 count = 1) {
    //TODO: Think about alignment!
    auto result = (T*)default_allocator(nullptr, 0, sizeof(T) * count, default_allocator_data);
    return result;
}

template<typename T>
T* realloc_(T* oldPointer, u64 oldCount, u64 newCount) {
    auto result = (T*)default_allocator(oldPointer, sizeof(T) * oldCount, sizeof(T) * newCount, default_allocator_data);
    return result;
}

template<typename T>
void free_(T* pointer) {
    default_allocator(pointer, sizeof(T), 0, default_allocator_data);
}


// Linear Arena allocator //

const u8 INVALID_MEMORY_VALUE = 0xba;

struct LinearArena {
    void* base;
    u64 size;
    u64 used;
};

static void init_arena(LinearArena* arena, u64 size, void* base = nullptr) {
    assert(size);

    if(!base) arena->base = malloc(size);
    else arena->base = base;
    
    arena->size = size;
    arena->used = 0;
}

static void reset_arena(LinearArena* arena, u64 marker = 0) {
#if DEBUG
    for(u64 i = marker; i < arena->used; i++) {
        *((u8*)arena->base + i) = INVALID_MEMORY_VALUE;
    }
#endif

    arena->used = marker;
}

static u64 get_marker(LinearArena* arena) {
    return arena->used;
}

static void* linear_allocator(void* old_pointer, u64 old_size, u64 new_size, void* allocator_data) {
    auto arena = (LinearArena*)allocator_data;
    assert(arena);

    if(old_size == 0) {
        //ALLOC
        assert(new_size);
        assert(arena->used + new_size <= arena->size);
        void* result = (u8*)arena->base + arena->used;
        arena->used += new_size;
        
        return result;
    }
    else if(new_size == 0) {
        //FREE: Not supported.
        //assert(false);
    }
    else {
        //RESIZE: We just copy the old data, we never deallocate.
        assert(new_size);
        assert(arena->used + new_size <= arena->size);
        void* result = (u8*)arena->base + arena->used;
        arena->used += new_size;

        assert(old_pointer);
        memcpy(result, old_pointer, old_size);
        return result;
    }

    return nullptr; //Unreachable
}

#endif