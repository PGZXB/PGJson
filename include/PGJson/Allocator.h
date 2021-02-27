//
// Created by 42025 on 2021/2/27.
//
#ifndef PGTEST_ALLOCATOR_H
#define PGTEST_ALLOCATOR_H

#include <PGJson/fwd.h>
PGJSON_NAMESPACE_START

class MallocAllocator {
public:
    MallocAllocator() = default;
    ~MallocAllocator() = default;

    void * allocate(SizeType size) {
        return ::malloc(size);
    }

    void * reallocate(void * ptr, SizeType newSize) {
        return ::realloc(ptr, newSize);
    }

    void deallocate(void * ptr) {
        free(ptr);
    }
};

using DefaultMemoryAllocator = MallocAllocator;

PGJSON_NAMESPACE_END
#endif //PGTEST_ALLOCATOR_H
