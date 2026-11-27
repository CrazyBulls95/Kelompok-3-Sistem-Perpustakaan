
cmake_minimum_required(VERSION 3.16)

project(SistemPerpustakaan LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(SistemPerpustakaan
    src/main.cpp
    src/dashboard.cpp
    src/dashboard.h
    src/buku.cpp
    src/buku.h
    src/sorting.cpp
    src/sorting.h
    src/validasi.cpp
    src/validasi.h
)

target_link_libraries(SistemPerpustakaan PRIVATE Qt6::Widgets)
