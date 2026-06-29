function(fqw_deploy_qt_executable target)
    get_target_property(_qt_qmake_location ${FQW_QT_PACKAGE}::qmake IMPORTED_LOCATION)
    if(_qt_qmake_location)
        get_filename_component(_qt_bin_dir "${_qt_qmake_location}" DIRECTORY)
    endif()

    if(WIN32 AND FQW_BUILD_SHARED)
        add_custom_command(
            TARGET ${target}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    $<TARGET_FILE:FluentQtWidgets>
                    $<TARGET_FILE_DIR:${target}>
            COMMENT "Copy FluentQtWidgets runtime next to ${target}"
        )
    endif()

    if(WIN32)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

        if(WINDEPLOYQT_EXECUTABLE)
            add_custom_command(
                TARGET ${target}
                POST_BUILD
                COMMAND "${WINDEPLOYQT_EXECUTABLE}" --no-translations "$<TARGET_FILE:${target}>"
                COMMENT "Deploy Qt runtime for ${target}"
            )
        endif()
        return()
    endif()

    if(APPLE)
        if(NOT "${target}" STREQUAL "FluentQtWidgetsGallery")
            return()
        endif()

        get_target_property(_is_macos_bundle ${target} MACOSX_BUNDLE)
        if(NOT _is_macos_bundle)
            return()
        endif()

        set_property(TARGET ${target} APPEND PROPERTY BUILD_RPATH "@executable_path/../Frameworks")

        if(FQW_BUILD_SHARED)
            add_custom_command(
                TARGET ${target}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:${target}>/Contents/Frameworks"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "$<TARGET_FILE:FluentQtWidgets>"
                        "$<TARGET_BUNDLE_DIR:${target}>/Contents/Frameworks/$<TARGET_SONAME_FILE_NAME:FluentQtWidgets>"
                COMMENT "Copy FluentQtWidgets runtime into ${target}.app"
            )
        endif()

        find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")
        if(MACDEPLOYQT_EXECUTABLE)
            add_custom_command(
                TARGET ${target}
                POST_BUILD
                COMMAND "${MACDEPLOYQT_EXECUTABLE}" "$<TARGET_BUNDLE_DIR:${target}>"
                COMMENT "Deploy Qt frameworks for ${target}.app"
            )
        endif()
    endif()
endfunction()
