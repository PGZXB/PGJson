//
// Created by 42025 on 2021/3/5.
//
#ifndef PGTEST_PARSER_H
#define PGTEST_PARSER_H

#include <PGJson/fwd.h>
#include <vector>
#include <cctype>
#include <cstring>
PGJSON_NAMESPACE_START

// statement
template<typename STREAM, typename NODE>
void parseArray(STREAM & stream, NODE * node);

template<typename STREAM, typename NODE>
void parseObject(STREAM & stream, NODE * node);

template<typename STREAM, typename NODE>
void parseValue(STREAM & stream, NODE * node);

// define
template<typename STREAM>
void skip1Line(STREAM & stream) {
    Char ch = stream.peek();
    while (ch != '\n') {  // \r\n or \n
        stream.get();
        ch = stream.peek();
    }
}

template<typename STREAM>
void skipWhiteChar(STREAM & stream) {
    // ' ' '\n' '\r' '\t'
    Char ch = stream.peek();
    while (ch == ' ' || ch == '\r' || ch == '\n' || ch == '\t') {
        stream.get();
        ch = stream.peek();
    }
}

template<typename STREAM>
void skipComments(STREAM & stream) {
#ifdef PGJSON_ALLOW_ONE_LINE_COMMENTS
    PGJSON_PARSE_ASSERT_EX("Parse Error, Skipping Comments", stream.peek() == '/');
    stream.get();
    Char ch = stream.get();
    PGJSON_PARSE_ASSERT_EX("Parse Error, Skipping Comments", ch == '/');
    skip1Line(stream);
    skipWhiteChar(stream);
    if (stream.peek() == '/') skipComments(stream);
#endif
// #ifdef PGJSON_ALLOW_MULTI_LINE_COMMENTS
//
// #endif
}

template<typename STREAM>
void skip(STREAM & stream) {
    skipWhiteChar(stream);
    if (stream.peek() == '/') skipComments(stream);
}

template<typename STREAM>
void parseString(STREAM & stream, std::vector<Char> & buffer) {
    // \0 \n \r \t \" \' \\ \u0000
    PGJSON_PARSE_ASSERT_EX("Parse String Error", stream.peek() == '\"');
    stream.get();

    while (stream.peek() != '\"') {
        Char ch = stream.get();
        if (ch == '\\') {
            switch (ch = stream.get()) {
                case '0' :
                    buffer.push_back('\0');
                    break;
                case 'n' :
                    buffer.push_back('\n');
                    break;
                case 'r' :
                    buffer.push_back('\r');
                    break;
                case 't' :
                    buffer.push_back('\t');
                    break;
                case '\"':
                    buffer.push_back('\"');
                    break;
                case '\'':
                    buffer.push_back('\'');
                    break;
                case '\\':
                    buffer.push_back('\\');
                    break;
                case 'u': {
                    Char u_[4] = { stream.get(), stream.get(), stream.get(), stream.get() };
                    std::uint16_t u = std::strtoull(u_, nullptr, 16);
                    PGJSON_PARSE_ASSERT_EX("Parse Error",
                                           (u_[0] == 0 && u_[1] == 0 && u_[2] == 0 && u_[3] == 0) || u != 0);
#if PGJSON_CHAR_SIZE >= 2
                    buffer.push_back(u);
#else
                    Char * ptr = reinterpret_cast<Char *>(&u);
                    buffer.push_back(ptr[1]);
                    buffer.push_back(ptr[0]);
#endif
                    break;
                }
                default:
                PGJSON_PARSE_ASSERT_EX("Parse Error", false);
            }
        } else {
            buffer.push_back(ch);
        }
    }

    // PGJSON_DEBUG_ASSERT_EX("Parse Error", stream.peek() == '\"');
    stream.get();

    buffer.push_back(0);
}

template<typename STREAM, typename NODE>
void parseNull(STREAM & stream, NODE * node) {
    if (stream.get() == 'n' &&
        stream.get() == 'u' &&
        stream.get() == 'l' &&
        stream.get() == 'l'
            ) {
        node->setNull();
        return;
    }
    PGJSON_PARSE_ASSERT_EX("Parse Null Error", false);
}

template<typename STREAM, typename NODE>
void parseFalse(STREAM & stream, NODE * node) {
    if (stream.get() == 'f' &&
        stream.get() == 'a' &&
        stream.get() == 'l' &&
        stream.get() == 's' &&
        stream.get() == 'e'
        ) {
        node->setFalse();
        return;
    }
    PGJSON_PARSE_ASSERT_EX("Parse False Error", false);
}

template<typename STREAM, typename NODE>
void parseTrue(STREAM & stream, NODE * node) {
    if (stream.get() == 't' &&
        stream.get() == 'r' &&
        stream.get() == 'u' &&
        stream.get() == 'e'
            ) {
        node->setTrue();
        return;
    }
    PGJSON_PARSE_ASSERT_EX("Parse True Error", false);
}

template<typename STREAM, typename NODE>
void parseString(STREAM & stream, NODE * node) {
    std::vector<Char> res;

    parseString(stream, res);

    node->setString(res.data(), res.size() - 1);
}

template<typename STREAM, typename NODE>
void parseNumber(STREAM & stream, NODE * node) {
    // Integer +666 -123 999 +4e9 -9e2 2e2 2e-2 2E2
    // Double +1.3 -4.5 3.14 3.4e3 -3.4e3 +3.4e3 3.33e-2
    Char ch = stream.peek();
    PGJSON_PARSE_ASSERT_EX("Parse Number Error", ch == '+' || ch == '-' || isdigit(stream.peek()));

    std::vector<Char> tmp;
    bool positive = true;

    if (!std::isdigit(ch)) {
        stream.get();
        if (ch == '-') positive = false;
    }

    ch = stream.peek();
    while (std::isdigit(ch) || std::strchr("-+.eE", ch)) {
        stream.get();
        tmp.push_back(ch);
        ch = stream.peek();
    }
    tmp.push_back(0);

    // 待实现范围判断 --
    long double d = std::strtold(tmp.data(), nullptr);
    uint64_t i = std::strtoull(tmp.data(), nullptr, 10);

    if (d == static_cast<long double>(i)) {
        if (positive) node->setUInt64(i);
        else node->setInt64(-1 * i);
    } else {
        if (positive) node->setDouble(d);
        else node->setDouble(-1 * d);
    }
}

template<typename STREAM, typename NODE>
void parseArray(STREAM & stream, NODE * node) {
    PGJSON_PARSE_ASSERT_EX("Parse Array Error", stream.peek() == '[');
    node->setArray();

    stream.get();
    skip(stream);
    if (stream.peek() == ']') {
        stream.get();
        return;
    }

    while (true) {
        auto iter = node->emplaceBack();
        parseValue(stream, &(*iter));
        skip(stream);
        if (stream.peek() == ',') {
            stream.get();
            skip(stream);
#ifdef PGJSON_ALLOW_AFTER_LAST_ITEM_HAS_COMMA
            if (stream.peek() == ']') break;
#else
            PGJSON_PARSE_ASSERT_EX("Don\'t Allow \',\'", stream.peek() != ']');
#endif
        }
        else if (stream.peek() == ']')
            break;
    }

    PGJSON_PARSE_ASSERT_EX("Parse Array Error", stream.peek() == ']');
    stream.get();
}

template<typename STREAM, typename NODE>
void parseObject(STREAM & stream, NODE * node) {
    PGJSON_PARSE_ASSERT_EX("Parse Object Error", stream.peek() == '{');
    stream.get();

    skip(stream);
    if (stream.peek() == '}') {
        node->setObject();
        stream.get();
        return;
    }
    PGJSON_PARSE_ASSERT_EX("Parse Object Error", stream.peek() == '\"');

    node->setObject();
    while (true) {
        std::vector<Char> buffer;
        parseString(stream, buffer);
        auto iter = node->addMember(buffer.data());

        skip(stream);
        PGJSON_PARSE_ASSERT_EX("Parse Object Error", stream.peek() == ':');
        stream.get();

        skip(stream);
        parseValue(stream, &(iter->value));

        skip(stream);
        if (stream.peek() == ',') {
            stream.get();
            skip(stream);
#ifdef PGJSON_ALLOW_AFTER_LAST_ITEM_HAS_COMMA
            if (stream.peek() == '}') break;
#else
            PGJSON_PARSE_ASSERT_EX("Don\'t Allow \',\'", stream.peek() != '}');
#endif
        }
        else if (stream.peek() == '}')
            break;
    }

    // skip;
    // while(true) {
    //      let buffer;
    //      parseString(stream, buffer);
    //      let iter = addMember(buffer.data());
    //      skip;
    //      assert(peek == ':');
    //      get;
    //      skip;
    //      parseValue(stream, &iter->value);
    //      skip;
    //    if (stream.peek() == ',') {
    //        stream.get();
    //        skip(stream);
    //#ifdef PGJSON_ALLOW_AFTER_LAST_ITEM_HAS_COMMA
    //        if (stream.peek() == '}') break;
    //#else
    //        PGJSON_PARSE_ASSERT_EX("Don\'t Allow \',\'", stream.peek() != '}');
    //#endif
    //    }
    //    else if (stream.peek() == '}')
    //        break;
    // }

    PGJSON_PARSE_ASSERT_EX("Parse Object Error", stream.peek() == '}');
    stream.get();
}

template<typename STREAM, typename NODE>
void parseValue(STREAM & stream, NODE * node) {
    switch (stream.peek()) {
    case 'n': parseNull(stream, node); break;
    case 'f' : parseFalse(stream, node); break;
    case 't': parseTrue(stream, node); break;
    case '\"' : parseString(stream, node); break;
    case '[' : parseArray(stream, node); break;
    case '{' : parseObject(stream, node); break;
    case '0' :
    case '1' :
    case '2' :
    case '3' :
    case '4' :
    case '5' :
    case '6' :
    case '7' :
    case '8' :
    case '9' :
    case '+' :
    case '-' :
        parseNumber(stream, node); break;
    default:
        PGJSON_PARSE_ASSERT_EX("Parse Error", false);
        break;
    }
}

template<typename STREAM, typename NODE>
NODE * parse(STREAM & istream, NODE * root) {
    skip(istream);

    parseObject(istream, root);

    return root;
}

PGJSON_NAMESPACE_END
#endif //PGTEST_PARSER_H
