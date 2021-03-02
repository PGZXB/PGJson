//
// Created by 42025 on 2021/2/27.
//
#ifndef PGTEST_MEMORYBLOCKPOOL_H
#define PGTEST_MEMORYBLOCKPOOL_H

#include <algorithm>
#include <PGJson/fwd.h>
#include <PGJson/Allocator.h>
#include <PGJson/utils.h>
PGJSON_NAMESPACE_START

template<SizeType BLOCK_SIZE>
struct Block {
    Byte data[BLOCK_SIZE] = { 0 };
};

// MemoryBlockPool for Node, ObjectMember
// can be better
template<SizeType BLOCK_SIZE, typename Type = Block<BLOCK_SIZE>, typename Allocator = DefaultMemoryAllocator>
class MemoryBlockPool {  // MemoryBlockPool
    using BlockType = Type;
    using BlockPtrType = BlockType *;
    using BlockRefType = BlockType &;
    using ConstBlockPtrType = const BlockType *;
    using ConstBlockRefType = const BlockType &;

    using ObjectType = Type;
    using ObjectPtrType = ObjectType *;
    using ObjectRefType = ObjectType &;
    using ConstObjectPtrType = const ObjectType *;
    using ConstObjectRefType = const ObjectType &;

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

    PGJSON_STATIC_ASSERT_EX("BLOCK_SIZE should equals to the size of Type", BLOCK_SIZE == sizeof(Type));

    static constexpr SizeType DEFAULT_BLOCK_SIZE = max(sizeof(Type), sizeof(BlockInfo));
    static constexpr std::uint16_t DEFAULT_BLOCK_NUM_PER_CHUNK = 64;  // 64
    static constexpr std::uint32_t DEFAULT_CHUNK_NUM = 8;  // 8

public:
    explicit MemoryBlockPool(uint32_t chunksCapacity = DEFAULT_CHUNK_NUM)
            : m_chunkNum(1),
              m_chunksCapacity(chunksCapacity),
              m_topBlockInfo(0, 0) {
        m_chunks = reinterpret_cast<void**>(m_allocator.allocate(sizeof(void *) * chunksCapacity));
        m_chunks[0] = newChunk();
        initChunk(m_chunks[0], 0);
    }

    ~MemoryBlockPool() {
        for (std::uint32_t i = 0; i < m_chunkNum; ++i) {  // release chunk in chunks one by one
            Byte * pBlock = reinterpret_cast<Byte *>(m_chunks[i]);
            for (std::uint16_t j = 0; j < DEFAULT_BLOCK_NUM_PER_CHUNK; ++j, pBlock += DEFAULT_BLOCK_SIZE) {  // deconstruct objects
                if (checkBlockUsed(pBlock)) {  // deconstruct object
                    auto pObject = reinterpret_cast<ObjectPtrType>(pBlock);
                    pObject->~ObjectType();
                }
            }
            m_allocator.deallocate(m_chunks[i]);  // release chunk
        }

        // release chunk-array
        m_allocator.deallocate(m_chunks);
    }

    template<typename... Args>
    ObjectPtrType createTypedBlock(Args && ... args) {  // can be better
        const void * ptr = reinterpret_cast<const void *>(createBlock());
        void * ptr_ = const_cast<void *>(ptr);

        return new (ptr_) ObjectType(std::forward<Args>(args)...);  // construct object and return
    }

    void releaseTypedBlock(ObjectPtrType /*&*/ pBlock) {
        if (checkBlockUsed(pBlock)) pBlock->~ObjectType();  // deconstruct object
        releaseBlock(pBlock);
    }

//    ObjectPtrType * createTypedBlockArray(SizeType num) {
//        ObjectPtrType * res = m_allocator.allocate(sizeof(void *) * num);
//
//        for (SizeType i = 0; i < num; ++i) {
//            res[i] = reinterpret_cast<ObjectPtrType>(createBlock());
//        }
//
//        return res;
//    }
//
//    ObjectPtrType * reserveTypeBlockArray(ObjectPtrType * blockArray, SizeType oldSize, SizeType newSize) {
//        if (oldSize == newSize) return blockArray;
//
//        if (newSize < oldSize) {
//            for (SizeType i = newSize; i < oldSize; ++i) {
//                releaseTypedBlock(blockArray[i]);
//            }
//            return m_allocator.reallocate(newSize);
//        }
//
//        ObjectPtrType * res = m_allocator.reallocate(newSize);
//
//        for (SizeType i = oldSize; i < newSize; ++i) {
//            res[i] = createBlock();
//        }
//
//        return res;
//    }
//
//    void releaseTypedBlockArray(ObjectPtrType * blockArray, SizeType size) {
//        for (SizeType i = 0; i < size; ++i) {
//            releaseTypedBlock(blockArray[i]);
//        }
//
//        m_allocator.deallocate(blockArray);
//    }

    static MemoryBlockPool * getGlobalInstance() {

        return s_pInstance;
    }
public:
    BlockPtrType createBlock() {
        extend();  // extend, check and extend

        Byte * ptr = reinterpret_cast<Byte *>(m_chunks[m_topBlockInfo.next.chunkIndexInChunks]);  // find chunk in chunks
        ptr += m_topBlockInfo.next.blockIndexInChunk * DEFAULT_BLOCK_SIZE;  // find block in chunk

        m_topBlockInfo = *reinterpret_cast<BlockInfo *>(ptr);  // update the m_topBlockInfo

        return reinterpret_cast<BlockPtrType>(ptr);
    }

    void releaseBlock(void * pBlock) {
        new (pBlock) BlockInfo(m_topBlockInfo);  // construct BlockInfo on *pBlock
        getBlockIndex(  // update the m_topBlockInfo
                pBlock,
                &m_topBlockInfo.next.blockIndexInChunk,
                &m_topBlockInfo.next.chunkIndexInChunks
        );
        pBlock = nullptr;
    }

private:
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
public:
    static MemoryBlockPool * s_pInstance;

private:
    void ** m_chunks = nullptr;
    std::uint32_t m_chunkNum;
    std::uint32_t m_chunksCapacity;
    BlockInfo m_topBlockInfo;
    Allocator m_allocator;
};

template<SizeType BLOCK_SIZE, typename Type, typename Allocator>
MemoryBlockPool<BLOCK_SIZE, Type, Allocator> * MemoryBlockPool<BLOCK_SIZE, Type, Allocator>::s_pInstance = nullptr;

template<SizeType BLOCK_SIZE, typename Type, typename Allocator>
const typename MemoryBlockPool<BLOCK_SIZE, Type, Allocator>::BlockInfo MemoryBlockPool<BLOCK_SIZE, Type, Allocator>::BlockInfo::NIL{
    std::numeric_limits<std::uint16_t>::max(),
    std::numeric_limits<std::uint32_t>::max()
};

template<typename Type, typename Allocator = MallocAllocator>
class ObjectPool : public MemoryBlockPool<sizeof(Type), Type, Allocator> {

};

PGJSON_NAMESPACE_END
#endif //PGTEST_MEMORYBLOCKPOOL_H
