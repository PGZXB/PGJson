//
// Created by 42025 on 2021/2/25.
//
#ifndef PGJSON_FWD_H
#define PGJSON_FWD_H

#include <cstdint>
//#include <iostream>  // ONLY FOR TEST
#define PGJSON_NAMESPACE_START namespace pg { namespace base { namespace json {
#define PGJSON_NAMESPACE_END } } }
#define PGJSON_PASS (void(0))
#define PGJSON_MIN_CPP_VERSION 201103L

#define PGJSON_MALLOC(size) (DefaultMemoryAllocator::getGlobalInstance()->allocate((size)))
#define PGJSON_REALLOC(ptr, newSize) (DefaultMemoryAllocator::getGlobalInstance()->reallocate((ptr), (newSize)))
#define PGJSON_FREE(ptr) (DefaultMemoryAllocator::getGlobalInstance()->deallocate((ptr)))

// PGJson-Use
#define PGJSON_WITH_STL 1
#define PGJSON_WITH_CXX_EXCEPTION 1

// Parse-Info
#define PGJSON_PARSE_ERROR_EXIT 1
#define PGJSON_PARSE_ERROR_RETURN 1
#define PGJSON_PARSE_ERROR_LOG 1

// json-standard
#define PGJSON_ALLOW_AFTER_LAST_ITEM_HAS_COMMA 1
#define PGJSON_ALLOW_ONE_LINE_COMMENTS 1
// #define PGJSON_ALLOW_MULTI_LINE_COMMENTS 1

#define PGJSON_STATIC_ASSERT_EX(msg, exp) static_assert(exp, msg)
#define PGJSON_STATIC_ASSERT(exp) PGJSON_STATIC_ASSERT_EX("", exp)

#if defined(PGJSON_PARSE_ERROR_EXIT) || defined(PGJSON_PARSE_ERROR_RETURN) || defined(PGJSON_PARSE_ERROR_LOG)
    #include <cstdio>
#endif

#ifdef PGJSON_PARSE_ERROR_EXIT  // 待实现, assert失败回收资源
    #define PGJSON_PARSE_ASSERT_EX(msg, exp) \
        if (!(exp)) { \
            std::fputs((msg), stderr); \
            std::fputs("\n", stderr); \
            assert((exp)); \
        } PGJSON_PASS
#elif defined(PGJSON_PARSE_ERROR_RETURN)  // 待实现, 设置errno, errmsg
    #define PGJSON_PARSE_ASSERT_EX(msg, exp) \
        if (!(exp)) { \
            std::fputs((msg), stderr); \
            std::fputs("\n", stderr); \
            return; \
        } PGJSON_PASS
#elif defined(PGJSON_PARSE_ERROR_LOG)
    #define PGJSON_PARSE_ASSERT_EX(msg, exp) \
        if (!(exp)) { \
            std::fputs((msg), stderr); \
            std::fputs("\n", stderr); \
        } PGJSON_PASS
#else
    #define PGJSON_PARSE_ASSERT_EX(msg, exp) PGJSON_PASS
#endif


#ifdef PGJSON_DEBUG

    #include <cstdio>
    #include <cassert>
    #include <string>

    #define PGJSON_ASSERT_IF(x) for ( ; !(x) ; )

    #define PGJSON_DEBUG_ASSERT(exp) \
        PGJSON_ASSERT_IF(exp) { assert(exp); break; } \
        PGJSON_PASS

    #define PGJSON_DEBUG_ASSERT_EX(msg, exp) \
        PGJSON_ASSERT_IF((exp)) { \
            std::fputs(msg, stderr); \
            std::fputs(" : \n", stderr); \
            assert(exp); \
            break; \
        } PGJSON_PASS
#else
    #define PGJSON_DEBUG_ASSERT_IF(x) PGJSON_PASS
    #define PGJSON_DEBUG_ASSERT(exp) PGJSON_PASS
    #define PGJSON_DEBUG_ASSERT_EX(msg, exp) PGJSON_PASS
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

constexpr SizeType DEFAULT_BUFFER_SIZE = 512;
constexpr SizeType PGJSON_CHAR_SIZE = sizeof(Char);

// class Stream {
// public:
//     Char peek();
//     Char get();
//     SizeType tell();
//
//     void put();
//     void flush();
// };

PGJSON_NAMESPACE_END
#endif //PGJSON_FWD_H
