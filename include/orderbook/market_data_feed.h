#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <memory>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "order_book.h"

namespace orderbook {

/**
 * @brief Abstract base class for market data messages
 */
class MarketDataMessage {
public:
    enum class Type {
        ORDER_ADD,
        ORDER_MODIFY,
        ORDER_CANCEL,
        TRADE,
        HEARTBEAT,
        SNAPSHOT
    };

    explicit MarketDataMessage(Type type) : type_(type) {}
    virtual ~MarketDataMessage() = default;

    Type getType() const { return type_; }

private:
    Type type_;
};

/**
 * @brief Message handler interface
 */
class MarketDataHandler {
public:
    virtual ~MarketDataHandler() = default;
    virtual void handleMessage(const MarketDataMessage& message) = 0;
};

/**
 * @brief Abstract base class for market data feeds
 */
class MarketDataFeed {
public:
    virtual ~MarketDataFeed() = default;

    /**
     * @brief Start the market data feed
     */
    virtual void start() = 0;

    /**
     * @brief Stop the market data feed
     */
    virtual void stop() = 0;

    /**
     * @brief Subscribe to a specific symbol
     * 
     * @param symbol The symbol to subscribe to
     */
    virtual void subscribe(const std::string& symbol) = 0;

    /**
     * @brief Unsubscribe from a specific symbol
     * 
     * @param symbol The symbol to unsubscribe from
     */
    virtual void unsubscribe(const std::string& symbol) = 0;

    /**
     * @brief Register a handler for market data messages
     * 
     * @param handler The handler to register
     */
    virtual void registerHandler(MarketDataHandler* handler) = 0;

    /**
     * @brief Factory method to create market data feeds
     * 
     * @param type The type of market data feed to create (e.g., "websocket", "fix", "multicast")
     * @param config The configuration string for the feed
     * @return std::unique_ptr<MarketDataFeed> A unique pointer to the created feed
     */
    static std::unique_ptr<MarketDataFeed> create(const std::string& type, const std::string& config);
};

/**
 * @brief Base class for market data feeds with common functionality
 */
class BaseMarketDataFeed : public MarketDataFeed {
public:
    BaseMarketDataFeed();
    ~BaseMarketDataFeed() override;

    void start() override;
    void stop() override;
    void subscribe(const std::string& symbol) override;
    void unsubscribe(const std::string& symbol) override;
    void registerHandler(MarketDataHandler* handler) override;

protected:
    // Helper method to dispatch a message to all registered handlers
    void dispatchMessage(const MarketDataMessage& message);

    // Queue for incoming messages
    std::queue<std::unique_ptr<MarketDataMessage>> message_queue_;
    
    // Thread for processing messages
    std::thread processing_thread_;
    
    // Synchronization
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<bool> running_;
    
    // Subscribed symbols
    std::unordered_set<std::string> subscribed_symbols_;
    
    // Registered handlers
    std::vector<MarketDataHandler*> handlers_;
    
    // Derived classes should implement this method to process messages from the feed
    virtual void processMessages() = 0;
};

/**
 * @brief WebSocket implementation of market data feed
 */
class WebSocketMarketDataFeed : public BaseMarketDataFeed {
public:
    explicit WebSocketMarketDataFeed(const std::string& url);
    ~WebSocketMarketDataFeed() override;

protected:
    void processMessages() override;

private:
    std::string url_;
    // Additional WebSocket-specific members
};

} // namespace orderbook 
