#pragma once
#include "types.h"
#include "order.h"

struct Node {
    Node* left;
    Node* right;
    Order* order;

    Node(Order* o) : left(nullptr), right(nullptr), order(o) {}
    Node(Node* l, Node* r, Order* o) : left(l), right(r), order(o) {}
};