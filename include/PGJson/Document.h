//
// Created by 42025 on 2021/3/8.
//
#ifndef PGTEST_DOCUMENT_H
#define PGTEST_DOCUMENT_H

#include <PGJson/fwd.h>
#include <PGJson/Node.h>
#include <PGJson/Parser.h>
#include <PGJson/FileStream.h>
#include <PGJson/StringReadStream.h>
#include <PGJson/StdStringWriteStream.h>

#include <string>
PGJSON_NAMESPACE_START

class Node;

class Document {
public:
    Document() = default;

    ~Document() {
        if (m_Node) Node::release(m_Node);
    }

    Node & parseFromFile(const Char * filename) {
        FileStream<> stream(filename, ReadMode);

        return parse(stream);
    }

    Node & parseFromString(const Char * json) {
        StringReadStream stream(json);

        return parse(stream);
    }

    template<typename STREAM>
    Node & parse(STREAM & stream) {
        if (m_Node == nullptr) m_Node = Node::create();

        return *pg::base::json::parse(stream, m_Node);
    }

    Node & d() { // DOM
        PGJSON_DEBUG_ASSERT_EX(__func__, m_Node != nullptr);

        return *m_Node;
    }

    template<typename STREAM>
    void save(STREAM & stream, bool fmt = true) {
        PGJSON_DEBUG_ASSERT_EX(__func__, m_Node != nullptr);

        if (fmt) {
            pg::base::json::toString(m_Node, stream, "\t");
        } else {
            pg::base::json::toString(m_Node, stream);
        }
        stream.flush();
    }

    void save(const Char * filename, bool fmt = true) {
        FileStream<> stream(filename, WriteMode);

        save(stream, fmt);
    }

#ifdef PGJSON_WITH_STL
    std::basic_string<Char> stringify(bool fmt = true) {
        StdStringWriteStream<std::basic_string<Char>> stream;

        save(stream, fmt);

        return std::move(stream.str());
    }
#endif

private:
    Node * m_Node = nullptr;
};

PGJSON_NAMESPACE_END
#endif //PGTEST_DOCUMENT_H
