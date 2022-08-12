cmake_minimum_required(VERSION 3.17)
project(GameLib_Tests)

set(CMAKE_CXX_STANDARD 20)

add_executable(GameLib_Tests
        Source/Entry.cpp
        Source/PRP.cpp
        Source/PRP_Typing.cpp
        Source/PRP_ComplexPack.cpp
)

target_include_directories(GameLib_Tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)

target_link_libraries(GameLib_Tests PUBLIC
        GameLib
        GTest::gtest_main)