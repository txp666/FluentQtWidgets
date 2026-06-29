# FluentQtWidgets Agent Guide

This file gives repository-specific guidance for AI agents working on
FluentQtWidgets.

## Project Direction

FluentQtWidgets is a C++/Qt Widgets port of
[PyQt-Fluent-Widgets](https://github.com/zhiyiYo/PyQt-Fluent-Widgets). Use the
Python project as the behavior and visual reference unless a Qt/C++ platform
constraint requires a documented difference.

Before changing widgets, Gallery pages, resources, QSS, translations, routing,
or window effects:

1. Read the C++ implementation under `include/`, `src/`, `examples/`, and
   `resources/`.
2. Compare the equivalent Python implementation.
3. Keep API behavior, signal behavior, route keys, layout, theme colors, QSS
   roles, resources, and Gallery text aligned where practical.
4. Add or update focused tests for behavior, translation, routing, resources,
   theme switching, or build/deploy changes.

## Repository Rules

- Use C++17.
- Public headers live under `include/FluentQtWidgets`.
- Implementations live under matching `src` folders.
- Examples and Gallery should consume public headers, not private internals.
- Use Qt parent-child ownership for widgets and QObjects.
- Keep resource paths compatible with the Python layout, especially the
  `:/qfluentwidgets` prefix.
- Keep Gallery route keys aligned with Python names such as
  `basicInputInterface`, `dateTimeInterface`, and `navigationViewInterface`.
- Repolish widgets when changing QSS-driven dynamic properties.
- Keep light and dark QSS behavior in sync.
- Keep platform-specific code behind narrow guards.
- Do not hard-code local Qt, compiler, debugger, or user-machine paths in shared
  project files. Use environment variables, IDE kits, or ignored
  `CMakeUserPresets.json`.
- Do not add alternate assets when the Python reference already provides the
  required compatible resource.

## Build And Test

Use the preset that matches the local Qt kit.

Common flow:

```bash
cmake --preset ninja-debug
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

Windows MinGW flow:

```powershell
$env:CMAKE_PREFIX_PATH = "<path-to-qt-mingw-kit>"
$env:Path = "<path-to-mingw-bin>;<path-to-ninja-dir>;<path-to-qt-bin>;$env:Path"

cmake --preset mingw-debug
cmake --build --preset mingw-debug --parallel
ctest --preset mingw-debug --output-on-failure
```

Run focused checks when relevant:

```bash
python scripts/verify_example_parity.py
ctest --preset debug --output-on-failure -R tst_basic_input_parity
ctest --preset debug --output-on-failure -R tst_gallery_translation
ctest --preset debug --output-on-failure -R tst_gallery_interface
```

On Windows, close running Gallery processes before rebuilding because the linker
cannot overwrite a running `.exe`.

## Gallery And Translations

Gallery is parity-sensitive:

- compare visible strings with the Python Gallery;
- keep translatable text behind `tr()` or translation helpers;
- update simplified and traditional Chinese `.ts` files when source strings
  change;
- regenerate `.qm` files when `.ts` files change;
- verify route keys, sidebar labels, search entries, sample card text, and
  light/dark behavior.

## Window Effects

Window, title bar, splash screen, rounded corner, Mica, Acrylic, and frameless
behavior are platform-sensitive:

- inspect the Python reference first;
- keep Windows-specific code behind platform guards;
- verify hit testing, resize borders, drag regions, activation, focus, and dark
  theme colors;
- test with Gallery, not only small demos.

## Documentation

Keep English and Chinese docs synchronized when build instructions, supported
platforms, presets, or public workflows change. If a local path is documented,
mark it as an example that users must replace with their own installation path.

## Change Discipline

- Read before editing.
- Keep patches scoped.
- Do not rewrite unrelated files.
- Do not revert user changes unless explicitly asked.
- Do not run destructive git commands.
- Keep generated files and source files consistent, especially `.ts`/`.qm` and
  qrc/resource updates.
- Summarize changed files and verification status before finishing.
