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

    bestBidIndex_ = -1;
    bestAskIndex_ = -1;

    nodeMap_.reserve(1100000);
    orderMap_.reserve(1100000);
    isValid_.reserve(1100000);
    bids_.reserve(50000);
    asks_.reserve(50000);
    bidOccupancy_.assign(313, 0ULL);
    askOccupancy_.assign(313, 0ULL);

    for (int i = 0; i < 1100000; i++) {
        nodeMap_[i] = new Node();
        orderMap_[i] = new Order();
        isValid_[i] = 0;
        if (i < 20000) {
            bids_[i] = pair<Node*, Node*>();
            asks_[i] = pair<Node*, Node*>();
        }
    }
}

pair<bool, U64> OrderBook::addOrder(Side side, int price, U64 quantity, U64 timestamp) {
    // add to appropriate map and list
    Order* order = orderMap_[orderId_];
    Node* newNode = nodeMap_[orderId_];
    match(price, side, quantity);
    if (side == BUY) {
        if (quantity > 0) {
            order->orderId = orderId_;
            order->quantity = quantity; order->participantId = participantId_; order->side = side;
            order->price = price; order->quantity = quantity; order->timestamp = timestamp;
            auto* DLL = &bids_[price];
            newNode->order = order;
            newNode->DLL = DLL;
            if (DLL->first == nullptr && DLL->second == nullptr) { // first entry at this price level
                Node* dummyHead = new Node();
                Node* dummyTail = new Node();
                
                newNode->left = dummyHead;
                newNode->right = dummyTail;
                
                newNode->DLL = DLL;
                
                bidOccupancy_[price / 64] |= (1ULL << (price % 64));
                if (bestBidIndex_ < price || bestBidIndex_ == -1) bestBidIndex_ = price;
                
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
                newNode->DLL = DLL;
            }
        }
    }
    else {
        if (quantity > 0) {
            order->orderId = orderId_;
            order->quantity = quantity; order->participantId = participantId_; order->side = side;
            order->price = price; order->quantity = quantity; order->timestamp = timestamp;
            auto* DLL = &asks_[price];
            newNode->order = order;
            newNode->DLL = DLL;
            if (DLL->first == nullptr && DLL->second == nullptr) { // first entry at this price level
                Node* dummyHead = new Node();
                Node* dummyTail = new Node();
                
                newNode->left = dummyHead;
                newNode->right = dummyTail;
                
                newNode->DLL = DLL;
                askOccupancy_[price / 64] |= (1ULL << (price % 64));
                if (bestAskIndex_ > price || bestAskIndex_ == -1) bestAskIndex_ = price;
                
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
    nodeMap_[orderId_] = newNode;
    orderMap_[orderId_] = order;
    isValid_[orderId_] = 1;
    orderId_++; // increment order id for now. more sophistication later on
    orderCount++; 

    return {true, orderId_ - 1};
}

void OrderBook::cancelOrder(U64 orderId) {
    // access node in question
    Node* node = nodeMap_[orderId];
    
    // unlink node in DLL according to where it is in its DLL
    Node* left = node->left;
    Node* right = node->right;
    
    if (left->left == nullptr && right->right == nullptr) { // only one order exists at this price level
        auto* DLL = node->DLL;
        DLL->first = nullptr;
        DLL->second = nullptr;

        int price = node->order->price;
        Side side = node->order->side;

        if (side == BUY) {
            bidOccupancy_[price / 64] &= ~(1ULL << (price % 64));
            if (price == bestBidIndex_) recomputeBestBid();
        } 
        else {
            askOccupancy_[price / 64] &= ~(1ULL << (price % 64));    
            if (price == bestAskIndex_) recomputeBestAsk();
        } 
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
    isValid_[node->order->orderId] = 0;
}

void OrderBook::match(int& price, Side& side, U64& quantity) {
    if (side == BUY) {
        while (bestAskIndex_ != -1 && bestAskIndex_ <= price && quantity > 0) { // loop only while bestAsk price is greater than curr price and quantity > 0            
            vector<U64> toRemove;
            auto& DLL = asks_[bestAskIndex_];
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
            
            for (int const& orderId : toRemove) cancelOrder(orderId);
        }
    }
    else {
        while (bestBidIndex_ != -1 && bestBidIndex_ >= price && quantity > 0) { // loop only while bestBid price is less than curr price and quantity > 0            
            vector<U64> toRemove;
            auto& DLL = bids_[bestBidIndex_];
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

            for (int const& orderId : toRemove) cancelOrder(orderId);
        }
    }
}

pair<bool, U64> OrderBook::updateOrder(U64 orderId, int newPrice, int newQuantity) {
    // access order in question
    Node* node = isValid_[orderId] ? nodeMap_[orderId] : nullptr;

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
    return isValid_[orderId];
}

Order* OrderBook::getOrder(U64 orderId) {
    return nodeMap_[orderId]->order;
}

int OrderBook::getDepth(int price, Side side) {
    int count = 0;
    if (side == BUY) {
        auto& DLL = bids_[price];
        if (DLL.first == nullptr && DLL.second == nullptr) return 0;
        Node* traverse = DLL.first->right;
        
        while (traverse != bids_[price].second) {
            count++;
            traverse = traverse->right;
        }
    }
    else {
        auto& DLL = asks_[price];
        if (DLL.first == nullptr && DLL.second == nullptr) return 0;
        Node* traverse = DLL.first->right;

        while (traverse != asks_[price].second) {
            count++;
            traverse = traverse->right;
        }
    }

    return count;
}

void OrderBook::recomputeBestBid() {
    int wordIdx = bestBidIndex_ / 64;
    // mask out bits above bestBidIdx_ within its own word
    // we're looking for the next-highest occupied bit at or below where best just was
    U64 word = bidOccupancy_[wordIdx] & ((1ULL << (bestBidIndex_ % 64 + 1)) - 1);
    while (word == 0 && wordIdx > 0) {
        word = bidOccupancy_[--wordIdx];
    }
    if (word == 0) { // book empty
        bestBidIndex_ = -1; 
        return; 
    }
    int bitInWord = getMSB(word); // highest set bit in this word
    bestBidIndex_ = wordIdx * 64 + bitInWord;
}

void OrderBook::recomputeBestAsk() {
    int wordIdx = bestAskIndex_ / 64;
    // mask out bits below bestAskIdx_ within its own word
    // we're looking for the next-lowest occupied bit at or above where best just was
    U64 word = askOccupancy_[wordIdx] & ~((1ULL << (bestAskIndex_ % 64)) - 1);
    while (word == 0 && wordIdx < 312) {
        word = askOccupancy_[++wordIdx];
    }
    if (word == 0) { // book empty
        bestAskIndex_ = -1;
        return;
    }
    int bitInWord = getLSB(word); // lowest set bit in this word
    bestAskIndex_ = wordIdx * 64 + bitInWord;
}

int OrderBook::getBestBid() {
    return bestBidIndex_;
}

int OrderBook::getBestAsk() {
    return bestAskIndex_;
}

int OrderBook::getOrderCount() {
    return orderCount;
}

void OrderBook::printOrders() {
    cout << "---------Bids Map---------" << endl;
    for (int i = 49999; i >= 0; i--) {
        auto& DLL = bids_[i];
        if (DLL == pair<Node*, Node*>()) continue;
        cout << "Price: " << i << endl;
        Node* traverse = DLL.first->right;
        while (traverse != DLL.second) {
            Order* order = traverse->order;
            cout << "   " << order->orderId << " " << order->participantId << " " << "BUY" << " "
                          << order->price << " " << order->quantity << " " << order->timestamp << endl;
            traverse = traverse->right;                  
        }
    }

    cout << "---------Asks Map---------" << endl;
    for (int i = 0; i < 50000; i++) {
        auto& DLL = asks_[i];
        if (DLL == pair<Node*, Node*>()) continue;
        cout << "Price: " << i << endl;
        Node* traverse = DLL.first->right;
        while (traverse != DLL.second) {
            Order* order = traverse->order;
            cout << "   " << order->orderId << " " << order->participantId << " " << "SELL" << " "
                          << order->price << " " << order->quantity << " " << order->timestamp << endl;
            traverse = traverse->right;                  
        }
    }
}