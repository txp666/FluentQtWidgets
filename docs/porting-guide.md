# Python to C++ Porting Guide

## Scope

The reference package exports `common`, `components`, `window`, and a separate `multimedia` package. The C++ port migrates by dependency order, not directory order.

## Priority Map

| Python API | C++ API | Status |
| --- | --- | --- |
| `Theme` | `FluentQt::Theme` | First milestone |
| `setTheme`, `themeColor` | `FluentQt::ThemeManager` | First milestone |
| `FluentStyleSheet` | `FluentQt::FluentStyleSheet` | First milestone |
| `FluentIcon` | `FluentQt::FluentIcon` | Resource-backed themed SVG icons |
| `PushButton`, `PrimaryPushButton` | same class names in `FluentQt` | First milestone |
| `ToolButton` | `FluentQt::ToolButton` | First milestone |
| `PillPushButton`, `PillToolButton` | same class names in `FluentQt` | First milestone |
| `HyperlinkButton` | `FluentQt::HyperlinkButton` | First milestone |
| `SplitPushButton`, `SplitToolButton` | same class names in `FluentQt` | First milestone |
| `PrimaryDropDownPushButton`, `PrimaryDropDownToolButton` | same class names in `FluentQt` | First milestone |
| `TransparentDropDownPushButton`, `TransparentDropDownToolButton` | same class names in `FluentQt` | First milestone |
| `TogglePushButton`, `ToggleToolButton` | same class names in `FluentQt` | First milestone |
| `TransparentTogglePushButton`, `TransparentToggleToolButton` | same class names in `FluentQt` | First milestone |
| `CheckBox`, `RadioButton` | same class names in `FluentQt` | First milestone |
| `LineEdit`, `SearchLineEdit`, `PasswordLineEdit` | same class names in `FluentQt` | First milestone, ClearButton / Completer / error state |
| `ComboBox`, `EditableComboBox` | same class names in `FluentQt` | Item-list popup with Fluent styling and `dropMenu()` access |
| `ModelComboBox`, `EditableModelComboBox` | same class names in `FluentQt` | Model-backed popup list with Fluent styling and `dropMenu()` access |
| `Slider`, `ClickableSlider` | same class names in `FluentQt` | First milestone, custom paint + HollowHandleStyle |
| `SpinBox`, `DoubleSpinBox`, `CompactSpinBox` | same class names in `FluentQt` | First milestone, dual-mode + error state |
| `DateEdit`, `DateTimeEdit`, `TimeEdit` | same class names in `FluentQt` | Fluent picker panels |
| `SwitchButton` | `FluentQt::SwitchButton` | First milestone, text label / IndicatorPosition |
| `CardWidget`, `ElevatedCardWidget`, `CardGroupWidget` | same class names in `FluentQt` | First milestone, background animation / shadow |
| `HyperlinkLabel`, `PixmapLabel` | same class names in `FluentQt` | First milestone, textColor / multi-radius |
| `InfoBar` | `FluentQt::InfoBar` | First milestone, InfoBarManager / 7 positions / slide-in animation |
| `StateToolTip` | `FluentQt::StateToolTip` | First milestone, loading spinner / fade-out animation |
| `ProgressBar`, `IndeterminateProgressBar` | same class names in `FluentQt` | First milestone, value animation / pause / error |
| `ProgressRing`, `IndeterminateProgressRing` | same class names in `FluentQt` | First milestone, value animation / pause / error |
| `RoundMenu`, `CheckableMenu`, `LabelContextMenu` | same class names in `FluentQt` | First milestone, QListWidget-backed / sub-menu / checkable indicators / label copy menu |
| `Flyout`, `FlyoutView` | same class names in `FluentQt` | First milestone, SLIDE_LEFT/RIGHT animation |
| `TeachingTip` | `FluentQt::TeachingTip` | First milestone, 12 tail positions / fade animation |
| `Dialog`, `MessageBox` | same class names in `FluentQt` | Rounded Fluent popup, fade-in animation |
| `ColorDialog`, `HuePanel`, `BrightnessSlider`, `ColorCard` | same class names in `FluentQt` | Fluent color dialog with hex/RGB/opacity editing, no native `QColorDialog` |
| `FluentTitleBar` | `FluentQt::FluentTitleBar` | First milestone |
| `SplashScreen` | `FluentQt::SplashScreen` | First milestone, fade-in/out animation |
| `NavigationPanel` | `FluentQt::NavigationPanel` | First milestone, QPropertyAnimation expand/collapse |
| `NavigationTreeWidget` | `FluentQt::NavigationTreeWidget` | First milestone, QPropertyAnimation expand/collapse |
| `Pivot` | `FluentQt::Pivot` | First milestone, QPropertyAnimation indicator slide |
| `PickerColumnButton`, `PickerPanel`, `CalendarView`, `CalendarPicker`, `DatePicker`, `TimePicker` | same class names in `FluentQt` | Rounded Fluent popup, fade-in animation, shared list-column picker foundation |
| `NavigationInterface` | `FluentQt::NavigationInterface` | First milestone |
| `FluentWindow.addSubInterface()` | `FluentQt::FluentWindow::addSubInterface()` | Route-key MVP |
| `FluentWindow` | `FluentQt::FluentWindow` | First milestone |
| `FolderListSettingCard` | `FluentQt::FolderListSettingCard` | QStringList model with Fluent folder picker |
| `CustomColorSettingCard` | `FluentQt::CustomColorSettingCard` | Default/custom color switching with Fluent color dialog |

## Key Differences

- Python multiple inheritance mixins become composition or small C++ base classes.
- Python dynamic QSS class selectors become dynamic properties or exact C++ class names.
- Every runtime property used by QSS must be followed by polish/unpolish.
- Button icons follow the Python `_icon` storage model. Fluent button classes keep the Qt base-class icon empty
  and paint the stored icon themselves, including wrapper classes such as split buttons that forward through a base
  `QPushButton*` or `QToolButton*`.
- Button spacing follows Python QSS constants across normal, transparent, primary, toggle, drop-down, and split
  variants. Do not add C++ compatibility padding that conflicts with Python text/icon/arrow positions.
- Popup list views that rely on Python class-name QSS, such as `MenuActionListWidget`, use an explicit `fqw` role in
  C++ so rounded corners, padding, and indicator spacing are applied reliably.
- `ExpandSettingCard` ports keep the Python `QScrollArea + HeaderSettingCard + view/space/border widgets`
  structure. Adding action widgets goes through the header card, while expandable content goes through `viewLayout()`.
- QObject ownership follows Qt parent-child ownership. Avoid raw owning pointers without parents.
- Python resource paths stay under `:/qfluentwidgets` unless all QSS and icon paths are migrated together.
- Gallery route keys keep the Python object names (`basicInputInterface`, `dateTimeInterface`,
  `navigationViewInterface`, and so on). Do not add parallel C++-style aliases unless the Python gallery also exposes
  them.
- Style-sheet ownership follows the Python package layout. For example, `RadioButton` is styled from `button.qss`
  through the `RadioButton` role instead of a separate `radio_button.qss` resource.

## Parity Checks

Run these checks before treating a porting pass as complete:

```bash
python3 scripts/verify_example_parity.py
cmake --preset ninja-debug -DFQW_BUILD_TESTS=ON
cmake --build build/debug --parallel
ctest --test-dir build/debug --output-on-failure
ctest --test-dir build/debug --output-on-failure -R tst_basic_input_parity
```

`verify_example_parity.py` compares the C++ example directory set with `PyQt-Fluent-Widgets/examples`. The CTest suite
covers the base widget behavior, gallery route/translation behavior, and representative demo/gallery integration points.
`tst_basic_input_parity` specifically guards the Python-style button icon paint path, combo popup role/delegate setup,
the `FolderListSettingCard` expandable structure, and representative basic input controls used by the standalone demos.

Gallery parity is checked against the Python pages after every broad UI pass. The current gallery keeps the
Python startup sequence: show the splash first, construct pages/navigation, then finish the splash. Menus, Flyout,
TeachingTip, Dialog, InfoBar, settings cards, and translated page text are treated as Python-owned behavior; remove
old C++ behavior when it conflicts.

## Resource Rules

Missing gallery or component resources are copied from `PyQt-Fluent-Widgets` when they are part of the Python
reference behavior, with the matching qrc entry and themed black/white icon pair. Do not introduce alternate C++
resources for a control when the Python asset exists. Large binary assets still need an explicit license check before
being added.

## License Rule

A line-by-line translation from the GPL Python implementation is a derivative work. Keep GPL-compatible licensing unless the code and resources are rebuilt clean-room.
