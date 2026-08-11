#pragma once
#include "types.h"
#include "order.h"
#include <list>
#include <map>
#include <unordered_map>
using namespace std;

class OrderBook {
private:
    U64 orderId_;
    U64 participantId_;
    // Bids: sorted descending by price  
    map<int, list<Order>, std::greater<double>> bids_;
    
    // Asks: sorted ascending by price  
    map<int, list<Order>, std::less<double>> asks_;

    // Fast lookup: order_id → iterator into the list + side + price
    unordered_map<uint64_t, Order> orderMap_;
public:
    OrderBook();
    void addOrder(Side side, int price, U64 quantity, U64 timestamp);
    void cancelOrder(U64 orderId);
    void updateOrder(U64 orderId, int newPrice, int newQuantity);
};