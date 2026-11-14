# ShopPro Backend - build toan bo (loi DSA + REST API + may chu web + kiem thu).
#
#   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
#   cmake --build build --parallel
#   ctest --test-dir build --output-on-failure
#   ./build/shoppro_api            (Windows: build\shoppro_api.exe)
#
# Phu thuoc: trinh bien C++17, CMake >= 3.18, OpenSSL (libssl-dev / vcpkg / MSYS2).
# nlohmann/json da kem san trong third_party/ nen KHONG can Internet de build.
cmake_minimum_required(VERSION 3.18)
project(ShopPro VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Kieu build" FORCE)
endif()

option(SHOPPRO_SANITIZE "Bat AddressSanitizer + UBSan cho TOAN BO target (GCC/Clang, Linux/macOS)" OFF)
option(SHOPPRO_BUILD_TESTS "Build cac chuong trinh kiem thu" ON)

# ---- Co bien dich ap dung cho MOI target (ke ca kiem thu va API) ----
if(MSVC)
    add_compile_options(/W4 /utf-8 /permissive-)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(SHOPPRO_SANITIZE)
        add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
        add_link_options(-fsanitize=address,undefined)
    endif()
endif()
if(WIN32)
    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# ---- nlohmann/json (header-only, kem san) ----
add_library(nlohmann_json INTERFACE)
add_library(nlohmann_json::nlohmann_json ALIAS nlohmann_json)
target_include_directories(nlohmann_json SYSTEM INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann/include)

# ---- Loi cau truc du lieu & thuat toan (4 module cua nhom) ----
add_library(shoppro_core STATIC
    Common/Node.cpp
    Common/DoublyLinkedList.cpp
    ProductManager/ProductValidation.cpp
    ProductManager/ProductManager.cpp
    ProductSearch/ProductSearch.cpp
    ProductSearch/ProductDisplay.cpp
    CartProcessing/CartCalculator.cpp
    CartProcessing/ProductSort.cpp
    Integration/CartController.cpp
)
target_include_directories(shoppro_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Common
    ${CMAKE_CURRENT_SOURCE_DIR}/ProductManager
    ${CMAKE_CURRENT_SOURCE_DIR}/ProductSearch
    ${CMAKE_CURRENT_SOURCE_DIR}/CartProcessing
    ${CMAKE_CURRENT_SOURCE_DIR}/Integration
)

# ---- Tang REST API (nghiep vu, luu tru, route) ----
add_library(shoppro_api_lib STATIC
    Api/Support.cpp
    Api/Router.cpp
    Api/CartEngine.cpp
    Api/storage/JsonStore.cpp
    Api/storage/ActivityLog.cpp
    Api/routes/Presenters.cpp
    Api/routes/PublicRoutes.cpp
    Api/routes/AuthRoutes.cpp
    Api/routes/CartRoutes.cpp
    Api/routes/OrderRoutes.cpp
    Api/routes/AdminCatalogRoutes.cpp
    Api/routes/AdminOpsRoutes.cpp
)
target_include_directories(shoppro_api_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Api)
target_link_libraries(shoppro_api_lib PUBLIC shoppro_core nlohmann_json::nlohmann_json OpenSSL::Crypto Threads::Threads)

# ---- May chu HTTP ----
add_library(shoppro_http STATIC Api/http/HttpServer.cpp)
target_include_directories(shoppro_http PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Api)
target_link_libraries(shoppro_http PUBLIC shoppro_api_lib Threads::Threads)
if(WIN32)
    target_link_libraries(shoppro_http PUBLIC ws2_32)
endif()

# ---- Chuong trinh chay ----
add_executable(shoppro_api Api/main.cpp)
target_link_libraries(shoppro_api PRIVATE shoppro_http)

add_executable(shoppro_cli Integration/main.cpp)      # ban dong lenh ban dau cua nhom
target_link_libraries(shoppro_cli PRIVATE shoppro_core)

# ---- Kiem thu ----
if(SHOPPRO_BUILD_TESTS)
    enable_testing()
    foreach(name test_doubly_linked_list test_product_manager test_product_search
                 test_cart_calculator test_cart_controller)
        add_executable(${name} tests/${name}.cpp)
        target_link_libraries(${name} PRIVATE shoppro_core)
        add_test(NAME ${name} COMMAND ${name})
    endforeach()

    add_executable(demo_kiem_tra_lien_ket tests/demo_kiem_tra_lien_ket.cpp)
    target_link_libraries(demo_kiem_tra_lien_ket PRIVATE shoppro_core)

    add_executable(test_api tests/test_api.cpp)
    target_link_libraries(test_api PRIVATE shoppro_api_lib)
    target_compile_definitions(test_api PRIVATE SHOPPRO_TEST_SEED_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/seed.json")
    add_test(NAME test_api COMMAND test_api)

    add_executable(test_http tests/test_http.cpp)
    target_link_libraries(test_http PRIVATE shoppro_http)
    target_compile_definitions(test_http PRIVATE SHOPPRO_TEST_SEED_FILE="${CMAKE_CURRENT_SOURCE_DIR}/data/seed.json")
    add_test(NAME test_http COMMAND test_http)
endif()
