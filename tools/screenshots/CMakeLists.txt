# The README screenshots and the Gallery tour GIF; see README.md here.
if(NOT FQW_QT_MAJOR_VERSION STREQUAL "6")
    message(FATAL_ERROR "FQW_BUILD_SCREENSHOTS needs Qt 6")
endif()

set(_fqw_gallery "${CMAKE_SOURCE_DIR}/examples/gallery")
file(GLOB _fqw_gallery_views CONFIGURE_DEPENDS "${_fqw_gallery}/app/view/*.cpp")

add_executable(galleryshots
    galleryshots.cpp
    shotutil.h
    ${_fqw_gallery_views}
    ${_fqw_gallery}/GalleryWindow.h
    ${_fqw_gallery}/GalleryTranslator.cpp
    ${_fqw_gallery}/app/update/GalleryUpdateChecker.cpp
    ${_fqw_gallery}/common/SignalBus.cpp
    ${_fqw_gallery}/common/SignalBus.h
    ${_fqw_gallery}/components/GalleryComponents.h
    ${_fqw_gallery}/resource/gallery.qrc
)
add_executable(showcase showcase.cpp shotutil.h ${_fqw_gallery}/GalleryTranslator.cpp ${_fqw_gallery}/resource/gallery.qrc)

foreach(_fqw_target galleryshots showcase)
    target_include_directories(${_fqw_target} PRIVATE "${_fqw_gallery}")
    target_link_libraries(${_fqw_target} PRIVATE FluentQtWidgets::Widgets)
    target_compile_features(${_fqw_target} PRIVATE cxx_std_17)
endforeach()

include(${CMAKE_SOURCE_DIR}/cmake/DeployQtTarget.cmake)
fqw_deploy_qt_executable(galleryshots)
fqw_deploy_qt_executable(showcase)
