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
    int orderCount;
    // Bids: sorted descending by price  
    map<int, list<Order*>, std::greater<double>> bids_;
    
    // Asks: sorted ascending by price  
    map<int, list<Order*>, std::less<double>> asks_;

    // Fast lookup: order_id → iterator into the list + side + price
    unordered_map<U64, Order*> orderMap_;
public:
    OrderBook();
    pair<bool, U64> addOrder(Side side, int price, U64 quantity, U64 timestamp);
    void cancelOrder(U64 orderId);
    pair<bool, U64> updateOrder(U64 orderId, int newPrice, int newQuantity);
    void match(int& price, Side& side, vector<U64>& toRemove, U64& quantity);
    bool orderExists(U64 orderId);
    Order* getOrder(U64 orderId);
    int getDepth(int price, Side side);
    int getBestBid();
    int getBestAsk();
    int getOrderCount();
    void printOrders();
};