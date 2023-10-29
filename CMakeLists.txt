cmake_minimum_required(VERSION 3.16)
project(test CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

if (NOT APPLE)
    set(CMAKE_INSTALL_RPATH $ORIGIN)
endif()
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include (FetchContent)
include (cmake/FetchLagom.cmake)
include(build/serenity/Meta/CMake/lagom_compile_options.cmake)

add_compile_options(-Wno-literal-suffix)
add_compile_options(-fno-gnu-keywords)

add_executable(test main.cpp)

include(build/serenity/Meta/Lagom/get_linked_lagom_libraries.cmake)
get_linked_lagom_libraries(js js_libraries)
target_link_libraries(test ${js_libraries})
