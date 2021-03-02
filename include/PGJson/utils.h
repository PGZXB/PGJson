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

template<typename CHAR>
inline SizeType getCStringLength(const CHAR * str) {
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

template<typename CHAR>
inline bool stringEquals(const CHAR * str, const CHAR * str2) {
    return std::strcmp(reinterpret_cast<char *>(str), reinterpret_cast<char *>(str2)) == 0;
}

template<>
inline bool stringEquals<char>(const char * str, const char * str2) {
    return std::strcmp(str, str2) == 0;
}

#if __cplusplus >= 201703L
#include <utility>

template<typename CHAR, SizeType LEN, SizeType ... I>
inline bool _stringEquals(CHAR (&str)[LEN], CHAR (&str2)[LEN], std::index_sequence<I...>) {
    return (... && (str[I] == str2[I]));
}

template<typename CHAR, SizeType LEN1, SizeType LEN2>
inline bool stringEquals(CHAR (&str)[LEN1], CHAR (&str2)[LEN2]) {
    if (LEN1 != LEN2) return false;

    return _stringEquals(std::forward<CHAR[LEN1]>(str), std::forward<CHAR[LEN1]>(str2), std::make_index_sequence<LEN1>());
}
#else
template<typename CHAR, SizeType LEN>
inline constexpr bool _stringEquals(const CHAR (&str)[LEN], const CHAR (&str2)[LEN], SizeType i = LEN - 1) {
    return i == 0 ? str[i] == str2[i] : (str[i] == str2[i] && _stringEquals(str, str2, --i));
}

template<typename CHAR, SizeType LEN1, SizeType LEN2>
inline constexpr bool stringEquals(const CHAR (&str)[LEN1], const CHAR (&str2)[LEN2]) {
    return (LEN1 == LEN2 && ( LEN1 == 0 || _stringEquals(str, str2)));
}
#endif

template<typename CHAR, SizeType LEN>
inline constexpr bool _stringEquals(const CHAR (&str)[LEN], const CHAR * str2, SizeType i = LEN - 1) {
    return i == 0 ? str[i] == str2[i] : (str[i] == str2[i] && _stringEquals(str, str2, --i));
}

template<typename CHAR, SizeType LEN1>
inline constexpr bool stringEquals(const CHAR (&str)[LEN1], const CHAR * str2, SizeType len2) {
    return (LEN1 == len2 && ( LEN1 == 0 || _stringEquals(str, str2)));
}

PGJSON_NAMESPACE_END
#endif //PGTEST_UTILS_H
