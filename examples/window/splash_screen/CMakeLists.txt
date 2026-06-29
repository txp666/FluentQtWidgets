add_executable(fqw_window_splash_screen_demo
    main.cpp
)

target_link_libraries(fqw_window_splash_screen_demo PRIVATE FluentQtWidgets::Widgets)
target_compile_features(fqw_window_splash_screen_demo PRIVATE cxx_std_17)

set_target_properties(
    fqw_window_splash_screen_demo
    PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "WindowSplashScreenDemo"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.fluentqtwidgets.examples.window.splashscreen"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

include(${CMAKE_SOURCE_DIR}/cmake/DeployQtTarget.cmake)
fqw_deploy_qt_executable(fqw_window_splash_screen_demo)
