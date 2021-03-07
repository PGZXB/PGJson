//
// Created by 42025 on 2021/3/3.
//
#ifndef PGTEST_FILESTREAM_H
#define PGTEST_FILESTREAM_H

#include <PGJson/fwd.h>
#include <PGJson/FILEWrapper.h>
PGJSON_NAMESPACE_START

constexpr struct WriteModeType { } WriteMode;
constexpr struct ReadModeType { } ReadMode;

template<SizeType BUF_SIZE = DEFAULT_BUFFER_SIZE>
class FileStream {
    static constexpr Enum InvaliFlag = 0U;
    static constexpr Enum WriteFlag = 1U << 1U;
    static constexpr Enum ReadFlag = 1U << 2U;
public:
    FileStream(const char * filename, WriteModeType mode)
    : m_flags(WriteFlag),
      m_FILEWrapper(filename, "wb"),
      m_current(m_buffer) {
    }

    FileStream(const char * filename, ReadModeType mode)
    : m_flags(ReadFlag),
      m_FILEWrapper(filename, "rb") {
        read();
    }

    ~FileStream() {
        if (m_flags == WriteFlag) flush();
    }

    bool eof() {
        return m_current == m_bufferEnd && m_current != m_buffer + BUF_SIZE;
    }

    Char peek() {
        PGJSON_DEBUG_ASSERT_EX("Read Only", m_flags == ReadFlag);
        PGJSON_DEBUG_ASSERT_EX("EOF!!", !eof());

        return *m_current;
    }

    Char get() {
        PGJSON_DEBUG_ASSERT_EX("Read Only", m_flags == ReadFlag);
        PGJSON_DEBUG_ASSERT_EX("EOF!!", !eof());

        Char c = *m_current;
        read();
        return c;
    }

    long tell() {
        return m_FILEWrapper.tell();
    }

    FileStream & put(Char ch) {
        PGJSON_DEBUG_ASSERT_EX("Write Only", m_flags == WriteFlag);

        *m_current = ch;
        if (++m_current == m_bufferEnd) write();

        return *this;
    }

    FileStream & put(const void * src, SizeType bytes) {
        PGJSON_DEBUG_ASSERT_EX("Write Only", m_flags == WriteFlag);

        if (bytes < (m_bufferEnd - m_current) * PGJSON_CHAR_SIZE) {
            std::memcpy(m_current, src, bytes);
            m_current += bytes / PGJSON_CHAR_SIZE;
        } else {
            write();
            m_FILEWrapper.write(src, bytes);
        }

        return *this;
    }

    bool flush() {
        PGJSON_DEBUG_ASSERT_EX("Write Only", m_flags == WriteFlag);

        write();
        return m_FILEWrapper.flush();
    }
private:
    void read() {
        // m_current == m_bufferEnd == m_buffer : Read Over
        if (m_current < m_bufferEnd) ++m_current;
        else if (!eof()) {  // not eof
            // read
            SizeType bytes = m_FILEWrapper.read(m_buffer, BUF_SIZE * PGJSON_CHAR_SIZE);
            m_current = m_buffer;
            if (bytes != BUF_SIZE * PGJSON_CHAR_SIZE) {  // will eof
                m_bufferEnd = m_buffer + bytes / PGJSON_CHAR_SIZE;
            }
        }
    }

    void write() {
        SizeType bytes = (m_current - m_buffer) * PGJSON_CHAR_SIZE;
        SizeType tBytes = m_FILEWrapper.write(m_buffer, bytes);
        PGJSON_DEBUG_ASSERT_EX("Write Error", tBytes == bytes);
        m_current = m_buffer;
    }
private:
    FILEWrapper m_FILEWrapper;
    std::uint8_t m_flags = InvaliFlag;
    Char m_buffer[BUF_SIZE + 1] = { 0 };
    Char * m_current = m_buffer + BUF_SIZE;
    Char * m_bufferEnd = m_buffer + BUF_SIZE;
};

PGJSON_NAMESPACE_END
#endif //PGTEST_FILESTREAM_H
