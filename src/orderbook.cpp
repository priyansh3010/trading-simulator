#pragma once
#include "types.h"
#include "order.h"
#include "orderbook.h"
#include <list>
#include <map>
#include <unordered_map>
using namespace std;

OrderBook::OrderBook() {
    orderId_ = 0;
    participantId_ = 0;
}

void OrderBook::addOrder(Side side, int price, U64 quantity, U64 timestamp) {
    // create new order
    Order order(orderId_, participantId_, side, price, quantity, timestamp);

    // add to appropriate map and list
    if (side == BUY) bids_[price].push_back(order);
    else asks_[price].push_back(order);
    
    // add to hashmap for quick lookups for future
    orderMap_[orderId_] = order;
    orderId_++; // increment order id for now. more sophistication later on
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