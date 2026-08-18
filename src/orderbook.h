#pragma once
#include "types.h"
#include "order.h"
#include "node.h"
#include <list>
#include <map>
#include <vector>
#include <string>
using namespace std;

class OrderBook {
private:
    U64 orderId_;
    U64 participantId_;
    int orderCount;
    // Bids: sorted descending by price  
    vector<pair<Node*, Node*>> bids_;
    vector<U64> bidOccupancy_;
    int bestBidIndex_;
    
    // Asks: sorted ascending by price  
    vector<pair<Node*, Node*>> asks_;
    vector<U64> askOccupancy_;
    int bestAskIndex_;

    // Fast lookup: order_id → iterator into the list + side + price
    vector<Node*> nodeMap_;
    vector<Order*> orderMap_;
    vector<U8> isValid_;
public:
    OrderBook();
    pair<bool, U64> addOrder(Side side, int price, U64 quantity, U64 timestamp);
    void cancelOrder(U64 orderId);
    pair<bool, U64> updateOrder(U64 orderId, int newPrice, int newQuantity);
    void match(int& price, Side& side, U64& quantity);
    bool orderExists(U64 orderId);
    Order* getOrder(U64 orderId);
    int getDepth(int price, Side side);
    void recomputeBestBid();
    void recomputeBestAsk();
    int getBestBid();
    int getBestAsk();
    int getOrderCount();
    void printOrders();
};