//
// Created by 42025 on 2021/3/2.
//
#ifndef PGTEST_STRING_H
#define PGTEST_STRING_H

#include <limits>
#include <cstring>

#include <PGJson/fwd.h>

#ifdef PGJSON_WITH_STL
    #include <string>
#endif

PGJSON_NAMESPACE_START

class Node;

// 待完善
struct String {
    static constexpr uint8_t SMALL_STRING_MAX_SIZE = 21;

    friend class Node;

    String() : sData{0}, sLen(0), usingSmall(true) {

    }

    // uint64_t hashcode;
    union {
        struct {  // normal-string  // 16 + 8 = 24 bytes
            SizeType length;
            Char * data;
            std::uint8_t DONT_USE[8];
        };
        struct {  // small string, max-byte-size : 21  // 24 bytes
            Byte sData[22];
            std::uint8_t sLen;
            std::uint8_t usingSmall;
        };
    };

    const Char * getCString() const {
        if (usingSmall) return reinterpret_cast<const Char *>(sData);
        return data;
    }

    SizeType getLength() const {
        return usingSmall ? sLen : length;
    }

    void copyFrom(const Char * str, SizeType len = std::numeric_limits<SizeType>::max());
    void destroy();
};

bool stringEquals(const Char * str, const String & str2);

inline bool stringEquals(const String & str, const Char * str2) {
    return stringEquals(str2, str);
}

#ifdef PGJSON_WITH_STL
inline bool stringEquals(const std::basic_string<Char> & str, const String & str2) {
    if (str2.usingSmall) {
        return str2.sLen == str.size() && (str == reinterpret_cast<const Char *>(str2.sData));
    }

    return str2.length == str.size() && (str == str2.data);
}

inline bool stringEquals(const String & str, const std::basic_string<Char> & str2) {
    return stringEquals(str2, str);
}
#endif

PGJSON_NAMESPACE_END
#endif //PGTEST_STRING_H
