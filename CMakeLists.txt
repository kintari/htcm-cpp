cmake_minimum_required(VERSION 3.24)
project(htcm-cpp VERSION 0.0.1 DESCRIPTION "Harvest Temple CM Log Tool" LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(htcm-cpp src/BinaryReader.cpp src/Encounter.cpp src/Loader.cpp src/main.cpp)

include(FetchContent)

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(htcm-cpp PRIVATE nlohmann_json::nlohmann_json)


