project(GlacierTypeInfoLoader)


add_library(GlacierTypeInfoLoader STATIC ${CMAKE_CURRENT_SOURCE_DIR}/Source/GlacierTypeInfoLoader.cpp)
target_include_directories(GlacierTypeInfoLoader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_link_libraries(GlacierTypeInfoLoader PRIVATE GameLib nlohmann_json::nlohmann_json fmt::fmt-header-only)