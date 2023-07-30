cmake_minimum_required(VERSION 3.17)
project(GameLib)

set(CMAKE_CXX_STANDARD 20)

set(ZBINARYREADER_BUILD_TESTS OFF)
add_subdirectory(ThirdParty/ZBinaryIO)

set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(ThirdParty/json)

set(GAME_LIB_SOURCES)
file(GLOB_RECURSE GAME_LIB_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp)

# --- Lib decl
add_library(GameLib STATIC ${GAME_LIB_SOURCES})
target_include_directories(GameLib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)

# --- Dependencies
target_link_libraries(GameLib PRIVATE ZBinaryReader) # Private libs
target_link_libraries(GameLib PUBLIC nlohmann_json::nlohmann_json fmt::fmt-header-only) # Public library to work with json
target_link_libraries(GameLib PUBLIC ${ZLIB_LIBRARIES}) # Public library to work with compressed streams
target_include_directories(GameLib PRIVATE ${ZLIB_INCLUDE_DIRS})

# --- Tests (temporary disabled)
#add_subdirectory(ThirdParty/gtest)
#add_subdirectory(Tests)