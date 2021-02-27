//
// Created by 42025 on 2021/2/25.
//
#ifndef PGJSON_FWD_H
#define PGJSON_FWD_H

#include <cstdint>

#define PGJSON_NAMESPACE_START namespace pg { namespace base { namespace json {
#define PGJSON_NAMESPACE_END } } }
#define PGJSON_PASS (void(0))
#define PGJSON_WITH_STL 1

#define PGJSON_STATIC_ASSERT_EX(msg, exp) static_assert(exp, msg)
#define PGJSON_STATIC_ASSERT(exp) PGJSON_STATIC_ASSERT_EX("", exp)

#ifdef PGJSON_DEBUG

    #include <cstdio>
    #include <cassert>

    #define PGJSON_ASSERT_IF(x) for ( ; !(x) ; assert(x) )

    #define PGJSON_DEBUG_ASSERT(exp) \
        PGJSON_ASSERT_IF(exp) PGJSON_PASS

    #define PGJSON_DEBUG_ASSERT_EX(msg, exp) \
        PGJSON_ASSERT_IF((exp)) { \
            std::fputs(msg, stderr); \
            std::fputs(" : \n", stderr); \
        } PGJSON_PASS
#else
    #define PGJSON_ASSERT_IF(x) PGJSON_PASS
    #define PGJSON_ASSERT(exp) PGJSON_PASS
    #define PGJSON_ASSERT_EX(msg, exp) PGJSON_PASS
#endif

PGJSON_NAMESPACE_START

using SizeType = uint64_t;
using Enum = uint32_t;
using Char = char;
using Byte = std::uint8_t;

namespace TypeFlag {

    constexpr Enum InvalidFlag = 0U;
    constexpr Enum FalseFlag = 1U << 0U;
    constexpr Enum TrueFlag = 1U << 1U;
    constexpr Enum BoolFlag = 1U << 2U;
    constexpr Enum StringFlag = 1U << 3U;
    constexpr Enum NumberFlag = 1U << 4U;
    constexpr Enum IntegerFlag = 1U << 5U;
    constexpr Enum UIntegerFlag = 1U << 6U;
    constexpr Enum DoubleFlag = 1U << 7U;
    constexpr Enum RawFlag = 1U << 8U;
    constexpr Enum ObjectFlag = 1U << 9U;
    constexpr Enum ArrayFlag = 1U << 10U;
    constexpr Enum NullFlag = 1U << 11U;

}

PGJSON_NAMESPACE_END
#endif //PGJSON_FWD_H