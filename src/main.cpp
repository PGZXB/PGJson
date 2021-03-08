//
// Created by 42025 on 2021/2/26.
//
#include <PGJson/Node.h>
#include <PGJson/StringReadStream.h>
#include <PGJson/Document.h>

#include <algorithm>
#include <iostream>

using namespace pg::base::json;

int main () {
    StringReadStream fileStream(
"{\n"
        "   \"sites\": [\n"
        "   {\"name\" : \"ZhanSan\", \"age\" : 18, \"Len\" : 19.24},\n"
        "   { \"name\":\"cainiao\" , \"url\": [\"www.runoob.com\", \"www.pp.com\"] },\n"
        "   { \"name\":\"google\" , \"url\":\"www.google.com\" },\n"
        "   ],\n"
        "   \"msg\" : \"I\\tam\\tyour\\tteacher\"\n"
        "}\n", true
    );

    Document document;

    document.parse(fileStream);

    std::cout << document.d().toDebugString() << "\n";

    std::string json = document.stringify();
    std::cout << json << "\n";

    return 0;
}
