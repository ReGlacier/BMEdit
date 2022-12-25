macro(qt6_register_deploy)
    cmake_parse_arguments(
            BM_ARG
            "" # boolean
            "TARGET" # single
            "" # lists
            ${ARGN}
    )

    if (NOT BM_ARG_TARGET)
        message(FATAL_ERROR "TARGET is required")
        return()
    endif()

    if (WIN32)
        add_custom_command(TARGET ${BM_ARG_TARGET} POST_BUILD
                COMMAND "${CMAKE_COMMAND}" -E
                env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
                "$<TARGET_FILE:${BM_ARG_TARGET}>" -network
                COMMENT "Running windeployqt for ${BM_ARG_TARGET}...")
    else()
        message(FATAL_ERROR "Qt deploy is not supported on this platform")
    endif()
endmacro()