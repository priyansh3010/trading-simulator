#pragma once
#include "types.h"
#include "order.h"

struct Node {
    Node* left;
    Node* right;
    Order* order;
    bool isDummy;

    Node(Order* o) : left(nullptr), right(nullptr), order(o), isDummy(false) {}
    Node(Node* l, Node* r, Order* o) : left(l), right(r), order(o), isDummy(false) {}
};