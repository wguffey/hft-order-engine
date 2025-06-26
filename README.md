# Ultra-Low-Latency Market Data Analyzer

A high-performance market data processing engine capable of ingesting, reconstructing, and analyzing high-frequency trading data with ultra-low latency.

## Architecture

The system is implemented with a hybrid approach:
- **C++ core components** for performance-critical processing
- **Python interface** for analytics, visualization, and user interaction

![System Architecture](docs/images/system_architecture.png)

### Core Components

1. **Order Book Engine**: Maintains and updates limit order books with nanosecond precision
2. **Market Data Feed**: Handles real-time market data from multiple sources
3. **Order Management**: Processes order lifecycle events
4. **Trade Execution**: Matches orders and generates trades
5. **Analytics Engine**: Computes key market metrics and detects patterns

### Performance Optimizations

- **Memory Efficiency**: Pre-allocated memory pools, zero-copy parsing, cache-friendly data structures
- **Concurrency**: Lock-free data structures, thread pools, work-stealing scheduler
- **Algorithmic**: SIMD vectorization, branch prediction hints, optimized data structures
- **I/O**: Asynchronous I/O, batch processing, zero-copy network stack

## Requirements

### C++ Requirements
- C++17 compatible compiler (GCC 8+, Clang 6+, MSVC 2019+)
- CMake 3.14+
- Pthread support

### Python Requirements
- Python 3.8+
- Dependencies listed in requirements.txt

## Building

### Using Docker (Recommended)

The easiest way to build and run the project is using Docker:

```bash
# Build the Docker image
docker build -t orderbook .

# Run the Python example
docker run orderbook

# Run the C++ example
docker run orderbook /app/build/examples/simple_order_book

# Run tests
docker run orderbook /app/build/tests/test_order_book
```

### Using Docker Compose

We also provide a Docker Compose setup for easier development:

```bash
# Start a development environment
docker compose run dev

# Run the Python example
docker compose run python-example

# Run the C++ example
docker compose run cpp-example

# Run tests
docker compose run tests

# Start Jupyter Notebook for analysis (available at http://localhost:8888)
docker compose up jupyter
```

### Manual Build

If you prefer to build manually:

```bash
# Build the C++ core
mkdir build && cd build
cmake ..
make

# Install the Python package
pip install -r requirements.txt
python setup.py install
```

## Usage

### Basic Usage

```python
from orderbook import OrderBook, MarketDataFeed

# Create an order book for a specific symbol
book = OrderBook("AAPL")

# Connect to a market data feed
feed = MarketDataFeed.create("websocket", "wss://example.com/market-data")
feed.subscribe("AAPL")

# Register the order book with the feed
feed.register_handler(book)

# Start processing
feed.start()

# Get the current market state
top_of_book = book.get_top_of_book()
print(f"Bid: {top_of_book['bid_price']} x {top_of_book['bid_size']}")
print(f"Ask: {top_of_book['ask_price']} x {top_of_book['ask_size']}")

# Calculate metrics
imbalance = book.calculate_order_flow_imbalance(5)  # depth of 5 levels
print(f"Order flow imbalance: {imbalance}")
```

## Documentation

- [Architecture Overview](docs/architecture/README.md)
- [API Reference](docs/api/README.md)
- [Performance Benchmarks](docs/benchmarks/README.md)
- [Development Guide](docs/development/README.md)
