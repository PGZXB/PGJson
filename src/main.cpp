//
// Created by 42025 on 2021/2/26.
//
#include <PGJson/Node.h>
#include <PGJson/MemoryBlockPool.h>
#include <PGJson/FileStream.h>
#include <PGJson/StringReadStream.h>
#include <PGJson/Parser.h>

#include <algorithm>
#include <vector>
#include <list>
#include <cstdlib>
#include <iostream>

using namespace pg::base::json;

int main () {
    MallocAllocator::s_pInstance = new MallocAllocator();
    MemoryBlockPool<sizeof(Node), Node>::s_pInstance = new MemoryBlockPool<sizeof(Node), Node>();
    MemoryBlockPool<sizeof(ObjectMember), ObjectMember>::s_pInstance = new MemoryBlockPool<sizeof(ObjectMember), ObjectMember>();
//
//    Node * object = Node::create();
//    object->setObject();
//
//    object->addMember("ADDDDDDD")->value.setInt64(1);
//    object->addMember("BDDDDDDD")->value.setInt64(2);
//    object->addMember("CDDDDDDD")->value.setInt64(3);
//    object->addMember("DDDDDDDD")->value.setInt64(4);
//    object->addMember("EDDDDDDD")->value.setInt64(5);
//    object->addMember("FDDDDDDD")->value.setInt64(6);
//    object->addMember("GDDDDDDD")->value.setInt64(7);
//    object->addMember("HDDDDDDDDD")->value.setInt64(8);
//    object->addMember("IDDDDDDDDD")->value.setInt64(9);
//    object->addMember("GDDDDDDDDD")->value.setInt64(10);
//
//    for (auto iter = object->memberBegin(), end = object->memberEnd(); iter != end; ++iter) {
//        std::cout << (const Char *)iter->name.getCString() << " : " << iter->value.getInt64() << "\n";
//    }
//
//    std::cout << "{";
//    for (SizeType i = 0; i < object->memberCount(); ++i) {
//        std::cout << (*object)[i].getInt64() << ", ";
//    } std::cout << "}\n";
//
//    for (auto iter = object->memberBegin(), end = object->memberEnd(); iter != end; ++iter) {
//        PGJSON_DEBUG_ASSERT((*object)[iter->name.getCString()].getInt64() == iter->value.getInt64());
//        std::cout << iter->name.getCString() << " : " << (*object)[iter->name.getCString()].getInt64() << " " << iter->value.getInt64() << "\n";
//    }
//
//    object->removeMember(object->memberBegin(), object->memberEnd() - 2);
//
//
//    std::cout << "{";
//    for (SizeType i = 0; i < object->memberCount(); ++i) {
//        std::cout << (*object)[i].getInt64() << ", ";
//    } std::cout << "}\n";
//
//    object->clear();
//    std::cout << object->memberCount() << "\n";
//

//    FileStream<> fileStream("../src/test.json", ReadMode);

    StringReadStream fileStream(
"{\n"
        "   \"sites\": [\n"
        "   {\"name\" : \"ZhanSan\", \"age\" : 18, \"DD Len\" : 19.25},\n"
        "   { \"name\":\"cainiao\" , \"url\":\"www.runoob.com\" },\n"
        "   { \"name\":\"google\" , \"url\":\"www.google.com\" },\n"
        "   ]\n"
        "}\n", true
    );
    // in test.json
    // {
    //     "sites": [
    //     { "name":"cainiao" , "url":"www.runoob.com" },
    //     { "name":"google" , "url":"www.google.com" },
    //     ]
    // }

    Node * node = Node::create();

    parse(fileStream, node);

    std::cout << node->toDebugString() << "\n";

    delete MemoryBlockPool<sizeof(Node), Node>::s_pInstance;
    delete MemoryBlockPool<sizeof(ObjectMember), ObjectMember>::s_pInstance;
    delete MallocAllocator::s_pInstance;
    return 0;
}
