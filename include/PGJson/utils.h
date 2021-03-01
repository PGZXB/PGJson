//
// Created by 42025 on 2021/2/27.
//
#ifndef PGTEST_UTILS_H
#define PGTEST_UTILS_H

#include <cstring>
#include <PGJson/fwd.h>
PGJSON_NAMESPACE_START

template<typename T1, typename T2>
inline constexpr auto max(const T1 & n1, const T2 & n2) -> decltype(n1 > n2 ? n1 : n2) {
    return n1 > n2 ? n1 : n2;
}

template<typename CharType>
inline SizeType getCStringLength(const CharType * str) {
    if (!str) return 0;

    SizeType res = 0;
    for (; *str; ++str, ++res);

    return res;
}

template<>
inline SizeType getCStringLength<char>(const char * str) {
    return std::strlen(str);
}

SizeType getUTF8StringLength(const char * str);

PGJSON_NAMESPACE_END
#endif //PGTEST_UTILS_H
