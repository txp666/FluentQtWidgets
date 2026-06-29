cmake_minimum_required(VERSION 3.21)

set(_fqw_default_repository_url "https://github.com/txp666/FluentQtWidgets")

if(NOT DEFINED FQW_REPOSITORY_URL OR FQW_REPOSITORY_URL STREQUAL "")
    set(_fqw_repo_url "")

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
        execute_process(
            COMMAND git -C "${CMAKE_CURRENT_SOURCE_DIR}" remote get-url origin
            OUTPUT_VARIABLE _fqw_git_url
            ERROR_QUIET
            OUTPUT_STRIP_TRAILING_WHITESPACE)

        if(_fqw_git_url)
            if(_fqw_git_url MATCHES "^git@([^:]+):(.+)$")
                set(_fqw_git_url "https://${CMAKE_MATCH_1}/${CMAKE_MATCH_2}")
            endif()
            string(REGEX REPLACE "\\.git$" "" _fqw_repo_url "${_fqw_git_url}")
        endif()
    endif()

    if(_fqw_repo_url STREQUAL "")
        set(_fqw_repo_url "${_fqw_default_repository_url}")
    endif()

    set(FQW_REPOSITORY_URL "${_fqw_repo_url}" CACHE STRING "Repository URL used by in-app links")
else()
    string(REGEX REPLACE "\\.git$" "" FQW_REPOSITORY_URL "${FQW_REPOSITORY_URL}")
endif()

project(
    FluentQtWidgets
    VERSION 0.1.4
    DESCRIPTION "A Fluent Design inspired QWidget library for C++/Qt"
    HOMEPAGE_URL "${FQW_REPOSITORY_URL}"
    LANGUAGES CXX
)

option(FQW_BUILD_SHARED "Build FluentQtWidgets as a shared library" ON)
option(FQW_BUILD_EXAMPLES "Build examples" ON)
option(FQW_BUILD_TESTS "Build tests" OFF)
set(FQW_QT_MAJOR_VERSION "6" CACHE STRING "Qt major version to use")
set_property(CACHE FQW_QT_MAJOR_VERSION PROPERTY STRINGS 6 5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(FQW_QT_MAJOR_VERSION STREQUAL "6")
    find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Svg Network)
    find_package(Qt6 6.5 QUIET COMPONENTS Multimedia MultimediaWidgets WebEngineWidgets)
    set(FQW_QT_PACKAGE Qt6)
elseif(FQW_QT_MAJOR_VERSION STREQUAL "5")
    find_package(Qt5 5.15 REQUIRED COMPONENTS Core Gui Widgets Svg Network)
    set(FQW_QT_PACKAGE Qt5)
else()
    message(FATAL_ERROR "FQW_QT_MAJOR_VERSION must be 6 or 5")
endif()

set(FQW_HAS_MULTIMEDIA OFF)
if(FQW_QT_MAJOR_VERSION STREQUAL "6" AND TARGET Qt6::Multimedia AND TARGET Qt6::MultimediaWidgets)
    set(FQW_HAS_MULTIMEDIA ON)
endif()
set(FQW_HAS_MULTIMEDIA "${FQW_HAS_MULTIMEDIA}" CACHE BOOL "Whether Qt Multimedia support is available" FORCE)

set(FQW_HAS_WEBENGINE_WIDGETS OFF)
if(FQW_QT_MAJOR_VERSION STREQUAL "6" AND TARGET Qt6::WebEngineWidgets)
    set(FQW_HAS_WEBENGINE_WIDGETS ON)
endif()
set(FQW_HAS_WEBENGINE_WIDGETS "${FQW_HAS_WEBENGINE_WIDGETS}" CACHE BOOL "Whether Qt WebEngineWidgets support is available" FORCE)

set(FQW_PUBLIC_HEADERS
    include/FluentQtWidgets/FluentQtWidgets.h
    include/FluentQtWidgets/Global.h
    include/FluentQtWidgets/Color.h
    include/FluentQtWidgets/Config.h
    include/FluentQtWidgets/ConfigItem.h
    include/FluentQtWidgets/DateTime/CalendarPicker.h
    include/FluentQtWidgets/DateTime/DatePicker.h
    include/FluentQtWidgets/DateTime/PickerBase.h
    include/FluentQtWidgets/DateTime/TimePicker.h
    include/FluentQtWidgets/Theme.h
    include/FluentQtWidgets/Version.h
    include/FluentQtWidgets/ThemeListener.h
    include/FluentQtWidgets/StyleSheet.h
    include/FluentQtWidgets/Translator.h
    include/FluentQtWidgets/FluentIcon.h
    include/FluentQtWidgets/Layout/FlowLayout.h
    include/FluentQtWidgets/Layout/VBoxLayout.h
    include/FluentQtWidgets/Multimedia/MediaPlayer.h
    include/FluentQtWidgets/Dialogs/ColorDialog.h
    include/FluentQtWidgets/Dialogs/Dialog.h
    include/FluentQtWidgets/Dialogs/FolderListDialog.h
    include/FluentQtWidgets/Navigation/BreadcrumbBar.h
    include/FluentQtWidgets/Navigation/NavigationBar.h
    include/FluentQtWidgets/Navigation/NavigationHistory.h
    include/FluentQtWidgets/Navigation/NavigationInterface.h
    include/FluentQtWidgets/Navigation/NavigationPanel.h
    include/FluentQtWidgets/Navigation/NavigationWidget.h
    include/FluentQtWidgets/Navigation/NavigationTreeWidget.h
    include/FluentQtWidgets/Navigation/Pivot.h
    include/FluentQtWidgets/Navigation/SegmentedWidget.h
    include/FluentQtWidgets/Settings/SettingCard.h
    include/FluentQtWidgets/Views/ItemViews.h
    include/FluentQtWidgets/Widgets/AcrylicLabel.h
    include/FluentQtWidgets/Widgets/Button.h
    include/FluentQtWidgets/Widgets/CardWidget.h
    include/FluentQtWidgets/Widgets/CommandBar.h
    include/FluentQtWidgets/Widgets/ComboBox.h
    include/FluentQtWidgets/Widgets/CycleListWidget.h
    include/FluentQtWidgets/Widgets/Flyout.h
    include/FluentQtWidgets/Widgets/FlipView.h
    include/FluentQtWidgets/Widgets/IconWidget.h
    include/FluentQtWidgets/Widgets/InfoBadge.h
    include/FluentQtWidgets/Widgets/InfoBadgeManager.h
    include/FluentQtWidgets/Widgets/InfoBar.h
    include/FluentQtWidgets/Widgets/InfoBarManager.h
    include/FluentQtWidgets/Widgets/Label.h
    include/FluentQtWidgets/Widgets/LineEdit.h
    include/FluentQtWidgets/Widgets/Menu.h
    include/FluentQtWidgets/Widgets/PipsPager.h
    include/FluentQtWidgets/Widgets/ProgressBar.h
    include/FluentQtWidgets/Widgets/ScrollArea.h
    include/FluentQtWidgets/Widgets/Separator.h
    include/FluentQtWidgets/Widgets/SmoothScrollDelegate.h
    include/FluentQtWidgets/Widgets/Selection.h
    include/FluentQtWidgets/Widgets/Slider.h
    include/FluentQtWidgets/Widgets/SpinBox.h
    include/FluentQtWidgets/Widgets/StackedWidget.h
    include/FluentQtWidgets/Widgets/StateToolTip.h
    include/FluentQtWidgets/Widgets/SwitchButton.h
    include/FluentQtWidgets/Widgets/TabView.h
    include/FluentQtWidgets/Widgets/TeachingTip.h
    include/FluentQtWidgets/Widgets/ToolTip.h
    include/FluentQtWidgets/Window/FluentTitleBar.h
    include/FluentQtWidgets/Window/FluentWindow.h
    include/FluentQtWidgets/Window/FramelessWindowHelper.h
    include/FluentQtWidgets/Window/SplashScreen.h
    include/FluentQtWidgets/Repository.h
)

set(FQW_SOURCES
    src/core/Color.cpp
    src/core/Config.cpp
    src/core/ConfigItem.cpp
    src/core/FluentIcon.cpp
    src/core/StyleSheet.cpp
    src/core/Theme.cpp
    src/core/ThemeListener.cpp
    src/core/Translator.cpp
    src/datetime/CalendarPicker.cpp
    src/datetime/DatePicker.cpp
    src/datetime/PickerBase.cpp
    src/datetime/TimePicker.cpp
    src/dialogs/ColorDialog.cpp
    src/dialogs/Dialog.cpp
    src/dialogs/FolderListDialog.cpp
    src/layout/FlowLayout.cpp
    src/layout/VBoxLayout.cpp
    src/multimedia/MediaPlayer.cpp
    src/navigation/BreadcrumbBar.cpp
    src/navigation/NavigationBar.cpp
    src/navigation/NavigationHistory.cpp
    src/navigation/NavigationInterface.cpp
    src/navigation/NavigationPanel.cpp
    src/navigation/NavigationTreeWidget.cpp
    src/navigation/Pivot.cpp
    src/navigation/SegmentedWidget.cpp
    src/settings/SettingCard.cpp
    src/views/ItemViews.cpp
    src/widgets/AcrylicLabel.cpp
    src/widgets/Button.cpp
    src/widgets/CardWidget.cpp
    src/widgets/CommandBar.cpp
    src/widgets/ComboBox.cpp
    src/widgets/CycleListWidget.cpp
    src/widgets/Flyout.cpp
    src/widgets/FlipView.cpp
    src/widgets/IconWidget.cpp
    src/widgets/InfoBadge.cpp
    src/widgets/InfoBadgeManager.cpp
    src/widgets/InfoBar.cpp
    src/widgets/InfoBarManager.cpp
    src/widgets/Label.cpp
    src/widgets/LineEdit.cpp
    src/widgets/Menu.cpp
    src/widgets/PipsPager.cpp
    src/widgets/ProgressBar.cpp
    src/widgets/ScrollArea.cpp
    src/widgets/Separator.cpp
    src/widgets/SmoothScrollDelegate.cpp
    src/widgets/Selection.cpp
    src/widgets/Slider.cpp
    src/widgets/SpinBox.cpp
    src/widgets/StackedWidget.cpp
    src/widgets/StateToolTip.cpp
    src/widgets/SwitchButton.cpp
    src/widgets/TabView.cpp
    src/widgets/TeachingTip.cpp
    src/widgets/ToolTip.cpp
    src/window/FluentTitleBar.cpp
    src/window/FluentWindow.cpp
    src/window/FramelessWindowHelper.cpp
    src/window/SplashScreen.cpp
    resources/fluentqtwidgets.qrc
)

if(NOT FQW_HAS_MULTIMEDIA)
    list(REMOVE_ITEM FQW_PUBLIC_HEADERS include/FluentQtWidgets/Multimedia/MediaPlayer.h)
    list(REMOVE_ITEM FQW_SOURCES src/multimedia/MediaPlayer.cpp)
endif()

set(FQW_LIBRARY_TYPE STATIC)
if(FQW_BUILD_SHARED)
    set(FQW_LIBRARY_TYPE SHARED)
endif()

add_library(FluentQtWidgets ${FQW_LIBRARY_TYPE} ${FQW_PUBLIC_HEADERS} ${FQW_SOURCES})
add_library(FluentQtWidgets::FluentQtWidgets ALIAS FluentQtWidgets)
add_library(FluentQtWidgets::Widgets ALIAS FluentQtWidgets)

target_compile_definitions(FluentQtWidgets PRIVATE FQW_BUILD_LIBRARY)
if(NOT FQW_BUILD_SHARED)
    target_compile_definitions(FluentQtWidgets PUBLIC FQW_STATIC)
endif()
if(FQW_HAS_MULTIMEDIA)
    target_compile_definitions(FluentQtWidgets PUBLIC FQW_HAS_MULTIMEDIA=1)
endif()
if(FQW_HAS_WEBENGINE_WIDGETS)
    target_compile_definitions(FluentQtWidgets PUBLIC FQW_HAS_WEBENGINE_WIDGETS=1)
endif()

target_include_directories(
    FluentQtWidgets
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(
    FluentQtWidgets
    PUBLIC
        ${FQW_QT_PACKAGE}::Core
        ${FQW_QT_PACKAGE}::Gui
        ${FQW_QT_PACKAGE}::Widgets
        ${FQW_QT_PACKAGE}::Svg
        ${FQW_QT_PACKAGE}::Network
)
if(WIN32)
    target_link_libraries(FluentQtWidgets PUBLIC dwmapi)
endif()
if(FQW_HAS_MULTIMEDIA)
    target_link_libraries(
        FluentQtWidgets
        PUBLIC
            ${FQW_QT_PACKAGE}::Multimedia
            ${FQW_QT_PACKAGE}::MultimediaWidgets
    )
endif()

target_compile_features(FluentQtWidgets PUBLIC cxx_std_17)
target_compile_definitions(
    FluentQtWidgets
    PUBLIC
        FQW_REPOSITORY_URL="${FQW_REPOSITORY_URL}"
        FQW_VERSION="${PROJECT_VERSION}"
)

set_target_properties(
    FluentQtWidgets
    PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        EXPORT_NAME FluentQtWidgets
)

install(
    TARGETS FluentQtWidgets
    EXPORT FluentQtWidgetsTargets
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/FluentQtWidgetsConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    cmake/FluentQtWidgetsConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/FluentQtWidgetsConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FluentQtWidgets
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/FluentQtWidgetsConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/FluentQtWidgetsConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FluentQtWidgets
)

install(
    EXPORT FluentQtWidgetsTargets
    NAMESPACE FluentQtWidgets::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/FluentQtWidgets
)

if(FQW_BUILD_EXAMPLES)
    add_subdirectory(examples/gallery)
    add_subdirectory(examples/basic_input)
    add_subdirectory(examples/date_time)
    add_subdirectory(examples/dialog_flyout)
    add_subdirectory(examples/layout)
    add_subdirectory(examples/material)
    add_subdirectory(examples/media)
    add_subdirectory(examples/menu)
    add_subdirectory(examples/navigation)
    add_subdirectory(examples/scroll)
    add_subdirectory(examples/status_info)
    add_subdirectory(examples/text)
    add_subdirectory(examples/view)
    add_subdirectory(examples/window)
endif()

if(FQW_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
