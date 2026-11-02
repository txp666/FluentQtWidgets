# Quick Start

How to build FluentQtWidgets, run the Gallery and the examples, and use the library in your own CMake project.
[Controls](controls.md) gives an overview of the widgets.

## Requirements

- CMake 3.21+ and a C++17 compiler
- Qt 6.5+ with Core, Gui, Widgets, Svg and Network
- Optional: Qt Multimedia and MultimediaWidgets (media widgets and their examples), Qt WebEngineWidgets (only the
  standalone WebEngine example)

## Build

The CMake options are `FQW_BUILD_EXAMPLES` (on by default), `FQW_BUILD_TESTS` (off by default),
`FQW_BUILD_SHARED` (on by default) and `FQW_BUILD_SCREENSHOTS` (off by default; the README image renderers in
`tools/screenshots`). Any generator works:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH=<path-to-qt> -DFQW_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

`CMakePresets.json` has presets with examples and tests on. They don't name a Qt kit, compiler or debugger, so
make your Qt visible through `CMAKE_PREFIX_PATH` or `Qt6_DIR` first.

```bash
# macOS / Linux, in build/debug
cmake --preset ninja-debug
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

### Windows

The compiler and generator must match the Qt kit: build a MinGW kit with MinGW and an MSVC kit with MSVC. If CMake
cannot find `Qt6Config.cmake`, point `CMAKE_PREFIX_PATH` at the kit's root or `Qt6_DIR` at the directory that
holds `Qt6Config.cmake`.

For a Qt MinGW kit, make the kit, the MinGW compiler and Ninja visible in the terminal, then use the
`mingw-debug` preset, which builds in `build/mingw`:

```powershell
$env:CMAKE_PREFIX_PATH = "<path-to-qt-mingw-kit>"
$env:Path = "<path-to-mingw-bin>;<path-to-ninja-dir>;<path-to-qt-bin>;$env:Path"

cmake --preset mingw-debug
cmake --build --preset mingw-debug --parallel
ctest --preset mingw-debug --output-on-failure
```

Machine-specific settings belong in environment variables, your IDE's kit selection or a `CMakeUserPresets.json`,
which git ignores; copy `CMakeUserPresets.json.example` to start one (it has MinGW and MSVC presets). Don't commit
it.

- VS Code tasks inherit the environment VS Code was started with: start it from a terminal set up as above, set
  the same user environment variables, or use a `CMakeUserPresets.json`. The `Gallery: Debug` launch entry uses
  `gdb.exe`, so the MinGW `bin` directory must be on `PATH`.
- Close a running Gallery before rebuilding; the linker cannot replace a running `.exe`.
- `ninja: error: loading 'build.ninja'` means the build directory isn't configured yet: run
  `cmake --preset mingw-debug` again.
- If PowerShell shows both `PATH` and `Path`, start a clean terminal or fix the user environment; otherwise the
  MinGW tools can fail without a compiler message.

Builds on Windows run `windeployqt`, so the Gallery and examples start from the build directory.

## Run the Gallery

```bash
cmake --build --preset debug --target FluentQtWidgetsGallery --parallel
./build/debug/examples/gallery/FluentQtWidgetsGallery              # Linux
open ./build/debug/examples/gallery/FluentQtWidgetsGallery.app     # macOS
```

```powershell
cmake --build --preset mingw-debug --target FluentQtWidgetsGallery --parallel
.\build\mingw\examples\gallery\FluentQtWidgetsGallery.exe
```

Changing the language in the Gallery's settings rebuilds the window in the new language and keeps the current
page. The Gallery checks GitHub Releases for a newer version; see [Release and OTA](release.md).

## Examples

`examples/` holds 75 standalone demos in the categories of the Python examples (`basic_input`, `date_time`,
`dialog_flyout`, `layout`, `material`, `media`, `menu`, `navigation`, `scroll`, `status_info`, `text`, `view`,
`window`). Each builds as `fqw_<category>_<name>_demo`, for example `fqw_basic_input_button_demo` from
`examples/basic_input/button`. `fqw_media_media_player_demo` needs Qt Multimedia, and `fqw_window_web_engine_demo`
Qt WebEngineWidgets.

`scripts/run_example.py` builds and runs them from the manifest `examples/examples.json`:

```bash
python3 scripts/run_example.py --list
python3 scripts/run_example.py --build examples/basic_input/button
python3 scripts/run_example.py --build-only fqw_basic_input_check_box_demo
```

After adding or renaming an example, regenerate the manifest and the VS Code `tasks.json` / `launch.json` with
`python3 scripts/generate_vscode_examples.py`. The manifest lists each target with its build and run commands,
the optional features it needs (`FQW_HAS_MULTIMEDIA`, `FQW_HAS_WEBENGINE_WIDGETS`) and whether the current build
enables it.

## Use the library

Install it, then find it from your project:

```bash
cmake --install build --prefix <install-prefix>
```

```cmake
find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(FluentQtWidgets CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE Qt6::Widgets FluentQtWidgets::Widgets)
```

One header brings in the whole API, in the `FluentQt` namespace:

```cpp
#include <FluentQtWidgets/FluentQtWidgets.h>
```

Set the theme after creating the `QApplication`. `FluentConfig` stores the user's theme, accent colour and other
settings in a JSON file in the application's config folder; its setters apply the theme and accent at once.

```cpp
auto *config = FluentQt::FluentConfig::instance();
config->load();
FluentQt::ThemeManager::instance()->setTheme(config->themeMode());
FluentQt::ThemeManager::instance()->setAccentColor(config->themeColor());

// later, when the user changes them
config->setThemeMode(FluentQt::Theme::Dark);
config->setThemeColor(QColor("#0078d4"));
config->save();
```

The Gallery (`examples/gallery`) shows how a complete application uses navigation, settings, translations and
themes.
