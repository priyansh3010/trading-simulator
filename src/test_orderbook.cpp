#include "types.h"
#include "order.h"
#include "orderbook.h"
#include <iostream>
#include <string>
#include <gtest/gtest.h>
using namespace std;

// some tests may be redundant / duplicate. ignore for now, will clean it up later 
// goal is to optimize order book for now, now that I have proper tests

TEST(OrderBookTest, AddSingleBid) {
    OrderBook book;
    
    auto order1 = book.addOrder("AAPL", BUY, 10000, 100, 1);
    ASSERT_TRUE(order1.first);
    ASSERT_TRUE(book.orderExists(order1.second));
    Order* order = book.getOrder(order1.second);
    EXPECT_EQ(book.getDepth(order->price, order->side), 1);
    
}

TEST(OrderBookTest, AddSingleAsk) {
    OrderBook book;

    auto order2 = book.addOrder("MSFT", SELL, 10000, 100, 2);
    ASSERT_TRUE(order2.first);
    ASSERT_TRUE(book.orderExists(order2.second));
    Order* order = book.getOrder(order2.second);
    EXPECT_EQ(book.getDepth(order->price, order->side), 1);
}

TEST(OrderBookTest, AddMultipleNonCrossingBids) {
    OrderBook book;
    
    auto order1 = book.addOrder("AAPL", BUY, 9900, 100, 1);
    auto order2 = book.addOrder("AAPL", BUY, 10000, 100, 2);
    auto order3 = book.addOrder("AAPL", BUY, 9950, 100, 3);
    
    ASSERT_TRUE(order1.first);
    ASSERT_TRUE(order2.first);
    ASSERT_TRUE(order3.first);
    
    EXPECT_EQ(book.getBestBid("AAPL"), 10000);
}

TEST(OrderBookTest, AddMultipleNonCrossingAsks) {
    OrderBook book;
    
    auto order1 = book.addOrder("AAPL", SELL, 10100, 100, 1);
    auto order2 = book.addOrder("AAPL", SELL, 10000, 100, 2);
    auto order3 = book.addOrder("AAPL", SELL, 10050, 100, 3);
    
    ASSERT_TRUE(order1.first);
    ASSERT_TRUE(order2.first);
    ASSERT_TRUE(order3.first);
    
    EXPECT_EQ(book.getBestAsk("AAPL"), 10000);
}

TEST(OrderBookTest, MultipleOrdersSamePriceLevel) {
    OrderBook book;

    auto order1 = book.addOrder("AAPL", BUY, 10000, 100, 1);
    auto order2 = book.addOrder("AAPL", BUY, 10000, 200, 2);
    auto order3 = book.addOrder("AAPL", BUY, 10000, 300, 3);

    ASSERT_TRUE(order1.first);
    ASSERT_TRUE(order2.first);
    ASSERT_TRUE(order3.first);

    EXPECT_EQ(book.getDepth(10000, BUY), 3);
}

TEST(OrderBookTest, MultipleOrdersSamePriceLevelPreservesFIFO) {
    OrderBook book;

    auto order1 = book.addOrder("AAPL", BUY, 10000, 100, 1);
    auto order2 = book.addOrder("AAPL", BUY, 10000, 200, 2);
    auto order3 = book.addOrder("AAPL", BUY, 10000, 300, 3);

    ASSERT_TRUE(order1.first);
    ASSERT_TRUE(order2.first);
    ASSERT_TRUE(order3.first);

    // A fully-crossing sell should fill order1 first, then order2, then order3.
    auto sell = book.addOrder("AAPL", SELL, 10000, 250, 4);
    ASSERT_FALSE(sell.first);

    EXPECT_FALSE(book.orderExists(order1.second)); // fully filled (100)
    ASSERT_TRUE(book.orderExists(order2.second));   // partially filled (50 left of 200)
    Order* partial = book.getOrder(order2.second);
    EXPECT_EQ(partial->quantity, 50);

    ASSERT_TRUE(book.orderExists(order3.second));   // untouched
    Order* untouched = book.getOrder(order3.second);
    EXPECT_EQ(untouched->quantity, 300);
}

TEST(OrderBookTest, AddOrderReturnsUniqueIncreasingIds) {
    OrderBook book;

    auto order1 = book.addOrder("AAPL", BUY, 10000, 100, 1);
    auto order2 = book.addOrder("AAPL", SELL, 10100, 100, 2);
    auto order3 = book.addOrder("MSFT", BUY, 20000, 100, 3);

    ASSERT_TRUE(order1.first);
    ASSERT_TRUE(order2.first);
    ASSERT_TRUE(order3.first);

    EXPECT_NE(order1.second, order2.second);
    EXPECT_NE(order2.second, order3.second);
    EXPECT_GT(order2.second, order1.second);
    EXPECT_GT(order3.second, order2.second);
}

TEST(OrderBookTest, AddOrderNewTickerCreatesBook) {
    OrderBook book;
    
    auto order1 = book.addOrder("GOOG", BUY, 15000, 100, 1);
    ASSERT_TRUE(order1.first);
    ASSERT_TRUE(book.orderExists(order1.second));
    // EXPECT_EQ(book.bestBid("GOOG"), 15000);
}

TEST(OrderBookTest, AddOrderExistingTickerResolvesSameBook) {
    OrderBook book;

    auto order1 = book.addOrder("AAPL", BUY, 10000, 100, 1);
    auto order2 = book.addOrder("AAPL", BUY, 9900, 100, 2);

    ASSERT_TRUE(order1.first);
    ASSERT_TRUE(order2.first);

    // Both should land in the same book -> bestBid reflects the higher of the two.
    EXPECT_EQ(book.getBestBid("AAPL"), 10000);
    EXPECT_EQ(book.getDepth(10000, BUY), 1);
    EXPECT_EQ(book.getDepth(9900, BUY), 1);
}

TEST(OrderBookTest, DifferentTickersDoNotCrossContaminate) {
    OrderBook book;

    auto aapl = book.addOrder("AAPL", BUY, 10000, 100, 1);
    auto msft = book.addOrder("MSFT", BUY, 20000, 100, 2);

    ASSERT_TRUE(aapl.first);
    ASSERT_TRUE(msft.first);

    EXPECT_EQ(book.getBestBid("AAPL"), 10000);
    EXPECT_EQ(book.getBestBid("MSFT"), 20000);
}

TEST(OrderBookTest, UpdateOrderDecreaseQuantity) {
    OrderBook book;
    auto order = book.addOrder("AAPL", BUY, 10000, 100, 1);
    ASSERT_TRUE(order.first);
    
    auto newOrderInfo = book.updateOrder(order.second, "AAPL", 10000, 50);
    
    ASSERT_TRUE(newOrderInfo.first);
    ASSERT_TRUE(book.orderExists(newOrderInfo.second));
    Order* newOrder = book.getOrder(newOrderInfo.second);
    EXPECT_EQ(book.getDepth(10000, BUY), 1);
    EXPECT_EQ(newOrder->quantity, 50);
}

TEST(OrderBookTest, UpdateOrderPriceChangeLosesTimePriority) {
    OrderBook book;
    auto order1 = book.addOrder("AAPL", BUY, 10000, 100, 1); // first at 10000
    auto order2 = book.addOrder("AAPL", BUY, 10000, 100, 2); // second at 10000
    auto order3 = book.addOrder("AAPL", BUY, 9900,  100, 3); // at 9900

    ASSERT_TRUE(order1.first);
    ASSERT_TRUE(order2.first);
    ASSERT_TRUE(order3.first);

    // Move order3 to 10000. It should lose priority vs order1 and order2.
    auto newOrder3 = book.updateOrder(order3.second, "AAPL", 10000, 100);

    // A sell of 250 should exhaust order1, order2, then 50 from order3.
    auto sell = book.addOrder("AAPL", SELL, 10000, 250, 4);

    EXPECT_FALSE(book.orderExists(order1.second));
    EXPECT_FALSE(book.orderExists(order2.second));
    ASSERT_TRUE(book.orderExists(newOrder3.second));
    Order* o3 = book.getOrder(newOrder3.second);
    EXPECT_EQ(o3->quantity, 50);
}

TEST(OrderBookTest, UpdateOrderPriceCausesCrossFullFill) {
    OrderBook book;
    auto bid = book.addOrder("AAPL", BUY,  10000, 100, 1);
    auto ask = book.addOrder("AAPL", SELL, 10100, 100, 2);
    ASSERT_TRUE(bid.first);
    ASSERT_TRUE(ask.first);

    auto newBid = book.updateOrder(bid.second, "AAPL", 10100, 100);

    EXPECT_FALSE(book.orderExists(newBid.second));
    EXPECT_FALSE(book.orderExists(ask.second));
    EXPECT_EQ(book.getBestBid("AAPL"), -1);
    EXPECT_EQ(book.getBestAsk("AAPL"), -1);
}

TEST(OrderBookTest, UpdateOrderPriceCausesCrossPartialFill) {
    OrderBook book;
    auto bid = book.addOrder("AAPL", BUY,  10000, 100, 1);
    auto ask = book.addOrder("AAPL", SELL, 10100,  30, 2);
    ASSERT_TRUE(bid.first);
    ASSERT_TRUE(ask.first);

    auto newBid = book.updateOrder(bid.second, "AAPL", 10100, 100);

    EXPECT_FALSE(book.orderExists(ask.second));
    ASSERT_TRUE(book.orderExists(newBid.second));
    Order* remainingBid = book.getOrder(newBid.second);
    EXPECT_EQ(remainingBid->quantity, 70);
}

TEST(OrderBookTest, UpdateOrderToZeroQuantityCancels) {
    OrderBook book;
    auto order = book.addOrder("AAPL", BUY, 10000, 100, 1);
    ASSERT_TRUE(order.first);

    auto newOrder = book.updateOrder(order.second, "AAPL", 10000, 0);

    EXPECT_FALSE(book.orderExists(newOrder.second));
    EXPECT_EQ(book.getDepth(10000, BUY), 0);
    EXPECT_EQ(book.getDepth(100, BUY), 0);
    EXPECT_EQ(book.getBestBid("AAPL"), -1);
}

TEST(OrderBookTest, UpdateOrderAfterPartialFill) {
    OrderBook book;
    auto bid = book.addOrder("AAPL", BUY,  10000, 100, 1);
    auto ask = book.addOrder("AAPL", SELL, 10000,  30, 2);
    ASSERT_TRUE(bid.first);
    ASSERT_FALSE(ask.first);

    // Bid partially filled: 70 remains
    ASSERT_TRUE(book.orderExists(bid.second));
    EXPECT_EQ(book.getOrder(bid.second)->quantity, 70);

    // Reduce remaining quantity from 70 to 50
    auto newBid = book.updateOrder(bid.second, "AAPL", 10000, 50);

    Order* o = book.getOrder(newBid.second);
    EXPECT_EQ(o->quantity, 50);
}

TEST(OrderBookTest, UpdateOrderCausesMultiLevelSweep) {
    OrderBook book;
    auto bid  = book.addOrder("AAPL", BUY,  10000, 300, 1);
    auto ask1 = book.addOrder("AAPL", SELL, 10100, 100, 2);
    auto ask2 = book.addOrder("AAPL", SELL, 10200, 100, 3);
    auto ask3 = book.addOrder("AAPL", SELL, 10300, 100, 4);

    ASSERT_TRUE(bid.first);
    ASSERT_TRUE(ask1.first);
    ASSERT_TRUE(ask2.first);
    ASSERT_TRUE(ask3.first);

    // Raise bid to sweep all three ask levels
    auto newBid = book.updateOrder(bid.second, "AAPL", 10300, 300);

    EXPECT_FALSE(book.orderExists(ask1.second));
    EXPECT_FALSE(book.orderExists(ask2.second));
    EXPECT_FALSE(book.orderExists(ask3.second));
    EXPECT_FALSE(book.orderExists(newBid.second));
}

TEST(OrderBookTest, UpdateNonExistentOrderDoesNotCorruptBook) {
    OrderBook book;
    auto order = book.addOrder("AAPL", BUY, 10000, 100, 1);
    ASSERT_TRUE(order.first);

    auto newOrder = book.updateOrder(99999, "AAPL", 10000, 100);

    // Original book state must be intact.
    EXPECT_FALSE(newOrder.first);
    EXPECT_TRUE(book.orderExists(order.second));
    EXPECT_EQ(book.getBestBid("AAPL"), 10000);
}

TEST(OrderBookTest, ExactFullFillBothSidesRemoved) {
    OrderBook book;
    auto bid = book.addOrder("AAPL", BUY,  10000, 100, 1);
    ASSERT_TRUE(bid.first);

    auto ask = book.addOrder("AAPL", SELL, 10000, 100, 2);
    ASSERT_FALSE(ask.first);

    EXPECT_FALSE(book.orderExists(bid.second));
    EXPECT_FALSE(book.orderExists(ask.second));
    EXPECT_EQ(book.getDepth(10000, BUY), 0);
    EXPECT_EQ(book.getDepth(10000, SELL), 0);
}

TEST(OrderBookTest, AggressorPartialFillIncomingOrderLeftOver) {
    OrderBook book;
    auto ask = book.addOrder("AAPL", SELL, 10000, 30, 1);
    ASSERT_TRUE(ask.first);

    auto bid = book.addOrder("AAPL", BUY, 10000, 100, 2);
    ASSERT_TRUE(bid.first); // crosses, so not resting as a full order

    EXPECT_FALSE(book.orderExists(ask.second));
    ASSERT_TRUE(book.orderExists(bid.second));
    Order* leftover = book.getOrder(bid.second);
    EXPECT_EQ(leftover->quantity, 70);
    EXPECT_EQ(book.getBestBid("AAPL"), 10000);
}

TEST(OrderBookTest, MultiLevelSweepUpdatesBestAsk) {
    OrderBook book;
    auto ask1 = book.addOrder("AAPL", SELL, 10000, 50, 1);
    auto ask2 = book.addOrder("AAPL", SELL, 10100, 50, 2);
    auto ask3 = book.addOrder("AAPL", SELL, 10200, 50, 3);
    ASSERT_TRUE(ask1.first);
    ASSERT_TRUE(ask2.first);
    ASSERT_TRUE(ask3.first);

    auto bid = book.addOrder("AAPL", BUY, 10200, 150, 4);
    ASSERT_FALSE(bid.first);

    EXPECT_FALSE(book.orderExists(ask1.second));
    EXPECT_FALSE(book.orderExists(ask2.second));
    EXPECT_FALSE(book.orderExists(ask3.second));
    EXPECT_FALSE(book.orderExists(bid.second));
    EXPECT_EQ(book.getBestAsk("AAPL"), -1); // empty-book sentinel
}

TEST(OrderBookTest, BestLevelDepletionRollsToNextBest) {
    OrderBook book;
    auto bid1 = book.addOrder("AAPL", BUY, 10000, 100, 1);
    auto bid2 = book.addOrder("AAPL", BUY,  9900, 100, 2);
    ASSERT_TRUE(bid1.first && bid2.first);

    auto ask = book.addOrder("AAPL", SELL, 10000, 100, 3);
    ASSERT_FALSE(ask.first);

    EXPECT_FALSE(book.orderExists(bid1.second));
    EXPECT_TRUE(book.orderExists(bid2.second));
    EXPECT_EQ(book.getBestBid("AAPL"), 9900);
}

TEST(OrderBookTest, IncomingOrderDoesNotCrossWhenPricesDoNotMeet) {
    OrderBook book;
    auto bid = book.addOrder("AAPL", BUY,  10000, 100, 1);
    ASSERT_TRUE(bid.first);

    auto ask = book.addOrder("AAPL", SELL, 10100, 100, 2);
    ASSERT_TRUE(ask.first); // does NOT cross

    EXPECT_TRUE(book.orderExists(bid.second));
    EXPECT_TRUE(book.orderExists(ask.second));
    EXPECT_EQ(book.getBestBid("AAPL"), 10000);
    EXPECT_EQ(book.getBestAsk("AAPL"), 10100);
}

TEST(OrderBookTest, PriceTimePriorityAcrossLevels) {
    OrderBook book;
    auto ask1 = book.addOrder("AAPL", SELL, 10000, 100, 1);
    auto ask2 = book.addOrder("AAPL", SELL, 10100, 100, 2);
    ASSERT_TRUE(ask1.first && ask2.first);

    auto bid = book.addOrder("AAPL", BUY, 10200, 100, 3);
    ASSERT_FALSE(bid.first);

    // Bid at 10200 should have matched the resting ask at 10000, not 10100.
    EXPECT_FALSE(book.orderExists(ask1.second));
    EXPECT_TRUE(book.orderExists(ask2.second));
}

TEST(OrderBookTest, TradeOccursAtRestingPriceNotAggressorPrice) {
    OrderBook book;
    auto ask = book.addOrder("AAPL", SELL, 10000, 100, 1);
    ASSERT_TRUE(ask.first);

    auto bid = book.addOrder("AAPL", BUY,  10100, 100, 2);
    ASSERT_FALSE(bid.first);
}

/*
TEST(OrderBookTest, SelfTradePrevention) {
    OrderBook book;
    auto bid = book.addOrder("AAPL", BUY,  10000, 100, 1);
    ASSERT_TRUE(bid.first);
    
    // Same user ID tries to sell to themselves.
    auto ask = book.addOrder("AAPL", SELL, 10000, 100, 1);
    
    // ASSERT_FALSE(ask.first);
    // EXPECT_TRUE(book.orderExists(bid.second));
    // EXPECT_FALSE(book.orderExists(ask.second));
}
*/

TEST(OrderBookTest, LargeQuantitySweepThroughMultipleOrdersSameLevel) {
    OrderBook book;
    auto ask1 = book.addOrder("AAPL", SELL, 10000, 50, 1);
    auto ask2 = book.addOrder("AAPL", SELL, 10000, 50, 2);
    auto ask3 = book.addOrder("AAPL", SELL, 10000, 50, 3);
    ASSERT_TRUE(ask1.first);
    ASSERT_TRUE(ask2.first);
    ASSERT_TRUE(ask3.first);

    auto bid = book.addOrder("AAPL", BUY, 10000, 125, 4);
    ASSERT_FALSE(bid.first);

    EXPECT_FALSE(book.orderExists(ask1.second));
    EXPECT_FALSE(book.orderExists(ask2.second));
    EXPECT_TRUE(book.orderExists(ask3.second));
    EXPECT_FALSE(book.orderExists(bid.second));
    EXPECT_EQ(book.getOrder(ask3.second)->quantity, 25);
}

TEST(OrderBookTest, AggressiveOrderWorsePriceThanRestingStillMatchesIfCrosses) {
    OrderBook book;
    auto ask = book.addOrder("AAPL", SELL, 10000, 100, 1);
    ASSERT_TRUE(ask.first);

    // Bid is much higher than ask, but should still match at 10000.
    auto bid = book.addOrder("AAPL", BUY, 10500, 100, 2);
    ASSERT_FALSE(bid.first);

    EXPECT_FALSE(book.orderExists(ask.second));
    EXPECT_FALSE(book.orderExists(bid.second));
}

TEST(OrderBookTest, NewBestLevelAfterPartialFillOfMultiOrderLevel) {
    OrderBook book;
    auto bid1 = book.addOrder("AAPL", BUY, 10000, 100, 1);
    auto bid2 = book.addOrder("AAPL", BUY, 10000, 100, 2);
    auto bid3 = book.addOrder("AAPL", BUY,  9900, 100, 3);
    ASSERT_TRUE(bid1.first && bid2.first && bid3.first);

    // Sell 100 -> wipes bid1 only.
    auto ask = book.addOrder("AAPL", SELL, 10000, 100, 4);
    ASSERT_FALSE(ask.first);

    EXPECT_FALSE(book.orderExists(bid1.second));
    EXPECT_TRUE(book.orderExists(bid2.second));
    EXPECT_TRUE(book.orderExists(bid3.second));
    EXPECT_EQ(book.getBestBid("AAPL"), 10000);
    EXPECT_EQ(book.getDepth(10000, BUY), 1);
}