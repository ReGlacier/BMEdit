cmake_minimum_required(VERSION 3.17)
project(BMEdit)

set(CMAKE_CXX_STANDARD 20)

# --- Deps
find_package(ZLIB REQUIRED HINTS ${CMAKE_BINARY_DIR})
find_package(libzip REQUIRED HINTS ${CMAKE_BINARY_DIR})
find_package(libsquish REQUIRED HINTS ${CMAKE_BINARY_DIR})

# --- Global dependencies
add_subdirectory(ThirdParty/fmt)

# --- Project modules
add_subdirectory(BMEdit/Editor)
add_subdirectory(BMEdit/GameLib)

# --- OS Specific things
set(BMEDIT_RC_FILE)
set(BMEDIT_OS_TYPE)
if (WIN32)
    set(BMEDIT_RC_FILE ${CMAKE_CURRENT_SOURCE_DIR}/Resources/BMEdit.rc)
    set(BMEDIT_OS_TYPE WIN32)
endif()

# --- Executable
add_executable(BMEdit ${BMEDIT_OS_TYPE} ${CMAKE_CURRENT_SOURCE_DIR}/BMEdit/BMEdit.cpp ${BMEDIT_RC_FILE})
target_link_libraries(BMEdit PUBLIC Editor GameLib)

# --- Qt Deployment
get_target_property(_qmake_executable Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qmake_executable}" DIRECTORY)
set(WINDEPLOYQT_EXECUTABLE "${_qt_bin_dir}/windeployqt.exe")

if (WIN32)
    message(STATUS "Qt6 Deploy tool is: ${WINDEPLOYQT_EXECUTABLE}")

    add_custom_command(TARGET BMEdit POST_BUILD
            COMMAND "${CMAKE_COMMAND}" -E
            env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
            "$<TARGET_FILE:BMEdit>" --no-translations -network
            COMMENT "Running windeployqt...")
endif()
# TODO: Support other OS later