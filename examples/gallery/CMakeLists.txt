add_executable(FluentQtWidgetsGallery
    main.cpp
    GalleryWindow.h
    app/view/GalleryViewHelpers.h
    app/view/main_window.cpp
    app/view/basic_input_interface.cpp
    app/view/date_time_interface.cpp
    app/view/dialog_interface.cpp
    app/view/gallery_interface.cpp
    app/view/home_interface.cpp
    app/view/icon_interface.cpp
    app/view/layout_interface.cpp
    app/view/material_interface.cpp
    app/view/menu_interface.cpp
    app/view/navigation_view_interface.cpp
    app/view/scroll_interface.cpp
    app/view/setting_interface.cpp
    app/view/status_info_interface.cpp
    app/view/text_interface.cpp
    app/view/view_interface.cpp
    app/update/GalleryUpdateChecker.cpp
    GalleryTranslator.cpp
    GalleryTranslator.h
    common/SignalBus.cpp
    common/SignalBus.h
    common/Icon.h
    components/GalleryComponents.h
    resource/gallery.qrc
)

set(FQW_GALLERY_APP_ICON "${CMAKE_CURRENT_SOURCE_DIR}/resource/app_icon.icns")
set(FQW_GALLERY_APP_ICON_ICO "${CMAKE_CURRENT_SOURCE_DIR}/resource/app_icon.ico")

if(APPLE)
    target_sources(FluentQtWidgetsGallery PRIVATE "${FQW_GALLERY_APP_ICON}")
    set_source_files_properties("${FQW_GALLERY_APP_ICON}" PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
endif()

if(WIN32)
    configure_file(resource/app_icon.rc.in "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc" @ONLY)
    target_sources(FluentQtWidgetsGallery PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/app_icon.rc")
endif()

target_link_libraries(FluentQtWidgetsGallery PRIVATE FluentQtWidgets::Widgets)
target_compile_features(FluentQtWidgetsGallery PRIVATE cxx_std_17)

set_target_properties(
    FluentQtWidgetsGallery
    PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "FluentQtWidgetsGallery"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.fluentqtwidgets.gallery"
        MACOSX_BUNDLE_ICON_FILE "app_icon.icns"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

include(${CMAKE_SOURCE_DIR}/cmake/DeployQtTarget.cmake)
fqw_deploy_qt_executable(FluentQtWidgetsGallery)
