add_executable(fqw_text_image_label_demo
    main.cpp
)

target_link_libraries(fqw_text_image_label_demo PRIVATE FluentQtWidgets::Widgets)
target_compile_features(fqw_text_image_label_demo PRIVATE cxx_std_17)
target_compile_definitions(fqw_text_image_label_demo PRIVATE
    FQW_EXAMPLE_RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resource"
)

set_target_properties(
    fqw_text_image_label_demo
    PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "TextImageLabelDemo"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.fluentqtwidgets.examples.text.imagelabel"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

include(${CMAKE_SOURCE_DIR}/cmake/DeployQtTarget.cmake)
fqw_deploy_qt_executable(fqw_text_image_label_demo)
