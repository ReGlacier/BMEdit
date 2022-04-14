cmake_minimum_required(VERSION 3.16)
project(minizip C)

file(GLOB MINIZIP_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/source/*.c)

add_library(minizip STATIC ${MINIZIP_SOURCES})
target_include_directories(
        minizip PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
        $<TARGET_FILE_DIR:zlibstatic>/..
        $<TARGET_FILE_DIR:zlibstatic>
        $<TARGET_PROPERTY:zlibstatic,INCLUDE_DIRECTORIES>
)
target_link_libraries(minizip PUBLIC zlibstatic)