//
// Created by 42025 on 2021/2/26.
//
#include <PGJson/Node.h>
#include <PGJson/MemoryBlockPool.h>

constexpr pg::base::json::SizeType SIZE_OF_NODE = 32;  // sizeof(pg::base::json::Node);
using NodePool = pg::base::json::MemoryBlockPool<SIZE_OF_NODE, pg::base::json::Node>;

// String
void pg::base::json::Node::setString(const Char *str, SizeType length) {
    // if length == SizeType's max, the last Char of str must be 0, truncate the str by Char-0
    // else we think len(str) equals to the param length, don't truncate the str by Char-0

    // reset
    reset(StringFlags);

    if (length == std::numeric_limits<decltype(length)>::max())
        length = pg::base::json::getCStringLength(str);

    // small-string, if the true-length <= String::SMALL_STRING_MAX_SIZE
    if (length <= String::SMALL_STRING_MAX_SIZE) {
        _m_smallStringUsed = true;
        std::memcpy(m_data.str.sData, str, length);
        m_data.str.sLen = static_cast<std::uint8_t>(length);
        return;
    }

    // if the true-length is more than String::SMALL_STRING_MAX_SIZE
    // allocate memory
    void * ptr = PGJSON_MALLOC(sizeof(Char) * (length + 1));

    // copy string from str to ptr
    std::memcpy(ptr, str, length);
    reinterpret_cast<Byte *>(ptr)[length] = 0;
    _m_smallStringUsed = false;
    m_data.str.data = reinterpret_cast<Char*>(ptr);
    m_data.str.length = length;
}

// Array
void pg::base::json::Node::clear() {
    PGJSON_DEBUG_ASSERT_EX(__func__, isArray());

    for (SizeType i = 0; i < m_data.array.size; ++i) {
        NodePool::getGlobalInstance()->releaseTypedBlock(m_data.array.values[i]);
    }
    m_data.array.size = 0;
}

void pg::base::json::Node::popBack() {
    PGJSON_DEBUG_ASSERT_EX(__func__, isArray());
    PGJSON_DEBUG_ASSERT_EX("empty array cannot popBack", !empty());
    if (empty()) return;

    --m_data.array.size;
    NodePool::getGlobalInstance()->releaseTypedBlock(m_data.array.values[m_data.array.size]);
}



// create
pg::base::json::Node * pg::base::json::Node::create() {
    return NodePool::getGlobalInstance()->createTypedBlock();
}

void pg::base::json::Node::remove(const pg::base::json::Node::ArrayIterator & begin,
                                  const pg::base::json::Node::ArrayIterator & end) {
    PGJSON_DEBUG_ASSERT_EX(__func__, isArray());
    PGJSON_DEBUG_ASSERT_EX(__func__, begin >= this->begin());
    PGJSON_DEBUG_ASSERT_EX(__func__, begin < end);
    PGJSON_DEBUG_ASSERT_EX(__func__, end <= this->end());

    ArrayIterator::difference_type lo = begin - this->begin();
    ArrayIterator::difference_type hi = end - this->begin();

    for (SizeType i = lo; i < hi; ++i) {
        NodePool::getGlobalInstance()->releaseTypedBlock(m_data.array.values[i]);
    }

    std::memmove(m_data.array.values + lo, m_data.array.values + hi, sizeof(void *) * (m_data.array.size - hi));
    m_data.array.size -= static_cast<SizeType>(hi - lo);
}




