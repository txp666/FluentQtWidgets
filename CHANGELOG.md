# Changelog

## Unreleased

Every widget was compared with PyQt-Fluent-Widgets 1.11.3, the reference the port follows, and the differences were fixed. A new test now keeps the sizes in line.

### API changes

- `themeColor()` and `derivedThemeColor(…, ThemeColor::Primary, …)` give the colour for the current theme, as Python's `themeColor()` does: lighter in dark mode (`#009faa` becomes `#29f1ff`). Every accent painted by the library uses it. Style sheets use `--ThemeColorPrimary`; the port-only `@accentColor` token is gone.
- `Slider(parent)` and `ClickableSlider(parent)` are vertical by default, like `QSlider` and the Python classes. Pass `Qt::Horizontal` to get a horizontal slider.
- `RangeSettingCard::slider()` returns the self-painted `Slider *`, not a `ClickableSlider *`.
- `ClickableSlider` has no style sheet of its own, as in Python. `FluentStyleSheetSource::Slider` gives the Fluent look to any `QSlider` it is applied to.
- `FluentStyleSheetSource::ProgressBar` and `FluentStyleSheetSource::RadioButton` are removed, along with other style sheet rules and roles that nothing used.
- `IconWidget` fills its rect and has no size hint, as in Python. Call `setIconSize()` for a fixed size.
- `NavigationPanel::setMinimumExpandWidth()` accepts any width. With `0`, the panel expands in place at any window width.
- New `CheckableSystemTrayMenu`, from Python: a checkable menu for tray icons, without the right and bottom shadow margins.
- `FolderListDialog` is rewritten as Python's `MaskDialogBase` with `AddFolderCard`, `FolderCard` and `ClickableWindow`: `folders()` → `folderPaths()`, `addFolderButton()` → `addFolderCard()`, `yesButton()` → `completeButton()`, and `setFolders()`, `addFolder()`, `removeFolder()` and `FolderPickerDialog` are removed. It emits `folderChanged(QStringList)` when Done is clicked after a change.
- `MessageDialog` is Python's Win10-style `MaskDialogBase` with `titleLabel()`, `contentLabel()`, `yesButton()`, `cancelButton()`, `yesSignal` and `cancelSignal`; the `MessageBox` members it inherited are gone.
- `ComboBoxSettingCard(configItem, icon, title, content, texts, parent)` takes an `OptionsConfigItem` and follows it both ways, as in Python, with `comboBox()`, `configItem()` and `setValue()`. Its forwarding members (`addItem()`, `addItems()`, `setCurrentIndex()`, `currentIndex()`, `currentText()` and their signals) and `bindComboBoxSettingCard()` are removed. New `AcrylicComboBoxSettingCard`.
- `FolderListSettingCard` emits Python's `folderChanged(QStringList)` in place of `foldersChanged`, `folderAdded` and `folderRemoved`.
- `CustomColorSettingCard` has Python's "Choose color" button: `colorPicker()` → `chooseColorButton()`, and new `radioWidget()`, `radioLayout()`, `buttonGroup()`, `customColorWidget()`, `customColorLayout()` and `customLabel()`. `ColorSettingCard` and `CustomColorSettingCard` take Python's trailing `enableAlpha`.
- `BreadcrumbBar` is rewritten as Python's painted items: `BreadcrumbItem` is a widget, with the new `BreadcrumbWidget` and `ElideButton`; `currentItem()`, `item()` and `itemAt()` return a `BreadcrumbItem *`; `setItems()` and `itemClicked` are removed.
- `NavigationInterface` is Python's thin wrapper around a `NavigationPanel`: `navigationPanel()` → `panel()`, and the page API (`addPage()`, `removePage()`, `currentIndex()`, `currentRouteKey()`, their signals) and `contentTopMargin` are removed. `FluentWindow` owns the page stack and keys pages by object name; its `addPage()` is removed. `NavigationPanel` gains `setCollapsible()`, `insertSeparator()` and `layoutMinHeight()`.
- Navigation items, separators and headers added without a position, and `FluentWindow::addSubInterface()` pages, go to `NavigationItemPosition::Top`, as in Python, not `Scroll`.
- `NavigationWidget::setExpandWidth()` and `expandWidth()`: `NavigationPanel::setExpandWidth()` widens its items and separators with it, as Python's `EXPAND_WIDTH` does.
- `NavigationBar` takes its width from the layout, as in Python; `NavigationBar::kBarWidth` is removed.
- `Pivot`, `SegmentedWidget` and `SegmentedToggleToolWidget` select nothing until `setCurrentItem()`, as in Python.
- New `SegmentedToolWidget` and `SegmentedToolItem`, Python's icon-only segmented control.
- `NavigationUserCard` has Python's `textOpacity` property and `setAnimationDuration()`.
- `TabBar` and `TabWidget` have Python's `setTabMinimumWidth()`, `tabMinimumWidth()`, `setTabTextColor()` and `setTabSelectedBackgroundColor()`; `TabItem` has `setTextColor()` and `setSelectedBackgroundColor()`.
- `AcrylicLabel` is rewritten as Python's `QLabel`: `AcrylicLabel(blurRadius, tintColor, luminosityColor, maxBlurSize, parent)`, `setImage(path)`, `imagePath()`, `blurPixmap()`, `acrylicTextureLabel()` and `blurThread()`, with the new `BlurCoverThread`, `AcrylicTextureLabel` and `gaussianBlur()`. The `QWidget *`, `QImage`, `QPixmap` and path constructors, `setImage(QImage)`, `setPixmap()`, `setImagePath()`, `image()`, `isNull()`, `tintColor()`, the aspect ratio mode and the changed signals are removed. `AcrylicBrush::setImagePath()` → `setImage(QString)`.
- New `AcrylicToolTipContainer` (`AcrylicToolTip::acrylicContainer()`) and a protected `ToolTip(QFrame *container, QWidget *parent)` constructor, Python's `_createContainer()`.
- New `AcrylicCompleterMenuActionListWidget`, `AcrylicComboMenuActionListWidget` and `AcrylicMenuActionListWidget::clipPath()`, as in Python; `AcrylicSystemTrayMenu::sizeHint()` and `AcrylicFlyoutViewBase::acrylicBorderColor()` are removed.
- `RoundMenu::adjustPosition()` is public, as in Python.
- `FluentWidget` and the window classes have Python's `setCustomBackgroundColor(light, dark)`, with `lightBackgroundColor()`, `darkBackgroundColor()` and `backgroundColor()`. The title bar's `navigationBackground` property is removed.
- The media classes are ported from Python's `multimedia` package: `MediaPlayerBase`, `MediaPlayer` (wrapping a `QMediaPlayer` and its `QAudioOutput`), `MediaPlayBarBase`, `SimpleMediaPlayBar`, `StandardMediaPlayBar`, `MediaPlayBarButton`, `PlayButton`, `VolumeButton`, `VolumeView` and `VideoWidget`, in the new headers `Multimedia/MediaPlayBar.h` and `Multimedia/VideoWidget.h`. The play bars no longer derive from each other and lose `setSource()`, `togglePlay()`, `audioOutput()`, `timeLabel()` and `volumeSlider()` (use `player()->setSource()`, `togglePlayState()`, the player's volume and the volume button); `player()` returns a `MediaPlayerBase *`. `VideoWidget` is a `QGraphicsView`: `videoOutput()` → `videoItem()`, and the audio output is the player's.

### Size and look

- Buttons: tool buttons, calendar pickers, primary and checked buttons are as tall as push buttons (33 px), and the 32 px minimums Python doesn't have are gone. The port-only buttons (`Filled*`, `Outlined*`, `Text*`, `LuminaPushButton`, `Round*`, `Chip`, `Tag`) share the Fluent button box: push variants are 33 px tall and tool variants 38 × 32. Disabled split buttons use Python's divider colour.
- Inputs: line edits are 33 px tall; editable combo boxes use the line-edit style; sliders are as thick as their 22 px handle, in native styles too; combo-box arrows dip on press and fade when disabled.
- Text: labels use Python's black and white text, caption labels included, and `setTextColor()` wins over the label style sheet. The avatar initial uses the regular font weight. The multi-line focus line runs along the bottom of the editor.
- Date and time: pickers take their size from their columns and use Python's frame, text colours, separators, scroll buttons and 33 px panel buttons. Panels open only on a click, at Python's position, centred on a value chosen before they are shown. Calendar panels, cells, year pages, title, weekdays and navigation carets look as in Python. Reset buttons show the cancel icon.
- Dialogs and popups: dialog titles, the message box frame and selected text look as in Python. Return accepts a dialog, and dialog buttons have no 96 px minimum. Flyout views and teaching tips are drawn and sized as in Python, and unanimated flyouts open above their target. Command bar buttons use 12 px text and the bar's font; an empty bar no longer collapses.
- Menus: the submenu arrow is drawn as in Python; ordinary menus hide their overlay scroll bar; "Select all" in the edit menus has no icon; fade-in pull-up menus stop where Python's do.
- Tooltips and status: tooltips no longer take focus when they appear, keep long text on one line and use Python's background and shadow. Item view tooltips stay on the screen. Dark-theme badge text is black. State tooltip icons use the opposite theme. Progress ring text is 14 px. Icon badges recolour on theme changes. Info badges and progress rings take Python's size hints.
- Navigation: tree items, the panel's border and separators, navigation bar items and item headers are laid out and animated as in Python, and the indicators use Python's curves. Tabs and the tab bar have Python's heights and painting. The pivot keeps its indicator when the first selection is made while hidden.
- Views: table and tree headers use the Python section style. List, table and tree items use a 13 px font, Python's text and check box positions, and the Python tree row height. Tables select several rows with Ctrl and Shift. Lists have no extra viewport margins. Bordered tables have square corners. A table's row indicator is hidden while it is scrolled sideways.
- Cards and scrolling: `HeaderCardWidget` titles use a 15 px font. Card backgrounds fade between states. An elevated card shows its shadow only under the mouse. `SimpleCardWidget` has no layout. Pips pagers and flip views use Python's sizes, buttons and fades, and the transition stacks use Python's easing curves.
- Settings: setting cards and group titles are drawn as in Python. The switch card puts its On/Off text left of the switch. The combo box card keeps the combo box's own width. Expandable cards use plain labels. Colour pickers are outlined and their dialog has a title. The folder list card asks before removing a folder.
- Windows: title bars place their buttons, icon and title as in Python. Without Mica the windows paint Python's rgb(240, 244, 249) or rgb(32, 32, 32) behind a transparent title bar and navigation, instead of rgb(249, 249, 249) with rgb(243, 243, 243) strips.
- Fonts: every widget that Python gives a font with `setFont()` or `getFont()` builds it from the Fluent families at Python's size, so an application font no longer changes the widgets (push buttons were 30 px tall in the Gallery). The line edit style sheet no longer overrides that font, and tooltips use the Fluent families.
- Dialogs: the frameless `Dialog` no longer cuts off its button row; it fits its content, at least 240 × 192, with Python's fonts and a painted 1 px frame. With a parent, the mask dialogs are child overlays of the parent's window everywhere, as in Python; `ColorDialog` has a maximum size and scrolls in a short window; `MessageBoxBase` honours a minimum width set on `widget()`; `MessageBox` wraps at 100 characters and is its layout's natural size. The mask dialogs' painted shadow is as soft as Python's drop shadow. `FolderListDialog`'s mask has Python's 1 px border.
- Setting cards: disabled card and group icons fade to 0.36 opacity instead of the style's grey. Expandable group cards have the same height whatever the order of show, adding groups and expanding.
- Navigation: long item texts are cut at the item's edge as in Python instead of elided; the tree arrow is 9.6 px. The navigation bar icon slides down as Python's when the selected text is hidden, and the user card animates its avatar and text when it expands or compacts.
- Material: `AcrylicLabel` blurs with Python's Gaussian filter on a worker thread, and `AcrylicBrush` uses the same blur. Acrylic menus are as tall as plain ones (Python's 6 px margins) and the completer's has square corners towards the line edit. `AcrylicToolTip` blurs what is behind it. The acrylic flyout border uses `borderColor()`. Acrylic tray menus move inside the screen when shown.
- `HeaderCardWidget` and `GroupHeaderCardWidget` titles follow theme changes.
- `MessageBox` widens for buttons added to `buttonLayout()`, so a third button such as "Don't save" no longer overlaps the others.

### Behaviour fixes

- Drop-down and pull-up menus (combo boxes, drop-down buttons, submenus) open without flashing on every platform: the window opens at its end position and the list slides inside it. Pull-up menus rise from their bottom edge. On macOS, menus open where they are placed and no longer settle 32 px low with their last row cut off.
- The arrow keys move through a line edit's completer menu while typing continues in the edit.
- Acrylic menus, flyouts and the navigation panel no longer repaint continuously while visible.
- `RealtimePlotWidget`'s context menu closes before any of its actions runs, also when triggered by a shortcut, so no save dialog opens over it.
- The library strings added by the rewrites (folder dialogs, Choose color, title bar and navigation tool tips, media bars) are translated to Simplified and Traditional Chinese.

### Tests and tools

- `tst_python_parity` compares the size, size hint, minimum and maximum size of every widget the port shares with qfluentwidgets 1.11.3 (206 rows) with measurements of the Python original. `tools/parity/measure_python.py` regenerates those measurements. See `tools/parity/README.md`.
- `tst_media` (built with Qt Multimedia) checks the media classes against geometry measured with `tools/parity/measure_python_media.py`; `tst_fonts` checks the widget fonts under a different application font.
- `tools/screenshots` renders the README screenshots and the Gallery tour GIF (`-DFQW_BUILD_SCREENSHOTS=ON`).

### Project

- FluentQtWidgets has its own logo, a small Fluent window, in place of the copy of PyQt-Fluent-Widgets' "W" logo. It is used for the Gallery, its app icons and the login example. The README screenshots show it.
- The README comes in Chinese (`README.md`) and English (`README_EN.md`), and Gallery packages include both. The guides describe the current API: `docs/controls.md` replaces `docs/controls-basic.md`, the build steps live in `docs/quick-start.md`, and the outdated API roadmap is removed.
- A Qt Multimedia installed apart from Qt needs its prefix in `QT_ADDITIONAL_PACKAGES_PREFIX_PATH` as well as `CMAKE_PREFIX_PATH`; `docs/quick-start.md` shows the command.

## 0.1.10

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
- Added regression tests for each of these cases.

### Behaviour fixes

- Inputs: dragging a `Slider` handle moves the slider; `ClickableSlider`, `ProgressBar` and `ProgressRing` honour `minimum()`; Ctrl+wheel zooms read-only `TextEdit`, `PlainTextEdit` and `TextBrowser` as in Qt; plain Fluent labels leave right-clicks to their parent; `SwitchButton`, navigation items, image labels and setting card headers act only on a left click released over them; drop-down buttons open their menu only on a left click.
- Combo boxes: the clear button of the editable combos clears the text; model combos no longer block the model's signals for other views and proxies, follow model resets and the removal of the current row, show the first item of a model set with `setModel()` (they kept the old model's text when row 0 was current before), and no longer take ownership of the caller's model; `clear()` and `setCurrentIndex(-1)` emit and bring the placeholder back; `insertItem()` into an empty combo selects the item; moving rows no longer emits spurious text changes.
- Scrolling: scroll bar handle maths no longer overflows on large item views; `SmoothScrollArea` no longer leaves the replaced delegate's scroll bars behind; `ExpandSettingCard` passes the wheel on to the page; zero-delta wheel events no longer flip `FlipView` or `CycleListWidget`.
- Item views: selection highlights follow inserted, removed, sorted and filtered rows; the pressed look ends on release; typed values get Qt's matching editor.
- Text editors: the focus underline of `TextEdit`, `PlainTextEdit` and `TextBrowser` stays at the bottom when the content scrolls instead of moving up with the text.
- Dialogs and popups: `MaskDialogBase` finishes once however often it is accepted or rejected during the fade-out; text wrapping in dialogs, info bars and flyouts is shared and never cuts a surrogate pair or an emoji sequence; on Wayland mask dialogs overlay the parent window as a child; info bar and state tooltip timers no longer pile up or run while hidden; `ToolTip::showText()` tooltips delete themselves once hidden; the compact spin box flyout no longer grabs the keyboard.
- Menus and tabs: `RoundMenu::clear()` removes separators, `setMenuView()` rebuilds existing rows, shortcuts are no longer cut to one character, and the new `execAndWait()` offers `QMenu::exec()` semantics; tabs dragged in from another `TabBar` are ignored and moving a tab keeps the current page.
- Navigation: `Pivot` keeps items in display order and follows item resizes; `NavigationHistory` back navigation emits through the interface and no longer keeps duplicate entries; the panel's window filter follows reparenting; tooltips are translatable; long item texts are elided.
- Windows and theme: hit testing on Windows uses device-independent coordinates at any scale factor, also for messages passed to `FramelessWindowHelper::handleNativeEvent()` without a window; the frameless helper is owned by its window and handles Wayland and finished system moves; title bars added later mirror their window; Auto theme can switch back to light where Qt does not report the colour scheme, without polling; `FluentConfig::save()` writes atomically; acrylic surfaces capture the screen only when allowed (never prompting on macOS) and fall back to an opaque tint. On a theme change the window updates its Windows attributes once, after its widgets are restyled, and no longer sets Mica off and on again where Windows recolours it by itself (from Windows 11 22H2): that restarted the backdrop's cross-fade with a jump, a flash 100 ms after every switch.
- Style sheets: `apply()` honours its theme, the manager's own properties and Qt's `_q_` properties no longer repolish, a style that sets properties while it polishes (Qt 6.8's Windows 11 style) no longer repolishes the widget until the stack overflows, registration is constant-time, clearing a custom style sheet takes effect, and lazy refreshes run on show instead of inside paint events. A theme change restyles the visible widgets at once and the others when they are shown or drawn, instead of every registered widget. The window, navigation interface and page stack keep the same style sheet in both themes, whose rules select the theme through the `fqwTheme` property the manager sets on each widget it styles: a changed style sheet on them restyled every page. `setCustomStyleSheet()` with the sheets a widget already has changes nothing. Switching theme in the Gallery took 5.4 s in 0.1.9 on a Windows test machine and takes 0.1 s now.
- Others: `CommandBar::clear()` keeps actions it does not own; `ConfigItemRegistry` reports a change once; option and colour config items correct their defaults; `OrchidLight` is `#C239B3`; `OptionsSettingCard` unchecks every option for an unknown value; `OpacityAniStackedWidget` applies its opacity effect only during transitions; charts handle negative values; `RealtimePlotWidget` stops dragging when the button is gone, exports every sample to CSV, shows negative x while auto-scrolling, fits all data on "View All", recovers its fast path once out-of-order samples age out, gains `setXAxisValueFormatter()`, and starts plot exports in an existing folder (the user's download folder by default) instead of creating `app/download`; `FlowLayout` and `VBoxLayout` follow widgets deleted from outside.
- Builds: on Windows, tests and examples load the rebuilt `FluentQtWidgets.dll` after a change inside the library (the copy beside them only followed relinks); Qt 5 builds no longer use Qt 6-only event signatures or APIs; the library compiles without warnings under `-Wall -Wextra -Wshadow -Wconversion -Woverloaded-virtual`; the macOS Gallery archive is signed ad hoc as a whole bundle, which macOS reported as damaged when only its executable was signed; the macOS archive keeps only the architecture the Gallery is built for and no second copy of the library, and Windows deployments leave out the software OpenGL fallback and the DXC shader compiler, which makes the macOS archive about half and the Windows archive about two thirds of 0.1.9's size.
- Added regression tests for these fixes.

### Additions

- `themedIcon(lightPath, darkPath)` returns an icon that paints the file for the current theme, for custom icons that come as one file per theme. The Gallery's navigation icons use it; they kept the colour of the theme they were created in.
- `AcrylicBrush::setScreenCaptureEnabled()` and `isScreenCaptureEnabled()` switch the screen capture behind acrylic surfaces for the whole application, and `isScreenCaptureAvailable()` tells whether a capture can be taken without side effects.
- `StyleSheetManager::ensureUpdated(root)` restyles the widgets in `root` that a theme change left for when they are shown, for drawing them while hidden; the drill-in page transition uses it for its snapshot.
- `RoundMenu::execAndWait()` shows the menu and returns the triggered action once it is hidden, as `QMenu::exec()` does, also when the menu is deleted while open. `RealtimePlotWidget::setXAxisValueFormatter()` sets the x-axis tick labels.

### Tests

- Grouped the widget tests into one suite per Gallery area (`tst_basic_input`, `tst_combo_box`, `tst_text`, `tst_status_info`, `tst_dialog_flyout`, `tst_menu`, `tst_navigation`, `tst_view`, `tst_layout`, `tst_settings`, `tst_window`, `tst_material`, `tst_charts`, `tst_datetime`), with `tst_theme`, `tst_qss` and `tst_config` for the shared parts and `tst_gallery_pages` and `tst_gallery_translation` for the Gallery. Overlapping tests were merged.
- Every test now starts from the same theme, accent colour and configuration, whatever an earlier test changed, and the suites share their helpers (`tests/TestSupport.h`).
- The Windows CI job builds and runs the tests, including the native hit tests at 150% scaling.

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
