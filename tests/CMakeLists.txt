include(FetchContent)

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
    ConverterJSONTests.cpp
    InvertedIndexTests.cpp
    SearchServerTests.cpp
)

target_link_libraries(
    search_engine_tests
    PRIVATE
        search_engine_lib
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(search_engine_tests)
