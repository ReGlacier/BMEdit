project(BMRender)

# --- Language standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Project
add_subdirectory(ThirdParty/glad)

file(GLOB_RECURSE BMRENDER_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp)
add_library(BMRender STATIC ${BMRENDER_SOURCES})
add_library(bm::gl3render ALIAS BMRender)

target_include_directories(BMRender PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(BMRender PRIVATE GameLib glad)
target_link_libraries(BMRender PUBLIC glm::glm)