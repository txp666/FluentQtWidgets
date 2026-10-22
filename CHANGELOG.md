# Changelog

## Unreleased

### Object lifetime fixes

- Skipped the macOS window-corner setup outside the Cocoa platform plugin, so `FluentWindow`, `MSFluentWindow`, and `FluentWidget` no longer crash under `offscreen` or `minimal`.
- Made `InfoBarManager` forget bars that are deleted without being closed and parents that are destroyed, so a reopened window that reuses the old address no longer stacks against freed bars. Slide animations are now owned by their bar.
- Removed deleted `ConfigItem`s from `ConfigItemRegistry` without dropping a newer item registered under the same key.
- Removed child entries when `NavigationPanel::removeWidget()` deletes a tree item that has children.
- Kept `PopUpAniStackedWidget` animation data per page instead of per index, so pages deleted or changed through the `QStackedWidget` API no longer leave it out of step with the stack.
- Gave `InfoBadgeManager` the badge as its default parent, retired the previous manager when a badge is attached again, and stopped it from touching a deleted badge or target.
- Closed a `TeachingTip` when its target is deleted, and stopped its window filter from going through the deleted target.
- Cleared a submenu's parent links when `RoundMenu::removeMenu()` removes it or its parent menu is deleted, and dropped a submenu's row when the submenu is deleted.
- Tracked `FluentWindow`'s splash screen and `LineEdit`'s completer and completer menu with `QPointer`. `LineEdit` now frees its parentless completer menu, and `CompleterMenu` frees the actions it creates for each completion list.
- Built the child widgets in `FlyoutView(QWidget *)` and `AcrylicFlyoutView(QWidget *)`, which previously crashed on `show()`.
- Added `tst_lifetime` regression coverage for each of these cases.

### Behaviour fixes

- Inputs: dragging a `Slider` handle moves the slider; `ClickableSlider`, `ProgressBar` and `ProgressRing` honour `minimum()`; Ctrl+wheel zooms read-only `TextEdit`, `PlainTextEdit` and `TextBrowser` as in Qt; plain Fluent labels leave right-clicks to their parent; `SwitchButton`, navigation items, image labels and setting card headers act only on a left click released over them; drop-down buttons open their menu only on a left click.
- Combo boxes: the clear button of the editable combos clears the text; model combos no longer block the model's signals for other views and proxies, follow model resets and the removal of the current row, and no longer take ownership of the caller's model; `clear()` and `setCurrentIndex(-1)` emit and bring the placeholder back; `insertItem()` into an empty combo selects the item; moving rows no longer emits spurious text changes.
- Scrolling: scroll bar handle maths no longer overflows on large item views; `SmoothScrollArea` no longer leaves the replaced delegate's scroll bars behind; `ExpandSettingCard` passes the wheel on to the page; zero-delta wheel events no longer flip `FlipView` or `CycleListWidget`.
- Item views: selection highlights follow inserted, removed, sorted and filtered rows; the pressed look ends on release; typed values get Qt's matching editor.
- Dialogs and popups: `MaskDialogBase` finishes once however often it is accepted or rejected during the fade-out; text wrapping in dialogs, info bars and flyouts is shared and never cuts a surrogate pair or an emoji sequence; on Wayland mask dialogs overlay the parent window as a child; info bar and state tooltip timers no longer pile up or run while hidden; `ToolTip::showText()` tooltips delete themselves once hidden; the compact spin box flyout no longer grabs the keyboard.
- Menus and tabs: `RoundMenu::clear()` removes separators, `setMenuView()` rebuilds existing rows, shortcuts are no longer cut to one character, and the new `execAndWait()` offers `QMenu::exec()` semantics; tabs dragged in from another `TabBar` are ignored and moving a tab keeps the current page.
- Navigation: `Pivot` keeps items in display order and follows item resizes; `NavigationHistory` back navigation emits through the interface and no longer keeps duplicate entries; the panel's window filter follows reparenting; tooltips are translatable; long item texts are elided.
- Windows and theme: hit testing on Windows uses device-independent coordinates at any scale factor, also for messages passed to `FramelessWindowHelper::handleNativeEvent()` without a window; the frameless helper is owned by its window and handles Wayland and finished system moves; title bars added later mirror their window; Auto theme can switch back to light where Qt does not report the colour scheme, without polling; `FluentConfig::save()` writes atomically; acrylic surfaces capture the screen only when allowed (never prompting on macOS) and fall back to an opaque tint.
- Style sheets: `apply()` honours its theme, the manager's own properties no longer repolish, registration is constant-time, clearing a custom style sheet takes effect, and lazy refreshes run on show instead of inside paint events.
- Others: `CommandBar::clear()` keeps actions it does not own; `ConfigItemRegistry` reports a change once; option and colour config items correct their defaults; `OrchidLight` is `#C239B3`; `OptionsSettingCard` unchecks every option for an unknown value; `OpacityAniStackedWidget` applies its opacity effect only during transitions; charts handle negative values; `RealtimePlotWidget` stops dragging when the button is gone, exports every sample to CSV, shows negative x while auto-scrolling, fits all data on "View All", recovers its fast path once out-of-order samples age out, gains `setXAxisValueFormatter()`, and starts plot exports in an existing folder (the user's download folder by default) instead of creating `app/download`; `FlowLayout` and `VBoxLayout` follow widgets deleted from outside.
- Builds: Qt 5 builds no longer use Qt 6-only event signatures or APIs; the library compiles without warnings under `-Wall -Wextra -Wshadow -Wconversion -Woverloaded-virtual`.
- Added `tst_regressions` coverage for these fixes.

## 0.1.9

### Expandable cards and scroll-bar synchronization

- Changed `ExpandSettingCard` expansion to animate the visible content height while keeping the internal scroll position at zero, preventing transient content overlap in scrolled containers.
- Kept interrupted expand/collapse animations continuous from the current visible height and updated the parent layout before painting resized cards and their siblings.
- Synchronized Fluent overlay scroll-bar ranges, page steps, and single steps after Qt completes layout, including initial display and content resizing.
- Guarded deferred scroll-bar updates when the partner scroll area or native scroll bar has been destroyed.
- Added regression coverage for animation frames, rapid direction changes, scrolled card layouts, scroll-bar synchronization, and partner lifetime.

## 0.1.8

### Realtime plot and item-view polish

- Added custom X-value formatting for `RealtimePlotWidget` crosshair readouts.
- Packaged Simplified Chinese and Traditional Chinese translations for realtime plot menus and controls.
- Added horizontal and vertical scroll-bar policy setters to `ListView`, `ListWidget`, `TableView`, `TableWidget`, `TreeView`, and `TreeWidget`, keeping their Fluent scroll delegates in sync.
- Fixed macOS table selection rendering so native selection highlights no longer cover the Fluent styling, and reserved space for the first-column selection indicator.
- Added regression coverage for Fluent table and tree scroll-bar policies.
- Fixed Linux Gallery archives to preserve the shared-library symbolic links and locate the bundled FluentQtWidgets library next to the executable.

## 0.1.7

### Layout, navigation, and scaling polish

- Improved `FlowLayout` wrapping, spacing, animation stability, and per-row vertical alignment.
- Made transparent `ScrollArea` state propagate correctly when content is attached after configuration.
- Kept navigation panels above animated page stacks and refined navigation-tree text spacing.
- Kept the native styled navigation-stack curve and matched the exposed corner backdrop to the compact navigation surface, removing the subtle light-theme square seam.
- Aligned Gallery root-page transparency with the Python reference so page surfaces preserve the navigation stack's rounded corner.
- Improved icon centering and fractional-scale rendering so Fluent icons retain their visible bounds.
- Removed unnecessary vertical scroll bars from combo-box popup menus.
- Refined native macOS window corner behavior and expanded regression coverage for layout, navigation, display scaling, and themes.
- Updated the FlowLayout and ScrollArea examples for the new public behavior.

## 0.1.6

### Native chart widgets and macOS fixes

- Rebuilt the Windows Gallery release packaging to include the Qt MultimediaWidgets runtime used by the shared FluentQtWidgets library.
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
