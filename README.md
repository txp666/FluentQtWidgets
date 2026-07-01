# FluentQtWidgets

[![CI](https://github.com/txp666/FluentQtWidgets/actions/workflows/ci.yml/badge.svg)](https://github.com/txp666/FluentQtWidgets/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/txp666/FluentQtWidgets)](https://github.com/txp666/FluentQtWidgets/releases)
[![License: GPL v3 or later](https://img.shields.io/badge/License-GPLv3%2B-blue.svg)](LICENSE)

FluentQtWidgets is a C++/Qt Widgets library that implements a Fluent Design component system with reusable controls, standalone demos, and a full Gallery application.

This repository is a **C++/Qt Widgets port of the Python project** [PyQt-Fluent-Widgets](https://github.com/zhiyiYo/PyQt-Fluent-Widgets). The Python project is the behavioral and visual reference for this port: widget state, spacing, QSS constants, demo content, resource paths, Gallery structure, and translations are aligned to the upstream Python implementation unless Qt/C++ platform behavior makes an exact match impossible.

Current release line: **0.1.6**. Local reference checkouts, build trees, and development-agent metadata are intentionally excluded from the source distribution.


## Requirements

- CMake 3.21+
- C++17
- Qt 6.5+ (Core, Gui, Widgets, Svg, Network)
- Optional Qt modules:
  - Multimedia + MultimediaWidgets
  - WebEngineWidgets

## Build

```bash
cmake -S . -B build -DFQW_BUILD_EXAMPLES=ON -DFQW_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

### Install / consume

```bash
cmake --install build --prefix <install-prefix>
```

```cmake
find_package(FluentQtWidgets CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE FluentQtWidgets::Widgets)
```

### Windows Qt kit selection

CMake must use a compiler/generator that matches the installed Qt kit. A MinGW Qt kit must be built with
MinGW, and an MSVC Qt kit must be built with MSVC. If CMake reports that it cannot find `Qt6Config.cmake`,
pass the Qt kit prefix through `CMAKE_PREFIX_PATH` or set `Qt6_DIR` to the directory that contains
`Qt6Config.cmake`.

The shared `CMakePresets.json` intentionally does not hard-code local Qt, compiler, or debugger paths.
Keep machine-specific paths in your environment, your IDE kit selection, or an ignored
`CMakeUserPresets.json`. A template is provided in `CMakeUserPresets.json.example`.

For a local Qt MinGW kit, make the Qt kit, MinGW compiler, and Ninja visible, then use the checked-in
`mingw-debug` preset:

```powershell
$env:CMAKE_PREFIX_PATH = "<path-to-qt-mingw-kit>"
$env:Path = "<path-to-mingw-bin>;<path-to-ninja-dir>;<path-to-qt-bin>;$env:Path"

cmake --preset mingw-debug
cmake --build --preset mingw-debug --parallel
ctest --preset mingw-debug
```

VS Code tasks inherit the environment that started VS Code. For one-click builds, either launch VS Code
from a terminal with the variables above set, set equivalent user environment variables, or copy
`CMakeUserPresets.json.example` to the ignored `CMakeUserPresets.json` and fill in local paths. On
Windows, `Gallery: Debug` uses `gdb.exe`, so the MinGW `bin` directory must be visible to VS Code before
the debug session starts. Close a running Gallery window before rebuilding because the linker cannot
overwrite a running `.exe`.
If you see `ninja: error: loading 'build.ninja': The system cannot find the file specified.`, run `cmake --preset mingw-debug` once to regenerate `build/mingw/build.ninja`, then build again.
If PowerShell shows duplicate `PATH`/`Path` environment variables, start a clean terminal or normalize
the user environment first; MinGW tools can otherwise fail with no compiler diagnostics.

If using Visual Studio/MSVC, install the matching Qt MSVC kit and point `CMAKE_PREFIX_PATH` or `Qt6_DIR`
to that kit instead. Do not commit your local `CMakeUserPresets.json`.

### Run Gallery

Use the preset that matches your platform. Presets use separate build trees because CMake caches the
generator, compiler, and Qt kit in each build directory.

```bash
# macOS/Linux
cmake --build --preset debug --target FluentQtWidgetsGallery --parallel
./build/debug/examples/gallery/FluentQtWidgetsGallery
# macOS:
# open ./build/debug/examples/gallery/FluentQtWidgetsGallery.app
```

```powershell
# Windows MinGW
cmake --build --preset mingw-debug --target FluentQtWidgetsGallery --parallel
.\build\mingw\examples\gallery\FluentQtWidgetsGallery.exe
```

## Standalone Examples

Use `--preset debug` on macOS/Linux, or `--preset mingw-debug` on Windows MinGW:

```bash
cmake --build --preset debug --target fqw_basic_input_button_demo --parallel
cmake --build --preset debug --target fqw_basic_input_check_box_demo --parallel
cmake --build --preset debug --target fqw_basic_input_combo_box_demo --parallel
cmake --build --preset debug --target fqw_basic_input_model_combo_box_demo --parallel
cmake --build --preset debug --target fqw_basic_input_radio_button_demo --parallel
cmake --build --preset debug --target fqw_basic_input_slider_demo --parallel
cmake --build --preset debug --target fqw_basic_input_switch_button_demo --parallel
```

(See `CMakeLists.txt` under each `examples/` subfolder for the full list.)

## Gallery update (OTA)

The app uses the CMake project version (`project(... VERSION ...)`) for display and update comparisons.
The default channel is GitHub Releases. See [Release and OTA](docs/release.md) for the tag, asset, and verification rules.

## Docs

- [Chinese README](docs/README_zh.md)
- [Quick Start](docs/quick-start.md)
- [Release and OTA](docs/release.md)
- [API Roadmap](docs/api-roadmap.md)
- [Basic Controls](docs/controls-basic.md)
- [Architecture](docs/architecture.md)
- [Porting Guide](docs/porting-guide.md)
- [Contributing](CONTRIBUTING.md)

## Build helpers

- `scripts/generate_vscode_examples.py` creates `.vscode/tasks.json` and `.vscode/launch.json`.
- `scripts/run_example.py` (if present) runs demo targets from the generated example manifest.

## Topics / Scope

The repository focuses on Fluent-style:

- Buttons and tool buttons (including split/dropdown/toggle)
- Menus, dialogs, flyouts, popups
- Navigation and layout controls
- Editors, selectors, info and feedback components
- Table/tree/list views and scrolling
- Basic Material helpers

## License

This project is distributed under **GPL-3.0-or-later**. See [Third Party Notices](THIRD_PARTY_NOTICES.md) for upstream attribution and copied resource licensing.
