# Changelog

## 0.1.3

### Windows build presets and documentation

- Added open source friendly Windows Qt kit guidance without committing machine-specific Qt paths.
- Moved the Windows MinGW preset output to `build/mingw` and kept macOS/Linux on `build/debug`.
- Fixed Windows post-build deployment command quoting for CMake paths containing spaces.
- Linked `dwmapi` on Windows and adjusted the Mica test expectation to match platform availability.

## 0.1.2

### Release stabilization

- Fixed macOS/Windows release blocker by resolving Windows DWM backdrop API compatibility in `FluentWindow.cpp`.
- Relaxed Linux picker popup alignment assertion tolerance in datetime tests (`tst_datetime`) to tolerate environment-independent rounding.
- Fixed Linux CI stability for feedback, datetime, menu, and Gallery translation tests under Xvfb.
- Removed the duplicate non-Xvfb Linux test step from CI.
- Updated release packaging so GitHub Actions publishes deployable Gallery archives for OTA detection from this tag.
- Formalized open source documentation, source attribution, release rules, and security reporting.

## 0.1.1

### OTA and release stabilization

- Fixed Gallery translation test linker failure in macOS builds (`checkGalleryUpdate` symbol not linked in test target).
- Updated CI release pipeline to reliably publish OTA artifacts to the same release tag (overwrite asset-safe).
- Added explicit release-migration notes and migration-source attribution in documentation.

## 0.1.0

### First official release

- Added a comprehensive Fluent-style Qt Widgets library and integrated `libFluentQtWidgets` core.
- Added CMake versioned build pipeline and release-aligned metadata.
- Added Gallery application with 14 pages, including settings, examples, and reference links.
- Added OTA (GitHub release) update checks in Gallery:
  - Manual check in Settings.
  - Optional startup check with persisted configuration.
- Added open source publishing documentation and release notes.
- Added configuration-driven defaults for Mica, theme, accent color, zoom, download directory, and update checks.

### Highlights

- Buttons: core button family, dropdown/split/toggle variants, icons and stateful rendering.
- Inputs: combo box, slider, switch, editable/typed fields, acrylic input variants.
- Navigation: navigation panel, views, tree/breadcrumb controls.
- Menu system: custom round menu, sub menus, checkable items.
- Feedback: info bars, tooltips, progress components, flyout/teaching tip.
- View widgets: table/tree/list with fluent scrolling and item delegates.
- Dialog and window system: Fluent dialogs, tooltips, splash screen, frameless shell.
