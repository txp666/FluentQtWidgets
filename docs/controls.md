# Controls

An overview of the library's widgets by area, with a short example for each. Every class lives in the
`FluentQt` namespace and comes with `#include <FluentQtWidgets/FluentQtWidgets.h>`. The Gallery
(`examples/gallery`) shows each control live, and `examples/` holds a standalone demo for most of them.

Class names, constructors and low-level accessors follow PyQt-Fluent-Widgets 1.11.3: a Python attribute
such as `widget.hBoxLayout` becomes `widget->hBoxLayout()`, and a Python `Signal` becomes a Qt signal of the
same name. Where the reference misspells a name (`scrollDelagate`, `MiniuteFormatter`), the port keeps the
spelling and, for `scrollDelagate()`, also offers `scrollDelegate()`.

## Buttons

```cpp
auto *normal = new FluentQt::PushButton("Default");
auto *primary = new FluentQt::PrimaryPushButton("Primary");
auto *transparent = new FluentQt::TransparentPushButton("Transparent");
```

- Push and tool buttons: `PushButton`, `PrimaryPushButton`, `TransparentPushButton`, `ToolButton`,
  `PrimaryToolButton`, `TransparentToolButton`, `PillPushButton`, `PillToolButton`, `HyperlinkButton`.
- Toggle: `TogglePushButton`, `ToggleToolButton`, `TransparentTogglePushButton`, `TransparentToggleToolButton`.
- Drop-down: `DropDownPushButton`, `DropDownToolButton`, `PrimaryDropDownPushButton`, `PrimaryDropDownToolButton`,
  `TransparentDropDownPushButton`, `TransparentDropDownToolButton`.
- Split: `SplitWidgetBase`, `SplitPushButton`, `SplitToolButton`, `PrimarySplitPushButton`,
  `PrimarySplitToolButton`, `SplitDropButton`, `PrimarySplitDropButton`.
- Not in the Python library: `FilledPushButton`, `FilledToolButton`, `OutlinedPushButton`, `OutlinedToolButton`,
  `TextPushButton`, `TextToolButton`, `LuminaPushButton`, `RoundPushButton`, `RoundToolButton`, `HyperlinkToolButton`,
  `Chip`, `Tag`.

Buttons keep their icon themselves and paint it, as Python's `_icon` does; the Qt base-class icon stays empty so
the platform style cannot draw a second one. `PushButton`, `ToolButton` and `HyperlinkButton` expose `isPressed`
and `isHover` properties. Drop-down buttons rotate their arrow while the menu is open. A split button is a
`SplitWidgetBase` holding the main button and a replaceable drop button, with accessors for both; its `icon` and
`iconSize` go to the main button, and the drop button takes its height from the main button.

All buttons share the Python spacing: push buttons are 33 px tall and tool buttons 38 × 32. The port-only
buttons use the same box. `Filled*`, `Text*`, `Outlined*`, `LuminaPushButton`, `Chip` and `Tag` take a
`ButtonStatus` (information, success, attention, warning, error) for their colour, or an `InfoBarSeverity` through
`setSeverity()`. `Chip` can be closable and emits `closedSignal()`.

## Inputs

```cpp
auto *edit = new FluentQt::LineEdit;
edit->setPlaceholderText("Name");

auto *search = new FluentQt::SearchLineEdit;
auto *combo = new FluentQt::ComboBox;
combo->addItems({"Light", "Dark"});
auto *slider = new FluentQt::Slider(Qt::Horizontal);
```

- Text: `LineEdit`, `SearchLineEdit`, `PasswordLineEdit`, `TextEdit`, `PlainTextEdit`, `TextBrowser`,
  `LineEditMenu`, `TextEditMenu`, `CompleterMenu`.
- Combo boxes: `ComboBox`, `EditableComboBox`, `ModelComboBox`, `EditableModelComboBox`, `ComboBoxMenu`.
- Sliders: `Slider`, `ClickableSlider`, `HollowHandleStyle`.
- Spin boxes and editors: `SpinBox`, `DoubleSpinBox`, `DateEdit`, `TimeEdit`, `DateTimeEdit`, and their `Compact*`
  variants.

`LineEdit` has a clear button, a completer, an error state and a focus underline, and exposes `clearButton()`,
`hBoxLayout()`, `leftButtons()`, `rightButtons()` and `completerMenu()`. While the completer menu is open, typing
goes to the edit and the arrow keys move through the completions. Line edits are 33 px tall. `TextEdit`,
`PlainTextEdit` and `TextBrowser` draw the focus line along the bottom of the editor and expose `layer()` and
`scrollDelegate()`. All editors use the Fluent `LineEditMenu` / `TextEditMenu` as their context menu.

`ComboBox` is a `QPushButton` with a `RoundMenu` popup, not a `QComboBox`. All four combo boxes expose `dropMenu()`,
`isPressed` and `isHover`; the push-button variants also `arrowAni()`, the editable ones `dropButton()`. Editable
combo boxes are styled as the line edits they are.

`Slider` paints its own track and handle (22 px) with hover and press animations, and only drags after a left
press on the slider; `handle()`, `pressedPos` and `grooveLength` are exposed. `ClickableSlider` is a plain
`QSlider` that jumps to the clicked position. Both are vertical unless an orientation is passed, as with
`QSlider`. Applying `FluentStyleSheetSource::Slider` to any `QSlider` gives it the Fluent style sheet, and
`HollowHandleStyle` draws a hollow handle.

Spin boxes reuse `LineEditMenu`, have an error state and expose `hBoxLayout()`, `upButton()`, `downButton()`,
`compactSpinButton()`, `spinFlyoutView()` and `spinFlyout()`. The compact variants open their buttons in a `Flyout`.

## Selection

```cpp
auto *check = new FluentQt::CheckBox("Enabled");
auto *radio = new FluentQt::RadioButton("Choice");
auto *toggle = new FluentQt::SwitchButton;
```

`CheckBox` and `RadioButton` paint their own indicators and expose their colour and hover/press state as
properties; `RadioButton` also `indicatorPos`. `SwitchButton` exposes `checked`, `indicatorPos`,
`checkedChanged(bool)`, `indicator()`, `label()`, `hBox()` and `hBoxLayout()`.

## Date and time

```cpp
auto *calendar = new FluentQt::CalendarPicker;
calendar->setDate(QDate(2026, 6, 23));
calendar->setResetEnabled(true);
```

- Pickers: `DatePicker`, `ZhDatePicker`, `TimePicker`, `AMTimePicker`, `CalendarPicker`, `FastCalendarPicker`.
- Calendar views: `CalendarView`, `FastCalendarView`.
- Building blocks: `PickerPanel`, `PickerColumnButton`, `SeparatorWidget`, `ItemMaskWidget`,
  `PickerColumnFormatter`, `DigitFormatter`, `MonthFormatter`, `ZhFormatter`, `ZhYearFormatter`,
  `ZhMonthFormatter`, `ZhDayFormatter`, `MiniuteFormatter`, `AMHourFormatter`, `AMPMFormatter`.

`DatePicker` and `TimePicker` open a panel of scrolling columns on a click. The picker takes its width from its
columns, and the panel opens over it, centred on the current value. `DatePicker` takes year, month and day
formatters and `setMonthTight()`; `MonthFormatter` uses `tr("January")` … `tr("December")`, so translations decide
the month names. `ZhDatePicker` adds the `年`/`月`/`日` suffixes, and `AMTimePicker` can show seconds.
`CalendarView` is the popup of `CalendarPicker`: a title, previous/next buttons and 42 day cells.
`FastCalendarView` switches between day, month and year pages. With `setResetEnabled(true)` the calendar shows a
reset button.

## Text and display

```cpp
auto *title = new FluentQt::TitleLabel("Settings");
auto *icon = new FluentQt::IconWidget(FluentQt::FluentIcon::Setting);
auto *image = new FluentQt::ImageLabel(QPixmap("avatar.png"));
auto *avatar = new FluentQt::AvatarWidget(":/images/profile.png");
auto *textAvatar = new FluentQt::AvatarWidget;
textAvatar->setText("Alice");
```

- Labels: `CaptionLabel`, `BodyLabel`, `StrongBodyLabel`, `SubtitleLabel`, `TitleLabel`, `LargeTitleLabel`,
  `DisplayLabel`, `HyperlinkLabel`, `FluentLabelBase`.
- Images and icons: `IconWidget`, `ImageLabel`, `PixmapLabel`, `AvatarWidget`.
- Other: `HorizontalSeparator`, `VerticalSeparator`, `CycleListWidget`, `ScrollButton`.

Labels paint black or white text by theme; `setTextColor()` sets a colour per theme that wins over the label
style sheet. Right-clicking a label opens `LabelContextMenu` (copy, select all). `IconWidget` paints a `QIcon` or
`FluentIcon` filling its rect; it has no size hint, so give it a size. `ImageLabel` paints local or animated
images with rounded corners. `AvatarWidget` takes an image path (also GIF), a `QPixmap` / `QImage`, or a remote
`http(s)` URL, and falls back to the initial of `setText()`. The separators are 3 px wide with a 1 px line.
`CycleListWidget` is the endless scrolling column used by the pickers.

## Cards

```cpp
auto *card = new FluentQt::HeaderCardWidget("General");
card->viewLayout()->addWidget(new FluentQt::BodyLabel("Content"));
```

`CardWidget`, `SimpleCardWidget`, `ElevatedCardWidget`, `HeaderCardWidget`, `CardGroupWidget`,
`GroupHeaderCardWidget`, `CardSeparator`.

`CardWidget` fades its background between the normal, hover and pressed colours and emits `clicked()` on a
left-button release. `ElevatedCardWidget` shows a shadow only under the mouse. `SimpleCardWidget` has no layout of
its own. `HeaderCardWidget`, `CardGroupWidget` and `GroupHeaderCardWidget` expose their named child widgets and
layouts; their titles follow theme changes.

## Charts

These widgets are not in the Python library. They paint natively, without Qt WebEngine, and follow theme and
accent changes.

```cpp
QJsonObject option{
    {"xAxis", QJsonObject{{"type", "category"}, {"data", QJsonArray{"Mon", "Tue"}}}},
    {"yAxis", QJsonObject{{"type", "value"}}},
    {"series", QJsonArray{QJsonObject{{"type", "bar"}, {"data", QJsonArray{120, 200}}}}}
};
auto *chart = new FluentQt::ChartWidget(option);

auto *waveform = new FluentQt::AudioWaveformWidget;
waveform->setSamples({0.1, 0.4, 0.9, 0.3});
waveform->setProgress(0.5);

auto *plot = new FluentQt::RealtimePlotWidget;
int memorySeries = plot->addSeries("Memory");
plot->appendSample(0, 42.0);
plot->appendSample(memorySeries, 58.0);
```

- `ChartWidget` takes an ECharts-style JSON option and draws bar, line, area, mixed bar/line, donut, scatter,
  radar, gauge and heatmap charts, with a staggered reveal animation, data labels and hover feedback. It exposes
  `option`, `chartTheme`, `setOptionJson()` and `reload()`.
- `AudioWaveformWidget` draws sampled audio levels with playback progress. It exposes `sampleLevels`, `progress`,
  bar metrics and light/dark colours, and emits click and drag signals, so it can serve as a seek bar.
- `RealtimePlotWidget` draws live line plots from bounded per-series buffers and stays fast with dense data. It
  auto-scrolls, zooms with the wheel, pans by dragging, shows a crosshair readout, offers view and display
  controls on right-click, exports CSV and images, and toggles series from its legend.
  `setXAxisValueFormatter()` formats the x-axis labels.

## Layout

```cpp
auto *flow = new FluentQt::AdaptiveFlowLayout;
flow->setWidgetMinimumWidth(220);
flow->setHorizontalSpacing(12);
```

`FlowLayout` wraps widgets into rows, optionally animated, with `setVerticalAlignment()` for each row.
`AdaptiveFlowLayout` (not in Python) spreads each row's width across its widgets, within optional minimum and
maximum widths. `ExpandLayout` stacks widgets of changing height and resizes its parent. `VBoxLayout` keeps its
widget list and adds `addWidgets()`, `deleteWidget()` and `removeAllWidget()`.

## Settings

```cpp
auto *group = new FluentQt::SettingCardGroup("Personalization");
group->addSettingCard(new FluentQt::SwitchSettingCard(
    FluentQt::FluentIcon::Setting, "Dark theme", "Use the dark Fluent palette."));
group->addSettingCard(new FluentQt::RangeSettingCard(
    80, 140, 100, FluentQt::FluentIcon::Volume, "Interface scale"));
group->addSettingCard(new FluentQt::FolderListSettingCard(
    {"C:/Projects"}, FluentQt::FluentIcon::Folder, "Project folders"));
```

- Cards: `SettingCard`, `SettingCardGroup`, `PushSettingCard`, `PrimaryPushSettingCard`, `HyperlinkCard`,
  `SwitchSettingCard`, `RangeSettingCard`, `ComboBoxSettingCard`, `ColorSettingCard`, `OptionsSettingCard`,
  `FolderListSettingCard`, `CustomColorSettingCard`.
- Expandable: `ExpandSettingCard`, `HeaderSettingCard`, `ExpandButton`, `ExpandGroupSettingCard`,
  `SimpleExpandGroupSettingCard`, `GroupWidget`, `GroupSeparator`, `FolderItem`.
- Colour: `ColorDialog`, `ColorPickerButton`, `HuePanel`, `BrightnessSlider`, `ColorCard`, `HexColorLineEdit`,
  `OpacityLineEdit`.
- Configuration: `FluentConfig`, `ConfigItem`, `RangeConfigItem`, `OptionsConfigItem`, `ColorConfigItem`,
  `ConfigItemRegistry`.

Cards expose their child widgets and Qt signals; binding them to a `ConfigItem` is optional.
`SwitchSettingCard` shows its On/Off text left of the switch. `RangeSettingCard::slider()` is a `Slider`.
`ComboBoxSettingCard` keeps the combo box as wide as its items. `ColorDialog` edits hue, brightness, RGB, hex and,
optionally, opacity; `ColorSettingCard`'s `ColorPickerButton` and `CustomColorSettingCard`'s Choose color
`chooseColorButton()` open it instead of Qt's colour dialog, with alpha when built with `enableAlpha`.

`ExpandSettingCard` is a `QScrollArea` with a `HeaderSettingCard` over the content view: actions go into the
header, content into `viewLayout()`. Expanding animates the visible height of the view. In a resizable outer
`QScrollArea`, give the content's layout `QLayout::SetMinAndMaxSize` so the scroll range follows the cards' height;
the library does not set it on your layouts. `FolderListSettingCard` keeps a `QStringList`, puts its Add folder
button in the header and one 53 px `FolderItem` per folder in the view, and asks before removing a folder.

## Status and info

```cpp
auto *bar = new FluentQt::ProgressBar;
bar->setValue(64);

auto *ring = new FluentQt::ProgressRing;
ring->setValue(72);

FluentQt::InfoBar::success("Saved", "Changes were written.", Qt::Horizontal, true, 2000,
                           FluentQt::InfoBarPosition::TopRight, parent);

auto *state = FluentQt::StateToolTip::showIn(parent, "Saving", "Writing settings");
state->setDone(true);
```

- Progress: `ProgressBar`, `IndeterminateProgressBar`, `ProgressRing`, `IndeterminateProgressRing`.
- Info bars: `InfoBar`, `InfoBarIcon`, `InfoBarIconWidget`, `InfoBarManager` and its per-position subclasses.
- Badges: `InfoBadge`, `DotInfoBadge`, `IconInfoBadge`, `InfoBadgeManager` and its per-position subclasses.
- Tooltips: `ToolTip`, `ToolTipFilter`, `StateToolTip`, `ItemViewToolTipDelegate`.

Progress bars and rings animate their value and have paused and error states; their animations are exposed
(`ani()`, `shortBarAni()`, `longBarAni()` and the animation groups). `InfoBar::info()`, `success()`, `warning()`,
`error()` and `newInfoBar()` create and show a bar; `InfoBarManager` places it at one of six positions and stacks and
slides the bars. `InfoBadge` and its variants attach to a target at an `InfoBadgePosition`, have count and text
factories (`InfoBadge::make()`), and take light and dark background colours.

`ToolTip` has eight positions, stays on the screen, fades in and does not take focus; `ToolTipFilter` shows it
after a delay, and a duration of 0 or less keeps it shown until the pointer leaves. `StateToolTip` shows a running,
done or cancelled state, with a spinner while running and a fade-out at the end.

## Dialogs and flyouts

```cpp
auto *dialog = new FluentQt::Dialog("Title", "Content", parent);
dialog->yesButton()->setText("Save");
dialog->exec();

FluentQt::Flyout::create("Tip", "Saved to your library.", FluentQt::icon(FluentQt::FluentIcon::Info), QPixmap(),
                         true, button, parent);
```

- Dialogs: `Dialog`, `MessageBox`, `MessageBoxBase`, `MessageDialog`, `MaskDialogBase`, `ColorDialog`,
  `FolderListDialog` (with `ClickableWindow`, `FolderCard`, `AddFolderCard`).
- Flyouts: `Flyout`, `FlyoutView`, `FlyoutViewBase`, `TeachingTip`, `PopupTeachingTip`, `TeachingTipView`.

`Dialog`, `MessageBox` and `MessageBoxBase` expose `view()`, `widget()`, `viewLayout()`, `titleLabel()`,
`contentLabel()`, `buttonGroup()`, `yesButton()`, `cancelButton()`, `vBoxLayout()`, `textLayout()`,
`buttonLayout()`, `yesSignal`, `cancelSignal`, `setContentCopyable()`, `hideYesButton()` and
`hideCancelButton()`. Return accepts the dialog through the virtual `validate()`. A `MessageBox` is at least as
wide as its title, message and buttons, including buttons added to `buttonLayout()`.

`FolderListDialog(folderPaths, title, content, parent)` is Python's masked folder list: an `AddFolderCard` that
opens a `QFileDialog`, one 292 x 72 `FolderCard` per folder that asks before removing it, and a Done
`completeButton()` that emits `folderChanged(QStringList)` when the folders changed, styled by
`folder_list_dialog.qss`.

`Flyout` has `make()` and `create()` factories for a point or a target widget, `FlyoutAnimationType` animations
(pull-up, drop-down, slide left/right, fade-in, none), and closes on an outside click; `FlyoutView` takes an icon,
an image, a close button and extra widgets. `TeachingTip` points at its target from one of twelve tail positions
and fades in and out.

## Menus and command bars

```cpp
auto *menu = new FluentQt::RoundMenu("Actions", parent);
menu->addAction(FluentQt::FluentIcon::Add, "Create");
menu->addCheckableAction("Pinned", true);
menu->exec(button->mapToGlobal(QPoint(0, button->height())), true);
```

- Menus: `RoundMenu`, `CheckableMenu`, `SystemTrayMenu`, `CheckableSystemTrayMenu`, `LabelContextMenu`, and the
  delegates `MenuItemDelegate`, `ShortcutMenuItemDelegate`, `IndicatorMenuItemDelegate`,
  `RadioIndicatorMenuItemDelegate`, `CheckIndicatorMenuItemDelegate`.
- Command bars: `CommandBar`, `CommandButton`, `CommandSeparator`, `MoreActionsButton`, `CommandMenu`,
  `CommandToolTipFilter`, `CommandBarView`, `CommandViewBar`, `CommandViewMenu`.

`RoundMenu` is a `QListWidget`-based menu with submenus, shortcuts, check and radio indicators, embedded widgets,
a shadow and open animations (`MenuAnimationType`); it moves to stay on the screen. Drop-down and pull-up menus
open at their final place and slide their list in. `exec()` shows the menu and returns at once, as in Python;
`execAndWait()` returns the triggered action once the menu closes, like `QMenu::exec()`. The tray menus leave out
the shadow margins on the right and bottom. `CommandBar` lays out `CommandButton`s, moves what does not fit into
the more-actions menu, and passes its font to its buttons.

## Navigation

```cpp
auto *pivot = new FluentQt::Pivot;
pivot->addItem("overview", "Overview");
pivot->addItem("api", "API");

auto *breadcrumb = new FluentQt::BreadcrumbBar;
breadcrumb->addItem("home", "Home");
breadcrumb->addItem("controls", "Controls");
breadcrumb->setCurrentItem("home");

auto *window = new FluentQt::FluentWindow;
window->addSubInterface(new FluentQt::BodyLabel("Home"), FluentQt::icon(FluentQt::FluentIcon::Home),
                        "Home", "home");
window->navigationInterface()->addSeparator();
window->navigationInterface()->addItemHeader("Library");
window->addSubInterface(new FluentQt::BodyLabel("Album 1"), FluentQt::icon(FluentQt::FluentIcon::Folder),
                        "Album 1", "album1", FluentQt::NavigationItemPosition::Scroll, "home");
window->switchTo("home");
```

- Side navigation: `NavigationInterface`, `NavigationPanel`, `NavigationWidget`, `NavigationToolButton`,
  `NavigationTreeWidget`, `NavigationAvatarWidget`, `NavigationUserCard`, `NavigationItemHeader`,
  `NavigationHistory`.
- Navigation bar: `NavigationBar`, `NavigationBarPushButton` (the layout of `MSFluentWindow`).
- Top navigation: `Pivot`, `PivotItem`, `SegmentedWidget`, `SegmentedItem`, `SegmentedToolWidget`, `SegmentedToolItem`, `SegmentedToggleToolWidget`,
  `BreadcrumbBar`, `BreadcrumbItem`.
- Tabs: `TabBar`, `TabItem`, `TabWidget`.
- Pages: `OpacityAniStackedWidget`, `PopUpAniStackedWidget`, `TransitionStackedWidget`,
  `EntranceTransitionStackedWidget`, `DrillInTransitionStackedWidget`.

`NavigationInterface` wraps a `NavigationPanel` as in Python: it is 48 px wide when compact, as wide as the panel
when expanded, and holds no pages. `FluentWindow` keeps the page stack beside it and switches pages on the panel's
`itemClicked()`; a window of your own does the same with its own stack. Items are identified by route keys. `addSubInterface()` and `addItem()` take a parent key for nested items, and
`addItem()` takes `selectable = false` for command items that emit their key without changing the page. The panel
expands in place when the window is at least `setMinimumExpandWidth()` wide (any width with 0) and opens as a menu
otherwise. `NavigationBar` buttons are 64 × 58. `BreadcrumbBar` paints text-high `BreadcrumbItem` widgets with
the chevron inside each one (20 px for its 14 px font) and hides the leading items behind an ellipsis menu when they
do not fit; it exposes `currentItem()`, `currentIndex`, `spacing`, `popItem()`, `item()`, `itemAt()` and
`setItemText()`.

```cpp
auto *tabs = new FluentQt::TabWidget;
tabs->addTab(new FluentQt::BodyLabel("Overview"), "Overview", QIcon(), "overview");
tabs->setTabsClosable(true);
tabs->setAddButtonVisible(true);
tabs->setMovable(true);
```

`TabBar` has route keys, add and close signals, drag reordering, scrolling when the tabs overflow, and tab shadows;
`TabWidget` puts it over a page stack.

The stacked widgets animate page changes: fade, pop-up, entrance and drill-in.

## Scrolling and item views

```cpp
auto *area = new FluentQt::SingleDirectionScrollArea(Qt::Vertical);
area->setWidget(content);
area->enableTransparentBackground();
area->verticalFluentScrollBar()->setHandleDisplayMode(FluentQt::ScrollBarHandleDisplayMode::OnHover);

auto *table = new FluentQt::TableWidget;
table->setBorderVisible(true);

auto *pager = new FluentQt::HorizontalPipsPager;
pager->setPageNumber(9);
pager->setVisibleNumber(5);
```

- Scrolling: `ScrollArea`, `SingleDirectionScrollArea`, `SmoothScrollArea`, `ScrollBar`, `SmoothScrollBar`,
  `ScrollBarGroove`, `ScrollBarHandle`, `ArrowButton`, `SmoothScrollDelegate`.
- Item views: `ListView`, `ListWidget`, `TableView`, `TableWidget`, `TreeView`, `TreeWidget`, `ListItemDelegate`,
  `TableItemDelegate`, `TreeItemDelegate`.
- Paging: `PipsPager`, `HorizontalPipsPager`, `VerticalPipsPager`, `FlipView`, `HorizontalFlipView`,
  `VerticalFlipView`.

Scroll areas use overlay Fluent scroll bars that fade in, and smooth wheel scrolling. Once
`enableTransparentBackground()` is on, widgets set later are transparent too. Item views paint Fluent rows with a
selection indicator, optional borders and Fluent headers; tables select several rows with Ctrl and Shift. Views
expose `delegate()` and `scrollDelegate()` (`scrollDelagate()` on tables and trees), and the scroll areas
`vScrollBar()` and `hScrollBar()`.

`PipsPager` and `FlipView` are `QWidget`s rather than `QListWidget`s, so `currentIndex()` is a plain integer.
The pager shows a window of pips with previous/next buttons (`PipsScrollButtonDisplayMode`); `FlipView` shows
images one at a time (`addImages()`) with hover buttons.

## Windows

```cpp
auto *window = new FluentQt::FluentWindow;
window->addSubInterface(new FluentQt::BodyLabel("Home"), FluentQt::icon(FluentQt::FluentIcon::Home),
                        "Home", "home");
window->titleBar()->setTitle("Home");

auto *splash = new FluentQt::SplashScreen(QPixmap("splash.png"));
splash->show();
```

`FluentWidget`, `FluentWindow`, `MSFluentWindow`, `SplitFluentWindow`, `FluentTitleBar`, `SplashScreen`,
`FramelessWindowHelper`.

The windows are frameless with a `FluentTitleBar`, resize from their edges, and use Mica on Windows 11
(`setMicaEffectEnabled()`). `FluentWidget` is the base window with custom light and dark backgrounds.
`FluentWindow` adds side navigation and back history, `MSFluentWindow` the Microsoft Store navigation bar, and
`SplitFluentWindow` a full-width title bar over the navigation. `SplashScreen` covers the window while it builds and
fades out.

## Material

```cpp
auto *search = new FluentQt::AcrylicSearchLineEdit;
auto *combo = new FluentQt::AcrylicComboBox;
auto *menu = new FluentQt::AcrylicMenu("Actions", parent);
```

`AcrylicBrush`, `AcrylicLabel`, `AcrylicLineEdit`, `AcrylicSearchLineEdit`, `AcrylicComboBox`,
`AcrylicEditableComboBox`, `AcrylicMenu`, `AcrylicCheckableMenu`, `AcrylicSystemTrayMenu`,
`AcrylicCheckableSystemTrayMenu`, `AcrylicFlyout`, `AcrylicFlyoutView`, `AcrylicFlyoutViewBase`, `AcrylicToolTip`,
`AcrylicToolTipFilter`.

`AcrylicBrush` blurs a screen capture or an image and adds tint, luminosity and noise; the acrylic widgets use it for
their popups. `AcrylicBrush::setScreenCaptureEnabled()` turns screen capture off for the whole application, and
where a capture is not available (`isScreenCaptureAvailable()`) the surfaces fall back to an opaque tint.

## Media

Built when Qt Multimedia and MultimediaWidgets are found (`FQW_HAS_MULTIMEDIA`).

`SimpleMediaPlayBar`, `StandardMediaPlayBar` and `VideoWidget` play audio and video through a `QMediaPlayer`
(`player()`), with Fluent play, volume and progress controls.

## Theme, style sheets and icons

See [Architecture](architecture.md) for how themes and style sheets work. In short: `ThemeManager` sets the
light, dark or automatic theme and the accent colour, `themeColor()` gives the accent for the current theme,
`FluentStyleSheet::setCustomStyleSheet()` adds per-theme style sheets to a widget, `FluentIcon` has 174 themed
icons, and `themedIcon(lightPath, darkPath)` builds a themed icon from your own files.
