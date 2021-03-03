//
// Created by 42025 on 2021/2/26.
//
#include <PGJson/Node.h>
#include <PGJson/MemoryBlockPool.h>

constexpr pg::base::json::SizeType SIZE_OF_NODE = sizeof(pg::base::json::Node);  // sizeof(pg::base::json::Node);
using NodePool = pg::base::json::MemoryBlockPool<SIZE_OF_NODE, pg::base::json::Node>;

using ObjectMemberPool = pg::base::json::MemoryBlockPool<sizeof(pg::base::json::ObjectMember), pg::base::json::ObjectMember>;

// String
void pg::base::json::Node::setString(const Char *str, SizeType length) {
    // if length == SizeType's max, the last Char of str must be 0, truncate the str by Char-0
    // else we think len(str) equals to the param length, don't truncate the str by Char-0

    // reset
    reset(StringFlags);
    m_data.str.sLen = 0;
    m_data.str.usingSmall = true;

    if (length == std::numeric_limits<decltype(length)>::max())
        length = pg::base::json::getCStringLength(str);

    m_data.str.copyFrom(str, length);
}

// Array
void pg::base::json::Node::popBack() {
    PGJSON_DEBUG_ASSERT_EX(__func__, isArray());
    PGJSON_DEBUG_ASSERT_EX("empty array cannot popBack", !empty());
    if (empty()) return;

    --m_data.array.size;
    NodePool::getGlobalInstance()->releaseTypedBlock(m_data.array.values[m_data.array.size]);
}

void pg::base::json::Node::remove(const pg::base::json::Node::ArrayIterator & begin,
                                  const pg::base::json::Node::ArrayIterator & end,
                                  bool keepOrder) {
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

// create & release
pg::base::json::Node * pg::base::json::Node::create() {
    return NodePool::getGlobalInstance()->createTypedBlock();
}

void pg::base::json::Node::release(pg::base::json::Node *pNode) {
    NodePool::getGlobalInstance()->releaseTypedBlock(pNode);
}

// Object
pg::base::json::Node::MemberIterator pg::base::json::Node::getMember(const pg::base::json::Char *name) {
    PGJSON_DEBUG_ASSERT_EX(__func__, isObject());

    MemberIterator iter = memberBegin(), end = memberEnd();
    for (; iter != end; ++iter) {
        if (stringEquals(name, iter->name)) {
            return iter;
        }
    }

    return end;
}

pg::base::json::Node::MemberIterator pg::base::json::Node::getMember(const std::basic_string<Char> &name) {
    PGJSON_DEBUG_ASSERT_EX(__func__, isObject());
    if (name.empty()) return memberEnd();

    MemberIterator iter = memberBegin(), end = memberEnd();
    for (; iter != end; ++iter) {
        if (stringEquals(name, iter->name))
            return iter;
    }

    return end;
}

// alloc memory for ObjectMember
pg::base::json::ObjectMember * pg::base::json::Node::newUninitializedMember(const Char * name, Node ** ppValue) {
    PGJSON_DEBUG_ASSERT_EX(__func__, name != nullptr);

    auto pMember = reinterpret_cast<ObjectMember *>(ObjectMemberPool::getGlobalInstance()->createBlock());

    // init name
    new (&pMember->name) String();
    pMember->name.copyFrom(name);

    // return
    ppValue && (*ppValue = &(pMember->value));
    return pMember;
}

void pg::base::json::Node::removeMember(const pg::base::json::Node::MemberIterator &begin,
                                        const pg::base::json::Node::MemberIterator &end, bool keepOrder) {
    PGJSON_DEBUG_ASSERT_EX(__func__, isObject());
    PGJSON_DEBUG_ASSERT_EX(__func__, begin >= memberBegin());
    PGJSON_DEBUG_ASSERT_EX(__func__, begin <= end);
    PGJSON_DEBUG_ASSERT_EX(__func__, end <= memberEnd());

    ArrayIterator::difference_type lo = begin - memberBegin();
    ArrayIterator::difference_type hi = end - memberBegin();

    for (SizeType i = lo; i < hi; ++i) {
        ObjectMemberPool::getGlobalInstance()->releaseTypedBlock(m_data.object.members[i]);
    }

    std::memmove(m_data.object.members + lo, m_data.object.members + hi, sizeof(void *) * (m_data.object.size - hi));
    m_data.array.size -= static_cast<SizeType>(hi - lo);
}

pg::base::json::Node &pg::base::json::Node::operator[] (const pg::base::json::Char *name) {
    static Node invalidNode;

    auto iter = getMember(name);

    if (iter != memberEnd()) return iter->value;
    return *( new (&invalidNode) Node() );
}

// shared-API Array Object
pg::base::json::Node &pg::base::json::Node::operator[] (pg::base::json::SizeType index) {  // Object Array -> member value  // 待修改
    PGJSON_DEBUG_ASSERT_EX(
            __func__,
            (isArray() && index < m_data.array.size) || (isObject() && index < m_data.object.size));

    if(isArray()) return *m_data.array.values[index];

    return getMember(index)->value;
}

// shared-API Array-Object but don't assert type
void pg::base::json::Node::clear() {

    if (isObject()) {
        for (SizeType i = 0; i < m_data.object.size; ++i) {
            ObjectMemberPool::getGlobalInstance()->releaseTypedBlock(m_data.object.members[i]);
        }
        m_data.object.size = 0;
    }

    if (isArray()) {
        for (SizeType i = 0; i < m_data.array.size; ++i) {
            NodePool::getGlobalInstance()->releaseTypedBlock(m_data.array.values[i]);
        }
        m_data.array.size = 0;
    }

}

pg::base::json::Node::~Node() {
    switch (m_typeFlags) {
        case StringFlags : {
            m_data.str.destroy();
            break;
        }
        case ObjectFlags : {
            clear();
            PGJSON_FREE(m_data.object.members);
            break;
        }
        case ArrayFlags : {
            clear();
            PGJSON_FREE(m_data.array.values);
            break;
        }
        default:
            break;
    }

    m_typeFlags = Node::InvalidFlags;
}
