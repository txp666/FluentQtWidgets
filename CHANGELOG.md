# Changelog

## 0.1.6

### Native chart widgets and macOS fixes

- Added `ChartWidget`, a native QWidget chart renderer for common bar, line, area, mixed, donut, scatter, radar, gauge, and heatmap displays with ECharts-style staggered reveal animation, data labels, and hover feedback.
- Added `AudioWaveformWidget`, an interactive QWidget-based waveform renderer for sampled audio amplitudes with progress, color, and metric customization.
- Added `RealtimePlotWidget`, a native QWidget live plot with bounded multi-series buffers, dense-data aggregation, pyqtgraph-style crosshair readouts, right-click view controls, and line legend toggles.
- Integrated the new chart components into Gallery with realtime, audio-waveform, native bar, line, stacked area, mixed bar/line, donut, scatter, radar, gauge, and heatmap demos.
- Fixed macOS Fluent window corner rendering so native rounded corners remain visible without clipping child content.
- Added tests for native chart rendering, waveform sample/progress behavior, realtime plot multi-series rendering, checkbox legend toggling, and Gallery translations.
- Updated release metadata and documentation for the 0.1.6 release line.

## 0.1.5

### Public release polish and Python parity

- Aligned the Gallery status/info, text, view, and settings pages more closely with the Python reference, including badge geometry, info bar layout, view containers, and settings-card spacing.
- Refined shared base widgets used by those pages, including info badges, info bars, item views, tree widgets, setting cards, and frameless window content surfaces.
- Improved Windows frameless-window behavior: native maximize/double-click handling, maximize hit testing, resize constraints, and smoother main-content resizing during state changes.
- Fixed settings-page release issues including overlapped cards, default theme/accent state, and native folder selection behavior.
- Updated release metadata and public documentation for the 0.1.5 open-source release line.
- Removed development-agent metadata, stale planning notes, and machine-specific example paths from the tracked source distribution.
- Adjusted the example parity checker so missing Python-reference examples fail release validation while documented C++-only examples are reported as additional coverage.
- Added tag-specific release notes so GitHub Releases include a human-readable description when `v0.1.5` is pushed.

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
