find_package(${FQW_QT_PACKAGE} REQUIRED COMPONENTS Test)

include(${CMAKE_SOURCE_DIR}/cmake/DeployQtTarget.cmake)

function(fqw_add_widget_test name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE FluentQtWidgets::Widgets ${FQW_QT_PACKAGE}::Test)
    fqw_deploy_qt_executable(${name})
    add_test(NAME ${name} COMMAND ${name})
endfunction()

fqw_add_widget_test(tst_theme tst_theme.cpp)
fqw_add_widget_test(tst_pips_pager tst_pips_pager.cpp)
fqw_add_widget_test(tst_feedback tst_feedback.cpp)
fqw_add_widget_test(tst_display tst_display.cpp)
fqw_add_widget_test(tst_shell tst_shell.cpp)
fqw_add_widget_test(tst_datetime tst_datetime.cpp)
fqw_add_widget_test(tst_config_item tst_config_item.cpp)
fqw_add_widget_test(tst_basic_input_parity tst_basic_input_parity.cpp)

add_executable(tst_gallery_interface
    tst_gallery_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/components/GalleryComponents.h
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/gallery_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/app/view/home_interface.cpp
    ${CMAKE_SOURCE_DIR}/examples/gallery/resource/gallery.qrc
)
target_include_directories(tst_gallery_interface PRIVATE ${CMAKE_SOURCE_DIR}/examples/gallery)
target_link_libraries(tst_gallery_interface PRIVATE FluentQtWidgets::Widgets ${FQW_QT_PACKAGE}::Test)
fqw_deploy_qt_executable(tst_gallery_interface)
add_test(NAME tst_gallery_interface COMMAND tst_gallery_interface)

add_executable(tst_gallery_translation
    tst_gallery_translation.cpp
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
target_include_directories(tst_gallery_translation PRIVATE ${CMAKE_SOURCE_DIR}/examples/gallery)
target_link_libraries(tst_gallery_translation PRIVATE FluentQtWidgets::Widgets ${FQW_QT_PACKAGE}::Test)
fqw_deploy_qt_executable(tst_gallery_translation)
add_test(NAME tst_gallery_translation COMMAND tst_gallery_translation)
