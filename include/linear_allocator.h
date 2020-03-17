#ifndef LINEAR_ALLOCATOR_H
#define LINEAR_ALLOCATOR_H

#include <stddef.h>
#include <cstdint>

class LinearAllocator {
    size_t size;
    size_t used;
    void* base;

public:
    LinearAllocator(size_t size, void* base);

    template<typename T, typename... args>
    T* push(args... argList) {
        assert(used + sizeof(T) <= size);
        
        used += sizeof(T);

        void* ptr = (uint8_t*)base + used;
        return new (ptr) T(argList...);
    }

    template<typename T>
    T* pushArray(size_t count) {
        assert(used + sizeof(T) * count <= size);
        
        void* ptr = (uint8_t*)base + used;
        used += sizeof(T) * count;
        
        return ptr;
    }

    void* getBase();
    size_t getUsed();
    size_t getSize();
    void clear();
};

#endif