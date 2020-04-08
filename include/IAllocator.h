#ifndef IALLOCATOR_H
#define IALLOCATOR_H

class IAllocator {
public:    
    virtual void* allocate(size_t objectSize) = 0;
    virtual void deallocate(void* object) = 0;
    virtual void clear() = 0;
};
/*
//* Placement new overload: This lets us do new(allocator) T(arg1, arg2, ...)
inline void* operator new(size_t objectSize, IAllocator& alloc) {
    return alloc.allocate(objectSize);
}

//* Placement delete overload: This gets called if an exception occurs (and we catch it eventually)
//* during object construction
inline void operator delete(void* object, IAllocator& alloc) {
    alloc.deallocate(object);
}

//* Global function destroy: This is a helper for deleting objects and should be used as the standard way
//* of deleting instead of the delete keyword when using custom allocators
template<typename T>
inline void destroy(T* object, IAllocator& alloc) {
    object->~T();
    alloc.deallocate(object);
}
*/
#endif