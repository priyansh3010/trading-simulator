#pragma once
#include "types.h"
#include "order.h"
#include <utility>

struct Node {
    Node* left;
    Node* right;
    Order* order;
    pair<Node*, Node*>* DLL;

    Node() = default;
    Node(Order* o, pair<Node*, Node*>* d) : left(nullptr), right(nullptr), order(o), DLL(d) {}
};