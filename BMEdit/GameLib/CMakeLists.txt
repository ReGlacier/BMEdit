cmake_minimum_required(VERSION 3.17)
project(GameLib)

set(CMAKE_CXX_STANDARD 20)

set(ZBINARYREADER_BUILD_TESTS OFF)
add_subdirectory(ThirdParty/ZBinaryIO)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(ThirdParty/json)

add_subdirectory(ThirdParty/gtest)

set(GAME_LIB_SOURCES)
file(GLOB_RECURSE GAME_LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp)

# --- Lib decl
add_library(GameLib STATIC ${GAME_LIB_SOURCES})
target_include_directories(GameLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)

# --- Dependencies
target_link_libraries(GameLib PRIVATE ZBinaryReader) # Private library to work with binary streams
target_link_libraries(GameLib PUBLIC nlohmann_json::nlohmann_json) # Public library to work with json
target_link_libraries(GameLib PUBLIC zlibstatic) # Public library to work with compressed streams

# --- ZLIB fixes
target_include_directories(GameLib PRIVATE
        $<TARGET_FILE_DIR:zlibstatic>/..
        $<TARGET_FILE_DIR:zlibstatic>
        $<TARGET_PROPERTY:zlibstatic,INCLUDE_DIRECTORIES>)

# --- Tests
add_subdirectory(Tests)