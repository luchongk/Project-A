#include "linear_allocator.h"

LinearAllocator::LinearAllocator(size_t size, void* base) : size(size), base(base), used(0) {}

void* LinearAllocator::getBase() {
    return this->base;
}

size_t LinearAllocator::getUsed() {
    return this->used;
}

size_t LinearAllocator::getSize() {
    return this->size;
} 

void LinearAllocator::clear() {
    this->used = 0;
}