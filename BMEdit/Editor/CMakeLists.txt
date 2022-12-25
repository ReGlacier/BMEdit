cmake_minimum_required(VERSION 3.17)
project(Editor VERSION 1.0.0 LANGUAGES CXX)

# --- Language standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Qt specific things
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

find_package(Qt6 6.4.1 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
find_package(OpenGL REQUIRED)

message(STATUS "Found Qt component versions: ")
message(STATUS "\tWidgets: ${Qt6Widgets_VERSION}")
message(STATUS "\tOpenGL: ${Qt6OpenGL_VERSION}")
message(STATUS "\tOpenGLWidgets: ${Qt6OpenGLWidgets_VERSION}")

# --- Sources
set(EDITOR_SOURCES)
file(GLOB_RECURSE EDITOR_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp)

# --- UI Sources
set(EDITOR_AUTOGEN_UI_SOURCES)
set(EDITOR_AUTOGEN_UI_FILES)
set(EDITOR_AUTOGEN_UI_HEADERS)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

file(GLOB_RECURSE EDITOR_AUTOGEN_UI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/UI/Source/*.cpp)
file(
        GLOB_RECURSE
        EDITOR_UI_HEADERS
        # --- UI
        ${CMAKE_CURRENT_SOURCE_DIR}/UI/Include/*.h
        # --- Qt based code
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/Editor/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/Models/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/Delegates/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/Widgets/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/Data/*.h
        ${CMAKE_CURRENT_SOURCE_DIR}/Include/Render/*.h
)
file(GLOB_RECURSE EDITOR_AUTOGEN_UI_FILES ${CMAKE_CURRENT_SOURCE_DIR}/UI/UI/*.ui)

qt6_wrap_ui(EDITOR_AUTOGEN_UI_HEADERS ${EDITOR_AUTOGEN_UI_FILES})
qt6_wrap_cpp(EDITOR_PREPARED_HEADERS ${EDITOR_UI_HEADERS})
qt6_add_resources(EDITOR_RESOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Resources/BMEdit.qrc)

# --- Editor is a library
add_library(Editor STATIC ${EDITOR_SOURCES} ${EDITOR_AUTOGEN_UI_SOURCES} ${EDITOR_AUTOGEN_UI_HEADERS} ${EDITOR_PREPARED_HEADERS} ${EDITOR_RESOURCES})
target_include_directories(Editor PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(Editor PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(Editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/UI/Include)
target_compile_definitions(Editor
        PUBLIC
            $<$<CONFIG:Debug>:BMEDIT_DEBUG>
            $<$<CONFIG:RelWithDebInfo>:BMEDIT_DEBUG>
            $<$<CONFIG:Release>:BMEDIT_RELEASE>
            $<$<CONFIG:MinSizeRel>:BMEDIT_RELEASE>
)

# --- Required GameLib & Qt6
target_link_libraries(Editor PUBLIC Qt6::Widgets OpenGL::GL Qt6::OpenGL Qt6::OpenGLWidgets)
target_link_libraries(Editor PRIVATE GameLib AssetProvider GlacierTypeInfoLoader GlacierTypeInfoLoader BMRender)