function(fqw_deploy_qt_executable target)
    get_target_property(_qt_qmake_location ${FQW_QT_PACKAGE}::qmake IMPORTED_LOCATION)
    if(_qt_qmake_location)
        get_filename_component(_qt_bin_dir "${_qt_qmake_location}" DIRECTORY)
    endif()

    if(WIN32 AND FQW_BUILD_SHARED)
        # A rule of its own that depends on the library, rather than a post-build step: that only
        # ran when the executable relinked, which a change inside the library does not cause (its
        # import library stays the same), so tests and examples kept loading the old DLL. One rule
        # per output directory; the tests share theirs.
        get_property(_fqw_runtime_target DIRECTORY PROPERTY FQW_RUNTIME_COPY_TARGET)
        if(NOT _fqw_runtime_target)
            get_property(_fqw_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
            set(_fqw_runtime_dir "${CMAKE_CURRENT_BINARY_DIR}")
            if(_fqw_multi_config)
                string(APPEND _fqw_runtime_dir "/$<CONFIG>")
            endif()
            set(_fqw_runtime_copy
                "${_fqw_runtime_dir}/${CMAKE_SHARED_LIBRARY_PREFIX}FluentQtWidgets${CMAKE_SHARED_LIBRARY_SUFFIX}")
            add_custom_command(
                OUTPUT "${_fqw_runtime_copy}"
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:FluentQtWidgets> "${_fqw_runtime_copy}"
                DEPENDS FluentQtWidgets
                COMMENT "Copy FluentQtWidgets runtime next to ${target}"
                VERBATIM
            )
            set(_fqw_runtime_target fqw_runtime_${target})
            add_custom_target(${_fqw_runtime_target} DEPENDS "${_fqw_runtime_copy}")
            set_property(DIRECTORY PROPERTY FQW_RUNTIME_COPY_TARGET ${_fqw_runtime_target})
        endif()
        add_dependencies(${target} ${_fqw_runtime_target})
    endif()

    if(WIN32)
        set(_fqw_qt_runtime_modules Core Gui Widgets Svg Network)
        if(FQW_HAS_MULTIMEDIA)
            list(APPEND _fqw_qt_runtime_modules Multimedia MultimediaWidgets)
        endif()

        foreach(_fqw_qt_module IN LISTS _fqw_qt_runtime_modules)
            if(TARGET ${FQW_QT_PACKAGE}::${_fqw_qt_module})
                add_custom_command(
                    TARGET ${target}
                    POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                            "$<TARGET_FILE:${FQW_QT_PACKAGE}::${_fqw_qt_module}>"
                            "$<TARGET_FILE_DIR:${target}>"
                    COMMENT "Copy Qt ${_fqw_qt_module} runtime next to ${target}"
                    VERBATIM
                )
            endif()
        endforeach()

        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

        if(WINDEPLOYQT_EXECUTABLE)
            add_custom_command(
                TARGET ${target}
                POST_BUILD
                COMMAND "${WINDEPLOYQT_EXECUTABLE}" --no-translations "$<TARGET_FILE:${target}>"
                COMMENT "Deploy Qt runtime for ${target}"
                VERBATIM
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
                VERBATIM
            )
        endif()

        find_program(MACDEPLOYQT_EXECUTABLE macdeployqt HINTS "${_qt_bin_dir}")
        if(MACDEPLOYQT_EXECUTABLE)
            add_custom_command(
                TARGET ${target}
                POST_BUILD
                COMMAND "${MACDEPLOYQT_EXECUTABLE}" "$<TARGET_BUNDLE_DIR:${target}>"
                COMMAND ${CMAKE_COMMAND}
                        -DAPP_BUNDLE="$<TARGET_BUNDLE_DIR:${target}>"
                        -P "${CMAKE_SOURCE_DIR}/cmake/FixMacWebEngineHelper.cmake"
                COMMENT "Deploy Qt frameworks for ${target}.app"
                VERBATIM
            )
        endif()
    endif()
endfunction()
