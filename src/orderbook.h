#pragma once
#include "types.h"
#include "order.h"
#include <list>
#include <map>
#include <unordered_map>
#include <string>
using namespace std;

class OrderBook {
private:
    U64 orderId_;
    U64 participantId_;
    U64 instrumentId_;
    // Bids: sorted descending by price  
    map<int, list<Order>, std::greater<double>> bids_;
    
    // Asks: sorted ascending by price  
    map<int, list<Order>, std::less<double>> asks_;

    // Fast lookup: order_id → iterator into the list + side + price
    unordered_map<U64, Order> orderMap_;

    // Fast lookup: order_id → iterator into the list + side + price
    unordered_map<string, U64> instrumentToId_;
public:
    OrderBook();
    pair<bool, U64> addOrder(string instrumentName, Side side, int price, U64 quantity, U64 timestamp);
    void cancelOrder(U64 orderId);
    void updateOrder(U64 orderId, int newPrice, int newQuantity);
    void match(U64& instrumentId, int& price, Side& side, vector<U64>& toRemove, U64& quantity);
    bool orderExists(U64 orderId);
    Order getOrder(U64 orderId);
    int getDepth(int price, Side side);
    void printOrders();
};