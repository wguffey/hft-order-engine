#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include "orderbook/order.h"
#include "orderbook/trade.h"
#include "orderbook/order_book.h"
#include "orderbook/market_data_feed.h"
#include "orderbook/market_data_handler.h"

namespace py = pybind11;
using namespace orderbook;

PYBIND11_MODULE(core, m) {
    m.doc() = "OrderBook - Ultra-Low-Latency Market Data Analyzer";

    // Enums
    py::enum_<Side>(m, "Side")
        .value("BUY", Side::BUY)
        .value("SELL", Side::SELL)
        .export_values();

    py::enum_<OrderType>(m, "OrderType")
        .value("LIMIT", OrderType::LIMIT)
        .value("MARKET", OrderType::MARKET)
        .value("STOP", OrderType::STOP)
        .value("STOP_LIMIT", OrderType::STOP_LIMIT)
        .value("IOC", OrderType::IOC)
        .value("FOK", OrderType::FOK)
        .export_values();

    py::enum_<OrderStatus>(m, "OrderStatus")
        .value("NEW", OrderStatus::NEW)
        .value("PARTIALLY_FILLED", OrderStatus::PARTIALLY_FILLED)
        .value("FILLED", OrderStatus::FILLED)
        .value("CANCELED", OrderStatus::CANCELED)
        .value("REJECTED", OrderStatus::REJECTED)
        .value("EXPIRED", OrderStatus::EXPIRED)
        .export_values();

    // Order class
    py::class_<Order>(m, "Order")
        .def(py::init<>())
        .def(py::init<Order::OrderId, const std::string&, Order::Price, Order::Quantity, 
                    Side, OrderType, Order::Timestamp>())
        .def("get_id", &Order::getId)
        .def("get_symbol", &Order::getSymbol)
        .def("get_price", &Order::getPrice)
        .def("get_quantity", &Order::getQuantity)
        .def("get_remaining_quantity", &Order::getRemainingQuantity)
        .def("get_side", &Order::getSide)
        .def("get_type", &Order::getType)
        .def("get_status", &Order::getStatus)
        .def("get_timestamp", &Order::getTimestamp)
        .def("set_price", &Order::setPrice)
        .def("set_quantity", &Order::setQuantity)
        .def("set_remaining_quantity", &Order::setRemainingQuantity)
        .def("set_status", &Order::setStatus)
        .def("fill", &Order::fill)
        .def("cancel", &Order::cancel);

    // Trade class
    py::class_<Trade>(m, "Trade")
        .def(py::init<>())
        .def(py::init<Trade::TradeId, const std::string&, Trade::Price, Trade::Quantity,
                    Trade::OrderId, Trade::OrderId, Trade::Timestamp>())
        .def("get_id", &Trade::getId)
        .def("get_symbol", &Trade::getSymbol)
        .def("get_price", &Trade::getPrice)
        .def("get_quantity", &Trade::getQuantity)
        .def("get_maker_order_id", &Trade::getMakerOrderId)
        .def("get_taker_order_id", &Trade::getTakerOrderId)
        .def("get_timestamp", &Trade::getTimestamp)
        .def("get_value", &Trade::getValue);

    // TopOfBook struct
    py::class_<TopOfBook>(m, "TopOfBook")
        .def(py::init<>())
        .def_readwrite("bid_price", &TopOfBook::bid_price)
        .def_readwrite("bid_size", &TopOfBook::bid_size)
        .def_readwrite("ask_price", &TopOfBook::ask_price)
        .def_readwrite("ask_size", &TopOfBook::ask_size)
        .def_readwrite("timestamp", &TopOfBook::timestamp);

    // PriceLevel struct
    py::class_<PriceLevel>(m, "PriceLevel")
        .def(py::init<>())
        .def_readwrite("price", &PriceLevel::price)
        .def_readwrite("total_quantity", &PriceLevel::total_quantity)
        .def_property_readonly("orders", [](const PriceLevel& pl) {
            return py::cast(pl.orders);
        });

    // OrderBook class
    py::class_<OrderBook>(m, "OrderBook")
        .def(py::init<const std::string&>())
        .def("get_symbol", &OrderBook::getSymbol)
        .def("add_order", &OrderBook::addOrder)
        .def("cancel_order", &OrderBook::cancelOrder)
        .def("modify_order", &OrderBook::modifyOrder)
        .def("get_top_of_book", &OrderBook::getTopOfBook)
        .def("get_depth", &OrderBook::getDepth)
        .def("register_trade_callback", &OrderBook::registerTradeCallback)
        .def("register_order_book_update_callback", &OrderBook::registerOrderBookUpdateCallback)
        .def("calculate_order_flow_imbalance", &OrderBook::calculateOrderFlowImbalance)
        .def("get_all_orders", &OrderBook::getAllOrders)
        .def("clear", &OrderBook::clear);

    // MarketDataMessage class
    py::class_<MarketDataMessage> market_data_message(m, "MarketDataMessage");
    py::enum_<MarketDataMessage::Type>(market_data_message, "Type")
        .value("ORDER_ADD", MarketDataMessage::Type::ORDER_ADD)
        .value("ORDER_MODIFY", MarketDataMessage::Type::ORDER_MODIFY)
        .value("ORDER_CANCEL", MarketDataMessage::Type::ORDER_CANCEL)
        .value("TRADE", MarketDataMessage::Type::TRADE)
        .value("HEARTBEAT", MarketDataMessage::Type::HEARTBEAT)
        .value("SNAPSHOT", MarketDataMessage::Type::SNAPSHOT)
        .export_values();

    // MarketDataHandler class
    py::class_<MarketDataHandler, std::shared_ptr<MarketDataHandler>>(m, "MarketDataHandler")
        .def("handle_message", &MarketDataHandler::handleMessage);

    // MarketDataHandlerImpl class
    py::class_<MarketDataHandlerImpl, MarketDataHandler, std::shared_ptr<MarketDataHandlerImpl>>(m, "MarketDataHandlerImpl")
        .def(py::init<>())
        .def("register_order_book", &MarketDataHandlerImpl::registerOrderBook)
        .def("unregister_order_book", &MarketDataHandlerImpl::unregisterOrderBook)
        .def("get_order_book", &MarketDataHandlerImpl::getOrderBook);

    // MarketDataFeed class
    py::class_<MarketDataFeed, std::shared_ptr<MarketDataFeed>>(m, "MarketDataFeed")
        .def("start", &MarketDataFeed::start)
        .def("stop", &MarketDataFeed::stop)
        .def("subscribe", &MarketDataFeed::subscribe)
        .def("unsubscribe", &MarketDataFeed::unsubscribe)
        .def("register_handler", &MarketDataFeed::registerHandler)
        .def_static("create", &MarketDataFeed::create);

    // BaseMarketDataFeed class
    py::class_<BaseMarketDataFeed, MarketDataFeed, std::shared_ptr<BaseMarketDataFeed>>(m, "BaseMarketDataFeed");

    // WebSocketMarketDataFeed class
    py::class_<WebSocketMarketDataFeed, BaseMarketDataFeed, std::shared_ptr<WebSocketMarketDataFeed>>(m, "WebSocketMarketDataFeed")
        .def(py::init<const std::string&>());

    // MarketDataHandlerFactory class
    py::class_<MarketDataHandlerFactory>(m, "MarketDataHandlerFactory")
        .def_static("create_handler", &MarketDataHandlerFactory::createHandler);
} 
