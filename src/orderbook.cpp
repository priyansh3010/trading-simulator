#pragma once
#include "types.h"
#include "order.h"
#include "orderbook.h"
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
    vector<U64> toRemove;
    // add to appropriate map and list
    Order* order;
    match(price, side, toRemove, quantity);
    if (side == BUY) {
        if (quantity > 0) {
            order = new Order(orderId_, participantId_, side, price, quantity, timestamp);
            bids_[price].push_back(order);
        }
    }
    
    else {
        if (quantity > 0) {
            order = new Order(orderId_, participantId_, side, price, quantity, timestamp);
            asks_[price].push_back(order);
        }
    }
    
    // remove completed orders
    for (int const& orderId : toRemove) cancelOrder(orderId);
    
    // early return if new order is already completed to avoid it being added to order book
    if (quantity == 0) return {false, orderId_};
    
    // add to hashmap for quick lookups for future
    orderMap_[orderId_] = order;
    orderId_++; // increment order id for now. more sophistication later on
    orderCount++; 
    return {true, order->orderId};
}

void OrderBook::cancelOrder(U64 orderId) {
    // access order in question
    Order* order = orderMap_[orderId];
    
    // delete order from the map and list it is in
    if (order->side == BUY) bids_[order->price].remove(order);
    else asks_[order->price].remove(order);
    
    // delete order from hashMap too
    orderCount--;
    orderMap_.erase(orderId);
    delete order;
}

void OrderBook::match(int& price, Side& side, vector<U64>& toRemove, U64& quantity) {
    if (side == BUY) {
        for (auto& [p, orderList] : asks_) {
            if (p > price) break; // break outer loop if p becomes greater than maximum buy price
            if (quantity == 0) break; // early break for order completion before insertion
            
            for (Order* order : orderList) {
                if (quantity >= order->quantity) {
                    quantity -= order->quantity;
                    
                    // just 'cancel' sell order for now (it is complete)
                    // will create seperate function for completed orders later
                    toRemove.push_back(order->orderId);
                }
                else {
                    order->quantity -= quantity;
                    quantity = 0;
                } 
            }
        }
    }
    else {
        for (auto& [p, orderList] : bids_) {
            if (p < price) break; // break outer loop if p becomes lesser than minimum sell price
            if (quantity == 0) break; // early break for order completion before insertion
            
            for (Order* order : orderList) {
                if (quantity >= order->quantity) {
                    quantity -= order->quantity;
                    order->quantity = 0;
                    
                    // just 'cancel' sell order for now (it is complete)
                    // will create seperate function for completed orders later
                    toRemove.push_back(order->orderId);
                }
                else {
                    order->quantity -= quantity;
                    quantity = 0;
                } 
            }
        }
    }
}

pair<bool, U64> OrderBook::updateOrder(U64 orderId, int newPrice, int newQuantity) {
    // access order in question
    Order* order = orderMap_.find(orderId) != orderMap_.end() ? orderMap_[orderId] : nullptr;

    if (order == nullptr) return {false, orderId_};

    // store order details before it is deleted from memory
    Side side = order->side;
    U64 timestamp = order->timestamp;

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
    return orderMap_[orderId];
}

int OrderBook::getDepth(int price, Side side) {
    if (side == BUY)
        return bids_[price].size();
    else
        return asks_[price].size();
}

int OrderBook::getBestBid() {
    for (auto& [p, orderList] : bids_) {
        for (Order* order : orderList) {
            return order->price;        
        }
    }
    
    return -1;
}

int OrderBook::getBestAsk() {
    for (auto& [p, orderList] : asks_) {
        for (Order* order : orderList) {
            return order->price;        
        }
    }

    return -1;
}

int OrderBook::getOrderCount() {
    return orderCount;
}

void OrderBook::printOrders() {
    cout << "---------Bids Map---------" << endl;
    for (auto& [p, orderList] : bids_) {
        cout << "Price: " << p << endl;
        for (Order* order : orderList) {
            cout << "   " << order->orderId << " " << order->participantId << " " << "BUY" << " "
                          << order->price << " " << order->quantity << " " << order->timestamp << endl;
        }
    }

    cout << "---------Asks Map---------" << endl;
    for (auto& [p, orderList] : asks_) {
        cout << "Price: " << p << endl;
        for (Order* order : orderList) {
            cout << "   " << order->orderId << " " << order->participantId << " " << "SELL" << " "
                          << order->price << " " << order->quantity << " " << order->timestamp << endl;
        }
    }
}