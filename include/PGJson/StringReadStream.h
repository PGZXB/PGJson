//
// Created by 42025 on 2021/3/6.
//
#ifndef PGTEST_STRINGREADSTREAM_H
#define PGTEST_STRINGREADSTREAM_H

#include <PGJson/fwd.h>
#include <PGJson/utils.h>
#include <PGJson/Allocator.h>
PGJSON_NAMESPACE_START

// only wrap char[], default no copy
class StringReadStream {
    static constexpr Enum InvalidFlag = 0U;
    static constexpr Enum STDStringFlag = 1U;
    static constexpr Enum CStrFlag = 1U << 2U;
    static constexpr Enum CopyFlag = 1U << 3U;

public:
    explicit StringReadStream(const Char * cStr, bool copy = false) : m_flags(CStrFlag) {
        PGJSON_DEBUG_ASSERT_EX(__func__, cStr != nullptr);

        init(cStr, getCStringLength(cStr), copy);
    }

    // template<SizeType LEN>
    // explicit StringReadStream(const Char (&cStr)[LEN], bool copy = false) : m_flags(CStrFlag) {
    //     PGJSON_DEBUG_ASSERT_EX(__func__, LEN > 1);
    //
    //     init(cStr, LEN - 1, copy);
    // }

#ifdef PGJSON_WITH_STL
    explicit StringReadStream(const std::string & str, bool copy = false) : m_flags(STDStringFlag) {
        init(str.data(), str.size(), copy);
    }
#endif

    // only move, cannot copy
    StringReadStream(const StringReadStream &) = delete;
    StringReadStream & operator= (StringReadStream &) = delete;

    StringReadStream(StringReadStream && other) noexcept
    : m_flags(other.m_flags), m_cStr(other.m_cStr), m_current(other.m_current), m_end(other.m_end) {
        other.m_flags = InvalidFlag;
        other.m_cStr = nullptr;
        other.m_current = nullptr;
        other.m_end = nullptr;
    }

    StringReadStream & operator= (StringReadStream && other) noexcept {
        if (this == &other) return *this;

        this->~StringReadStream();

        m_flags = other.m_flags;
        m_cStr = other.m_cStr;
        m_current = other.m_current;
        m_end = other.m_end;

        other.m_flags = InvalidFlag;
        other.m_cStr = nullptr;
        other.m_current = nullptr;
        other.m_end = nullptr;

        return *this;
    }

    ~StringReadStream() {
        if (m_flags & CopyFlag) PGJSON_FREE(const_cast<Char *>(m_cStr));
    }

    bool eof() const {
        return m_current == m_end;
    }

    Char peek() const {
        PGJSON_DEBUG_ASSERT_EX("EOF!!", !eof());

        return *m_current;
    }

    Char get() {
        PGJSON_DEBUG_ASSERT_EX("EOF!!", !eof());

        return *(m_current++);
    }

    SizeType tell() {
        return m_current - m_cStr;
    }
private:
    void init(const Char * cStr, SizeType len, bool copy) {
        if (!copy) {
            m_cStr = m_current = cStr;
            m_end = m_cStr + len;
        } else {
            m_flags |= CopyFlag;
            Char * ptr = reinterpret_cast<Char*>(PGJSON_MALLOC((len + 1) * PGJSON_CHAR_SIZE));
            std::memcpy(ptr, cStr, (len + 1) * PGJSON_CHAR_SIZE);
            m_cStr = m_current = ptr;
            m_end = m_cStr + len;
        }
    }
private:
    Enum m_flags = InvalidFlag;
    const Char * m_cStr = nullptr;
    const Char * m_current = nullptr;
    const Char * m_end = nullptr;
};

PGJSON_NAMESPACE_END
#endif //PGTEST_STRINGREADSTREAM_H
