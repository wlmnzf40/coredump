set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O2 -g -Wall -Wextra")

# jemalloc
add_library(jemalloc STATIC IMPORTED)
set_target_properties(jemalloc PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../build/lib/libjemalloc.a)

# gflags
add_library(gflags STATIC IMPORTED)
set_target_properties(gflags PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../build/lib/libgflags.a)

# protobuf
add_library(protobuf STATIC IMPORTED)
set_target_properties(protobuf PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../build/lib/libprotobuf.a)

# brpc
add_library(brpc STATIC IMPORTED)
set_target_properties(brpc PROPERTIES
        IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/../build/lib/libbrpc.a)

# toft
add_executable(demo main.cc ${CMAKE_CURRENT_SOURCE_DIR}/../build/include/toft/base/closure.cpp)

target_link_libraries(demo
        PRIVATE
        jemalloc
        brpc
        gflags
        protobuf
        pthread dl ssl crypto z)

target_include_directories(demo PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../build/include
)
