project(RenderDoc)

add_library(RenderDoc_AppInterface INTERFACE)
add_library(RenderDoc::AppInterface ALIAS RenderDoc_AppInterface)

target_include_directories(RenderDoc_AppInterface INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)