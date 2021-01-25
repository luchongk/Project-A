/** 
 * TODO: WE NEED A WAY TO MANAGE ALLOCATION CONTEXT.
 * REMEMBER https://www.youtube.com/watch?time_continue=4&v=ciGQCP6HgqI&feature=emb_title
 * AND ALSO Game Engine Architecture, the way Naughty Dog do it
 */


#ifndef MEMORY_H
#define MEMORY_H

typedef void* (*Allocator)(void* old_pointer, size_t old_size, size_t new_size, bool free_all, void* allocator_data);

struct LinearArena {
    void* base;
    size_t size;
    size_t used;
};

static void init_arena(LinearArena* arena, size_t size, void* base) {
    assert(base && size);
    
    arena->base = base;
    arena->size = size;
    arena->used = 0;
}

static void* linear_allocator(void* old_pointer, size_t old_size, size_t new_size, bool free_all, void* allocator_data) {
    auto arena = (LinearArena*)allocator_data;

    if(free_all) {
        arena->used = 0;
    }
    else if(old_size == 0) {
        //Alloc
        assert(new_size);
        assert(arena->used + new_size <= arena->size);
        void* result = (uint8_t*)arena->base + arena->used;
        arena->used += new_size;
        
        return result;
    }
    else if(new_size == 0) {
        //Free: Not supported
        assert(false);
    }
    else {
        //Resize: Not supported
        assert(false);
    }

    return nullptr; //Unreachable
}

Allocator default_allocator = linear_allocator;
void* default_allocator_data = nullptr;

inline static void* alloc_size(size_t size) {
    //TODO: Think about alignment!
    auto result = default_allocator(nullptr, 0, size, false, default_allocator_data);
    return result;
}

template<typename T>
inline static T* alloc() {
    //TODO: Think about alignment!
    auto result = (T*)alloc_size(sizeof(T));
    return result;
}

template<typename T>
inline static void _free(T* pointer) {
    default_allocator(pointer, sizeof(T), 0, false, default_allocator_data);
}

inline static void freeAll() {
    default_allocator(nullptr, 0, 0, true, default_allocator_data);
}

#endif