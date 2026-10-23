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
    ConverterJSON.cpp
    InvertedIndex.cpp
    SearchServer.cpp
)

target_include_directories(search_engine_lib PUBLIC "${SEARCH_ENGINE_INCLUDE_DIR}")
target_link_libraries(
    search_engine_lib
    PUBLIC
        nlohmann_json::nlohmann_json
        Threads::Threads
)

if(MSVC)
    target_compile_options(search_engine_lib PRIVATE /W4 /permissive-)
else()
    target_compile_options(search_engine_lib PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_executable(search_engine main.cpp)
target_link_libraries(search_engine PRIVATE search_engine_lib)
