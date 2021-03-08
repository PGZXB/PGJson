#ifndef __FILEWRAPPER_H__
#define __FILEWRAPPER_H__

// copy from PGLib : https://gitee.com/pgzxb/pglib/tree/master & redesign for PGJson

#include <cstdio>

#include <PGJson/fwd.h>

PGJSON_NAMESPACE_START

class FILEWrapper {
public:
    FILEWrapper(const char * name, const char * mode);
    ~FILEWrapper() {
        if (m_pFILE) {
            std::fflush(m_pFILE);
            close();
        }
    }

    FILEWrapper(const FILEWrapper &) = delete;
    FILEWrapper(FILEWrapper && other) noexcept : m_pFILE(other.m_pFILE) {
        other.m_pFILE = nullptr;
    }

    FILEWrapper & operator = (const FILEWrapper &) = delete;
    FILEWrapper & operator = (FILEWrapper && other) noexcept {
        if (this == &other) return *this;

        m_pFILE = other.m_pFILE;
        other.m_pFILE = nullptr;

        return *this;
    }

    void close();

    SizeType write(const void * src, SizeType bytes);

    SizeType read(void * dest, SizeType bytes) const;

    bool seek(long int offset, int whence);

    bool flush();

    long tell();
private:
    std::FILE * m_pFILE = nullptr;
};

PGJSON_NAMESPACE_END

inline pg::base::json::FILEWrapper::FILEWrapper(const char * name, const char * mode) {
    m_pFILE = std::fopen(name, mode);
    PGJSON_DEBUG_ASSERT_EX("open file error", m_pFILE != nullptr);
}

inline void pg::base::json::FILEWrapper::close() {
    if (m_pFILE != nullptr) std::fclose(m_pFILE);
    m_pFILE = nullptr;
}

inline pg::base::json::SizeType pg::base::json::FILEWrapper::write(const void * src, SizeType bytes) {
    return std::fwrite(src, 1, bytes, m_pFILE);
}

inline pg::base::json::SizeType pg::base::json::FILEWrapper::read(void * dest, SizeType bytes) const {
    return std::fread(dest, 1, bytes, m_pFILE);
}

inline bool pg::base::json::FILEWrapper::seek(long offset, int whence) {
    if (m_pFILE == nullptr) return false;

    return std::fseek(m_pFILE, offset, whence) == 0;
}

inline bool pg::base::json::FILEWrapper::flush() {
    if (m_pFILE == nullptr) return false;

    return std::fflush(m_pFILE) == 0;
}

inline long pg::base::json::FILEWrapper::tell() {
    if (m_pFILE == nullptr) return -1;

    return std::ftell(m_pFILE);
}
#endif