cmake_minimum_required(VERSION 3.17)
project(BMEdit)

set(CMAKE_CXX_STANDARD 20)

# --- Conan deps
if(EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
else()
    message(FATAL_ERROR "The file conanbuildinfo.cmake doesn't exist (lookup ${CMAKE_BINARY_DIR}), please, follow README.md \"Build\" for more details!")
endif()

# --- Global dependencies
add_subdirectory(ThirdParty/fmt)

# --- Project modules
add_subdirectory(BMEdit/Editor)
add_subdirectory(BMEdit/GameLib)

add_executable(BMEdit ${CMAKE_CURRENT_SOURCE_DIR}/BMEdit/BMEdit.cpp)
target_link_libraries(BMEdit PUBLIC Editor GameLib)

# --- Qt Deployment
get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
#find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")

if (WIN32)
    message(STATUS "Qt6 Deploy tool is: ${WINDEPLOYQT_EXECUTABLE}")

    add_custom_command(TARGET BMEdit POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E
            env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
            "$<TARGET_FILE:BMEdit>"
            COMMENT "Running windeployqt...")

    # Copy binary dependencies
    add_custom_command(TARGET BMEdit POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E copy "${CONAN_BIN_DIRS_LIBZIP}/zip.dll" "$<TARGET_FILE_DIR:BMEdit>"
            COMMENT "Copy conan dependencies to bin")
endif()
# TODO: Support other OS later