//
// Created by 42025 on 2021/2/25.
//
#ifndef PGJSON_NODE_H
#define PGJSON_NODE_H

#include <limits>
#include <cstring>
#include <PGJson/fwd.h>

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

struct ObjectMember;

class Node {
    union Variant;

    union Number {
        std::int64_t i64;
        std::uint64_t u64;
        double d;
    };

    struct String {
        static constexpr uint8_t SMALL_STRING_MAX_SIZE = 22;
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
        SizeType size = 0;
        SizeType capacity = 0;
        ObjectMember * members = nullptr;
    };

    struct Array {
        SizeType size = 0;
        SizeType capacity = 0;
        Node * values = nullptr;
    };

    union Variant {
        Number number;
        String str;
        Object object;
        Array array;
    };

public:
    Node() = default;
    ~Node() {
        std::cout << __FUNCTION__ << "\n";
    }

    Enum getType() const { return m_typeFlags; }
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

    void setString(const Char * str, SizeType length = std::numeric_limits<SizeType>::max()) {
        // if length == SizeType's max, the last Char of str must be 0, truncate the str by Char-0
        // else we think len(str) equals to the param length, don't truncate the str by Char-0

        // reset
        reset(StringFlags);

        if (length == std::numeric_limits<SizeType>::max())
            // length = pgjson_strlen(str);
            PGJSON_PASS;

        // small-string, if the true-length <= String::SMALL_STRING_MAX_SIZE
        if (length <= String::SMALL_STRING_MAX_SIZE) {
            _m_smallStringUsed = true;
            std::memcpy(m_data.str.sData, str, length);
            m_data.str.sLen = static_cast<std::uint8_t>(length);
            return;
        }

        // if the true-length is more than String::SMALL_STRING_MAX_SIZE
        // allocate memory
        // void * ptr = PGJsonStringAllocator::allocate(length);
        // copy string from str to ptr
        // std::memcpy(ptr, str, length);
        // _m_smallStringUsed = false;
        // m_data.str.data = static_cast<Char*>(ptr);
        // m_data.str.length = length;
    }

#ifdef PGJSON_WITH_STL
    void setString(const std::basic_string<Char> & str) {
        setString(str.c_str(), str.size());
    }
#endif

    // Array
    // Object
private:
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

// struct ObjectMember {
//     const Char * name;
//     Node value;
// };

PGJSON_NAMESPACE_END
#endif //PGJSON_NODE_H
