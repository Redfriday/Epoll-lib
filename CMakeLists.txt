cmake_minimum_required(VERSION 3.0)
set(PROJECT_NAME "Epoll")
project(${PROJECT_NAME})

# Add src code.
aux_source_directory(${CMAKE_SOURCE_DIR}/src SRC)

# Include headers in below directory.
include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/src)

list(APPEND LIBS pthread)

# Use c++11.
set(CMAKE_CXX_FLAGS "-g -fPIC -std=c++11 -ldl -pipe -Wno-deprecated-declarations")

# Add static lib.
ADD_LIBRARY(epoll STATIC ${SRC})
target_link_libraries(epoll ${LIBS})

# Build server program.
add_executable(echo_server echo_server.cpp ${SRC})
target_link_libraries(echo_server ${LIBS})

# Build client program.
add_executable(echo_client echo_client.cpp)
