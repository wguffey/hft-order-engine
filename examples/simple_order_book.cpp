#include "orderbook/order_book.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <iomanip>

using namespace orderbook;
using namespace std::chrono;

void printTopOfBook(const TopOfBook& tob) {
    std::cout << "Top of Book: "
              << "Bid: " << tob.bid_price << " x " << tob.bid_size << " | "
              << "Ask: " << tob.ask_price << " x " << tob.ask_size 
              << std::endl;
}

void printTrade(const Trade& trade) {
    std::cout << "Trade: " 
              << trade.getSymbol() << " @ " 
              << trade.getPrice() << " x " 
              << trade.getQuantity() 
              << " (ID: " << trade.getId() << ")"
              << std::endl;
}

int main() {
    // Create an order book for a specific symbol
    OrderBook book("AAPL");
    
    // Register callbacks
    book.registerOrderBookUpdateCallback(printTopOfBook);
    book.registerTradeCallback(printTrade);
    
    std::cout << "Order Book Example for " << book.getSymbol() << std::endl;
    std::cout << "-----------------------------------" << std::endl;
    
    // Create a timestamp generator
    auto now = high_resolution_clock::now().time_since_epoch();
    auto next_time = [&now]() {
        now += milliseconds(100);
        return now;
    };
    
    // Add some initial orders
    std::cout << "\nAdding initial orders...\n" << std::endl;
    
    // Add some buy orders
    book.addOrder(Order(1, "AAPL", 150'00, 100, Side::BUY, OrderType::LIMIT, next_time()));
    book.addOrder(Order(2, "AAPL", 149'50, 200, Side::BUY, OrderType::LIMIT, next_time()));
    book.addOrder(Order(3, "AAPL", 149'00, 300, Side::BUY, OrderType::LIMIT, next_time()));
    
    // Add some sell orders
    book.addOrder(Order(4, "AAPL", 150'50, 150, Side::SELL, OrderType::LIMIT, next_time()));
    book.addOrder(Order(5, "AAPL", 151'00, 250, Side::SELL, OrderType::LIMIT, next_time()));
    book.addOrder(Order(6, "AAPL", 151'50, 350, Side::SELL, OrderType::LIMIT, next_time()));
    
    // Print the order book depth
    std::cout << "\nOrder Book Depth (3 levels):" << std::endl;
    auto [bids, asks] = book.getDepth(3);
    
    std::cout << "Bids:" << std::endl;
    for (const auto& level : bids) {
        std::cout << "  " << level.price << " x " << level.total_quantity << std::endl;
    }
    
    std::cout << "Asks:" << std::endl;
    for (const auto& level : asks) {
        std::cout << "  " << level.price << " x " << level.total_quantity << std::endl;
    }
    
    // Calculate order flow imbalance
    std::cout << "\nOrder Flow Imbalance: " << book.calculateOrderFlowImbalance(3) << std::endl;
    
    // Add a matching order that will generate trades
    std::cout << "\nAdding a matching order...\n" << std::endl;
    auto trades = book.addOrder(Order(7, "AAPL", 151'00, 300, Side::BUY, OrderType::LIMIT, next_time()));
    
    std::cout << "\nGenerated " << trades.size() << " trades" << std::endl;
    
    // Check the updated order book
    std::cout << "\nUpdated Order Book Depth (3 levels):" << std::endl;
    std::tie(bids, asks) = book.getDepth(3);
    
    std::cout << "Bids:" << std::endl;
    for (const auto& level : bids) {
        std::cout << "  " << level.price << " x " << level.total_quantity << std::endl;
    }
    
    std::cout << "Asks:" << std::endl;
    for (const auto& level : asks) {
        std::cout << "  " << level.price << " x " << level.total_quantity << std::endl;
    }
    
    // Modify an order
    std::cout << "\nModifying order ID 1...\n" << std::endl;
    book.modifyOrder(1, 150'25, 150);
    
    // Cancel an order
    std::cout << "\nCanceling order ID 3...\n" << std::endl;
    book.cancelOrder(3);
    
    // Final order book state
    std::cout << "\nFinal Order Book Depth (3 levels):" << std::endl;
    std::tie(bids, asks) = book.getDepth(3);
    
    std::cout << "Bids:" << std::endl;
    for (const auto& level : bids) {
        std::cout << "  " << level.price << " x " << level.total_quantity << std::endl;
    }
    
    std::cout << "Asks:" << std::endl;
    for (const auto& level : asks) {
        std::cout << "  " << level.price << " x " << level.total_quantity << std::endl;
    }
    
    return 0;
} 
