cmake_minimum_required(VERSION 3.17)
project(BMEdit)

set(CMAKE_CXX_STANDARD 20)

# --- Global dependencies
add_subdirectory(ThirdParty/zlib)
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
endif()
# TODO: Support other OS later