#pragma once

#include "order.h"
#include "trade.h"
#include <string>
#include <unordered_map>
#include <map>
#include <list>
#include <functional>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <shared_mutex>

namespace orderbook {

/**
 * @brief Represents the top of the book (best bid and ask)
 */
struct TopOfBook {
    Order::Price bid_price = 0;
    Order::Quantity bid_size = 0;
    Order::Price ask_price = 0;
    Order::Quantity ask_size = 0;
    std::chrono::nanoseconds timestamp{};
};

/**
 * @brief Represents a price level in the order book
 */
struct PriceLevel {
    Order::Price price = 0;
    Order::Quantity total_quantity = 0;
    std::list<Order> orders;
};

/**
 * @brief Callback function type for trade notifications
 */
using TradeCallback = std::function<void(const Trade&)>;

/**
 * @brief Callback function type for order book updates
 */
using OrderBookUpdateCallback = std::function<void(const TopOfBook&)>;

/**
 * @brief High-performance limit order book implementation
 * 
 * This class maintains a limit order book for a specific symbol.
 * It provides methods for adding, modifying, and canceling orders,
 * as well as matching incoming orders against the book.
 */
class OrderBook {
public:
    /**
     * @brief Construct a new Order Book for a specific symbol
     * 
     * @param symbol The ticker symbol for this order book
     */
    explicit OrderBook(const std::string& symbol);

    /**
     * @brief Get the symbol for this order book
     */
    const std::string& getSymbol() const { return symbol_; }

    /**
     * @brief Add a new order to the book
     * 
     * If the order matches with existing orders, trades will be generated.
     * 
     * @param order The order to add
     * @return std::vector<Trade> Any trades that were generated
     */
    std::vector<Trade> addOrder(const Order& order);

    /**
     * @brief Cancel an existing order
     * 
     * @param order_id The ID of the order to cancel
     * @return bool True if the order was found and canceled, false otherwise
     */
    bool cancelOrder(Order::OrderId order_id);

    /**
     * @brief Modify an existing order
     * 
     * @param order_id The ID of the order to modify
     * @param new_price The new price for the order
     * @param new_quantity The new quantity for the order
     * @return bool True if the order was found and modified, false otherwise
     */
    bool modifyOrder(Order::OrderId order_id, Order::Price new_price, Order::Quantity new_quantity);

    /**
     * @brief Get the current top of the book
     * 
     * @return TopOfBook The best bid and ask prices and sizes
     */
    TopOfBook getTopOfBook() const;

    /**
     * @brief Get the depth of the book at a specified number of levels
     * 
     * @param levels The number of price levels to return
     * @return std::pair<std::vector<PriceLevel>, std::vector<PriceLevel>> Bid and ask levels
     */
    std::pair<std::vector<PriceLevel>, std::vector<PriceLevel>> getDepth(size_t levels) const;

    /**
     * @brief Register a callback for trade notifications
     * 
     * @param callback The callback function to be called when a trade occurs
     */
    void registerTradeCallback(TradeCallback callback);

    /**
     * @brief Register a callback for order book updates
     * 
     * @param callback The callback function to be called when the order book is updated
     */
    void registerOrderBookUpdateCallback(OrderBookUpdateCallback callback);

    /**
     * @brief Calculate order flow imbalance (OFI) at a specified depth
     * 
     * OFI measures the net aggression of market participants by comparing
     * the volume of limit orders at the bid vs the ask.
     * 
     * @param depth The number of price levels to include in the calculation
     * @return double The order flow imbalance value (-1.0 to 1.0)
     */
    double calculateOrderFlowImbalance(size_t depth) const;

    /**
     * @brief Get all orders in the book
     * 
     * @return std::vector<Order> All orders currently in the book
     */
    std::vector<Order> getAllOrders() const;

    /**
     * @brief Clear the order book
     */
    void clear();

private:
    std::string symbol_;
    
    // Bid side (buy orders), sorted in descending order of price
    std::map<Order::Price, PriceLevel, std::greater<>> bids_;
    
    // Ask side (sell orders), sorted in ascending order of price
    std::map<Order::Price, PriceLevel, std::less<>> asks_;
    
    // Fast lookup for orders by ID
    std::unordered_map<Order::OrderId, std::pair<Side, Order::Price>> order_lookup_;
    
    // Callbacks
    TradeCallback trade_callback_;
    OrderBookUpdateCallback update_callback_;
    
    // Thread safety
    mutable std::shared_mutex mutex_;
    
    // Trade ID generator
    std::atomic<Trade::TradeId> next_trade_id_{1};
    
    // Helper methods
    std::vector<Trade> matchOrder(const Order& order);
    void notifyTradeCallback(const Trade& trade);
    void notifyOrderBookUpdateCallback();
};

} // namespace orderbook 
