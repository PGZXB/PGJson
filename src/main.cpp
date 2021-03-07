//
// Created by 42025 on 2021/2/26.
//
#include <PGJson/Node.h>
#include <PGJson/MemoryBlockPool.h>
#include <PGJson/FileStream.h>
#include <PGJson/StringReadStream.h>
#include <PGJson/Parser.h>

#include <algorithm>
#include <iostream>

using namespace pg::base::json;

int main () {
    MallocAllocator::s_pInstance = new MallocAllocator();
    MemoryBlockPool<sizeof(Node), Node>::s_pInstance = new MemoryBlockPool<sizeof(Node), Node>();
    MemoryBlockPool<sizeof(ObjectMember), ObjectMember>::s_pInstance = new MemoryBlockPool<sizeof(ObjectMember), ObjectMember>();


//    StringReadStream fileStream(
//"{\n"
//        "   \"sites\": [\n"
//        "   {\"name\" : \"ZhanSan\", \"age\" : 18, \"Len\" : 19.24},\n"
//        "   { \"name\":\"cainiao\" , \"url\": [\"www.runoob.com\", \"www.pp.com\"] },\n"
//        "   { \"name\":\"google\" , \"url\":\"www.google.com\" },\n"
//        "   ],\n"
//        "   \"msg\" : \"I\\tam\\tyour\\tteacher\"\n"
//        "}\n", true
//    );

    FileStream<> fileStream("../src/test-parse.json", ReadMode);

    FileStream<> fileStream2("../src/test-write.json", WriteMode);

    Node * node = Node::create();

    parse(fileStream, node);

//    std::cout << node->toDebugString() << "\n";

    char tab[] = "\t";
    toString(node, fileStream2, tab);

    delete MemoryBlockPool<sizeof(Node), Node>::s_pInstance;
    delete MemoryBlockPool<sizeof(ObjectMember), ObjectMember>::s_pInstance;
    delete MallocAllocator::s_pInstance;
    return 0;
}
