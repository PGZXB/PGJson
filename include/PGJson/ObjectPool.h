//
// Created by 42025 on 2021/2/27.
//
#ifndef PGTEST_OBJECTPOOL_H
#define PGTEST_OBJECTPOOL_H

#include <algorithm>
#include <PGJson/fwd.h>
#include <PGJson/Allocator.h>
#include <PGJson/utils.h>
PGJSON_NAMESPACE_START


// ObjectPool for Node, ObjectMember
template<typename Type, typename Allocator = DefaultMemoryAllocator>
class ObjectPool {
    using ObjectType = Type;
    using ObjectPtrType = Type *;
    using ObjectRefType = Type &;
    using ConstObjectPtrType = const Type *;
    using ConstObjectRefType = const Type &;

    struct BlockInfo {  // 8 bytes
        static const BlockInfo NIL;

        BlockInfo(std::uint32_t blockIndexInChunk, std::uint32_t chunkIndexInChunks)
        : next(blockIndexInChunk, chunkIndexInChunks) {

        }

        bool operator== (const BlockInfo & other) {
            PGJSON_DEBUG_ASSERT(next.magicTag == other.next.magicTag && next.magicTag == Next::MAGIC_TAG);

            return next.blockIndexInChunk == other.next.blockIndexInChunk &&
                   next.chunkIndexInChunks == other.next.chunkIndexInChunks;
        }

        struct Next {
            static constexpr std::uint16_t MAGIC_TAG = 1693689493U % 65535U;

            Next(std::uint32_t bi, std::uint32_t ci)
            : blockIndexInChunk(bi), chunkIndexInChunks(ci), magicTag(MAGIC_TAG) {

            }

            std::uint16_t magicTag;
            std::uint16_t blockIndexInChunk;
            std::uint32_t chunkIndexInChunks;
        } next;
    };

    static constexpr SizeType DEFAULT_BLOCK_SIZE = max(sizeof(Type), sizeof(BlockInfo));
    static constexpr std::uint16_t DEFAULT_BLOCK_NUM_PER_CHUNK = 64;  // 64
    static constexpr std::uint32_t DEFAULT_CHUNK_NUM = 8;  // 8

public:
    ~ObjectPool() {
        for (std::uint32_t i = 0; i < m_chunkNum; ++i) {  // release chunk in chunks one by one
            Byte * pBlock = reinterpret_cast<Byte *>(m_chunks[i]);
            for (std::uint16_t j = 0; j < DEFAULT_BLOCK_NUM_PER_CHUNK; ++j, pBlock += DEFAULT_BLOCK_SIZE) {  // deconstruct objects
                if (checkBlockUsed(pBlock)) {  // deconstruct object
                    auto pObject = reinterpret_cast<ObjectPtrType>(m_chunks[i]);
                    pObject->~ObjectType();
                }
            }
            m_allocator.deallocate(m_chunks[i]);  // release chunk
        }

        // release chunk-array
        m_allocator.deallocate(m_chunks);
    }

    template<typename... Args>
    ObjectPtrType createObject(Args && ... args) {  // can be better
        extend();  // extend, check and extend

        Byte * ptr = reinterpret_cast<Byte *>(m_chunks[m_topBlockInfo.next.chunkIndexInChunks]);  // find chunk in chunks
        ptr += m_topBlockInfo.next.blockIndexInChunk * DEFAULT_BLOCK_SIZE;  // find block in chunk

        m_topBlockInfo = *reinterpret_cast<BlockInfo *>(ptr);  // update the m_topBlockInfo

        return new (ptr) ObjectType(std::forward<Args>(args)...);  // construct object and return
    }

    void destroyObject(ObjectPtrType /*&*/ ptr) {
        ptr->~ObjectType();  // construct object
        new (ptr) BlockInfo(m_topBlockInfo);  // construct BlockInfo on *ptr
        getBlockIndex(  // update the m_topBlockInfo
            ptr,
            &m_topBlockInfo.next.blockIndexInChunk,
            &m_topBlockInfo.next.chunkIndexInChunks
        );
        ptr = nullptr;
    }


    static ObjectPool * getInstance() {
        static ObjectPool s_instance;

        return &s_instance;
    }
private:
    explicit ObjectPool(uint32_t chunksCapacity = DEFAULT_CHUNK_NUM)
            : m_chunkNum(1),
              m_chunksCapacity(chunksCapacity),
              m_topBlockInfo(0, 0) {
        m_chunks = reinterpret_cast<void**>(m_allocator.allocate(sizeof(void *) * chunksCapacity));
        m_chunks[0] = newChunk();
        initChunk(m_chunks[0], 0);
    }

    void * newChunk() {
        return m_allocator.allocate(DEFAULT_BLOCK_SIZE * DEFAULT_BLOCK_NUM_PER_CHUNK);
    }

    void initChunk(void * pChunk, uint32_t chunkIndexInChunks) {  // link all blocks in the chunk
        Byte * pbChunk = reinterpret_cast<Byte *>(pChunk);
        BlockInfo * ptr = nullptr;

        PGJSON_STATIC_ASSERT(DEFAULT_BLOCK_NUM_PER_CHUNK >= 1);
        for (std::uint16_t i = 0; i < DEFAULT_BLOCK_NUM_PER_CHUNK - 1; ++i, pbChunk += DEFAULT_BLOCK_SIZE) {
            ptr = reinterpret_cast<BlockInfo *>(pbChunk);
            new (ptr) BlockInfo(i + 1, chunkIndexInChunks);
        }
        ptr = reinterpret_cast<BlockInfo *>(pbChunk);
        new (ptr) BlockInfo(BlockInfo::NIL);
    }

    bool checkBlockUsed(void * pBlock) {  // check : if the block was used by checking : magicTag == MAGIC_TAG
        BlockInfo * ptr = nullptr;
        ptr = reinterpret_cast<BlockInfo *>(pBlock);
        return ptr->next.magicTag != BlockInfo::Next::MAGIC_TAG;
    }

    void getBlockIndex(void * ptr, std::uint16_t * pBlockIndexInChunk, std::uint32_t * pChunkIndexInChunks) {
        std::uint32_t i = 0;
        std::uint64_t diff = 0;
        for (; i < m_chunkNum; ++i) {
            if (ptr >= m_chunks[i]) {
                diff = (reinterpret_cast<Byte *>(ptr) - reinterpret_cast<Byte *>(m_chunks[i])) / DEFAULT_BLOCK_SIZE;
                if (diff < DEFAULT_BLOCK_NUM_PER_CHUNK) {
                    pBlockIndexInChunk && (*pBlockIndexInChunk = diff);
                    pChunkIndexInChunks && (*pChunkIndexInChunks = i);
                    return;
                }
            }
        }
    }

    void extend() {
        //      if the m_topBlockInfo == NIL:
        //          if m_chunks is full:
        //              reallocate m_chunks to m_chunksCapacity * 2
        //              update m_chunksCapacity
        //          add a Chunk and init it
        //          update m_chunkNum
        //          update m_topBlockInfo
        if (m_topBlockInfo == BlockInfo::NIL) {
            if (m_chunksCapacity == m_chunkNum)
                m_chunks = reinterpret_cast<void**>(m_allocator.reallocate(m_chunks, sizeof(void *) * (m_chunksCapacity <<= 1U)));

            m_chunks[m_chunkNum] = newChunk();
            initChunk(m_chunks[m_chunkNum], m_chunkNum);

            new (&m_topBlockInfo) BlockInfo(0, m_chunkNum);
            ++m_chunkNum;
        }
    }

private:
    void ** m_chunks = nullptr;
    std::uint32_t m_chunkNum;
    std::uint32_t m_chunksCapacity;
    BlockInfo m_topBlockInfo;
    Allocator m_allocator;
};

template<typename Type, typename Allocator>
const typename ObjectPool<Type, Allocator>::BlockInfo ObjectPool<Type, Allocator>::BlockInfo::NIL{
    std::numeric_limits<std::uint16_t>::max(),
    std::numeric_limits<std::uint32_t>::max()
};

PGJSON_NAMESPACE_END
#endif //PGTEST_OBJECTPOOL_H
