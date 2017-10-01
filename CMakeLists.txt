cmake_minimum_required(VERSION 3.8)

project(crsio2 C CXX)

string(REPLACE "/MDd" "/MD" CMAKE_C_FLAGS_DEBUG ${CMAKE_C_FLAGS_DEBUG})
string(REPLACE "/MDd" "/MD" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

if("${MSVC_RUNTIME}" STREQUAL "static")
    message(STATUS "Using static MSVC runtime")

    set(cflag_vars
        CMAKE_C_FLAGS
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_MINSIZEREL
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_MINSIZEREL
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_RELWITHDEBINFO
    )
    foreach(var ${cflag_vars})
        if(${var} MATCHES "/MD")
            string(REPLACE "/MD" "/MT" ${var} "${${var}}")
        else()
            set(var "${var} /MT")
        endif()
    endforeach()
endif()

add_subdirectory(chromium)
add_subdirectory(api/c)
