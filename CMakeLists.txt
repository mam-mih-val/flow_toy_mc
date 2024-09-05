cmake_minimum_required(VERSION 3.0)
project( ToyMC )
set(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")

find_package( ROOT REQUIRED )

add_executable( generate src/main.cc )
target_compile_options( generate PRIVATE -O -Wall -Wextra -Wpedantic)
target_link_libraries( generate PRIVATE ROOT::RIO ROOT::Tree ROOT::EG ROOT::GenVector )
