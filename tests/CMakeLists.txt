enable_testing()

add_executable(test_order_book test_order_book.cpp)
target_link_libraries(test_order_book PRIVATE orderbook_core)
add_test(NAME test_order_book COMMAND test_order_book)

# Add more tests as needed 
