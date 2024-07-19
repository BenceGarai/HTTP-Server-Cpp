cmake_minimum_required(VERSION 3.13)

project(http-server-starter-cpp)

file(GLOB_RECURSE SOURCE_FILES src/*.cpp src/*.hpp)

# Add zlib source files
file(GLOB ZLIB_SOURCES third_party/zlib/*.c)
list(APPEND SOURCE_FILES ${ZLIB_SOURCES})

set(CMAKE_CXX_STANDARD 23) # Enable the C++23 standard
set(THREADS_PREFER_PTHREAD_FLAG ON)

find_package(Threads REQUIRED)

# Add zlib include directory
include_directories(third-party/zlib)

add_executable(server ${SOURCE_FILES})

target_link_libraries(server PRIVATE Threads::Threads)
