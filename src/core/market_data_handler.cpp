#include "orderbook/market_data_handler.h"
#include <iostream>

namespace orderbook {

// Define concrete message types
class OrderAddMessage : public MarketDataMessage {
public:
    OrderAddMessage(const std::string& symbol, Order::OrderId id, Order::Price price, 
                   Order::Quantity quantity, Side side, OrderType type)
        : MarketDataMessage(Type::ORDER_ADD),
          symbol_(symbol),
          id_(id),
          price_(price),
          quantity_(quantity),
          side_(side),
          type_(type) {}

    const std::string& getSymbol() const { return symbol_; }
    Order::OrderId getId() const { return id_; }
    Order::Price getPrice() const { return price_; }
    Order::Quantity getQuantity() const { return quantity_; }
    Side getSide() const { return side_; }
    OrderType getType() const { return type_; }

private:
    std::string symbol_;
    Order::OrderId id_;
    Order::Price price_;
    Order::Quantity quantity_;
    Side side_;
    OrderType type_;
};

class OrderModifyMessage : public MarketDataMessage {
public:
    OrderModifyMessage(const std::string& symbol, Order::OrderId id, 
                      Order::Price new_price, Order::Quantity new_quantity)
        : MarketDataMessage(Type::ORDER_MODIFY),
          symbol_(symbol),
          id_(id),
          new_price_(new_price),
          new_quantity_(new_quantity) {}

    const std::string& getSymbol() const { return symbol_; }
    Order::OrderId getId() const { return id_; }
    Order::Price getNewPrice() const { return new_price_; }
    Order::Quantity getNewQuantity() const { return new_quantity_; }

private:
    std::string symbol_;
    Order::OrderId id_;
    Order::Price new_price_;
    Order::Quantity new_quantity_;
};

class OrderCancelMessage : public MarketDataMessage {
public:
    OrderCancelMessage(const std::string& symbol, Order::OrderId id)
        : MarketDataMessage(Type::ORDER_CANCEL),
          symbol_(symbol),
          id_(id) {}

    const std::string& getSymbol() const { return symbol_; }
    Order::OrderId getId() const { return id_; }

private:
    std::string symbol_;
    Order::OrderId id_;
};

class TradeMessage : public MarketDataMessage {
public:
    TradeMessage(const std::string& symbol, Trade::TradeId id, Trade::Price price,
                Trade::Quantity quantity, Trade::OrderId buy_order_id, 
                Trade::OrderId sell_order_id)
        : MarketDataMessage(Type::TRADE),
          symbol_(symbol),
          id_(id),
          price_(price),
          quantity_(quantity),
          buy_order_id_(buy_order_id),
          sell_order_id_(sell_order_id) {}

    const std::string& getSymbol() const { return symbol_; }
    Trade::TradeId getId() const { return id_; }
    Trade::Price getPrice() const { return price_; }
    Trade::Quantity getQuantity() const { return quantity_; }
    Trade::OrderId getBuyOrderId() const { return buy_order_id_; }
    Trade::OrderId getSellOrderId() const { return sell_order_id_; }

private:
    std::string symbol_;
    Trade::TradeId id_;
    Trade::Price price_;
    Trade::Quantity quantity_;
    Trade::OrderId buy_order_id_;
    Trade::OrderId sell_order_id_;
};

MarketDataHandlerImpl::MarketDataHandlerImpl() {}

void MarketDataHandlerImpl::handleMessage(const MarketDataMessage& message) {
    try {
        switch (message.getType()) {
            case MarketDataMessage::Type::ORDER_ADD: {
                const auto& order_add = dynamic_cast<const OrderAddMessage&>(message);
                auto book = getOrderBook(order_add.getSymbol());
                if (book) {
                    Order order(order_add.getId(), order_add.getSymbol(), 
                               order_add.getPrice(), order_add.getQuantity(),
                               order_add.getSide(), order_add.getType(), 
                               std::chrono::high_resolution_clock::now().time_since_epoch());
                    book->addOrder(order);
                }
                break;
            }
            case MarketDataMessage::Type::ORDER_MODIFY: {
                const auto& order_modify = dynamic_cast<const OrderModifyMessage&>(message);
                auto book = getOrderBook(order_modify.getSymbol());
                if (book) {
                    book->modifyOrder(order_modify.getId(), 
                                     order_modify.getNewPrice(), 
                                     order_modify.getNewQuantity());
                }
                break;
            }
            case MarketDataMessage::Type::ORDER_CANCEL: {
                const auto& order_cancel = dynamic_cast<const OrderCancelMessage&>(message);
                auto book = getOrderBook(order_cancel.getSymbol());
                if (book) {
                    book->cancelOrder(order_cancel.getId());
                }
                break;
            }
            case MarketDataMessage::Type::TRADE: {
                // Trades are usually handled directly by the matching engine
                // But for external trades, we might need to update our state
                const auto& trade_msg = dynamic_cast<const TradeMessage&>(message);
                // Process the trade as needed
                break;
            }
            default:
                std::cerr << "Unknown message type" << std::endl;
                break;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error handling market data message: " << e.what() << std::endl;
    }
}

void MarketDataHandlerImpl::registerOrderBook(const std::string& symbol, std::shared_ptr<OrderBook> book) {
    std::lock_guard<std::mutex> lock(mutex_);
    order_books_[symbol] = book;
}

void MarketDataHandlerImpl::unregisterOrderBook(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(mutex_);
    order_books_.erase(symbol);
}

std::shared_ptr<OrderBook> MarketDataHandlerImpl::getOrderBook(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = order_books_.find(symbol);
    if (it != order_books_.end()) {
        return it->second;
    }
    return nullptr;
}

std::shared_ptr<MarketDataHandlerImpl> MarketDataHandlerFactory::createHandler(std::shared_ptr<MarketDataFeed> feed) {
    auto handler = std::make_shared<MarketDataHandlerImpl>();
    feed->registerHandler(handler.get());
    return handler;
}

} // namespace orderbook 
