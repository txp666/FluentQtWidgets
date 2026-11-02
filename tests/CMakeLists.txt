find_package(${FQW_QT_PACKAGE} REQUIRED COMPONENTS Test)

include(${CMAKE_SOURCE_DIR}/cmake/DeployQtTarget.cmake)

# Labels (see README.md): each suite's area, "smoke" for the fast whole-library checks, "native" for the
# runs that only mean something on a real windowing system.
function(fqw_add_widget_test name)
    add_executable(${name} ${name}.cpp TestSupport.h)
    target_link_libraries(${name} PRIVATE FluentQtWidgets::Widgets ${FQW_QT_PACKAGE}::Test ${ARGN})
    fqw_deploy_qt_executable(${name})
    add_test(NAME ${name} COMMAND ${name})
    string(REGEX REPLACE "^tst_" "" area ${name})
    set_tests_properties(${name} PROPERTIES LABELS ${area})
endfunction()

# One suite per area of the Gallery.
foreach(test IN ITEMS
        tst_theme tst_qss tst_config tst_icon tst_basic_input tst_combo_box tst_text tst_status_info
        tst_dialog_flyout tst_menu tst_navigation tst_view tst_scroll tst_layout tst_settings tst_window
        tst_material tst_charts tst_datetime tst_fonts)
    fqw_add_widget_test(${test})
endforeach()

# The tests that need window activation, a movable pointer or Windows itself, once more under their own
# name: offscreen they skip, so "ctest -L native" on a desktop is the run that exercises them.
add_test(NAME tst_text_native COMMAND tst_text multilineFocusLineSpansTheBox)
add_test(NAME tst_status_info_native COMMAND tst_status_info toolTipLeavesTheActiveWindowAndFocusAlone)
add_test(NAME tst_navigation_native COMMAND tst_navigation navigationTreeParentHoverCoversOnlyItsOwnRow)
set_tests_properties(tst_text_native PROPERTIES LABELS "text;native")
set_tests_properties(tst_status_info_native PROPERTIES LABELS "status_info;native")
set_tests_properties(tst_navigation_native PROPERTIES LABELS "navigation;native")

# The media play bars and the video widget, only built with Qt Multimedia. The backend plugins may live in
# their own prefix (a Qt Multimedia built apart from Qt itself): QT_PLUGIN_PATH adds their directory.
if(FQW_HAS_MULTIMEDIA)
    fqw_add_widget_test(tst_media ${FQW_QT_PACKAGE}::Multimedia ${FQW_QT_PACKAGE}::MultimediaWidgets)
    set(fqw_media_plugin_dirs "")
    foreach(plugin IN ITEMS QDarwinMediaPlugin QFFmpegMediaPlugin QGstreamerMediaPlugin QWindowsMediaPlugin)
        if(TARGET ${FQW_QT_PACKAGE}::${plugin})
            list(APPEND fqw_media_plugin_dirs "$<TARGET_FILE_DIR:${FQW_QT_PACKAGE}::${plugin}>/..")
        endif()
    endforeach()
    if(fqw_media_plugin_dirs)
        string(REPLACE ";" "$<SEMICOLON>" fqw_media_plugin_dirs "${fqw_media_plugin_dirs}")
        set_tests_properties(tst_media PROPERTIES ENVIRONMENT "QT_PLUGIN_PATH=${fqw_media_plugin_dirs}")
    endif()
    # Playing a real file needs the platform's media framework, which on macOS only reports through Cocoa.
    add_test(NAME tst_media_native COMMAND tst_media standardPlayBarPlaysAWavFile volumeButtonPullsUpItsFlyout)
    set_tests_properties(tst_media_native PROPERTIES LABELS "media;native")
    if(fqw_media_plugin_dirs)
        set_tests_properties(tst_media_native PROPERTIES ENVIRONMENT "QT_PLUGIN_PATH=${fqw_media_plugin_dirs}")
    endif()
endif()

# Sizes against PyQt-Fluent-Widgets 1.11.3, measured by tools/parity/measure_python.py.
fqw_add_widget_test(tst_python_parity)
target_compile_definitions(tst_python_parity PRIVATE FQW_TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")
set_tests_properties(tst_python_parity PROPERTIES LABELS "python_parity;smoke")
set_tests_properties(tst_qss PROPERTIES LABELS "qss;smoke")

if(WIN32)
    # The native hit tests once more at 150%, where physical and device-independent pixels differ.
    add_test(NAME tst_window_scaled COMMAND tst_window
        framelessWindowInteriorHitTestStaysClientArea
        framelessWindowHitTestWithoutWindowUsesTheHostWindow
        framelessWindowMaximizeButtonUsesNativeHitTest
        framelessWindowMaximizeButtonReleaseDoesNotStick
        framelessWindowMaximizeButtonNativeClickTogglesWindow
        fluentWindowCaptionNativeDoubleClickTogglesWindow)
    set_tests_properties(tst_window_scaled PROPERTIES ENVIRONMENT "QT_SCALE_FACTOR=1.5" LABELS "window;native")
endif()

# The Gallery's pages, compiled once for the two suites that open them.
add_library(fqw_gallery_for_tests OBJECT
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/update/GalleryUpdateChecker.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/GalleryWindow.h
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/GalleryViewHelpers.h
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/main_window.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/basic_input_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/chart_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/date_time_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/dialog_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/gallery_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/home_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/icon_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/layout_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/material_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/menu_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/navigation_view_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/scroll_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/setting_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/status_info_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/text_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/view_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/GalleryTranslator.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/common/SignalBus.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/components/GalleryComponents.h
    ${CMAKE_SOURCE_DIR}/examples/gallery/resource/gallery.qrc
)
target_include_directories(fqw_gallery_for_tests PUBLIC ${CMAKE_SOURCE_DIR}/examples/gallery)
target_link_libraries(fqw_gallery_for_tests PUBLIC FluentQtWidgets::Widgets)

fqw_add_widget_test(tst_gallery_pages fqw_gallery_for_tests)
fqw_add_widget_test(tst_gallery_translation fqw_gallery_for_tests)
set_tests_properties(tst_gallery_pages PROPERTIES LABELS "gallery;smoke")
set_tests_properties(tst_gallery_translation PROPERTIES LABELS "gallery")
