#include "orderbook/order_book.h"
#include <cassert>
#include <iostream>
#include <chrono>
#include <string>

using namespace orderbook;
using namespace std::chrono;

// Simple test framework
#define TEST(name) void test_##name()
#define RUN_TEST(name) do { \
    std::cout << "Running test: " << #name << "... "; \
    test_##name(); \
    std::cout << "PASSED" << std::endl; \
} while(0)

// Tests
TEST(order_creation) {
    Order order(1, "AAPL", 150'00, 100, Side::BUY, OrderType::LIMIT, 
                nanoseconds(1000000));
    
    assert(order.getId() == 1);
    assert(order.getSymbol() == "AAPL");
    assert(order.getPrice() == 150'00);
    assert(order.getQuantity() == 100);
    assert(order.getRemainingQuantity() == 100);
    assert(order.getSide() == Side::BUY);
    assert(order.getType() == OrderType::LIMIT);
    assert(order.getStatus() == OrderStatus::NEW);
    assert(order.getTimestamp() == nanoseconds(1000000));
}

TEST(order_fill) {
    Order order(1, "AAPL", 150'00, 100, Side::BUY, OrderType::LIMIT, 
                nanoseconds(1000000));
    
    order.fill(50);
    assert(order.getRemainingQuantity() == 50);
    assert(order.getStatus() == OrderStatus::PARTIALLY_FILLED);
    
    order.fill(50);
    assert(order.getRemainingQuantity() == 0);
    assert(order.getStatus() == OrderStatus::FILLED);
    
    // Should throw when trying to fill more than remaining
    bool exception_thrown = false;
    try {
        order.fill(1);
    } catch (const std::invalid_argument&) {
        exception_thrown = true;
    }
    assert(exception_thrown);
}

TEST(order_cancel) {
    Order order(1, "AAPL", 150'00, 100, Side::BUY, OrderType::LIMIT, 
                nanoseconds(1000000));
    
    order.cancel();
    assert(order.getStatus() == OrderStatus::CANCELED);
    assert(order.getRemainingQuantity() == 0);
    
    // Test canceling a filled order (should have no effect)
    Order filled_order(2, "AAPL", 150'00, 100, Side::BUY, OrderType::LIMIT, 
                      nanoseconds(1000000));
    filled_order.fill(100);
    assert(filled_order.getStatus() == OrderStatus::FILLED);
    
    filled_order.cancel();
    assert(filled_order.getStatus() == OrderStatus::FILLED);
}

TEST(order_book_basic) {
    OrderBook book("AAPL");
    assert(book.getSymbol() == "AAPL");
    
    // Add an order
    Order order(1, "AAPL", 150'00, 100, Side::BUY, OrderType::LIMIT, 
                nanoseconds(1000000));
    auto trades = book.addOrder(order);
    assert(trades.empty());
    
    // Get top of book
    auto tob = book.getTopOfBook();
    assert(tob.bid_price == 150'00);
    assert(tob.bid_size == 100);
    assert(tob.ask_price == 0);
    assert(tob.ask_size == 0);
    
    // Cancel the order
    bool success = book.cancelOrder(1);
    assert(success);
    
    // Check top of book again
    tob = book.getTopOfBook();
    assert(tob.bid_price == 0);
    assert(tob.bid_size == 0);
}

TEST(order_book_matching) {
    OrderBook book("AAPL");
    
    // Add a sell order
    Order sell_order(1, "AAPL", 150'00, 100, Side::SELL, OrderType::LIMIT, 
                    nanoseconds(1000000));
    auto trades = book.addOrder(sell_order);
    assert(trades.empty());
    
    // Add a matching buy order
    Order buy_order(2, "AAPL", 150'00, 50, Side::BUY, OrderType::LIMIT, 
                   nanoseconds(2000000));
    trades = book.addOrder(buy_order);
    
    // Should generate one trade
    assert(trades.size() == 1);
    assert(trades[0].getPrice() == 150'00);
    assert(trades[0].getQuantity() == 50);
    
    // Check top of book
    auto tob = book.getTopOfBook();
    assert(tob.bid_price == 0);
    assert(tob.bid_size == 0);
    assert(tob.ask_price == 150'00);
    assert(tob.ask_size == 50);
}

TEST(order_flow_imbalance) {
    OrderBook book("AAPL");
    
    // Add some buy orders
    book.addOrder(Order(1, "AAPL", 150'00, 100, Side::BUY, OrderType::LIMIT, nanoseconds(1000000)));
    book.addOrder(Order(2, "AAPL", 149'00, 200, Side::BUY, OrderType::LIMIT, nanoseconds(2000000)));
    
    // Add some sell orders
    book.addOrder(Order(3, "AAPL", 151'00, 150, Side::SELL, OrderType::LIMIT, nanoseconds(3000000)));
    
    // Calculate OFI with depth 2
    double ofi = book.calculateOrderFlowImbalance(2);
    
    // Bids: 100 + 200 = 300, Asks: 150
    // OFI = (300 - 150) / (300 + 150) = 0.33...
    assert(std::abs(ofi - 0.333333) < 0.001);
}

int main() {
    std::cout << "Running OrderBook Tests" << std::endl;
    std::cout << "=======================" << std::endl;
    
    RUN_TEST(order_creation);
    RUN_TEST(order_fill);
    RUN_TEST(order_cancel);
    RUN_TEST(order_book_basic);
    RUN_TEST(order_book_matching);
    RUN_TEST(order_flow_imbalance);
    
    std::cout << "\nAll tests passed!" << std::endl;
    return 0;
} 
