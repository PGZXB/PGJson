#ifndef __FILEWRAPPER_H__
#define __FILEWRAPPER_H__

// copy from PGLib : https://gitee.com/pgzxb/pglib/tree/master & redesign for PGJson

#include <PGJson/fwd.h>

PGJSON_NAMESPACE_START

class FILEWrapper {
public:
    FILEWrapper(const char * name, const char * mode);
    ~FILEWrapper() { close(); }

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
#endif