cmake_minimum_required(VERSION 3.16)
project(GioHangOnline LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

add_library(gio_hang_backend
    src/DanhSachLienKetDoi.cpp
    src/GioHang.cpp
    src/DonHang.cpp
    src/KhoHang.cpp
    src/LuuTru.cpp
    src/api/ChuyenDoiJson.cpp
    src/api/MayChu.cpp
)
target_include_directories(gio_hang_backend PUBLIC Include)

add_executable(chay_test tests/test_backend.cpp)
target_link_libraries(chay_test PRIVATE gio_hang_backend)

add_executable(gio_hang_web src/main_web.cpp)
target_link_libraries(gio_hang_web PRIVATE gio_hang_backend)
if(WIN32)
    target_link_libraries(gio_hang_backend PUBLIC ws2_32)
endif()

enable_testing()
add_test(NAME backend COMMAND chay_test)
