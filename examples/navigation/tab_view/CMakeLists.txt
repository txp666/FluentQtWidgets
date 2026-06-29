qt_add_resources(FQW_NAVIGATION_TAB_VIEW_RESOURCES
    resource.qrc
)

add_executable(fqw_navigation_tab_view_demo
    main.cpp
    ${FQW_NAVIGATION_TAB_VIEW_RESOURCES}
)

target_link_libraries(fqw_navigation_tab_view_demo PRIVATE FluentQtWidgets::Widgets)
target_compile_features(fqw_navigation_tab_view_demo PRIVATE cxx_std_17)

set_target_properties(
    fqw_navigation_tab_view_demo
    PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "NavigationTabViewDemo"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.fluentqtwidgets.examples.navigation.tabview"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

include(${CMAKE_SOURCE_DIR}/cmake/DeployQtTarget.cmake)
fqw_deploy_qt_executable(fqw_navigation_tab_view_demo)
