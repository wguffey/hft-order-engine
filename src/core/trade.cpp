#include "orderbook/trade.h"

namespace orderbook {

Trade::Trade(TradeId id, const std::string& symbol, Price price, Quantity quantity,
             OrderId maker_order_id, OrderId taker_order_id, Timestamp timestamp)
    : id_(id),
      symbol_(symbol),
      price_(price),
      quantity_(quantity),
      maker_order_id_(maker_order_id),
      taker_order_id_(taker_order_id),
      timestamp_(timestamp) {}

} // namespace orderbook 
