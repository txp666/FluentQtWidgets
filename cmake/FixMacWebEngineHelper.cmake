if(NOT DEFINED APP_BUNDLE OR APP_BUNDLE STREQUAL "")
    message(FATAL_ERROR "APP_BUNDLE is required")
endif()

set(_app_frameworks "${APP_BUNDLE}/Contents/Frameworks")
set(_helper_app
    "${_app_frameworks}/QtWebEngineCore.framework/Versions/A/Helpers/QtWebEngineProcess.app")
set(_helper_frameworks "${_helper_app}/Contents/Frameworks")

if(NOT EXISTS "${_helper_app}")
    return()
endif()

file(MAKE_DIRECTORY "${_helper_frameworks}")
file(GLOB _qt_frameworks LIST_DIRECTORIES true "${_app_frameworks}/Qt*.framework")

foreach(_framework IN LISTS _qt_frameworks)
    get_filename_component(_framework_name "${_framework}" NAME)
    set(_helper_link "${_helper_frameworks}/${_framework_name}")

    if(EXISTS "${_helper_link}" OR IS_SYMLINK "${_helper_link}")
        file(REMOVE_RECURSE "${_helper_link}")
    endif()

    file(RELATIVE_PATH _relative_framework "${_helper_frameworks}" "${_framework}")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E create_symlink "${_relative_framework}" "${_helper_link}"
        RESULT_VARIABLE _symlink_result)
    if(NOT _symlink_result EQUAL 0)
        message(FATAL_ERROR "Failed to create ${_helper_link} -> ${_relative_framework}")
    endif()
endforeach()

file(GLOB _framework_dylibs "${_app_frameworks}/*.dylib")
foreach(_dylib IN LISTS _framework_dylibs)
    get_filename_component(_dylib_name "${_dylib}" NAME)
    set(_helper_link "${_helper_frameworks}/${_dylib_name}")

    if(EXISTS "${_helper_link}" OR IS_SYMLINK "${_helper_link}")
        file(REMOVE_RECURSE "${_helper_link}")
    endif()

    file(RELATIVE_PATH _relative_dylib "${_helper_frameworks}" "${_dylib}")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E create_symlink "${_relative_dylib}" "${_helper_link}"
        RESULT_VARIABLE _symlink_result)
    if(NOT _symlink_result EQUAL 0)
        message(FATAL_ERROR "Failed to create ${_helper_link} -> ${_relative_dylib}")
    endif()
endforeach()

set(_helper_executable "${_helper_app}/Contents/MacOS/QtWebEngineProcess")
if(EXISTS "${_helper_executable}")
    find_program(_install_name_tool install_name_tool)
    if(_install_name_tool)
        execute_process(
            COMMAND otool -L "${_helper_executable}"
            OUTPUT_VARIABLE _helper_dependencies
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(REPLACE "\n" ";" _helper_dependency_lines "${_helper_dependencies}")

        foreach(_line IN LISTS _helper_dependency_lines)
            string(STRIP "${_line}" _dependency)

            if(_dependency MATCHES "^(@rpath/(Qt[^/]+\\.framework/Versions/A/[^ ]+)) ")
                set(_old_name "${CMAKE_MATCH_1}")
                set(_new_name "@executable_path/../Frameworks/${CMAKE_MATCH_2}")
            elseif(_dependency MATCHES "^(/[^ ]+/(Qt[^/]+\\.framework)/Versions/A/([^ /]+)) ")
                set(_old_name "${CMAKE_MATCH_1}")
                set(_new_name "@executable_path/../Frameworks/${CMAKE_MATCH_2}/Versions/A/${CMAKE_MATCH_3}")
            elseif(_dependency MATCHES "^(/[^ ]+/([^/]+\\.dylib)) ")
                set(_old_name "${CMAKE_MATCH_1}")
                set(_dylib_name "${CMAKE_MATCH_2}")
                if(EXISTS "${_app_frameworks}/${_dylib_name}")
                    set(_new_name "@executable_path/../Frameworks/${_dylib_name}")
                else()
                    set(_new_name "")
                endif()
            else()
                set(_old_name "")
                set(_new_name "")
            endif()

            if(NOT _old_name STREQUAL "" AND NOT _new_name STREQUAL "")
                execute_process(
                    COMMAND "${_install_name_tool}" -change "${_old_name}" "${_new_name}" "${_helper_executable}"
                    RESULT_VARIABLE _install_name_result)
                if(NOT _install_name_result EQUAL 0)
                    message(FATAL_ERROR "Failed to change ${_old_name} to ${_new_name} in ${_helper_executable}")
                endif()
            endif()
        endforeach()
    endif()
endif()

message(STATUS "Fixed Qt WebEngine helper framework links in ${_helper_app}")
