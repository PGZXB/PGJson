//
// Created by 42025 on 2021/3/8.
//

#include <PGJson/Allocator.h>
#include <PGJson/MemoryBlockPool.h>
#include <PGJson/Node.h>
#include <PGJson/pgjson.h>

class PGJsonManager {
public:
    void p() {}

    PGJsonManager();

    ~PGJsonManager();
};


PGJsonManager::PGJsonManager() {
    using namespace pg::base::json;

    MallocAllocator::s_pInstance = new MallocAllocator();
    MemoryBlockPool<sizeof(Node), Node>::s_pInstance = new MemoryBlockPool<sizeof(Node), Node>();
    MemoryBlockPool<sizeof(ObjectMember), ObjectMember>::s_pInstance = new MemoryBlockPool<sizeof(ObjectMember), ObjectMember>();
}

PGJsonManager::~PGJsonManager() {
    using namespace pg::base::json;

    delete MemoryBlockPool<sizeof(Node), Node>::s_pInstance;
    delete MemoryBlockPool<sizeof(ObjectMember), ObjectMember>::s_pInstance;
    delete MallocAllocator::s_pInstance;
}

static PGJsonManager g_manager;

void pg::base::json::PGJSON_INNER_NAMESPACE::PGJSON_INIT() {
}