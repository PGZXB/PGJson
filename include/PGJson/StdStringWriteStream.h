//
// Created by 42025 on 2021/3/8.
//
#ifndef PGTEST_STDSTRINGWRITESTREAM_H
#define PGTEST_STDSTRINGWRITESTREAM_H

#include <PGJson/fwd.h>
PGJSON_NAMESPACE_START

template<typename STRING>
class StdStringWriteStream {
public:
    StdStringWriteStream() = default;

    // copy and move is default

    STRING & str() { return m_string; }

    StdStringWriteStream & put(Char ch) {
        m_string.push_back(ch);

        return *this;
    }

    StdStringWriteStream & put(const void * src, SizeType len) {
        m_string.append(reinterpret_cast<const Char *>(src), len);

        return *this;
    }

    bool flush() {
        return true;
    }

private:
    STRING m_string;
};

PGJSON_NAMESPACE_END
#endif //PGTEST_STDSTRINGWRITESTREAM_H
