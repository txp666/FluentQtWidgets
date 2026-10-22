find_package(${FQW_QT_PACKAGE} REQUIRED COMPONENTS Test)

include(${CMAKE_SOURCE_DIR}/cmake/DeployQtTarget.cmake)

function(fqw_add_widget_test name)
    add_executable(${name} ${name}.cpp TestSupport.h)
    target_link_libraries(${name} PRIVATE FluentQtWidgets::Widgets ${FQW_QT_PACKAGE}::Test ${ARGN})
    fqw_deploy_qt_executable(${name})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

# One suite per area of the Gallery.
foreach(test IN ITEMS
        tst_theme tst_qss tst_config tst_basic_input tst_combo_box tst_text tst_status_info tst_dialog_flyout
        tst_menu tst_navigation tst_view tst_layout tst_settings tst_window tst_material tst_charts tst_datetime)
    fqw_add_widget_test(${test})
endforeach()

if(WIN32)
    # The native hit tests once more at 150%, where physical and device-independent pixels differ.
    add_test(NAME tst_window_scaled COMMAND tst_window
        framelessWindowInteriorHitTestStaysClientArea
        framelessWindowHitTestWithoutWindowUsesTheHostWindow
        framelessWindowMaximizeButtonUsesNativeHitTest
        framelessWindowMaximizeButtonReleaseDoesNotStick
        framelessWindowMaximizeButtonNativeClickTogglesWindow
        fluentWindowCaptionNativeDoubleClickTogglesWindow)
    set_tests_properties(tst_window_scaled PROPERTIES ENVIRONMENT "QT_SCALE_FACTOR=1.5")
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
