cmake_minimum_required(VERSION 3.10)

project(Neptune)

add_executable(neptune src/main.cpp)

include(FetchContent)

# libcpr
FetchContent_Declare(cpr URL https://github.com/libcpr/cpr/archive/refs/tags/1.9.2.zip)
FetchContent_MakeAvailable(cpr)

# nlohmann json
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(neptune PRIVATE cpr::cpr nlohmann_json::nlohmann_json)
