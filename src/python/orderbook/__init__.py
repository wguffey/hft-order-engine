"""
OrderBook - Ultra-Low-Latency Market Data Analyzer

This package provides Python bindings for the high-performance C++
order book implementation, allowing for real-time processing and
analysis of market data.
"""

import sys
import os
import glob

# Print debugging information
print(f"Python version: {sys.version}")
print(f"Python path: {sys.path}")
print(f"Current directory: {os.getcwd()}")

# Try to find the core module
so_files = []
for path in sys.path:
    pattern = os.path.join(path, "orderbook", "*.so")
    found = glob.glob(pattern)
    if found:
        so_files.extend(found)
        print(f"Found .so files in {path}/orderbook: {found}")

if not so_files:
    print("No .so files found in any orderbook directory in the Python path.")

try:
    # First try direct import
    try:
        from orderbook.core import (
            Order,
            OrderBook,
            Trade,
            MarketDataFeed,
            Side,
            OrderType,
            OrderStatus,
        )

        print("Successfully imported through 'from orderbook.core'")
    except ImportError as e1:
        print(f"Import error with 'from orderbook.core': {e1}")

        # Try relative import
        try:
            from .core import (
                Order,
                OrderBook,
                Trade,
                MarketDataFeed,
                Side,
                OrderType,
                OrderStatus,
            )

            print("Successfully imported through 'from .core'")
        except ImportError as e2:
            print(f"Import error with 'from .core': {e2}")

            # Look for the module file and try direct loading
            try:
                import importlib.util

                # Search for the core module in various locations
                module_paths = so_files
                if not module_paths:
                    # Add fallback paths
                    for path in sys.path:
                        patterns = [
                            os.path.join(path, "orderbook", "core*.so"),
                            os.path.join(path, "core*.so"),
                            os.path.join(path, "*", "orderbook", "core*.so"),
                        ]
                        for pattern in patterns:
                            found = glob.glob(pattern)
                            module_paths.extend(found)

                if module_paths:
                    print(f"Found potential module paths: {module_paths}")

                    # Try to load the first module found
                    spec = importlib.util.spec_from_file_location(
                        "core", module_paths[0]
                    )
                    core = importlib.util.module_from_spec(spec)
                    spec.loader.exec_module(core)

                    # Import symbols from the loaded module
                    Order = core.Order
                    OrderBook = core.OrderBook
                    Trade = core.Trade
                    MarketDataFeed = core.MarketDataFeed
                    Side = core.Side
                    OrderType = core.OrderType
                    OrderStatus = core.OrderStatus
                    print(f"Successfully loaded module from {module_paths[0]}")
                else:
                    raise ImportError("Could not find core module in any path")
            except Exception as e3:
                print(f"Error with manual module loading: {e3}")
                raise ImportError(
                    f"Failed to import orderbook.core module after multiple attempts. "
                    f"Original errors: {e1}, then {e2}, then {e3}"
                ) from e1

except Exception as e:
    print(f"Final import error: {e}")
    raise

__version__ = "0.1.0"
