#include "orderbook/market_data_feed.h"
#include <stdexcept>
#include <iostream>
#include <chrono>

namespace orderbook {

// Factory method implementation
std::unique_ptr<MarketDataFeed> MarketDataFeed::create(const std::string& type, const std::string& config) {
    if (type == "websocket") {
        return std::make_unique<WebSocketMarketDataFeed>(config);
    }
    // Add other feed types as needed
    
    throw std::invalid_argument("Unknown market data feed type: " + type);
}

// Base market data feed implementation
BaseMarketDataFeed::BaseMarketDataFeed() 
    : running_(false) {}

BaseMarketDataFeed::~BaseMarketDataFeed() {
    stop();
}

void BaseMarketDataFeed::start() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (running_) {
        return;  // Already running
    }
    
    running_ = true;
    processing_thread_ = std::thread(&BaseMarketDataFeed::processMessages, this);
}

void BaseMarketDataFeed::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!running_) {
            return;  // Already stopped
        }
        
        running_ = false;
    }
    
    // Notify the processing thread to check the running flag
    condition_.notify_all();
    
    // Wait for the processing thread to finish
    if (processing_thread_.joinable()) {
        processing_thread_.join();
    }
}

void BaseMarketDataFeed::subscribe(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(mutex_);
    subscribed_symbols_.insert(symbol);
}

void BaseMarketDataFeed::unsubscribe(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(mutex_);
    subscribed_symbols_.erase(symbol);
}

void BaseMarketDataFeed::registerHandler(MarketDataHandler* handler) {
    std::lock_guard<std::mutex> lock(mutex_);
    handlers_.push_back(handler);
}

void BaseMarketDataFeed::dispatchMessage(const MarketDataMessage& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto handler : handlers_) {
        handler->handleMessage(message);
    }
}

// WebSocket market data feed implementation
WebSocketMarketDataFeed::WebSocketMarketDataFeed(const std::string& url)
    : BaseMarketDataFeed(), url_(url) {}

WebSocketMarketDataFeed::~WebSocketMarketDataFeed() {
    stop();
}

void WebSocketMarketDataFeed::processMessages() {
    // TODO: Implement WebSocket connection and message processing
    // For now, just simulate receiving messages periodically
    
    while (running_) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            
            // Wait for a new message or stop signal
            condition_.wait_for(lock, std::chrono::milliseconds(100), 
                [this] { return !running_ || !message_queue_.empty(); });
            
            if (!running_) {
                break;
            }
            
            // Process all messages in the queue
            while (!message_queue_.empty()) {
                auto message = std::move(message_queue_.front());
                message_queue_.pop();
                
                // Dispatch message to handlers
                dispatchMessage(*message);
            }
        }
        
        // Simulate receiving messages
        // In a real implementation, this would come from a WebSocket callback
        if (running_) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // TODO: Replace with actual WebSocket message handling
            std::cout << "WebSocket feed: simulated message processing" << std::endl;
        }
    }
}

} // namespace orderbook 
