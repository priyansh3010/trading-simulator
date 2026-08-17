#pragma once
#include "types.h"
#include "order.h"
#include "orderbook.h"
#include "node.h"
#include <list>
#include <map>
#include <unordered_map>
#include <utility>
#include <string>
#include <iostream>
using namespace std;

OrderBook::OrderBook() {
    orderId_ = 0;
    participantId_ = 0;
    orderCount = 0;
}

pair<bool, U64> OrderBook::addOrder(Side side, int price, U64 quantity, U64 timestamp) {
    // add to appropriate map and list
    Order* order;
    Node* newNode;
    match(price, side, quantity);
    if (side == BUY) {
        if (quantity > 0) {
            order = new Order(orderId_, participantId_, side, price, quantity, timestamp);
            auto* DLL = &bids_[price];
            newNode = new Node(order, DLL);
            if (*DLL == pair<Node*, Node*>()) { // first entry at this price level
                Node* dummyHead = new Node();
                Node* dummyTail = new Node();
                
                newNode->left = dummyHead;
                newNode->right = dummyTail;

                dummyHead->right = newNode;
                dummyTail->left = newNode;

                newNode->DLL = DLL;
                
                DLL->first = dummyHead; DLL->second = dummyTail;
            }
            else {
                Node* currTail = DLL->second->left;
                currTail->right = newNode;
                newNode->left = currTail;
                newNode->right = DLL->second;
                DLL->second->left = newNode;
                newNode->DLL = DLL;
            }
        }
    }
    
    else {
        if (quantity > 0) {
            order = new Order(orderId_, participantId_, side, price, quantity, timestamp);
            auto* DLL = &asks_[price];
            newNode = new Node(order, DLL);
            if (*DLL == pair<Node*, Node*>()) { // first entry at this price level
                Node* dummyHead = new Node();
                Node* dummyTail = new Node();
                
                newNode->left = dummyHead;
                newNode->right = dummyTail;
                
                newNode->DLL = DLL;

                dummyHead->right = newNode;
                dummyTail->left = newNode;

                DLL->first = dummyHead; DLL->second = dummyTail;
            }
            else {
                Node* currTail = DLL->second->left;
                currTail->right = newNode;
                newNode->left = currTail;
                newNode->right = DLL->second;
                DLL->second->left = newNode;
            }
        }
    }
    
    // early return if new order is already completed to avoid it being added to order book
    if (quantity == 0) return {false, orderId_};
    
    // add to hashmap for quick lookups for future
    orderMap_[orderId_] = newNode;
    orderId_++; // increment order id for now. more sophistication later on
    orderCount++; 
    return {true, order->orderId};
}

void OrderBook::cancelOrder(U64 orderId) {
    // access node in question
    Node* node = orderMap_[orderId];
    
    // unlink node in DLL according to where it is in its DLL
    Node* left = node->left;
    Node* right = node->right;
    
    if (left->left == nullptr && right->right == nullptr) { // only one order exists at this price level
        auto* DLL = node->DLL;
        DLL->first = nullptr;
        DLL->second = nullptr;
    }
    else if (right->right == nullptr) { // is the tail node
        node->right->left = left;
        node->left->right = right;
    }
    else { // is a node in the middle of the DLL, or a head node
        node->left->right = right;
        node->right->left = left;
    }

    // delete node from hashMap too
    orderCount--;
    orderMap_.erase(orderId);
    delete node;
}

void OrderBook::match(int& price, Side& side, U64& quantity) {
    vector<U64> toRemove;
    if (side == BUY) {
        for (auto& [p, DLL] : asks_) {
            if (p > price) break; // break outer loop if p becomes greater than maximum buy price
            if (quantity == 0) break; // early break for order completion before insertion
            if (DLL == pair<Node*, Node*>()) continue;

            Node* traverse = DLL.first->right;
            
            while (traverse != DLL.second && quantity > 0) {
                Order* order = traverse->order;
                if (quantity >= order->quantity) {
                    quantity -= order->quantity;
                    
                    // 'cancel' sell order for now (it is complete)
                    toRemove.push_back(order->orderId);
                }
                else {
                    order->quantity -= quantity;
                    quantity = 0;
                }

                traverse = traverse->right;
            }
        }
    }
    else {
        for (auto& [p, DLL] : bids_) {
            if (p < price) break; // break outer loop if p becomes lesser than minimum sell price
            if (quantity == 0) break; // early break for order completion before insertion
            if (DLL == pair<Node*, Node*>()) continue;

            Node* traverse = DLL.first->right;
            
            while (traverse != DLL.second && quantity > 0) {
                Order* order = traverse->order;
                if (quantity >= order->quantity) {
                    quantity -= order->quantity;
                    
                    // 'cancel' sell order for now (it is complete)
                    toRemove.push_back(order->orderId);
                }
                else {
                    order->quantity -= quantity;
                    quantity = 0;
                }

                traverse = traverse->right;
            }
        }
    }

    // delete completed orders
    for (int const& orderId : toRemove) cancelOrder(orderId);
}

pair<bool, U64> OrderBook::updateOrder(U64 orderId, int newPrice, int newQuantity) {
    // access order in question
    Node* node = orderMap_.find(orderId) != orderMap_.end() ? orderMap_[orderId] : nullptr;

    if (node == nullptr) return {false, orderId_};

    // store order details before it is deleted from memory
    Side side = node->order->side;
    U64 timestamp = node->order->timestamp;

    // delete order from map
    cancelOrder(orderId);
    
    if (newQuantity == 0) return {false, orderId_}; // if new quantity is 0, take it as order has cancelled
    
    // add new order to map
    return addOrder(side, newPrice, newQuantity, timestamp);
}

bool OrderBook::orderExists(U64 orderId){
    return orderMap_.find(orderId) != orderMap_.end();
}

Order* OrderBook::getOrder(U64 orderId) {
    return orderMap_[orderId]->order;
}

int OrderBook::getDepth(int price, Side side) {
    int count = 0;
    if (side == BUY) {
        auto& DLL = bids_[price];
        if (DLL == pair<Node*, Node*>()) return 0;
        Node* traverse = DLL.first->right;
        
        while (traverse != bids_[price].second) {
            count++;
            traverse = traverse->right;
        }
    }
    else {
        auto& DLL = asks_[price];
        if (DLL == pair<Node*, Node*>()) return 0;
        Node* traverse = DLL.first->right;

        while (traverse != asks_[price].second) {
            count++;
            traverse = traverse->right;
        }
    }

    return count;
}

int OrderBook::getBestBid() {
    for (auto& [p, DLL] : bids_) {
        if (DLL == pair<Node*, Node*>()) continue;
        return p;
    }
    
    return -1;
}

int OrderBook::getBestAsk() {
    for (auto& [p, DLL] : asks_) {
        if (DLL == pair<Node*, Node*>()) continue;
        return p;
    }

    return -1;
}

int OrderBook::getOrderCount() {
    return orderCount;
}

void OrderBook::printOrders() {
    cout << "---------Bids Map---------" << endl;
    for (auto& [p, DLL] : bids_) {
        cout << "Price: " << p << endl;
        if (DLL == pair<Node*, Node*>()) continue;
        Node* traverse = DLL.first->right;
        while (traverse != DLL.second) {
            Order* order = traverse->order;
            cout << "   " << order->orderId << " " << order->participantId << " " << "BUY" << " "
                          << order->price << " " << order->quantity << " " << order->timestamp << endl;
            traverse = traverse->right;                  
        }
    }

    cout << "---------Asks Map---------" << endl;
    for (auto& [p, DLL] : asks_) {
        cout << "Price: " << p << endl;
        if (DLL == pair<Node*, Node*>()) continue;
        Node* traverse = DLL.first->right;
        while (traverse != DLL.second) {
            Order* order = traverse->order;
            cout << "   " << order->orderId << " " << order->participantId << " " << "SELL" << " "
                          << order->price << " " << order->quantity << " " << order->timestamp << endl;
            traverse = traverse->right;                  
        }
    }
}