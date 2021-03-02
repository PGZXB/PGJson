//
// Created by 42025 on 2021/2/27.
//
#ifndef PGTEST_ALLOCATOR_H
#define PGTEST_ALLOCATOR_H

#include <new>
#include <PGJson/fwd.h>
PGJSON_NAMESPACE_START

class MallocAllocator {
public:
    MallocAllocator() = default;
    ~MallocAllocator() = default;

    void * allocate(SizeType size) {
        void * p = ::malloc(size);

#ifdef PGJSON_WITH_CXX_EXCEPTION
        if (!p) throw std::bad_alloc();
#endif

        return p;
    }

    void * reallocate(void * ptr, SizeType newSize) {
        void * p = ::realloc(ptr, newSize);

#ifdef PGJSON_WITH_CXX_EXCEPTION
        if (!p) throw std::bad_alloc();
#endif

        return p;
    }

    void deallocate(void * ptr) noexcept {
        free(ptr);
    }

    static MallocAllocator * getGlobalInstance() {

        return s_pInstance;
    }

    static MallocAllocator * s_pInstance;
};

using DefaultMemoryAllocator = MallocAllocator;

PGJSON_NAMESPACE_END
#endif //PGTEST_ALLOCATOR_H
