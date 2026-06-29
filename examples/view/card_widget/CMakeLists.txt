add_executable(fqw_view_card_widget_demo
    main.cpp
    resource/card_widget.qrc
)

target_link_libraries(fqw_view_card_widget_demo PRIVATE FluentQtWidgets::Widgets)
target_compile_features(fqw_view_card_widget_demo PRIVATE cxx_std_17)

set_target_properties(
    fqw_view_card_widget_demo
    PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "ViewCardWidgetDemo"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.fluentqtwidgets.examples.view.cardwidget"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

include(${CMAKE_SOURCE_DIR}/cmake/DeployQtTarget.cmake)
fqw_deploy_qt_executable(fqw_view_card_widget_demo)
