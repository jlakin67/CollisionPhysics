#include <iostream>
#include "../linked_list.h"

struct Test {
    int a = 0;
    int b = 0;
};

int main() {
    LinkedList<Test, 20> linkedList1;
    for (int i = 0; i < 10; i++) {
        Test test{ i, i };
        linkedList1.push_back(test);
    }
    linkedList1.erase(++linkedList1.begin());
    for (int i = 10; i < 20; i++) {
        Test test{ i, i };
        linkedList1.push_back(test);
    }
    for (Test test : linkedList1) {
        std::cout << test.a << " " << test.b << std::endl;
    }
    LinkedList<Test, 20> linkedList2;
    for (int i = 0; i < 20; i++) {
        Test test2{ i,i };
        auto it = linkedList2.push_back(test2);
        linkedList2.erase(it);
    }
    for (auto it = linkedList2.begin(); it != linkedList2.end(); it++) {
        assert(!it);
        assert(false);
    }
    std::cout << "------------------\n";
    LinkedList<Test, 20> linkedList3;
    LinkedList<Test, 20>::Iterator tail;
    for (int i = 0; i < 20; i++) {
        auto temp = linkedList3.push_back({ i,i });
        if (i == 19) tail = temp;
    }
    linkedList3.erase(linkedList3.begin());
    auto it = linkedList3.begin();
    it++;
    linkedList3.erase(it);
    linkedList3.erase(tail);
    for (auto it = linkedList3.begin(); it != linkedList3.end(); it++) {
        std::cout << it->a << " " << it->b << std::endl;
    }

    std::cout << "------------------\n";

    for (auto it = linkedList3.begin(); it != linkedList3.end();) {
        linkedList3.erase(it++);
    }
    linkedList3.push_back({ 99,99 });
    linkedList3.push_back({ 100, 100 });
    for (auto it = linkedList3.begin(); it != linkedList3.end(); it++) {
        std::cout << it->a << " " << it->b << std::endl;
    }
    for (auto it = linkedList3.begin(); it != linkedList3.end();) {
        linkedList3.erase(it++);
    }
    for (auto it = linkedList3.begin(); it != linkedList3.end(); it++) {
        assert(false);
        std::cout << it->a << " " << it->b << std::endl;
    }
}
