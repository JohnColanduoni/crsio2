cmake_minimum_required(VERSION 3.8)

include_directories("include")

add_library(crsio2
  src/error.cc
  src/broker_services.cc
  src/target_services.cc

  src/windows/error.cc 
  src/windows/broker_services_impl.cc
  src/windows/target_services_impl.cc
)
target_link_libraries(crsio2 chromium_sandbox)
target_include_directories(crsio2 PRIVATE ${CMAKE_SOURCE_DIR}/chromium ${CMAKE_SOURCE_DIR}/chromium_config)
