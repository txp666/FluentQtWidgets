# FluentQtWidgets

[![CI](https://github.com/txp666/FluentQtWidgets/actions/workflows/ci.yml/badge.svg)](https://github.com/txp666/FluentQtWidgets/actions/workflows/ci.yml)
[![Release](https://img.shields.io/github/v/release/txp666/FluentQtWidgets)](https://github.com/txp666/FluentQtWidgets/releases)
[![License: GPL v3 or later](https://img.shields.io/badge/License-GPLv3%2B-blue.svg)](LICENSE)

FluentQtWidgets is a C++/Qt Widgets library that implements a Fluent Design component system with reusable controls, standalone demos, and a full Gallery application.

This repository is a **Codex C++ migration of the Python project** [PyQt-Fluent-Widgets](https://github.com/zhiyiYo/PyQt-Fluent-Widgets). The Python project is the behavioral and visual reference for this port: widget state, spacing, QSS constants, demo content, resource paths, Gallery structure, and translations are aligned to the upstream Python implementation unless Qt/C++ platform behavior makes an exact match impossible.

## Release

- **Current release**: `0.1.2`
- **Release page**: <https://github.com/txp666/FluentQtWidgets/releases>
- **Repository**: <https://github.com/txp666/FluentQtWidgets>
- **Default update channel in Gallery**: `https://github.com/txp666/FluentQtWidgets/releases/latest`

Gallery includes release-based update checks:

- Manual check: Settings -> Software update -> Check update.
- Startup check: Settings -> Software update -> Check for updates when the application starts.
- Release assets: CI uploads Gallery archives for Linux, Windows, and macOS tags. These archives are the OTA payloads used by Gallery update detection.

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

### Run Gallery

```bash
cmake --build build --target FluentQtWidgetsGallery
./build/examples/gallery/FluentQtWidgetsGallery
# macOS:
# ./build/examples/gallery/FluentQtWidgetsGallery.app/Contents/MacOS/FluentQtWidgetsGallery
```

## Standalone Examples

```bash
cmake --build build --target fqw_basic_input_button_demo
cmake --build build --target fqw_basic_input_check_box_demo
cmake --build build --target fqw_basic_input_combo_box_demo
cmake --build build --target fqw_basic_input_model_combo_box_demo
cmake --build build --target fqw_basic_input_radio_button_demo
cmake --build build --target fqw_basic_input_slider_demo
cmake --build build --target fqw_basic_input_switch_button_demo
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
