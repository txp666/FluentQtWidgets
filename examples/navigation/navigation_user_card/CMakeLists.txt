qt_add_resources(FQW_NAVIGATION_USER_CARD_RESOURCES
    resource.qrc
)

add_executable(fqw_navigation_navigation_user_card_demo
    main.cpp
    ${FQW_NAVIGATION_USER_CARD_RESOURCES}
)

target_link_libraries(fqw_navigation_navigation_user_card_demo PRIVATE FluentQtWidgets::Widgets)
target_compile_features(fqw_navigation_navigation_user_card_demo PRIVATE cxx_std_17)

set_target_properties(
    fqw_navigation_navigation_user_card_demo
    PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "NavigationUserCardDemo"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.fluentqtwidgets.examples.navigation.usercard"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

include(${CMAKE_SOURCE_DIR}/cmake/DeployQtTarget.cmake)
fqw_deploy_qt_executable(fqw_navigation_navigation_user_card_demo)
