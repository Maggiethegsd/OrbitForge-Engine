cmake_minimum_required(VERSION 3.10)
project(OrbitForgeEngine)

# Require C++17 to unlock <filesystem>
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp")
add_executable(Simulation ${SOURCES})
target_include_directories(Simulation PRIVATE ${CMAKE_SOURCE_DIR}/include)