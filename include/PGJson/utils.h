//
// Created by 42025 on 2021/2/27.
//
#ifndef PGTEST_UTILS_H
#define PGTEST_UTILS_H

#include <PGJson/fwd.h>
PGJSON_NAMESPACE_START

template<typename T1, typename T2>
inline constexpr auto max(const T1 & n1, const T2 & n2) -> decltype(n1 > n2 ? n1 : n2) {
    return n1 > n2 ? n1 : n2;
}

PGJSON_NAMESPACE_END
#endif //PGTEST_UTILS_H
