cmake_minimum_required(VERSION 3.10)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        set(BUILD_FOR_WIN TRUE)
endif()
option(STATIC_BUILD "Build a static binary." ${BUILD_FOR_WIN})

if (STATIC_BUILD)
        set(CMAKE_EXE_LINKER_FLAGS "-static")
        set(CMAKE_FIND_LIBRARY_SUFFIXES ".a" CONFIG)
        set(BUILD_SHARED_LIBS OFF)
endif()


project(Neptune)

add_executable(neptune src/main.cpp)

include(FetchContent)

# libcpr
FetchContent_Declare(cpr URL https://github.com/libcpr/cpr/archive/refs/tags/1.9.2.zip)
FetchContent_MakeAvailable(cpr)

# nlohmann json
FetchContent_Declare(json URL https://github.com/nlohmann/json/archive/refs/tags/v3.11.0.tar.gz)
FetchContent_MakeAvailable(json)

target_link_libraries(neptune PRIVATE cpr::cpr nlohmann_json::nlohmann_json)
