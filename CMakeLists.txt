cmake_minimum_required(VERSION 3.16)

project(search_engine VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

include(FetchContent)

find_package(nlohmann_json 3.11 QUIET)
if(NOT nlohmann_json_FOUND)
    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

find_package(Threads REQUIRED)

add_library(search_engine_lib
    src/ConverterJSON.cpp
    src/InvertedIndex.cpp
    src/SearchServer.cpp
)

target_include_directories(search_engine_lib PUBLIC include)
target_link_libraries(search_engine_lib PUBLIC nlohmann_json::nlohmann_json Threads::Threads)

if(MSVC)
    target_compile_options(search_engine_lib PRIVATE /W4 /permissive-)
else()
    target_compile_options(search_engine_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(search_engine src/main.cpp)
target_link_libraries(search_engine PRIVATE search_engine_lib)

include(CTest)
if(BUILD_TESTING)
    find_package(GTest QUIET)
    if(NOT GTest_FOUND)
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        FetchContent_MakeAvailable(googletest)
    endif()

    add_executable(search_engine_tests
        tests/ConverterJSONTests.cpp
        tests/InvertedIndexTests.cpp
        tests/SearchServerTests.cpp
    )
    target_link_libraries(search_engine_tests PRIVATE search_engine_lib GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(search_engine_tests)
endif()

