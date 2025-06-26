#include "orderbook/order_book.h"
#include <stdexcept>
#include <algorithm>
#include <chrono>

namespace orderbook {

OrderBook::OrderBook(const std::string& symbol) 
    : symbol_(symbol) {}

std::vector<Trade> OrderBook::addOrder(const Order& order) {
    if (order.getSymbol() != symbol_) {
        throw std::invalid_argument("Order symbol does not match order book symbol");
    }

    std::vector<Trade> trades;
    
    // First check if we can match the incoming order
    if (order.getType() == OrderType::LIMIT || order.getType() == OrderType::MARKET) {
        trades = matchOrder(order);
    }
    
    // If the order wasn't fully filled and it's a limit order, add it to the book
    if (order.getRemainingQuantity() > 0 && order.getType() == OrderType::LIMIT) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        auto order_id = order.getId();
        auto side = order.getSide();
        auto price = order.getPrice();
        
        if (side == Side::BUY) {
            auto& price_level = bids_[price];
            if (price_level.orders.empty()) {
                price_level.price = price;
            }
            price_level.orders.push_back(order);
            price_level.total_quantity += order.getRemainingQuantity();
        } else {
            auto& price_level = asks_[price];
            if (price_level.orders.empty()) {
                price_level.price = price;
            }
            price_level.orders.push_back(order);
            price_level.total_quantity += order.getRemainingQuantity();
        }
        
        // Add order to lookup map
        order_lookup_[order_id] = {side, price};
        
        lock.unlock();
        
        // Notify listeners about the book update
        notifyOrderBookUpdateCallback();
    }
    
    return trades;
}

bool OrderBook::cancelOrder(Order::OrderId order_id) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto it = order_lookup_.find(order_id);
    if (it == order_lookup_.end()) {
        return false;
    }
    
    auto [side, price] = it->second;
    
    if (side == Side::BUY) {
        auto bid_it = bids_.find(price);
        if (bid_it != bids_.end()) {
            auto& orders = bid_it->second.orders;
            auto order_it = std::find_if(orders.begin(), orders.end(),
                [order_id](const Order& o) { return o.getId() == order_id; });
            
            if (order_it != orders.end()) {
                bid_it->second.total_quantity -= order_it->getRemainingQuantity();
                orders.erase(order_it);
                
                if (orders.empty()) {
                    bids_.erase(bid_it);
                }
                
                order_lookup_.erase(it);
                
                lock.unlock();
                notifyOrderBookUpdateCallback();
                return true;
            }
        }
    } else {
        auto ask_it = asks_.find(price);
        if (ask_it != asks_.end()) {
            auto& orders = ask_it->second.orders;
            auto order_it = std::find_if(orders.begin(), orders.end(),
                [order_id](const Order& o) { return o.getId() == order_id; });
            
            if (order_it != orders.end()) {
                ask_it->second.total_quantity -= order_it->getRemainingQuantity();
                orders.erase(order_it);
                
                if (orders.empty()) {
                    asks_.erase(ask_it);
                }
                
                order_lookup_.erase(it);
                
                lock.unlock();
                notifyOrderBookUpdateCallback();
                return true;
            }
        }
    }
    
    return false;
}

bool OrderBook::modifyOrder(Order::OrderId order_id, Order::Price new_price, Order::Quantity new_quantity) {
    // For simplicity, implement as cancel + add
    // In a real system, we would optimize this to avoid unnecessary processing
    
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto it = order_lookup_.find(order_id);
    if (it == order_lookup_.end()) {
        return false;
    }
    
    auto [side, price] = it->second;
    Order modified_order;
    
    if (side == Side::BUY) {
        auto bid_it = bids_.find(price);
        if (bid_it != bids_.end()) {
            auto& orders = bid_it->second.orders;
            auto order_it = std::find_if(orders.begin(), orders.end(),
                [order_id](const Order& o) { return o.getId() == order_id; });
            
            if (order_it != orders.end()) {
                modified_order = *order_it;
                bid_it->second.total_quantity -= order_it->getRemainingQuantity();
                orders.erase(order_it);
                
                if (orders.empty()) {
                    bids_.erase(bid_it);
                }
                
                order_lookup_.erase(it);
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        auto ask_it = asks_.find(price);
        if (ask_it != asks_.end()) {
            auto& orders = ask_it->second.orders;
            auto order_it = std::find_if(orders.begin(), orders.end(),
                [order_id](const Order& o) { return o.getId() == order_id; });
            
            if (order_it != orders.end()) {
                modified_order = *order_it;
                ask_it->second.total_quantity -= order_it->getRemainingQuantity();
                orders.erase(order_it);
                
                if (orders.empty()) {
                    asks_.erase(ask_it);
                }
                
                order_lookup_.erase(it);
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
    
    lock.unlock();
    
    // Create a new order with the modified parameters
    modified_order.setPrice(new_price);
    modified_order.setQuantity(new_quantity);
    
    // Add the modified order back to the book
    addOrder(modified_order);
    
    return true;
}

TopOfBook OrderBook::getTopOfBook() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    TopOfBook result;
    result.timestamp = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch());
    
    if (!bids_.empty()) {
        const auto& best_bid = bids_.begin()->second;
        result.bid_price = best_bid.price;
        result.bid_size = best_bid.total_quantity;
    }
    
    if (!asks_.empty()) {
        const auto& best_ask = asks_.begin()->second;
        result.ask_price = best_ask.price;
        result.ask_size = best_ask.total_quantity;
    }
    
    return result;
}

std::pair<std::vector<PriceLevel>, std::vector<PriceLevel>> OrderBook::getDepth(size_t levels) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<PriceLevel> bid_levels;
    std::vector<PriceLevel> ask_levels;
    
    bid_levels.reserve(std::min(levels, bids_.size()));
    ask_levels.reserve(std::min(levels, asks_.size()));
    
    size_t count = 0;
    for (const auto& [price, level] : bids_) {
        if (count >= levels) break;
        bid_levels.push_back(level);
        ++count;
    }
    
    count = 0;
    for (const auto& [price, level] : asks_) {
        if (count >= levels) break;
        ask_levels.push_back(level);
        ++count;
    }
    
    return {bid_levels, ask_levels};
}

void OrderBook::registerTradeCallback(TradeCallback callback) {
    trade_callback_ = std::move(callback);
}

void OrderBook::registerOrderBookUpdateCallback(OrderBookUpdateCallback callback) {
    update_callback_ = std::move(callback);
}

double OrderBook::calculateOrderFlowImbalance(size_t depth) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    // Calculate OFI as (sum of bid volumes - sum of ask volumes) / (sum of bid volumes + sum of ask volumes)
    Order::Quantity total_bid_volume = 0;
    Order::Quantity total_ask_volume = 0;
    
    size_t count = 0;
    for (const auto& [price, level] : bids_) {
        if (count >= depth) break;
        total_bid_volume += level.total_quantity;
        ++count;
    }
    
    count = 0;
    for (const auto& [price, level] : asks_) {
        if (count >= depth) break;
        total_ask_volume += level.total_quantity;
        ++count;
    }
    
    // Calculate imbalance, avoiding division by zero
    double total_volume = static_cast<double>(total_bid_volume + total_ask_volume);
    if (total_volume < 1e-10) {
        return 0.0;  // No volume, so no imbalance
    }
    
    return (static_cast<double>(total_bid_volume) - static_cast<double>(total_ask_volume)) / total_volume;
}

std::vector<Order> OrderBook::getAllOrders() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    std::vector<Order> all_orders;
    
    for (const auto& [price, level] : bids_) {
        all_orders.insert(all_orders.end(), level.orders.begin(), level.orders.end());
    }
    
    for (const auto& [price, level] : asks_) {
        all_orders.insert(all_orders.end(), level.orders.begin(), level.orders.end());
    }
    
    return all_orders;
}

void OrderBook::clear() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    bids_.clear();
    asks_.clear();
    order_lookup_.clear();
    
    lock.unlock();
    
    notifyOrderBookUpdateCallback();
}

std::vector<Trade> OrderBook::matchOrder(const Order& order) {
    std::vector<Trade> trades;
    
    if (order.getRemainingQuantity() == 0) {
        return trades;
    }
    
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    Order remaining_order = order;  // Working copy of the order
    
    if (order.getSide() == Side::BUY) {
        // Buy order matches against asks
        while (remaining_order.getRemainingQuantity() > 0 && !asks_.empty()) {
            auto& best_ask = asks_.begin()->second;
            
            // Check if the price is acceptable
            if (remaining_order.getType() == OrderType::MARKET || 
                remaining_order.getPrice() >= best_ask.price) {
                
                auto& ask_orders = best_ask.orders;
                auto ask_it = ask_orders.begin();
                
                while (ask_it != ask_orders.end() && remaining_order.getRemainingQuantity() > 0) {
                    auto& ask_order = *ask_it;
                    
                    // Calculate the trade quantity
                    auto trade_quantity = std::min(remaining_order.getRemainingQuantity(), 
                                                  ask_order.getRemainingQuantity());
                    
                    // Create a trade
                    Trade trade(next_trade_id_++, symbol_, ask_order.getPrice(), 
                                trade_quantity, ask_order.getId(), 
                                remaining_order.getId(), remaining_order.getTimestamp());
                    trades.push_back(trade);
                    
                    // Update remaining quantities
                    ask_order.fill(trade_quantity);
                    remaining_order.fill(trade_quantity);
                    
                    // Update the price level total quantity
                    best_ask.total_quantity -= trade_quantity;
                    
                    // If the ask order is fully filled, remove it
                    if (ask_order.getRemainingQuantity() == 0) {
                        // Remove from lookup
                        order_lookup_.erase(ask_order.getId());
                        
                        // Remove from orders list
                        ask_it = ask_orders.erase(ask_it);
                    } else {
                        ++ask_it;
                    }
                }
                
                // If the price level is empty, remove it
                if (ask_orders.empty()) {
                    asks_.erase(asks_.begin());
                }
            } else {
                // Best ask price is higher than this limit buy order's price
                break;
            }
        }
    } else {
        // Sell order matches against bids
        while (remaining_order.getRemainingQuantity() > 0 && !bids_.empty()) {
            auto& best_bid = bids_.begin()->second;
            
            // Check if the price is acceptable
            if (remaining_order.getType() == OrderType::MARKET || 
                remaining_order.getPrice() <= best_bid.price) {
                
                auto& bid_orders = best_bid.orders;
                auto bid_it = bid_orders.begin();
                
                while (bid_it != bid_orders.end() && remaining_order.getRemainingQuantity() > 0) {
                    auto& bid_order = *bid_it;
                    
                    // Calculate the trade quantity
                    auto trade_quantity = std::min(remaining_order.getRemainingQuantity(), 
                                                  bid_order.getRemainingQuantity());
                    
                    // Create a trade
                    Trade trade(next_trade_id_++, symbol_, bid_order.getPrice(), 
                                trade_quantity, bid_order.getId(), 
                                remaining_order.getId(), remaining_order.getTimestamp());
                    trades.push_back(trade);
                    
                    // Update remaining quantities
                    bid_order.fill(trade_quantity);
                    remaining_order.fill(trade_quantity);
                    
                    // Update the price level total quantity
                    best_bid.total_quantity -= trade_quantity;
                    
                    // If the bid order is fully filled, remove it
                    if (bid_order.getRemainingQuantity() == 0) {
                        // Remove from lookup
                        order_lookup_.erase(bid_order.getId());
                        
                        // Remove from orders list
                        bid_it = bid_orders.erase(bid_it);
                    } else {
                        ++bid_it;
                    }
                }
                
                // If the price level is empty, remove it
                if (bid_orders.empty()) {
                    bids_.erase(bids_.begin());
                }
            } else {
                // Best bid price is lower than this limit sell order's price
                break;
            }
        }
    }
    
    lock.unlock();
    
    // Notify listeners about the trades
    for (const auto& trade : trades) {
        notifyTradeCallback(trade);
    }
    
    if (!trades.empty()) {
        notifyOrderBookUpdateCallback();
    }
    
    return trades;
}

void OrderBook::notifyTradeCallback(const Trade& trade) {
    if (trade_callback_) {
        trade_callback_(trade);
    }
}

void OrderBook::notifyOrderBookUpdateCallback() {
    if (update_callback_) {
        update_callback_(getTopOfBook());
    }
}

} // namespace orderbook 
