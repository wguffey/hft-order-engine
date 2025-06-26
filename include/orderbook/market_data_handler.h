#pragma once

#include "market_data_feed.h"
#include "order_book.h"
#include <memory>
#include <unordered_map>
#include <string>

namespace orderbook {

/**
 * @brief Implementation of MarketDataHandler that updates order books based on market data messages
 */
class MarketDataHandlerImpl : public MarketDataHandler {
public:
    MarketDataHandlerImpl();
    ~MarketDataHandlerImpl() override = default;

    /**
     * @brief Handle a market data message
     * 
     * This method processes incoming market data messages and updates the appropriate order books.
     * 
     * @param message The market data message to handle
     */
    void handleMessage(const MarketDataMessage& message) override;

    /**
     * @brief Register an order book for a specific symbol
     * 
     * @param symbol The symbol to register the order book for
     * @param book The order book to register
     */
    void registerOrderBook(const std::string& symbol, std::shared_ptr<OrderBook> book);

    /**
     * @brief Unregister an order book for a specific symbol
     * 
     * @param symbol The symbol to unregister the order book for
     */
    void unregisterOrderBook(const std::string& symbol);

    /**
     * @brief Get the order book for a specific symbol
     * 
     * @param symbol The symbol to get the order book for
     * @return std::shared_ptr<OrderBook> The order book, or nullptr if not found
     */
    std::shared_ptr<OrderBook> getOrderBook(const std::string& symbol);

private:
    std::unordered_map<std::string, std::shared_ptr<OrderBook>> order_books_;
    std::mutex mutex_;
};

/**
 * @brief A factory for creating order book handlers for different data sources
 */
class MarketDataHandlerFactory {
public:
    /**
     * @brief Create a handler for processing market data and updating order books
     * 
     * @param feed The market data feed to use
     * @return std::shared_ptr<MarketDataHandlerImpl> The created handler
     */
    static std::shared_ptr<MarketDataHandlerImpl> createHandler(std::shared_ptr<MarketDataFeed> feed);
};

} // namespace orderbook 
