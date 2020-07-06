/** 
 * TODO: WE NEED A WAY TO MANAGE ALLOCATION CONTEXT.
 * REMEMBER https://www.youtube.com/watch?time_continue=4&v=ciGQCP6HgqI&feature=emb_title
 * AND ALSO Game Engine Architecture, the way Naughty Dog do it
 */


#ifndef LINEAR_ALLOCATOR_H
#define LINEAR_ALLOCATOR_H

#include <stddef.h>
#include <cstdint>

#include "reflection.h"
#include "IAllocator.h"

class LinearAllocator /*: public IAllocator*/ {
    size_t size;
    size_t used;
    void* base;
public:
    LinearAllocator() = default;
    LinearAllocator(size_t size, void* base);

    void* allocate(size_t objectSize);
    void deallocate(void* object);
    void clear();

    void* getBase();
    size_t getUsed();
    size_t getSize();

    REFLECT()
};

REFLECTION_REGISTRATION(LinearAllocator) {
    CLASS->addField("size", &LinearAllocator::size)
        ->addField("used", &LinearAllocator::used)
        ->addField("base", &LinearAllocator::base);
}

//* Placement new overload: This lets us do new(allocator) T(arg1, arg2, ...)
inline void* operator new(size_t objectSize, LinearAllocator& alloc) {
    return alloc.allocate(objectSize);
}

//* Placement delete overload: This gets called if an exception occurs (and we catch it eventually)
//* during object construction
inline void operator delete(void* object, LinearAllocator& alloc) {
    alloc.deallocate(object);
}

//* Global function destroy: This is a helper for deleting objects and should be used as the standard way
//* of deleting instead of the delete keyword when using custom allocators
template<typename T>
inline void destroy(T* object, LinearAllocator& alloc) {
    object->~T();
    alloc.deallocate(object);
}

#endif