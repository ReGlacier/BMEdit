project(BMRenderSandbox)

# --- Language standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Modules
find_package(Qt6 6.4.1 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
find_package(OpenGL REQUIRED)

# --- Qt specific things
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(EDITOR_AUTOGEN_UI_SOURCES)
set(EDITOR_AUTOGEN_UI_FILES)

file(GLOB_RECURSE EDITOR_AUTOGEN_UI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/UI/*.cpp)
file(GLOB_RECURSE EDITOR_AUTOGEN_UI_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/UI/*.h)
file(GLOB_RECURSE EDITOR_AUTOGEN_UI_FILES ${CMAKE_CURRENT_SOURCE_DIR}/UI/*.ui)

qt6_wrap_ui(EDITOR_AUTOGEN_UI_HEADERS ${EDITOR_AUTOGEN_UI_FILES})
qt6_wrap_cpp(EDITOR_PREPARED_HEADERS ${EDITOR_UI_HEADERS})

# --- Project
add_executable(BMRenderSandbox
        # Qt
        ${EDITOR_AUTOGEN_UI_SOURCES}
        ${EDITOR_AUTOGEN_UI_HEADERS}
        ${EDITOR_PREPARED_HEADERS}
        # Sandbox
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/Sandbox.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Source/SandboxLogger.cpp)

target_include_directories(BMRenderSandbox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Include)
target_include_directories(BMRenderSandbox PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/UI)
target_link_libraries(BMRenderSandbox PUBLIC
        GameLib
        BMRender
        AssetProvider
        GlacierTypeInfoLoader
        Qt6::Widgets
        OpenGL::GL
        Qt6::OpenGL
        Qt6::OpenGLWidgets
        glad)

# --- Deploy
qt6_register_deploy(TARGET BMRenderSandbox)