# Build toan bo Backend ShopPro.
# Cach dung (tu thu muc Backend/):
#   cmake -S Integration -B build -DSHOPPRO_SANITIZE=ON
#   cmake --build build
#   ctest --test-dir build --output-on-failure
#   ./build/shoppro_cli          (Windows: build\shoppro_cli.exe)
cmake_minimum_required(VERSION 3.10)
project(ShopProBackend CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Duong dan goc cua Backend (thu muc cha cua Integration)
set(BE "${CMAKE_CURRENT_SOURCE_DIR}/..")

option(SHOPPRO_SANITIZE "Bat AddressSanitizer + UBSan (can GCC/Clang tren Linux/macOS; MinGW thuong khong ho tro)" OFF)

add_library(shoppro_core STATIC
    ${BE}/Common/Node.cpp
    ${BE}/Common/DoublyLinkedList.cpp
    ${BE}/ProductManager/ProductValidation.cpp
    ${BE}/ProductManager/ProductManager.cpp
    ${BE}/ProductSearch/ProductSearch.cpp
    ${BE}/ProductSearch/ProductDisplay.cpp
    ${BE}/CartProcessing/CartCalculator.cpp
    ${BE}/CartProcessing/ProductSort.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/CartController.cpp
)
target_include_directories(shoppro_core PUBLIC
    ${BE}/Common ${BE}/ProductManager ${BE}/ProductSearch ${BE}/CartProcessing
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(shoppro_core PUBLIC -Wall -Wextra -Wpedantic)
    if(SHOPPRO_SANITIZE)
        target_compile_options(shoppro_core PUBLIC -fsanitize=address,undefined -g)
        target_link_libraries(shoppro_core PUBLIC -fsanitize=address,undefined)
    endif()
endif()

add_executable(shoppro_cli main.cpp)
target_link_libraries(shoppro_cli PRIVATE shoppro_core)

enable_testing()
foreach(ten_test
        test_doubly_linked_list test_product_manager test_product_search
        test_cart_calculator test_cart_controller)
    add_executable(${ten_test} ${BE}/tests/${ten_test}.cpp)
    target_link_libraries(${ten_test} PRIVATE shoppro_core)
    add_test(NAME ${ten_test} COMMAND ${ten_test})
endforeach()

add_executable(demo_kiem_tra_lien_ket ${BE}/tests/demo_kiem_tra_lien_ket.cpp)
target_link_libraries(demo_kiem_tra_lien_ket PRIVATE shoppro_core)
