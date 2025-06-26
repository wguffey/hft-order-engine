cmake_minimum_required(VERSION 3.14)
project(OrderBook VERSION 0.1.0 LANGUAGES CXX)

# Set C++17 as the standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Enable optimization flags for Release builds
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native -DNDEBUG")
  endif()
endif()

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Find dependencies
find_package(Threads REQUIRED)

# Add include directories
include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/third_party
)

# Core library
add_library(orderbook_core STATIC
  src/core/order_book.cpp
  src/core/order.cpp
  src/core/trade.cpp
  src/core/market_data_feed.cpp
  src/core/market_data_handler.cpp
)

target_link_libraries(orderbook_core PUBLIC Threads::Threads)

# Python bindings
option(ORDERBOOK_BUILD_PYTHON "Build Python bindings" ON)
if(ORDERBOOK_BUILD_PYTHON)
  # Find Python
  find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
  message(STATUS "Found Python: ${Python3_EXECUTABLE} (${Python3_VERSION})")
  message(STATUS "Python include dirs: ${Python3_INCLUDE_DIRS}")
  message(STATUS "Python libraries: ${Python3_LIBRARIES}")
  
  # Find pybind11
  if(NOT EXISTS "${PROJECT_SOURCE_DIR}/third_party/pybind11/CMakeLists.txt")
    # If pybind11 is not in third_party, try to find it via find_package
    find_package(pybind11 QUIET)
    
    if(NOT pybind11_FOUND)
      # Download pybind11 if not found
      include(FetchContent)
      FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG        v2.10.4
      )
      FetchContent_MakeAvailable(pybind11)
    endif()
  else()
    # Use the local copy
    add_subdirectory(third_party/pybind11)
  endif()

  # Create Python module
  pybind11_add_module(core MODULE src/python/orderbook/core.cpp)
  target_link_libraries(core PRIVATE orderbook_core)
  
  # Get Python module extension suffix (platform dependent)
  execute_process(
    COMMAND "${Python3_EXECUTABLE}" -c "import sysconfig; print(sysconfig.get_config_var('EXT_SUFFIX'))"
    OUTPUT_VARIABLE PYTHON_MODULE_EXTENSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  
  if(NOT PYTHON_MODULE_EXTENSION)
    set(PYTHON_MODULE_EXTENSION ".so")
  endif()
  
  message(STATUS "Python module extension: ${PYTHON_MODULE_EXTENSION}")
  
  # Set output name and location
  set_target_properties(core PROPERTIES
    PREFIX ""
    SUFFIX "${PYTHON_MODULE_EXTENSION}"
    OUTPUT_NAME "core"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src/python/orderbook"
  )
  
  # Create __init__.py if it doesn't exist
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/src/python/orderbook")
  configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/src/python/orderbook/__init__.py"
    "${CMAKE_CURRENT_BINARY_DIR}/src/python/orderbook/__init__.py"
    COPYONLY
  )
  
  # Print the output location for debugging
  add_custom_command(TARGET core POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Python module built at: $<TARGET_FILE:core>"
  )
  
  # Install the Python module
  install(TARGETS core
    LIBRARY DESTINATION "src/python/orderbook"
  )
  
  # Install the Python package files
  install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/python/orderbook/__init__.py"
    DESTINATION "src/python/orderbook"
  )
endif()

# Examples
option(ORDERBOOK_BUILD_EXAMPLES "Build examples" ON)
if(ORDERBOOK_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

# Tests
option(ORDERBOOK_BUILD_TESTS "Build tests" ON)
if(ORDERBOOK_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# Install targets
install(TARGETS orderbook_core
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
)

# Install headers
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
  DESTINATION include
) 
