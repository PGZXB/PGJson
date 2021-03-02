//
// Created by 42025 on 2021/2/25.
//
#ifndef PGJSON_NODE_H
#define PGJSON_NODE_H

#include <limits>
#include <cstring>
#include <PGJson/fwd.h>
#include <PGJson/String.h>
#include <PGJson/utils.h>
#include <PGJson/Allocator.h>
#include <PGJson/Iterator.h>

#ifdef PGJSON_WITH_STL
    #include <iostream>
    #include <string>
#endif

PGJSON_NAMESPACE_START

constexpr struct CreateObjectTag {} objectTag;
constexpr struct CreateArrayTag{} arrayTag;

class Node;
struct ObjectMember;

template<SizeType BLOCK_SIZE, typename Type, typename Allocator>
class MemoryBlockPool;

class Node {
    static constexpr Enum InvalidFlags = TypeFlag::InvalidFlag;
    static constexpr Enum FalseFlags = TypeFlag::FalseFlag | TypeFlag::BoolFlag;
    static constexpr Enum TrueFlags = TypeFlag::TrueFlag | TypeFlag::BoolFlag;
    static constexpr Enum BoolFlags = TypeFlag::BoolFlag;
    static constexpr Enum StringFlags = TypeFlag::StringFlag;
    static constexpr Enum NumberFlags = TypeFlag::NumberFlag;
    static constexpr Enum Int64Flags = TypeFlag::IntegerFlag | TypeFlag::NumberFlag;
    static constexpr Enum UInt64Flags = TypeFlag::UIntegerFlag | TypeFlag::NumberFlag;
    static constexpr Enum DoubleFlags = TypeFlag::DoubleFlag | TypeFlag::NumberFlag;
    static constexpr Enum RawFlags = TypeFlag::RawFlag;
    static constexpr Enum ObjectFlags = TypeFlag::ObjectFlag;
    static constexpr Enum ArrayFlags = TypeFlag::ArrayFlag;
    static constexpr Enum NullFlags = TypeFlag::NullFlag;

    friend class MemoryBlockPool<32, Node, DefaultMemoryAllocator>;

    union Variant;

    union Number {
        std::int64_t i64;
        std::uint64_t u64;
        double d;
    };

    struct Object {
        static constexpr SizeType DEFAULT_CAPACITY = 4;

        SizeType size = 0;
        SizeType capacity = 0;
        ObjectMember ** members = nullptr;
    };

    struct Array {
        static constexpr SizeType DEFAULT_CAPACITY = 4;

        SizeType size = 0;
        SizeType capacity = 0;
        Node ** values = nullptr;
    };

    union Variant {
        Number number;
        String str;
        Object object;
        Array array;
    };
public:
    using ArrayIterator = PoPSeqIterator<Node>;
    using ConstArrayIterator = PoPSeqIterator<const Node>;
    using MemberIterator = PoPSeqIterator<ObjectMember>;
    using ConstMemberIterator = const PoPSeqIterator<const ObjectMember>;

public:
    ~Node();

    // type
    Enum getType() const {
        return m_typeFlags;
    }

    bool isValid() const { return m_typeFlags != InvalidFlags; }
    bool isNull() const { return is(TypeFlag::NullFlag); }
    bool isFalse() const { return is(TypeFlag::FalseFlag); }
    bool isTrue() const { return is(TypeFlag::TrueFlag); }
    bool isBool() const { return is(TypeFlag::BoolFlag); }
    bool isObject() const { return is(TypeFlag::ObjectFlag); }
    bool isArray() const { return is(TypeFlag::ArrayFlag); }
    bool isNumber() const { return is(TypeFlag::NumberFlag); }
    bool isString() const { return is(TypeFlag::StringFlag); }
    bool isDouble() const { return is(TypeFlag::DoubleFlag); }
    bool isInt64() const { return is(TypeFlag::IntegerFlag); }
    bool isUInt64() const { return is(TypeFlag::UIntegerFlag); }

    bool isInt32() const {
        if (!isInt64()) return false;

        const auto i64 = m_data.number.i64;
        using limitInt32 = std::numeric_limits<std::int32_t>;
        return i64 >= limitInt32::min() && i64 <= limitInt32::max();
    }

    bool isUInt32() const {
        if (!isUInt64()) return false;

        const auto u64 = m_data.number.u64;
        using limitUInt32 = std::numeric_limits<std::uint32_t>;
        return u64 <= limitUInt32::max();
    }

    // Number
    std::int32_t getInt32() const {
        PGJSON_DEBUG_ASSERT_EX(__func__, isInt32());

        return static_cast<int32_t>(m_data.number.i64);
    }

    std::uint32_t getUInt32() const {
        PGJSON_DEBUG_ASSERT_EX(__func__, isUInt32());

        return static_cast<uint32_t>(m_data.number.u64);
    }

    std::int64_t getInt64() const {
        PGJSON_DEBUG_ASSERT_EX(__func__, isInt64());

        return m_data.number.i64;
    }

    std::uint64_t getUInt64() const {
        PGJSON_DEBUG_ASSERT_EX(__func__, isUInt64());

        return m_data.number.u64;
    }

    double getDouble() const {
        PGJSON_DEBUG_ASSERT_EX(__func__, isNumber());

        if (isDouble()) return m_data.number.d;
        if (isInt64()) return static_cast<double>(m_data.number.i64);
        if (isUInt64()) return static_cast<double>(m_data.number.u64);

        PGJSON_DEBUG_ASSERT(0); return 0.f;
    }

    void setInt32(std::int32_t i32) {
        setInt64(i32);
    }
    void setUInt32(std::uint32_t u32) {
        setUInt64(u32);
    }
    void setInt64(std::int64_t i64) {
        reset(Int64Flags);

        m_data.number.i64 = i64;
    }
    void setUInt64(std::uint64_t u64) {
        reset(UInt64Flags);

        m_data.number.u64 = u64;
    }
    void setDouble(double d) {
        reset(DoubleFlags);

        m_data.number.d = d;
    }

    // String
    const Char * getString() const {
        PGJSON_DEBUG_ASSERT_EX(__func__, isString());

        if (m_data.str.usingSmall) return reinterpret_cast<const Char *>(m_data.str.sData);

        return m_data.str.data;
    }

    SizeType getStringLength() const {  // the number of Char, which equals to std::strlen(m_data.str.data) if Char == char
        PGJSON_DEBUG_ASSERT_EX(__func__, isString());

        if (m_data.str.usingSmall) return m_data.str.sLen;

        return m_data.str.length;
    }

    void setString(const Char * str, SizeType length = std::numeric_limits<SizeType>::max());

    template<SizeType LEN>
    void setString(const Char (&str)[LEN]) {
        PGJSON_STATIC_ASSERT(LEN > 0);

        setString(&str[0], LEN);
    }

#ifdef PGJSON_WITH_STL
    void setString(const std::basic_string<Char> & str) {
        setString(str.c_str(), str.size());
    }
#endif

    // Array
    void setArray() {
        reset(ArrayFlags);

        m_data.array.size = 0;
        m_data.array.capacity = 0;
        m_data.array.values = nullptr;
    }

    SizeType size() const {
        PGJSON_DEBUG_ASSERT_EX(__func__, isArray());

        return m_data.array.size;
    }

    SizeType capacity() const {
        PGJSON_DEBUG_ASSERT_EX(__func__, isArray());

        return m_data.array.capacity;
    }

    ArrayIterator begin() {
        PGJSON_DEBUG_ASSERT_EX(__func__, isArray());

        return ArrayIterator(m_data.array.values);
    }

    ArrayIterator end() {
        PGJSON_DEBUG_ASSERT_EX(__func__, isArray());

        return ArrayIterator(m_data.array.values + m_data.array.size);
    }

    ConstArrayIterator begin() const {
        return ConstArrayIterator(const_cast<Node &>(*this).begin());
    }

    ConstArrayIterator end() const {
        return ConstArrayIterator(const_cast<Node &>(*this).end());
    }

    bool empty() const {
        PGJSON_DEBUG_ASSERT_EX(__func__, isArray());

        return m_data.array.size == 0;
    }

    void reserve(SizeType newCapacity)  {
        PGJSON_DEBUG_ASSERT_EX("IsArray", isArray());
        PGJSON_DEBUG_ASSERT_EX("Reserve Invalid", m_data.array.size <= newCapacity);

        m_data.array.values = reinterpret_cast<Node **>(reinterpret_cast<void **>(
                PGJSON_REALLOC(m_data.array.values, sizeof(void *) * (newCapacity + 1))
        ));
        m_data.array.capacity = newCapacity;
    }

    void pushBack(Node * pNode) {
        PGJSON_DEBUG_ASSERT_EX(__func__, isArray());
        PGJSON_DEBUG_ASSERT_EX(__func__, pNode != nullptr);

        // extend
        if (m_data.array.size == m_data.array.capacity)
            reserve(m_data.array.capacity == 0 ? Array::DEFAULT_CAPACITY : m_data.array.capacity << 1U);

        // push-pNode-back
        m_data.array.values[m_data.array.size] = pNode;
        ++m_data.array.size;
    }

    template<typename ... Args>
    void emplaceBack(Args && ... args) {
        pushBack(Node::create(std::forward<Args>(args)...));
    }

    void popBack();

    void remove(const ArrayIterator & begin, const ArrayIterator & end, bool keepOrder = true);  // 待修改

    void remove(const ArrayIterator& iterator, bool keepOrder = true)  {
        remove(iterator, iterator + 1);
    }  // 待实现

    ArrayIterator erase(const ArrayIterator& iterator) {
        remove(iterator);
        return iterator;
    }

    ArrayIterator erase(const ArrayIterator & begin, const ArrayIterator & end) {
        remove(begin, end);
        return begin;
    }

    // Object
    void setObject() {
        reset(ObjectFlags);

        m_data.object.capacity = 0;
        m_data.object.size = 0;
        m_data.object.members = nullptr;
    }

    SizeType memberCount() const {
        PGJSON_DEBUG_ASSERT_EX(__func__, isObject());

        return m_data.object.size;
    }

    SizeType memberCapacity() {
        PGJSON_DEBUG_ASSERT_EX(__func__, isObject());

        return m_data.object.capacity;
    }

    Node & operator[] (const Char * name);

    const Node & operator[] (const Char * name) const {
        return const_cast<Node &>(*this)[name];
    }

#ifdef PGJSON_WITH_STL
    Node & operator[] (const std::basic_string<Char> & name) {
        return (*this)[name.c_str()];
    }

    const Node & operator[] (const std::basic_string<Char> & name) const {
        return const_cast<Node &>(*this)[name];
    }
#endif

    bool hasMember(const Char * name) const {
        return getMember(name) != memberEnd();
    }

#ifdef PGJSON_WITH_STL
    bool hasMember(const std::basic_string<Char> & name) const {
        return getMember(name) != memberEnd();
    }
#endif

    MemberIterator memberBegin() {
        PGJSON_DEBUG_ASSERT_EX(__func__, isObject());

        return MemberIterator(m_data.object.members);
    }

    ConstMemberIterator memberBegin() const {
        PGJSON_DEBUG_ASSERT_EX(__func__, isObject());

        return ConstMemberIterator(m_data.object.members);
    }

    MemberIterator memberEnd() {
        PGJSON_DEBUG_ASSERT_EX(__func__, isObject());

        return MemberIterator(m_data.object.members + m_data.object.size);
    }

    ConstMemberIterator memberEnd() const {
        PGJSON_DEBUG_ASSERT_EX(__func__, isObject());

        return ConstMemberIterator(m_data.object.members + m_data.object.size);
    }

    void memberReverse(SizeType newCapacity) {
        PGJSON_DEBUG_ASSERT_EX("IsObject", isObject());
        PGJSON_DEBUG_ASSERT_EX("Reserve Invalid", m_data.object.size <= newCapacity);

        m_data.object.members = reinterpret_cast<ObjectMember **>(reinterpret_cast<void **>(
                PGJSON_REALLOC(m_data.object.members, sizeof(void *) * (newCapacity + 1))
        ));
        m_data.object.capacity = newCapacity;
    }

    MemberIterator getMember(SizeType index) {
        PGJSON_DEBUG_ASSERT_EX(__func__, isObject());
        PGJSON_DEBUG_ASSERT_EX(__func__, index < m_data.object.size);

        return memberBegin() + index;
    }

    ConstMemberIterator getMember(SizeType index) const {
        return {const_cast<Node &>(*this).getMember(index)};
    }

    MemberIterator getMember(const Char * name);

    ConstMemberIterator getMember(const Char * name) const {
        return {const_cast<Node &>(*this).getMember(name)};
    }

//    template<SizeType LEN>
//    MemberIterator getMember(const Char (&name)[LEN]);
//
//    template<SizeType LEN>
//    ConstMemberIterator getMember(const Char (&name)[LEN]);

#ifdef PGJSON_WITH_STL
    MemberIterator getMember(const std::basic_string<Char> & name);

    ConstMemberIterator getMember(const std::basic_string<Char> & name) const {
        return {const_cast<Node &>(*this).getMember(name)};
    }
#endif

    void addMember(ObjectMember * pMember) {
        PGJSON_DEBUG_ASSERT_EX(__func__, isObject());
        PGJSON_DEBUG_ASSERT_EX(__func__, pMember != nullptr);

        // extend
        if (m_data.object.size == m_data.object.capacity)
            memberReverse(m_data.object.capacity == 0 ? Object::DEFAULT_CAPACITY : m_data.object.capacity << 1U);

        // push-pNode-back
        m_data.object.members[m_data.object.size] = pMember;
        ++m_data.object.size;
    }

    template<typename ... Args>
    MemberIterator addMember(const Char * name, Args && ... args) {
        // alloc
        Node * pValue = nullptr;
        ObjectMember * pMember = newUninitializedMember(name, &pValue);

        // construct
        new (pValue) Node(std::forward<Args>(args)...);

        // add
        addMember(pMember);

        return memberEnd() - 1;
    }

#ifdef PGJSON_WITH_STL
    template<typename ... Args>
    MemberIterator addMember(const std::basic_string<Char> & name, Args && ... args) {
        return addMember(name.c_str(), std::forward<Args>(args)...);
    }
#endif

    bool removeMember(const Char * name) {
        auto iter = getMember(name);
        if (iter == memberEnd()) return false;

        removeMember(iter);

        return true;
    }

#ifdef PGJSON_WITH_STL
    bool removeMember(const std::basic_string<Char> & name) {
        auto iter = getMember(name);
        if (iter == memberEnd()) return false;

        removeMember(iter);

        return true;
    }
#endif

    void removeMember(const MemberIterator & begin, const MemberIterator & end, bool keepOrder = true);  // 待修改

    void removeMember(const MemberIterator & itertor, bool keepOrder = true) {  // 待修改
        removeMember(itertor, itertor + 1);
    }

    MemberIterator eraseMember(const MemberIterator & begin, const MemberIterator & end) {
        removeMember(begin, end);
        return begin;
    }

    MemberIterator eraseMember(const MemberIterator & itertor) {
        removeMember(itertor);
        return itertor;
    }

    // shared-API
    Node & operator[] (SizeType index);

    const Node & operator[] (SizeType index) const {  // Object Array -> member value
        return const_cast<Node &>(*this)[index];
    }

    void clear();  // All types, clear but don't change type

    // create
    static Node * create();

    template<typename ... Args>
    static Node * create(Args && ... args);

    static void release(Node * pNode);

private:
    static ObjectMember * newUninitializedMember(const Char * name, Node ** ppValue = nullptr);

private:
    Node() = default;

    bool is(Enum typeFlags) const { return (m_typeFlags & typeFlags) != 0; }

    void reset(Enum typeFlags) {
        this->~Node();
        m_typeFlags = typeFlags;
    }

private:
    Variant m_data{};
    Enum m_typeFlags = InvalidFlags;
};


template<typename... Args>
inline pg::base::json::Node * pg::base::json::Node::create(Args &&... args) {
    return new (create()) Node(std::forward<Args>(args)...);
}

struct ObjectMember {
    ~ObjectMember() {
        name.destroy();
    }

    String name;
    Node value;
};

PGJSON_NAMESPACE_END
#endif //PGJSON_NODE_H
