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
    instrumentId_ = 0;
}

pair<bool, U64> OrderBook::addOrder(string instrumentName, Side side, int price, U64 quantity, U64 timestamp) {
    // check if current instrument has entered book before
    bool found;
    U64 instrumentId;
    if (instrumentToId_.find(instrumentName) == instrumentToId_.end()) {
        instrumentToId_[instrumentName] = instrumentId_;
        instrumentId = instrumentId_++;
        found = false;
    }
    else {
        instrumentId = instrumentToId_[instrumentName];
        found = true;
    }
    
    vector<U64> toRemove;
    // add to appropriate map and list
    Order* order;
    if (found) match(instrumentId, price, side, toRemove, quantity);
    if (side == BUY) {
        if (quantity > 0) {
            order = new Order(orderId_, participantId_, instrumentId, side, price, quantity, timestamp);
            bids_[price].push_back(*order);
        }
    }
    
    else {
        if (quantity > 0) {
            order = new Order(orderId_, participantId_, instrumentId, side, price, quantity, timestamp);
            asks_[price].push_back(*order);
        }
    }
    
    // remove completed orders
    for (int const& orderId : toRemove) cancelOrder(orderId);
    
    // early return if new order is already completed to avoid it being added to order book
    if (quantity == 0) return {false, orderId_};
    
    // add to hashmap for quick lookups for future
    orderMap_[orderId_] = *order;
    orderId_++; // increment order id for now. more sophistication later on
    return {true, order->orderId};
}

void OrderBook::cancelOrder(U64 orderId) {
    // access order in question
    Order order = orderMap_[orderId];
    
    // delete order from the map and list it is in
    if (order.side == BUY) bids_[order.price].remove(order);
    else asks_[order.price].remove(order);
    
    // delete order from hashMap too
    orderMap_.erase(orderId);
}

void OrderBook::match(U64& instrumentId, int& price, Side& side, vector<U64>& toRemove, U64& quantity) {
    if (side == BUY) {
        for (auto& [p, orderList] : asks_) {
            if (p > price) break; // break outer loop if p becomes greater than maximum buy price
            if (quantity == 0) break; // early break for order completion before insertion
            
            for (auto it = orderList.begin(); it != orderList.end(); it++) {
                if (it->instrumentId == instrumentId) {
                    if (quantity >= it->quantity) {
                        quantity -= it->quantity;
                        
                        // just 'cancel' sell order for now (it is complete)
                        // will create seperate function for completed orders later
                        toRemove.push_back(it->orderId);
                    }
                    else {
                        it->quantity -= quantity;
                        quantity = 0;
                    }
                } 
            }
        }
    }
    else {
        for (auto& [p, orderList] : bids_) {
            if (p < price) break; // break outer loop if p becomes lesser than minimum sell price
            if (quantity == 0) break; // early break for order completion before insertion
            
            for (auto it = orderList.begin(); it != orderList.end(); it++) {
                if (it->instrumentId == instrumentId) {
                    if (quantity >= it->quantity) {
                        quantity -= it->quantity;
                        
                        // just 'cancel' sell order for now (it is complete)
                        // will create seperate function for completed orders later
                        toRemove.push_back(it->orderId);
                    }
                    else {
                        it->quantity -= quantity;
                        quantity = 0;
                    }
                } 
            }
        }
    }
}

void OrderBook::updateOrder(U64 orderId, int newPrice, int newQuantity) {
    // access order in question
    Order order = orderMap_[orderId];
    
    // updating order from the map and list it is in
    // minor optimization possible here
    // think unnecessary deletions in some cases (when price isnt the thing being updated)
    if (order.side == BUY) {
        bids_[order.price].remove(order);
        order.price = newPrice;
        order.quantity = newQuantity;
        bids_[order.price].push_back(order);
    }
    else {
        asks_[order.price].remove(order);
        order.price = newPrice;
        order.quantity = newQuantity;
        asks_[order.price].push_back(order);    
    }

    // replace order with the updated one in the hashMap
    orderMap_[orderId] = order;
}

void OrderBook::printOrders() {
    cout << "---------Bids Map---------" << endl;
    for (auto& [p, orderList] : bids_) {
        cout << "Price: " << p << endl;
        for (auto it = orderList.begin(); it != orderList.end(); it++) {
            cout << "   " << it->instrumentId << " " << it->orderId << " " << it->participantId <<
                      " " << (it->side == BUY ? "Buy" : "Sell") << " " << it->price << " " << it->quantity << endl;
        }
    }

    cout << "---------Asks Map---------" << endl;
    for (auto& [p, orderList] : asks_) {
        cout << "Price: " << p << endl;
        for (auto it = orderList.begin(); it != orderList.end(); it++) {
            cout << "   " << it->instrumentId << " " << it->orderId << " " << it->participantId <<
                      " " << (it->side == BUY ? "Buy" : "Sell") << " " << it->price << " " << it->quantity << endl;
        }
    }
}

bool OrderBook::orderExists(U64 orderId){
    return orderMap_.find(orderId) != orderMap_.end();
}

Order OrderBook::getOrder(U64 orderId) {
    return orderMap_[orderId];
}

int OrderBook::getDepth(int price, Side side) {
    if (side == BUY)
        return bids_[price].size();
    else
        return asks_[price].size();
}