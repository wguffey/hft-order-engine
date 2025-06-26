#pragma once

#include <cstdint>
#include <string>
#include <chrono>

namespace orderbook {

// Side of the order
enum class Side : uint8_t {
    BUY = 0,
    SELL = 1
};

// Type of order
enum class OrderType : uint8_t {
    LIMIT = 0,
    MARKET = 1,
    STOP = 2,
    STOP_LIMIT = 3,
    IOC = 4,  // Immediate or Cancel
    FOK = 5   // Fill or Kill
};

// Order status
enum class OrderStatus : uint8_t {
    NEW = 0,
    PARTIALLY_FILLED = 1,
    FILLED = 2,
    CANCELED = 3,
    REJECTED = 4,
    EXPIRED = 5
};

/**
 * @brief Represents an order in a limit order book
 * 
 * This class is designed to be as memory-efficient as possible
 * while maintaining all necessary information for order book reconstruction.
 */
class Order {
public:
    using OrderId = uint64_t;
    using Price = int64_t;  // Price is stored as an integer multiplied by a scale factor
    using Quantity = uint64_t;
    using Timestamp = std::chrono::nanoseconds;

    // Default constructor
    Order() = default;

    // Constructor for creating a new order
    Order(OrderId id, const std::string& symbol, Price price, Quantity quantity,
          Side side, OrderType type, Timestamp timestamp);

    // Getters
    OrderId getId() const { return id_; }
    const std::string& getSymbol() const { return symbol_; }
    Price getPrice() const { return price_; }
    Quantity getQuantity() const { return quantity_; }
    Quantity getRemainingQuantity() const { return remaining_quantity_; }
    Side getSide() const { return side_; }
    OrderType getType() const { return type_; }
    OrderStatus getStatus() const { return status_; }
    Timestamp getTimestamp() const { return timestamp_; }

    // Setters
    void setPrice(Price price) { price_ = price; }
    void setQuantity(Quantity quantity) { 
        quantity_ = quantity;
        remaining_quantity_ = quantity;
    }
    void setRemainingQuantity(Quantity quantity) { remaining_quantity_ = quantity; }
    void setStatus(OrderStatus status) { status_ = status; }

    // Operations
    void fill(Quantity fill_quantity);
    void cancel();

    // Comparison operators for priority queue
    bool operator<(const Order& other) const;
    bool operator>(const Order& other) const;
    bool operator==(const Order& other) const { return id_ == other.id_; }

private:
    OrderId id_ = 0;
    std::string symbol_;
    Price price_ = 0;
    Quantity quantity_ = 0;
    Quantity remaining_quantity_ = 0;
    Side side_ = Side::BUY;
    OrderType type_ = OrderType::LIMIT;
    OrderStatus status_ = OrderStatus::NEW;
    Timestamp timestamp_{};
};

} // namespace orderbook 
