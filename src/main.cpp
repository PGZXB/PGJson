//
// Created by 42025 on 2021/2/26.
//
#define private public
#include <PGJson/Node.h>
#include <PGJson/ObjectPool.h>
#include <iostream>

using namespace pg::base::json;

int main () {
//    Node node;
//
//    node.setString("Test", 4);
//
//    std::cout << node.getString() << "\n";
//    std::cout << node.getStringLength() << "\n";

    ObjectPool<Node> pool;

    pool.createObject();
    auto p = pool.createObject();
    pool.createObject();
    pool.createObject();
    pool.createObject();
    pool.createObject();
    pool.createObject();



    p->setString(std::string("void"));
    std::cout << p->getString() << '\n';

    p->setInt32(1024);
    std::cout << p->getInt32() << '\n';
    std::cout << p->getInt64() << '\n';
    std::cout << p->getDouble() << '\n';

    std::cout << p->isDouble() << "\n";
    std::cout << p->isString() << "\n";
    std::cout << p->isUInt64() << "\n";
    std::cout << p->isInt32() << "\n";
    std::cout << p->isDouble() << "\n";
    std::cout << p->isNumber() << "\n";
    std::cout << p->isNull() << "\n";
    std::cout << p->isArray() << "\n";
    std::cout << p->isBool() << "\n";
    std::cout << p->isObject() << "\n";

    pool.destroyObject(p);

    ObjectPool<Node>::BlockInfo* ptr = (ObjectPool<Node>::BlockInfo*)p;
    std::cout << ptr->next.blockIndexInChunk << '\n';
    std::cout << ptr->next.chunkIndexInChunks << '\n';
    std::cout << ptr->next.magicTag << '\n';
    std::cout << ObjectPool<Node>::BlockInfo::Next::MAGIC_TAG << '\n';

    std::cout << pool.m_topBlockInfo.next.blockIndexInChunk << '\n';

    return 0;
}
