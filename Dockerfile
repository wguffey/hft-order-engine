###############################################################################
# Builder stage - Compiles C++ code and builds Python bindings
###############################################################################
FROM ubuntu:22.04 AS builder

# Set non-interactive install mode
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make \
    python3 \
    python3-dev \
    python3-pip \
    libpthread-stubs0-dev \
    git \
    ninja-build \
    ccache \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Set up Python environment
RUN pip3 install --no-cache-dir --upgrade pip setuptools wheel

# Create app directory
WORKDIR /app

# Copy requirements first for better caching
COPY requirements.txt .
RUN pip3 install --no-cache-dir -r requirements.txt

# Copy project files
COPY . .

# Build with CMake
RUN mkdir -p build && cd build \
    && cmake -GNinja -DCMAKE_BUILD_TYPE=Release .. \
    && ninja -j$(nproc) \
    && ninja install

# Install Python package
RUN pip3 install -e .

###############################################################################
# Runtime stage - Minimal image with only necessary components
###############################################################################
FROM ubuntu:22.04

# Set non-interactive install mode
ENV DEBIAN_FRONTEND=noninteractive

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    python3 \
    python3-pip \
    libstdc++6 \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Install Python runtime dependencies
COPY --from=builder /app/requirements.txt /tmp/requirements.txt
RUN pip3 install --no-cache-dir -r /tmp/requirements.txt && rm /tmp/requirements.txt

# Copy compiled binaries and Python package from builder
COPY --from=builder /usr/local/lib/python3.10/dist-packages /usr/local/lib/python3.10/dist-packages
COPY --from=builder /usr/local/lib /usr/local/lib
COPY --from=builder /usr/local/include /usr/local/include
COPY --from=builder /app/build /app/build
COPY --from=builder /app/examples /app/examples

# Set working directory
WORKDIR /app

# Add examples directory to Python path
ENV PYTHONPATH=/app:$PYTHONPATH

# Create a wrapper script for running examples
RUN echo '#!/bin/bash\n\
echo "Ultra-Low-Latency Market Data Analyzer"\n\
echo "======================================"\n\
echo "Available commands:"\n\
echo "  python3 /app/examples/python_example.py - Run Python example"\n\
echo "  /app/build/examples/simple_order_book   - Run C++ example"\n\
echo "  /app/build/tests/test_order_book        - Run tests"\n\
echo "======================================"\n\
if [ "$1" != "" ]; then\n\
  exec "$@"\n\
else\n\
  exec python3 /app/examples/python_example.py\n\
fi' > /app/entrypoint.sh \
    && chmod +x /app/entrypoint.sh

# Default command
ENTRYPOINT ["/app/entrypoint.sh"] 
