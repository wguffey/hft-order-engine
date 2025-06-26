#!/usr/bin/env python3
"""
Simple example demonstrating the use of the orderbook Python API.
"""

import time
import datetime

# Try to import optional dependencies
try:
    import pandas as pd
    import matplotlib.pyplot as plt

    VISUALIZATION_AVAILABLE = True
except ImportError:
    print(
        "Warning: pandas and/or matplotlib not available. Visualization will be disabled."
    )
    VISUALIZATION_AVAILABLE = False

from orderbook import Order, OrderBook, Trade, Side, OrderType, OrderStatus


def print_top_of_book(tob):
    """Callback function for order book updates"""
    print(
        f"Top of Book: Bid: {tob['bid_price']} x {tob['bid_size']} | Ask: {tob['ask_price']} x {tob['ask_size']}"
    )


def print_trade(trade):
    """Callback function for trade notifications"""
    print(
        f"Trade: {trade.get_symbol()} @ {trade.get_price()} x {trade.get_quantity()} (ID: {trade.get_id()})"
    )


def main():
    # Create an order book for a specific symbol
    book = OrderBook("AAPL")

    # Register callbacks
    book.register_trade_callback(print_trade)
    book.register_order_book_update_callback(print_top_of_book)

    print(f"Order Book Example for {book.get_symbol()}")
    print("-----------------------------------")

    # Create a timestamp generator
    def next_time():
        return int(time.time_ns())

    # Add some initial orders
    print("\nAdding initial orders...\n")

    # Add some buy orders
    book.add_order(
        Order(1, "AAPL", 15000, 100, Side.BUY, OrderType.LIMIT, next_time())
    )
    book.add_order(
        Order(2, "AAPL", 14950, 200, Side.BUY, OrderType.LIMIT, next_time())
    )
    book.add_order(
        Order(3, "AAPL", 14900, 300, Side.BUY, OrderType.LIMIT, next_time())
    )

    # Add some sell orders
    book.add_order(
        Order(4, "AAPL", 15050, 150, Side.SELL, OrderType.LIMIT, next_time())
    )
    book.add_order(
        Order(5, "AAPL", 15100, 250, Side.SELL, OrderType.LIMIT, next_time())
    )
    book.add_order(
        Order(6, "AAPL", 15150, 350, Side.SELL, OrderType.LIMIT, next_time())
    )

    # Print the order book depth
    print("\nOrder Book Depth (3 levels):")
    bids, asks = book.get_depth(3)

    print("Bids:")
    for level in bids:
        print(f"  {level.price} x {level.total_quantity}")

    print("Asks:")
    for level in asks:
        print(f"  {level.price} x {level.total_quantity}")

    # Calculate order flow imbalance
    ofi = book.calculate_order_flow_imbalance(3)
    print(f"\nOrder Flow Imbalance: {ofi}")

    # Add a matching order that will generate trades
    print("\nAdding a matching order...\n")
    trades = book.add_order(
        Order(7, "AAPL", 15100, 300, Side.BUY, OrderType.LIMIT, next_time())
    )

    print(f"\nGenerated {len(trades)} trades")

    # Modify an order
    print("\nModifying order ID 1...\n")
    book.modify_order(1, 15025, 150)

    # Cancel an order
    print("\nCanceling order ID 3...\n")
    book.cancel_order(3)

    # Get all orders
    all_orders = book.get_all_orders()
    print(f"\nTotal orders in book: {len(all_orders)}")

    # Visualization example (if dependencies are available)
    if VISUALIZATION_AVAILABLE:
        visualize_order_book(book)
    else:
        print("\nVisualization skipped due to missing dependencies.")


def visualize_order_book(book):
    """Create a simple visualization of the order book"""
    if not VISUALIZATION_AVAILABLE:
        print("Visualization not available - missing dependencies")
        return

    print("\nCreating order book visualization...")

    # Get depth data
    bids, asks = book.get_depth(10)  # Get up to 10 levels

    # Create DataFrames
    bid_data = [(level.price, level.total_quantity) for level in bids]
    ask_data = [(level.price, level.total_quantity) for level in asks]

    bid_df = pd.DataFrame(bid_data, columns=["Price", "Quantity"])
    ask_df = pd.DataFrame(ask_data, columns=["Price", "Quantity"])

    # Plot
    plt.figure(figsize=(10, 6))

    # Plot bids (green)
    plt.barh(range(len(bid_df)), bid_df["Quantity"], color="green", alpha=0.5)
    plt.yticks(range(len(bid_df)), [f"${p/100:.2f}" for p in bid_df["Price"]])

    # Plot asks (red)
    if not ask_df.empty:
        import numpy as np

        ask_pos = len(bid_df) + 1 + np.arange(len(ask_df))
        plt.barh(ask_pos, ask_df["Quantity"], color="red", alpha=0.5)
        plt.yticks(
            np.concatenate([plt.yticks()[0], ask_pos]),
            np.concatenate(
                [plt.yticks()[1], [f"${p/100:.2f}" for p in ask_df["Price"]]]
            ),
        )

    plt.title(f"Order Book for {book.get_symbol()}")
    plt.xlabel("Quantity")
    plt.ylabel("Price")
    plt.grid(alpha=0.3)

    # Instead of showing, we just save the plot
    plt.savefig(f"order_book_{book.get_symbol()}.png")
    print(f"Visualization saved to order_book_{book.get_symbol()}.png")


if __name__ == "__main__":
    main()
