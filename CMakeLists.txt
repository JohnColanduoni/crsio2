cmake_minimum_required(VERSION 3.8)

project(crsio2 C CXX)

string(REPLACE "/MDd" "/MD" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
string(REPLACE "/MDd" "/MD" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})

add_subdirectory(chromium)
add_subdirectory(api/c)
