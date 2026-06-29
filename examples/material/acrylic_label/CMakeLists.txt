qt_add_resources(FQW_MATERIAL_ACRYLIC_LABEL_RESOURCES
    resource.qrc
)

add_executable(fqw_material_acrylic_label_demo
    main.cpp
    ${FQW_MATERIAL_ACRYLIC_LABEL_RESOURCES}
)

target_link_libraries(fqw_material_acrylic_label_demo PRIVATE FluentQtWidgets::Widgets)
target_compile_features(fqw_material_acrylic_label_demo PRIVATE cxx_std_17)

set_target_properties(
    fqw_material_acrylic_label_demo
    PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "MaterialAcrylicLabelDemo"
        MACOSX_BUNDLE_GUI_IDENTIFIER "org.fluentqtwidgets.examples.material.acryliclabel"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
)

include(${CMAKE_SOURCE_DIR}/cmake/DeployQtTarget.cmake)
fqw_deploy_qt_executable(fqw_material_acrylic_label_demo)
