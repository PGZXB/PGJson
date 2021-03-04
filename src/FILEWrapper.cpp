//
// Created by 42025 on 2021/3/3.
//
#include <PGJson/FILEWrapper.h>

#include <cstdio>

pg::base::json::FILEWrapper::FILEWrapper(const char * name, const char * mode) {
    m_pFILE = std::fopen(name, mode);
    PGJSON_DEBUG_ASSERT_EX("open file error", m_pFILE != nullptr);
}

void pg::base::json::FILEWrapper::close() {
    if (m_pFILE != nullptr) std::fclose(m_pFILE);
}

pg::base::json::SizeType pg::base::json::FILEWrapper::write(const void * src, SizeType bytes) {
    return std::fwrite(src, 1, bytes, m_pFILE);
}

pg::base::json::SizeType pg::base::json::FILEWrapper::read(void * dest, SizeType bytes) const {
    return std::fread(dest, 1, bytes, m_pFILE);
}

bool pg::base::json::FILEWrapper::seek(long offset, int whence) {
    if (m_pFILE == nullptr) return false;

    return std::fseek(m_pFILE, offset, whence) == 0;
}

bool pg::base::json::FILEWrapper::flush() {
    if (m_pFILE == nullptr) return false;

    return std::fflush(m_pFILE) == 0;
}

long pg::base::json::FILEWrapper::tell() {
    if (m_pFILE == nullptr) return -1;

    return std::ftell(m_pFILE);
}

