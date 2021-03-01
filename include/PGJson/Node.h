//
// Created by 42025 on 2021/2/25.
//
#ifndef PGJSON_NODE_H
#define PGJSON_NODE_H

#include <limits>
#include <cstring>
#include <PGJson/fwd.h>
#include <PGJson/utils.h>
#include <PGJson/Allocator.h>
#include <PGJson/Iterator.h>


#ifdef PGJSON_WITH_STL
#include <iostream>
    #include <string>
#endif

PGJSON_NAMESPACE_START

namespace {
    constexpr Enum InvalidFlags = TypeFlag::InvalidFlag;
    constexpr Enum FalseFlags = TypeFlag::FalseFlag | TypeFlag::BoolFlag;
    constexpr Enum TrueFlags = TypeFlag::TrueFlag | TypeFlag::BoolFlag;
    constexpr Enum BoolFlags = TypeFlag::BoolFlag;
    constexpr Enum StringFlags = TypeFlag::StringFlag;
    constexpr Enum NumberFlags = TypeFlag::NumberFlag;
    constexpr Enum Int64Flags = TypeFlag::IntegerFlag | TypeFlag::NumberFlag;
    constexpr Enum UInt64Flags = TypeFlag::UIntegerFlag | TypeFlag::NumberFlag;
    constexpr Enum DoubleFlags = TypeFlag::DoubleFlag | TypeFlag::NumberFlag;
    constexpr Enum RawFlags = TypeFlag::RawFlag;
    constexpr Enum ObjectFlags = TypeFlag::ObjectFlag;
    constexpr Enum ArrayFlags = TypeFlag::ArrayFlag;
    constexpr Enum NullFlags = TypeFlag::NullFlag;
}

class Node;
class ObjectMember;

template<SizeType BLOCK_SIZE, typename Type, typename Allocator>
class MemoryBlockPool;

//class ArrayIterator : public PoPSeqIterator<Node> {
//    friend class Node;
//public:
//    using PoPSeqIterator::PoPSeqIterator;
//
//private:
//    explicit ArrayIterator(Node ** pPtr) : PoPSeqIterator(pPtr) {
//
//    }
//};

//using ArrayIterator = PoPSeqIterator<Node>;

//class ConstArrayIterator : public PoPSeqIterator<const Node> {
//    friend class Node;
//public:
//    using PoPSeqIterator::PoPSeqIterator;
//
//private:
//    explicit ConstArrayIterator(Node ** pPtr) : PoPSeqIterator(pPtr) {
//
//    }
//};
//
//class ObjectMemberIterator : public PoPSeqIterator<ObjectMember> {
//    friend class Node;
//public:
//    using PoPSeqIterator::PoPSeqIterator;
//
//private:
//    explicit ObjectMemberIterator(ObjectMember ** pPtr) : PoPSeqIterator(pPtr) {
//
//    }
//};
//
//class ConstObjectMemberIterator : public PoPSeqIterator<const ObjectMember> {
//    friend class Node;
//public:
//    using PoPSeqIterator::PoPSeqIterator;
//
//private:
//    explicit ConstObjectMemberIterator(ObjectMember ** pPtr) : PoPSeqIterator(pPtr) {
//
//    }
//};

class Node {
    friend class MemoryBlockPool<32, Node, DefaultMemoryAllocator>;

    union Variant;

    union Number {
        std::int64_t i64;
        std::uint64_t u64;
        double d;
    };

    struct String {
        static constexpr uint8_t SMALL_STRING_MAX_SIZE = 2;
        // uint64_t hashcode;
        union {
            struct {  // normal-string
                SizeType length;
                const Char * data;
            };
            struct {  // small string, max-byte-size : 22
                Byte sData[23] = { 0 };
                std::uint8_t sLen = 0;
            };
        };
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
    using MemberItertor = PoPSeqIterator<ObjectMember>;
    using ConstMemberIterator = const PoPSeqIterator<const ObjectMember>;

public:
    ~Node() = default;

    Enum getType() const {
        ArrayIterator::value_type i;
        return m_typeFlags; }

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

        if (_m_smallStringUsed) return reinterpret_cast<const Char *>(m_data.str.sData);

        return m_data.str.data;
    }

    SizeType getStringLength() const {  // the number of Char, which equals to std::strlen(m_data.str.data) if Char == char
        PGJSON_DEBUG_ASSERT_EX(__func__, isString());

        if (_m_smallStringUsed) return m_data.str.sLen;

        return m_data.str.length;
    }

    void setString(const Char * str, SizeType length = std::numeric_limits<SizeType>::max());

#ifdef PGJSON_WITH_STL
    void setString(const std::basic_string<Char> & str) {
        setString(str.c_str(), str.size());
    }
#endif

    // Array
    void setArray() {
        reset(ArrayFlags);
    }

    Node & operator[] (SizeType index) {
        PGJSON_DEBUG_ASSERT_EX(__func__, isArray());
        PGJSON_DEBUG_ASSERT_EX("index < size", index < m_data.array.size);

        return *m_data.array.values[index];
    }

    const Node & operator[] (SizeType index) const {
        return const_cast<Node &>(*this)[index];
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

        // extend
        if (m_data.array.size == m_data.array.capacity)
            reserve(m_data.array.capacity == 0 ? Array::DEFAULT_CAPACITY : m_data.array.capacity << 1U);

        // push-pNode-back
        m_data.array.values[m_data.array.size] = pNode;
        ++m_data.array.size;
    }

    void popBack();

    void clear();

    void remove(const ArrayIterator & begin, const ArrayIterator & end);

    void remove(const ArrayIterator& iterator)  {
        remove(iterator, iterator + 1);
    }

    ArrayIterator erase(const ArrayIterator& iterator) {
        remove(iterator);
        return iterator;
    }

    ArrayIterator erase(const ArrayIterator & begin, const ArrayIterator & end) {
        remove(begin, end);
        return begin;
    }
    // Object

    // create
    static Node * create();

    template<typename ... Args>
    static Node * create(Args && ... args);
private:
    Node() = default;

    bool is(Enum typeFlags) const { return (m_typeFlags & typeFlags) != 0; }

    void reset(Enum typeFlags) {
        this->~Node();
        m_typeFlags = typeFlags;
    }

private:
    Variant m_data{};
    Enum m_typeFlags = TypeFlag::InvalidFlag;
    bool _m_smallStringUsed = true;  // declared here for saving memory
};

template<typename... Args>
inline pg::base::json::Node * pg::base::json::Node::create(Args &&... args) {
    return new (create()) Node(std::forward<Args>(args)...);
}

PGJSON_NAMESPACE_END
#endif //PGJSON_NODE_H
