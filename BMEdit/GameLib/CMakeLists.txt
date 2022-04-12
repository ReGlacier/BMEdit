cmake_minimum_required(VERSION 3.17)
project(GameLib)

set(CMAKE_CXX_STANDARD 20)

set(ZBINARYREADER_BUILD_TESTS OFF)
add_subdirectory(ThirdParty/ZBinaryIO)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(ThirdParty/json)

set(GAME_LIB_SOURCES)
file(GLOB_RECURSE GAME_LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp)

add_library(GameLib STATIC ${GAME_LIB_SOURCES})
target_include_directories(GameLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(GameLib PRIVATE ZBinaryReader nlohmann_json::nlohmann_json)