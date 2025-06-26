#pragma once

#include "order.h"
#include <cstdint>
#include <string>
#include <chrono>

namespace orderbook {

/**
 * @brief Represents a trade execution in the market
 * 
 * A Trade occurs when a buy order matches with a sell order
 */
class Trade {
public:
    using TradeId = uint64_t;
    using Price = Order::Price;
    using Quantity = Order::Quantity;
    using Timestamp = std::chrono::nanoseconds;
    using OrderId = Order::OrderId;

    // Default constructor
    Trade() = default;

    // Constructor for a new trade
    Trade(TradeId id, const std::string& symbol, Price price, Quantity quantity,
          OrderId maker_order_id, OrderId taker_order_id, Timestamp timestamp);
    
    // Getters
    TradeId getId() const { return id_; }
    const std::string& getSymbol() const { return symbol_; }
    Price getPrice() const { return price_; }
    Quantity getQuantity() const { return quantity_; }
    OrderId getMakerOrderId() const { return maker_order_id_; }
    OrderId getTakerOrderId() const { return taker_order_id_; }
    Timestamp getTimestamp() const { return timestamp_; }

    // Compute trade value (price * quantity)
    int64_t getValue() const { return price_ * quantity_; }

private:
    TradeId id_ = 0;
    std::string symbol_;
    Price price_ = 0;
    Quantity quantity_ = 0;
    OrderId maker_order_id_ = 0;  // The passive/resting order that was hit
    OrderId taker_order_id_ = 0;  // The aggressive/incoming order that took liquidity
    Timestamp timestamp_{};
};

} // namespace orderbook 
