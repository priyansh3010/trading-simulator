#include "types.h"
#include "order.h"
#include "orderbook.h"
#include <iostream>
#include <string>
using namespace std;

int main() {
    OrderBook orderBook;

    // basic test to verify functionality
    // benchmarking suite is next step
    orderBook.addOrder("AAPL", SELL, 10000, 10, 1);
    orderBook.addOrder("AAPL", BUY, 10000, 8, 1);
    orderBook.addOrder("AAPL", BUY, 11000, 1, 1);
    orderBook.addOrder("AAPL", BUY, 9000, 5, 1);
    orderBook.addOrder("MSFT", SELL, 9000, 3, 1);
    orderBook.addOrder("AAPL", SELL, 7000, 3, 1);

    orderBook.printOrders();

    return 0;
}