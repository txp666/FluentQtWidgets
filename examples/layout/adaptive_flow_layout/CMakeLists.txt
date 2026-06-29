add_executable(fqw_layout_adaptive_flow_layout_demo
    main.cpp
)

target_link_libraries(fqw_layout_adaptive_flow_layout_demo PRIVATE FluentQtWidgets::Widgets)
target_compile_features(fqw_layout_adaptive_flow_layout_demo PRIVATE cxx_std_17)

set_target_properties(
    fqw_layout_adaptive_flow_layout_demo
    PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "LayoutAdaptiveFlowLayoutDemo"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.fluentqtwidgets.examples.layout.adaptiveflowlayout"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

include(${CMAKE_SOURCE_DIR}/cmake/DeployQtTarget.cmake)
fqw_deploy_qt_executable(fqw_layout_adaptive_flow_layout_demo)
