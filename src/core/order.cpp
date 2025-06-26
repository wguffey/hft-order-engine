#include "orderbook/order.h"
#include <stdexcept>

namespace orderbook {

Order::Order(OrderId id, const std::string& symbol, Price price, Quantity quantity,
             Side side, OrderType type, Timestamp timestamp)
    : id_(id), 
      symbol_(symbol), 
      price_(price), 
      quantity_(quantity), 
      remaining_quantity_(quantity), 
      side_(side), 
      type_(type), 
      status_(OrderStatus::NEW), 
      timestamp_(timestamp) {}

void Order::fill(Quantity fill_quantity) {
    if (fill_quantity > remaining_quantity_) {
        throw std::invalid_argument("Fill quantity exceeds remaining quantity");
    }

    remaining_quantity_ -= fill_quantity;
    
    if (remaining_quantity_ == 0) {
        status_ = OrderStatus::FILLED;
    } else {
        status_ = OrderStatus::PARTIALLY_FILLED;
    }
}

void Order::cancel() {
    if (status_ != OrderStatus::FILLED) {
        status_ = OrderStatus::CANCELED;
        remaining_quantity_ = 0;
    }
}

bool Order::operator<(const Order& other) const {
    // For buy orders (bids), higher prices have higher priority
    // For sell orders (asks), lower prices have higher priority
    if (side_ == Side::BUY) {
        if (price_ != other.price_) {
            return price_ < other.price_;
        }
    } else {
        if (price_ != other.price_) {
            return price_ > other.price_;
        }
    }
    
    // If prices are equal, earlier timestamp has higher priority (FIFO)
    return timestamp_ > other.timestamp_;
}

bool Order::operator>(const Order& other) const {
    return other < *this;
}

} // namespace orderbook 
