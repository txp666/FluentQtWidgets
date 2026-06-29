qt_add_resources(FQW_NAVIGATION_TAB_WIDGET_RESOURCES
    resource.qrc
)

add_executable(fqw_navigation_tab_widget_demo
    main.cpp
    ${FQW_NAVIGATION_TAB_WIDGET_RESOURCES}
)

target_link_libraries(fqw_navigation_tab_widget_demo PRIVATE FluentQtWidgets::Widgets)
target_compile_features(fqw_navigation_tab_widget_demo PRIVATE cxx_std_17)

set_target_properties(
    fqw_navigation_tab_widget_demo
    PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "NavigationTabWidgetDemo"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.fluentqtwidgets.examples.navigation.tabwidget"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

include(${CMAKE_SOURCE_DIR}/cmake/DeployQtTarget.cmake)
fqw_deploy_qt_executable(fqw_navigation_tab_widget_demo)
