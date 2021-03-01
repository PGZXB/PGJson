//
// Created by 42025 on 2021/2/26.
//
#include <PGJson/Node.h>

//#include <PGJson/MemoryBlockPool.h>
#include <algorithm>
#include <vector>
#include <list>
#include <cstdlib>
#include <iostream>

using namespace pg::base::json;

#define TEST_ARRAY(pArray) \
    std::cout << "size : " << (pArray)->size() << " \n"; \
    std::cout << "capacity : " << (pArray)->capacity() << " \n"

bool cmpNodeNumber(const Node & n1, const Node & n2) {
    PGJSON_DEBUG_ASSERT_EX(__func__, n1.isNumber());
    PGJSON_DEBUG_ASSERT_EX(__func__, n2.isNumber());

    return n1.getDouble() < n2.getDouble();
}

int main () {

    Node * array = Node::create();

    Node * numbers[20] = { nullptr };

    for (auto & number : numbers) {
        number = Node::create();
        number->setInt64(std::rand() % 1000);
    }

    numbers[9]->setInt64(666);
    numbers[10]->setInt64(456);

    for (auto & number : numbers) {
        std::cout << number->getInt64() << ", ";
    } std::cout << "\b\b \n";

    array->setArray();
    TEST_ARRAY(array);

    for (auto & number : numbers) {
        array->pushBack(number);
    }

    TEST_ARRAY(array);

    array->begin();
    array->end();

    for (int i = 0; i < array->size(); ++i) {
        std::cout << (*array)[i].getInt64() << ", ";
    } std::cout << "\b\b \n";

    for (auto & e : *array) {
        std::cout << e.getInt64() << ", ";
    } std::cout << "\b\b \n";

    std::sort(array->begin(), array->end(), cmpNodeNumber);

    TEST_ARRAY(array);

    for (auto & e : *array) {
        std::cout << e.getInt64() << ", ";
    } std::cout << "\b\b \n";

    array->popBack();
    array->popBack();
    array->popBack();
    array->popBack();
    array->popBack();

    TEST_ARRAY(array);

    for (auto & e : *array) {
        std::cout << e.getInt64() << ", ";
    } std::cout << "\b\b \n\n";

    for (auto iter = array->begin(); iter != array->end(); ++iter) {
        if (iter->getInt64() == 456) {
            array->remove(iter);
            std::cout << iter->getInt64() << "\n";
        }
//        if (iter->getInt64() == 666) {
//            array->remove(iter);
//            std::cout << iter->getInt64() << "\n";
//        }
    }

    for (auto & e : *array) {
        std::cout << e.getInt64() << ", ";
    } std::cout << "\b\b \n";

    TEST_ARRAY(array);

    std::list<Node> test(array->begin(), array->end());

    array->remove(array->begin() + 1, array->begin() + 4);

//    array->clear();

    std::cout << "\n{";
    for (auto iter = array->begin(); iter != array->end(); ++iter) {
        std::cout << iter->getInt64() << ", ";
    } std::cout << "}\n";

    TEST_ARRAY(array);


//    for (auto & e : test) {
//        std::cout << e.getInt64() << ", ";
//    } std::cout << "\b\b\n";
//
//    for (auto iter = test.begin(); iter != test.end(); ++iter) {
//        if (iter->getInt64() == 666) {
//            iter = test.erase(iter);
//            std::cout << iter->getInt64() << "\n";
//        }
//    }

    return 0;
}
