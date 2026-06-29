# Quick Start

## Configure

Use the preset that matches your local Qt kit. On macOS/Linux the default debug
preset uses `build/debug`; on Windows MinGW the checked-in preset uses
`build/mingw`.

```powershell
cmake --preset ninja-debug
```

On Windows with Qt MinGW, make the matching Qt kit, MinGW compiler, and Ninja
visible before configuring:

```powershell
$env:CMAKE_PREFIX_PATH = "<path-to-qt-mingw-kit>"
$env:Path = "<path-to-mingw-bin>;<path-to-ninja-dir>;<path-to-qt-bin>;$env:Path"
cmake --preset mingw-debug
```

## Build

```powershell
cmake --build --preset debug --parallel
# Windows MinGW:
cmake --build --preset mingw-debug --parallel
```

## One-Click Demo Entries

Regenerate `.vscode/launch.json`, `.vscode/tasks.json`, and `examples/examples.json` after example changes:

```powershell
python3 scripts/generate_vscode_examples.py
```

`examples/examples.json` includes a compact manifest with every demo target, platform-specific build/run commands, optional feature flags (`FQW_HAS_MULTIMEDIA`, `FQW_HAS_WEBENGINE_WIDGETS`), and an `enabled` state derived from the current `build/CMakeCache.txt` when present.

Run one entry directly from the manifest:

```powershell
python3 scripts/run_example.py --list
python3 scripts/run_example.py --build-only examples/basic_input/button
python3 scripts/run_example.py --build examples/basic_input/button
```

## Run Gallery

Gallery builds run `windeployqt` automatically on Windows, so the executable directory already contains the required Qt DLLs.

```powershell
cmake --build --preset mingw-debug --target FluentQtWidgetsGallery --parallel
.\build\mingw\examples\gallery\FluentQtWidgetsGallery.exe
```

The Gallery language selector applies immediately in the current process by rebuilding the Gallery window with the selected locale while preserving the current route and window state.

## Standalone Examples

Standalone examples follow the Python reference categories. Prefer the generated
manifest when browsing or launching demos:

```powershell
python3 scripts/run_example.py --list
python3 scripts/run_example.py --build examples/basic_input/button
cmake --build --preset mingw-debug --target fqw_basic_input_button_demo --parallel
```

`fqw_media_media_player_demo` is generated when Qt Multimedia and Qt MultimediaWidgets are available. `fqw_window_web_engine_demo` is generated when Qt WebEngineWidgets is available.

Current standalone demos:

- `examples/basic_input/button`: push, tool, dropdown, split, toggle, hyperlink, and pill button variants
- `examples/basic_input/check_box`: two-state and three-state Fluent checkbox behavior
- `examples/basic_input/combo_box`: placeholder and selection behavior for `ComboBox`
- `examples/basic_input/model_combo_box`: placeholder and selection behavior for `ModelComboBox`
- `examples/basic_input/radio_button`: grouped Fluent radio buttons
- `examples/basic_input/slider`: hollow-handle style and Fluent horizontal/vertical sliders
- `examples/basic_input/switch_button`: Fluent switch with checked-state text
- `examples/date_time/calendar_picker`: Fluent calendar picker
- `examples/date_time/fast_calendar_picker`: fast calendar picker variant
- `examples/date_time/time_picker`: date, zh-date, AM/PM time, 24-hour time, and seconds pickers
- `examples/dialog_flyout/color_dialog`: Fluent color picker button with alpha support
- `examples/dialog_flyout/custom_message_box`: subclassed `MessageBoxBase` with URL validation
- `examples/dialog_flyout/dialog`: frameless dialog and custom message box
- `examples/dialog_flyout/folder_list_dialog`: Fluent folder list dialog
- `examples/dialog_flyout/flyout`: simple, image, and custom flyout views
- `examples/dialog_flyout/message_dialog`: draggable message box API
- `examples/dialog_flyout/teaching_tip`: image, icon, and popup teaching tips
- `examples/layout/adaptive_flow_layout`: responsive card-width flow layout
- `examples/layout/flow_layout`: animated wrapping flow layout
- `examples/material/acrylic_brush`: acrylic brush with clipped blurred texture
- `examples/material/acrylic_combo_box`: acrylic combo and editable combo popups
- `examples/material/acrylic_flyout`: acrylic flyout views and custom acrylic popup content
- `examples/material/acrylic_label`: acrylic blurred image label with tint overlay
- `examples/material/acrylic_line_edit`: acrylic search line edit with completer/context menus
- `examples/material/acrylic_menu`: acrylic round menu with submenu and shortcuts
- `examples/material/acrylic_tool_tip`: acrylic tooltip filter over Fluent buttons
- `examples/material/acrylic_widget_menu`: acrylic round menu with embedded profile widget
- `examples/media/avatar_widget`: circular static and animated avatar widgets
- `examples/media/media_player`: simple and standard Fluent media play bars plus video widget
- `examples/menu/command_bar`: command bar actions, overflow menu, and dropdown button
- `examples/menu/menu`: animated round menu with submenu, shortcuts, and checkable actions
- `examples/menu/system_tray_menu`: Fluent system tray context menu
- `examples/menu/widget_menu`: round menu with embedded profile widget
- `examples/navigation/breadcrumb_bar`: Python-style breadcrumb route stack and page switching
- `examples/navigation/navigation1`: expanded FluentWindow navigation with nested album routes and avatar item
- `examples/navigation/navigation2`: SplitFluentWindow navigation with return history and bottom avatar item
- `examples/navigation/navigation3`: compact top navigation backed by a popup NavigationPanel
- `examples/navigation/navigation_bar`: Microsoft Store-style vertical navigation bar
- `examples/navigation/navigation_header`: navigation item headers that collapse in compact mode
- `examples/navigation/navigation_user_card`: FluentWindow navigation user card with avatar, title, and subtitle
- `examples/navigation/pivot`: pivot navigation over stacked pages
- `examples/navigation/segmented_tool_widget`: icon-only segmented toggle navigation
- `examples/navigation/segmented_widget`: segmented text navigation over stacked pages
- `examples/navigation/stacked_widget`: entrance and drill-in transition stacked widgets
- `examples/navigation/tab_view`: low-level `TabBar` with stacked pages
- `examples/navigation/tab_widget`: `TabWidget` with add, close, and movable tabs
- `examples/scroll/pips_pager`: horizontal and vertical pips pagers
- `examples/scroll/scroll_area`: smooth scroll area with image content
- `examples/status_info/info_badge`: count, dot, icon, custom, and attached info badges
- `examples/status_info/info_bar`: severity, custom icon, and positioned info bars
- `examples/status_info/progress_bar`: determinate and indeterminate progress bars
- `examples/status_info/progress_ring`: determinate and indeterminate progress rings
- `examples/status_info/state_tool_tip`: state tooltip lifecycle
- `examples/status_info/tool_tip`: Fluent tooltip filter positions
- `examples/text/font_icon`: Fluent icon buttons with theme switching
- `examples/text/image_label`: static and animated image labels with corner radii
- `examples/text/label`: Fluent text label scale and hyperlink labels
- `examples/text/line_edit`: search line edit with completer
- `examples/text/spin_box`: inline and compact spin/date/time editors
- `examples/text/text_browser`: Fluent text browser with markdown content
- `examples/view/card_widget`: app, gallery, and emoji card compositions
- `examples/view/flip_view`: image flip view with pips pager
- `examples/view/list_view`: Fluent list widget rows
- `examples/view/table_view`: Fluent table widget with border styling
- `examples/view/tree_view`: Fluent tree view with file system model
- `examples/view/tree_widget`: Fluent tree widget hierarchy
- `examples/window/clock`: SplitFluentWindow clock app with focus and stopwatch pages
- `examples/window/fluent_widget`: frameless Fluent base widget with title bar and theme toggle
- `examples/window/fluent_window`: frameless Fluent window with nested navigation, separator, bottom item, and badge
- `examples/window/login`: Fluent login window with image background and right-side form
- `examples/window/ms_fluent_window`: Microsoft Store-style Fluent window backed by `NavigationBar`
- `examples/window/settings`: settings page built from Fluent setting cards
- `examples/window/split_fluent_window`: split-style Fluent window with full-width title bar
- `examples/window/splash_screen`: Fluent splash screen attached to a frameless Fluent window
- `examples/window/web_engine`: SplitFluentWindow hosting a WebEngine page when Qt WebEngineWidgets is available

## Test

```powershell
ctest --preset debug --output-on-failure
# Windows MinGW:
ctest --preset mingw-debug --output-on-failure
```

## Release Artifacts

Tag builds publish Gallery archives through GitHub Actions. The archives are uploaded to the matching GitHub Release and are used by Gallery update detection.

```bash
git tag v0.1.4
git push origin main v0.1.4
```

Release rules, asset names, and OTA behavior are documented in [Release and OTA](release.md).

## Consume

```cmake
find_package(FluentQtWidgets CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE FluentQtWidgets::Widgets)
```

The umbrella header is:

```cpp
#include <FluentQtWidgets/FluentQtWidgets.h>
```

Apply the theme once after creating `QApplication`:

```cpp
FluentQt::FluentConfig::instance()->load();
FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Auto);
FluentQt::ThemeManager::instance()->applyTo(&app);
```

Persist user-facing settings through `FluentConfig`:

```cpp
FluentQt::FluentConfig::instance()->setThemeMode(FluentQt::Theme::Dark);
FluentQt::FluentConfig::instance()->setThemeColor(QColor("#0078d7"));
FluentQt::FluentConfig::instance()->save();
```
