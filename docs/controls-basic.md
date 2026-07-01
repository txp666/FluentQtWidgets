# Basic Controls

This page documents the controls available in the first C++ milestone.

## Buttons

```cpp
auto *normal = new FluentQt::PushButton("Default");
auto *primary = new FluentQt::PrimaryPushButton("Primary");
auto *transparent = new FluentQt::TransparentPushButton("Transparent");
```

Supported classes:

- `PushButton`
- `PrimaryPushButton`
- `TransparentPushButton`
- `PillPushButton`
- `PillToolButton`
- `HyperlinkButton`
- `HyperlinkToolButton`
- `FilledPushButton`
- `FilledToolButton`
- `TextPushButton`
- `TextToolButton`
- `LuminaPushButton`
- `OutlinedPushButton`
- `OutlinedToolButton`
- `RoundPushButton`
- `RoundToolButton`
- `Chip`
- `Tag`
- `SplitDropButton`
- `PrimarySplitDropButton`
- `SplitWidgetBase`
- `SplitPushButton`
- `SplitToolButton`
- `PrimarySplitPushButton`
- `PrimarySplitToolButton`
- `PrimaryDropDownPushButton`
- `PrimaryDropDownToolButton`
- `TransparentDropDownPushButton`
- `TransparentDropDownToolButton`
- `TogglePushButton`
- `ToggleToolButton`
- `TransparentTogglePushButton`
- `TransparentToggleToolButton`
- `DropDownPushButton`
- `ToolButton`
- `PrimaryToolButton`
- `TransparentToolButton`

Drop-down button variants include an arrow rotation animation when the menu opens or closes. `PushButton`, `ToolButton`, and `HyperlinkButton` expose Python-style `isPressed` and `isHover` state properties for low-level styling and tests. Button icons follow the Python `_icon` model: Fluent buttons keep the icon in the widget and paint it themselves, while the Qt base-class icon is kept empty so the platform style cannot draw a second icon or reserve a conflicting label rect. Split buttons expose the same low-level structure as the Python reference: a `SplitWidgetBase` with a content widget and a replaceable `SplitDropButton` / `PrimarySplitDropButton`, direct accessors for the main button and drop button, and wrapper-level `icon` / `iconSize` forwarding to the main button. Split drop-button height is synchronized from the content widget's size hint/minimum height, matching the Python layout behavior instead of locking in a transient gallery row height.

Transparent, primary, toggle, drop-down, split, and Pro-style colored button variants intentionally share the same Python spacing rules. Icon buttons are custom-painted instead of using Qt's default icon path, which prevents duplicate icons and keeps text, icon, arrow, and status-color spacing aligned with the Python gallery. `Filled*`, `Text*`, `Outlined*`, `LuminaPushButton`, `Chip`, and `Tag` use `ButtonStatus` for information/success/attention/warning/error colors; `setSeverity(InfoBarSeverity)` remains available for code that only needs the original four severity levels. `Chip` exposes a closable tag-like button with `closedSignal()`.

## Inputs

```cpp
auto *edit = new FluentQt::LineEdit;
edit->setPlaceholderText("Name");

auto *search = new FluentQt::SearchLineEdit;
auto *combo = new FluentQt::ComboBox;
auto *slider = new FluentQt::ClickableSlider;
```

Supported classes:

- `LineEdit` (ClearButton, completer, error state, focus underline)
- `SearchLineEdit`
- `PasswordLineEdit` (view-toggle button)
- `TextEdit`
- `PlainTextEdit`
- `TextBrowser`
- `LineEditMenu`
- `TextEditMenu`
- `ComboBox` (item-list Fluent popup + arrow rotation animation)
- `EditableComboBox`
- `ModelComboBox` (model-backed Fluent popup)
- `EditableModelComboBox`
- `Slider` (custom-painted track + handle, hover/press animations)
- `ClickableSlider`
- `SpinBox` (dual-mode: normal / compact, custom buttons, error state)
- `DoubleSpinBox`
- `CompactSpinBox`
- `CompactDoubleSpinBox`
- `DateEdit`
- `DateTimeEdit`
- `TimeEdit`
- `CompactDateEdit`
- `CompactDateTimeEdit`
- `CompactTimeEdit`

`LineEditMenu` and `TextEditMenu` expose the Python reference's Fluent edit context menus for line edits and multiline editors without falling back to Qt's native context menu. `LineEdit` exposes its Python-style low-level structure through `clearButton()`, `hBoxLayout()`, `leftButtons()`, `rightButtons()`, and `completerMenu()`, while `LineEditButton` exposes the bound `action`. `TextEdit`, `PlainTextEdit`, and `TextBrowser` expose their Python-style `layer()` and `scrollDelegate()` objects for focus underline painting and smooth scrolling. `SpinBox` variants reuse `LineEditMenu` for editing commands and expose their Python-style low-level controls through `hBoxLayout()`, `upButton()`, `downButton()`, `compactSpinButton()`, `spinFlyoutView()`, and `spinFlyout()`; `SpinFlyoutView` also exposes `vBoxLayout()`. Compact variants use a Fluent `Flyout` in popup mode so the spin-button flyout is frameless/translucent and closes like the Python reference when focus leaves or symbols are hidden. `Slider` supports `HollowHandleStyle` for a hollow handle appearance, only drags after a left-button press starts on the slider, and exposes `handle()`, `pressedPos`, and `grooveLength` for Python-style low-level customization. `ComboBox` no longer inherits `QComboBox`; it is built from `QPushButton` + `RoundMenu`. `ComboBoxMenu` uses the Python reference's plain `IndicatorMenuItemDelegate` chain rather than the shortcut-menu delegate, and its `MenuActionListWidget` view is explicitly tagged with the `MenuActionListWidget` role so C++ QSS applies the same rounded popup, item padding, and selected indicator spacing as Python. All four combo-box variants expose `dropMenu()`, `isPressed`, and `isHover` so advanced code can inspect the current `ComboBoxMenu` and low-level input state just like the Python reference's `dropMenu`, `isPressed`, and `isHover` attributes. Push-button combo variants also expose `arrowAni()` for the drop-arrow offset animation, while editable combo variants expose their embedded `dropButton()`.

## Date Time

```cpp
auto *calendar = new FluentQt::CalendarPicker;
calendar->setDate(QDate(2026, 6, 23));
calendar->setResetEnabled(true);
```

Supported classes:

- `SeparatorWidget`
- `ItemMaskWidget`
- `PickerColumnFormatter`
- `DigitFormatter`
- `PickerColumnButton`
- `PickerPanel`
- `CalendarView`
- `FastCalendarView`
- `CalendarPicker`
- `FastCalendarPicker`
- `DatePicker`
- `MonthFormatter`
- `ZhFormatter`
- `ZhYearFormatter`
- `ZhMonthFormatter`
- `ZhDayFormatter`
- `TimePicker`
- `MiniuteFormatter`
- `AMHourFormatter`
- `AMPMFormatter`
- `AMTimePicker`

`PickerColumnButton`, `PickerPanel`, `SeparatorWidget`, and `ItemMaskWidget` expose the Python reference's picker foundation for list-column date/time popups. `CalendarView` exposes the Python-style calendar popup structure used by `CalendarPicker`: title, reset, previous/next buttons, 42 date cells, rounded popup chrome, and fade-in animation. `FastCalendarView` uses the Python reference's stacked day/month/year fast-selection structure with separate item grids instead of wrapping the regular `CalendarView`. `DatePicker` and `TimePicker` use Fluent list-column popups rather than native spin boxes or platform dialogs. `DatePicker` also exposes the Python-style year/month/day formatter hooks plus `setMonthTight()`, and `MonthFormatter` follows the original `tr("January")` ... `tr("December")` month-name contract so translators control the default month text. `ZhDatePicker` applies `年`/`月`/`日` suffix formatters. `TimePicker` and `AMTimePicker` expose the Python-style minute, AM-hour, and AM/PM formatters; `AMTimePicker` also supports the original `showSeconds` constructor flag.

## Selection

```cpp
auto *check = new FluentQt::CheckBox("Enabled");
auto *radio = new FluentQt::RadioButton("Choice");
auto *toggle = new FluentQt::SwitchButton;
```

`CheckBox` and `RadioButton` self-paint their Fluent indicators and expose Python-style color and hover/press state properties; `RadioButton` also exposes `indicatorPos` for exact indicator placement. `SwitchButton` exposes `checked`, `indicatorPos`, `checkedChanged(bool)`, and Python-style low-level accessors for `indicator()`, `label()`, `hBox()`, and `hBoxLayout()`.

## Display

```cpp
auto *title = new FluentQt::TitleLabel("Settings");
auto *icon = new FluentQt::IconWidget(FluentQt::FluentIcon::Setting);
auto *image = new FluentQt::ImageLabel(QPixmap("avatar.png"));
auto *avatar = new FluentQt::AvatarWidget(":/images/profile.png");
auto *textAvatar = new FluentQt::AvatarWidget;
textAvatar->setText("Alice");
auto *card = new FluentQt::HeaderCardWidget("General");
card->viewLayout()->addWidget(new FluentQt::BodyLabel("Content"));
```

Supported classes:

- `CaptionLabel`
- `BodyLabel`
- `StrongBodyLabel`
- `SubtitleLabel`
- `TitleLabel`
- `LargeTitleLabel`
- `DisplayLabel`
- `IconWidget`
- `ImageLabel`
- `AvatarWidget`
- `CardWidget`
- `SimpleCardWidget`
- `ElevatedCardWidget`
- `CardSeparator`
- `HeaderCardWidget`
- `CardGroupWidget`
- `GroupHeaderCardWidget`
- `HorizontalSeparator`
- `VerticalSeparator`
- `CycleListWidget`
- `ScrollButton`

`IconWidget` paints a `QIcon` or `FluentIcon` inside a QWidget. `ImageLabel` paints rounded local images. `AvatarWidget` follows the Python constructor semantics where `QString` is an image path, supports `QPixmap` / `QImage` / GIF paths, remote `http(s)` URLs via `setImage()` / `setImageUrl()`, and still supports text fallback avatars through `setText()`. `CardWidget` emits `clicked()` on left-button release like the Python reference, while `HeaderCardWidget`, `CardGroupWidget`, and `GroupHeaderCardWidget` expose the same named child widgets/layouts for advanced composition. `HorizontalSeparator` and `VerticalSeparator` match the Python reference's 3px separator widgets with a theme-aware 1px line. `CycleListWidget` matches the Python picker-column behavior with repeated circular items, hidden smooth scrollbar, hover scroll buttons, and `setSelectedItem()` / `setScrollButtonRepeatEnabled()` helpers.

## Charts

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

Supported classes:

- `ChartWidget`
- `AudioWaveformWidget`
- `RealtimePlotWidget`

`ChartWidget` renders Apache ECharts options inside a FluentQtWidgets application using Qt WebEngine when `Qt6::WebEngineWidgets` is available. It stores the ECharts runtime in the Qt resource system, exposes `option`, `chartTheme`, `setOptionJson()`, and `reload()`, and automatically refreshes on theme/accent changes. macOS app bundle deployment includes a Qt WebEngine helper fix so `QtWebEngineProcess.app` can resolve bundled Qt frameworks.

`AudioWaveformWidget` is a native QWidget renderer for sampled audio amplitudes. It exposes `sampleLevels`, `progress`, bar metrics, light/dark waveform colors, and click/drag signals so callers can use it as a seekable waveform display without WebEngine.

`RealtimePlotWidget` is a native QWidget renderer for live line plots. It keeps bounded per-series buffers, draws dense data through pixel-column min/max aggregation, supports auto-scrolling, wheel zoom, drag pan, crosshair readout, and an in-plot checkbox legend for toggling series visibility.

## Layout

```cpp
auto *flow = new FluentQt::AdaptiveFlowLayout;
flow->setWidgetMinimumWidth(220);
flow->setHorizontalSpacing(12);
```

Supported classes:

- `FlowLayout`
- `AdaptiveFlowLayout`
- `ExpandLayout`
- `VBoxLayout`

`FlowLayout` matches the Python reference's wrapping layout with optional geometry animation and tight hidden-widget mode. `AdaptiveFlowLayout` keeps the same API shape and distributes each row's available width across cards, with optional minimum and maximum card widths. `ExpandLayout` stacks variable-height widgets vertically and updates parent height when a child expands. `VBoxLayout` mirrors the Python helper's widget list plus `addWidgets()`, `deleteWidget()`, and `removeAllWidget()` convenience API.

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

Supported classes:

- `SettingCard`
- `SettingCardGroup`
- `PushSettingCard`
- `PrimaryPushSettingCard`
- `HyperlinkCard`
- `SwitchSettingCard`
- `RangeSettingCard`
- `ComboBoxSettingCard`
- `ColorDialog`
- `ColorPickerButton`
- `ColorSettingCard`
- `ExpandButton`
- `HeaderSettingCard`
- `ExpandSettingCard`
- `GroupSeparator`
- `GroupWidget`
- `ExpandGroupSettingCard`
- `SimpleExpandGroupSettingCard`
- `OptionsSettingCard`
- `FolderItem`
- `FolderListSettingCard`
- `CustomColorSettingCard`

`ExpandSettingCard` is a `QScrollArea`-backed composite like Python: the fixed `HeaderSettingCard` is overlaid above a scrollable `view`, the expand animation drives the vertical scrollbar, and expanded height is computed from the header plus view layout. `FolderListSettingCard` follows the Python header/action split: the Add folder button lives in the header action layout, folder rows live in the expanded view, each `FolderItem` is 53 px high, and the remove tool button is 39 x 29 px. `CustomColorSettingCard` uses the same expandable group structure as Python. The C++ setting cards intentionally expose normal Qt child widgets and signals instead of depending on the Python reference's config item system.
`HyperlinkCard` embeds a Fluent `HyperlinkButton` for repository and documentation links. `HeaderSettingCard`, `ExpandButton`, `GroupWidget`, and `ExpandGroupSettingCard` expose the same composition points as the Python setting-card stack for grouped expandable settings. `ColorDialog` mirrors the Python Fluent color dialog with hue, brightness, RGB, hex, and optional opacity editing; `ColorPickerButton` and the color setting cards open it instead of Qt's built-in color dialog. `FolderListSettingCard` manages a `QStringList` and opens the library's Fluent folder picker only when `chooseFolder()` is called.
Gallery's theme controls are wired through `FluentConfig` and `ThemeManager`: the toolbar theme button persists Light/Dark selection, the settings combo tracks external theme changes, and the Dark switch follows the effective theme when `Theme::Auto` is active.

## Feedback

```cpp
auto *bar = new FluentQt::ProgressBar;
bar->setValue(64);

auto *ring = new FluentQt::ProgressRing;
ring->setValue(72);

FluentQt::InfoBar::success("Saved", "Changes were written.", parent)->showIn(parent);

auto *state = FluentQt::StateToolTip::showIn(parent, "Saving", "Writing settings");
state->setDone(true);
```

Supported classes:

- `ProgressBar`
- `IndeterminateProgressBar`
- `ProgressRing`
- `IndeterminateProgressRing`
- `InfoBar`
- `InfoBarIcon`
- `InfoBarIconWidget`
- `InfoBadge`
- `DotInfoBadge`
- `IconInfoBadge`
- `StateToolTip`
- `ToolTip`
- `ToolTipFilter`
- `FlyoutViewBase`
- `FlyoutView`
- `Flyout`
- `TeachingTipView`
- `TeachingTip`

`ProgressBar` exposes its Python-style `ani()` value animation, while `IndeterminateProgressBar` exposes `shortBarAni()`, `longBarAni()`, `aniGroup()`, and `longBarAniGroup()` for low-level animation inspection. `IndeterminateProgressRing` similarly exposes its start/span angle animations and animation groups. `InfoBadge`, `DotInfoBadge`, and `IconInfoBadge` expose Python-style `target + InfoBadgePosition` factory overloads, `InfoBadge::make()`, text/count factory overloads, and light/dark custom background colors through `lightBackgroundColor` and `darkBackgroundColor`; badge backgrounds are painted by the widgets to match the Python implementation rather than by native/QSS background fallbacks.

`ToolTip` supports the Python reference's 8 positions, screen-bound placement, fade-in, delayed `ToolTipFilter`, and `duration <= 0` non-auto-hide behavior. Its transparent outer popup owns the same low-level `container`, `containerLayout`, `label`, `opacityAni`, and `shadowEffect` structure as the Python implementation, so rounded background and shadow styling live on the inner container. `StateToolTip` provides a compact in-progress/completed/cancelled popup with a loading spinner animation in the RUNNING state, a fade-out animation on completion, Python-style rotate/delta angle properties, and accessors for the title label, content label, and close button. `TeachingTip` supports all 12 tail positions with fade-in/out animation, shadow, and `isDeleteOnClose`. `Flyout` supports `SLIDE_LEFT` / `SLIDE_RIGHT` animation, QPoint and target-widget factories, Python-style `make()` / `create()` overloads, activate-on-show behavior, and outside-click dismissal for both popup and optional `popupMode(false)` tool-window mode; `FlyoutView` exposes icon, image, closeButton, shadow, `addWidget()`, and `isDeleteOnClose`. `InfoBar` exposes Python-style `InfoBarIcon`, `InfoBarIconWidget`, `newInfoBar()` overloads, and accessors for labels, icon widget, close button, title/content/orient/duration/position. `InfoBarManager` provides a slide-in/stacking animation system with support for 7 positions, `addWidget()`, `setCustomBackgroundColor()`, `orient`, and `isClosable`.

## Menus

```cpp
auto *menu = new FluentQt::RoundMenu("Actions", parent);
menu->addAction(FluentQt::FluentIcon::Add, "Create");
menu->addCheckableAction("Pinned", true);
menu->exec(button->mapToGlobal({0, button->height()}));
```

Supported classes:

- `RoundMenu`
- `CheckableMenu`
- `SystemTrayMenu`
- `LabelContextMenu`
- `CommandButton`
- `CommandToolTipFilter`
- `MoreActionsButton`
- `CommandSeparator`
- `CommandMenu`
- `CommandBar`
- `CommandViewMenu`
- `CommandViewBar`
- `CommandBarView`

`RoundMenu` is now backed by `QListWidget` with custom painting, matching the Python reference. It supports sub-menus with animations and edge detection, checkable indicators (radio/check), shadow, separators, keyboard shortcuts, and an animation mask. `CheckableMenu` supports both Radio and Check indicator types. `LabelContextMenu` exposes the Python label copy/select-all context menu and is used by Fluent label classes instead of Qt's standard menu. `CommandButton` mirrors the Python command button size rules, action binding, icon-only detection, tight mode, and command-only tooltip behavior. `MoreActionsButton`, `CommandSeparator`, `CommandMenu`, `CommandViewMenu`, and `CommandViewBar` are public low-level classes aligned with the Python module; `CommandBar` exposes spacing, icon size, tight mode, menu direction, hidden widget/action accessors, and suitable-width resizing.

## Flyouts

`Flyout` and `TeachingTip` are covered under Feedback above (see `FlyoutView`, `Flyout`, `TeachingTipView`, `TeachingTip`).

## Navigation

```cpp
auto *pivot = new FluentQt::Pivot;
pivot->addItem("overview", "Overview");
pivot->addItem("api", "API");

auto *segmented = new FluentQt::SegmentedWidget;
segmented->addItem("day", "Day");
segmented->addItem("week", "Week");

auto *breadcrumb = new FluentQt::BreadcrumbBar;
breadcrumb->addItem("home", "Home");
breadcrumb->addItem("controls", "Controls");
breadcrumb->setCurrentItem("home");

auto *window = new FluentQt::FluentWindow;
window->addSubInterface(new FluentQt::BodyLabel("Home"), FluentQt::icon(FluentQt::FluentIcon::Home),
                        "Home", "home");
window->navigationInterface()->addSeparator();
window->navigationInterface()->addItemHeader("Library");
window->navigationInterface()->addUserCard("user", ":/images/avatar.png", "FluentQt", "fluentqt@example.com");
window->addSubInterface(new FluentQt::BodyLabel("Album 1"), FluentQt::icon(FluentQt::FluentIcon::Folder),
                        "Album 1", "album1", FluentQt::NavigationItemPosition::Scroll, "home");
window->switchTo("home");

auto *bar = new FluentQt::NavigationBar;
bar->addItem("home", FluentQt::icon(FluentQt::FluentIcon::Home), "Home");
bar->setCurrentItem("home");
```

Supported classes:

- `NavigationInterface`
- `NavigationBar`
- `NavigationBarPushButton`
- `NavigationWidget`
- `NavigationAvatarWidget`
- `NavigationUserCard`
- `NavigationItemHeader`
- `Pivot`
- `PivotItem`
- `SegmentedWidget`
- `SegmentedItem`
- `BreadcrumbBar`
- `BreadcrumbItem`

`NavigationInterface` and `FluentWindow` support route keys for Python-style sub-interface switching, nested parent route keys, `addSeparator()`, `addWidget()`, `addItemHeader()`, and `addUserCard()` for Python-style navigation composition. `NavigationPanel::addItem()` and `NavigationInterface::addItem()` accept `selectable=false` for bottom command items such as the Gallery support entry: clicks emit the route key without changing the current page. `NavigationBar` and `NavigationBarPushButton` mirror the Microsoft Store-style vertical bar used by Python `MSFluentWindow`, including 64x58 navigation buttons, selected-state indicator painting, bottom command items, and optional selected-text hiding. `NavigationAvatarWidget`, `NavigationUserCard`, and `NavigationItemHeader` share the same compact/expanded state as built-in navigation items, so split/window demos can use the low-level API instead of gallery-only custom widgets. `BreadcrumbBar` exposes `currentItem`, `currentIndex`, `spacing`, `currentItemChanged`, `currentIndexChanged`, `popItem()`, `itemAt()`, and `setItemText()` for Python-style route-stack navigation. Existing index-based `addPage()` calls remain available; route-key pages use `addSubInterface()` for stable identifiers.

Gallery pages intentionally use the same route keys as the Python reference, such as `basicInputInterface`,
`dateTimeInterface`, `dialogInterface`, `navigationViewInterface`, and `statusInfoInterface`. Use those names when
calling `FluentWindow::switchTo()` from gallery tests or sample-card navigation.

## Tabs

```cpp
auto *tabs = new FluentQt::TabWidget;
tabs->addTab(new FluentQt::BodyLabel("Overview"), "Overview", QIcon(), "overview");
tabs->addTab(new FluentQt::BodyLabel("Settings"), "Settings", QIcon(), "settings");
tabs->setTabsClosable(true);
tabs->setAddButtonVisible(true);
tabs->setMovable(true);
```

Supported classes:

- `TabItem`
- `TabBar`
- `TabWidget`

The current tabs milestone covers route-key management, `TabBar::currentTab()`, `TabWidget::setMovable()`, add/close signals, icon/text tab labels, `QWidget` pages, drag reordering, overflow scrolling, and tab shadows.

## Pips Pager

```cpp
auto *pager = new FluentQt::HorizontalPipsPager;
pager->setPageNumber(9);
pager->setVisibleNumber(5);
pager->setNextButtonDisplayMode(FluentQt::PipsScrollButtonDisplayMode::OnHover);
```

Supported classes:

- `PipsPager`
- `HorizontalPipsPager`
- `VerticalPipsPager`

The C++ pager uses a QWidget composition instead of inheriting `QListWidget`, so `currentIndex()` remains a simple integer API. The current milestone supports page count, visible pip count, horizontal/vertical orientation, current page changes, previous/next button display modes, pixel-scroll animation, Python-style aliases such as `getPageNumber()`, `getVisibleNumber()`, and `isHorizontal()`, and Python-matched 12px transparent scroll buttons. It exposes `isHover()`, `pipContainer()`, and `scrollAnimation()` so tests and advanced styling can inspect the same low-level state as the Python reference.

## Item Views

```cpp
auto *list = new FluentQt::ListWidget;
list->addItems({"Overview", "Settings"});

auto *table = new FluentQt::TableWidget;
table->setBorderVisible(true);

auto *tree = new FluentQt::TreeWidget;
tree->setBorderVisible(true);
```

Supported classes:

- `ListView`
- `ListWidget`
- `TableView`
- `TableWidget`
- `TreeView`
- `TreeWidget`
- `ListItemDelegate`
- `TableItemDelegate`
- `TreeItemDelegate`

The current item-view milestone covers Fluent row backgrounds, selected-row indicators, table/tree border styling, header styling, the `selectRightClickedRow` compatibility property, smooth scrolling via `SmoothScrollDelegate`, tooltip delegates via `ItemViewToolTipDelegate`, and overlay scrollbars. It also exposes Python-style low-level aliases: `delegate()` on list/table/tree views, `scrollDelegate()` on list views, and the original `scrollDelagate()` spelling on table/tree views.

## Scroll

```cpp
auto *area = new FluentQt::SingleDirectionScrollArea(Qt::Vertical);
area->setWidget(content);
area->enableTransparentBackground();
area->verticalFluentScrollBar()->setHandleDisplayMode(
    FluentQt::ScrollBarHandleDisplayMode::OnHover);
```

Supported classes:

- `ArrowButton`
- `ScrollBarGroove`
- `ScrollBarHandle`
- `ScrollBar`
- `SmoothScrollBar`
- `ScrollArea`
- `SingleDirectionScrollArea`
- `SmoothScrollArea`

This is the Fluent scrollbar and scroll-area foundation. It now follows the Python reference's low-level structure: `ScrollBar` owns an `ArrowButton` pair through `ScrollBarGroove` and a separate `ScrollBarHandle`, exposing those subcontrols for styling and tests. It provides custom styling, transparent scroll areas, single-direction policy control, overlay scrollbar geometry, handle/groove fade behavior, and animation settings storage. Python-style accessors are available for low-level parity: `vScrollBar()`, `hScrollBar()`, `scrollDelagate()` (matching the original spelling), `scrollDelegate()`, `SmoothScrollArea::delegate()`, and `SmoothScrollDelegate::vScrollBar()` / `hScrollBar()`.

## Shell

```cpp
auto *window = new FluentQt::FluentWindow;
window->addSubInterface(new FluentQt::BodyLabel("Home"), FluentQt::icon(FluentQt::FluentIcon::Home),
                        "Home", "home");
window->titleBar()->setTitle("Home");

auto *dialog = new FluentQt::Dialog("Title", "Content", parent);
dialog->acceptButton()->setText("Save");
dialog->exec();

auto *splash = new FluentQt::SplashScreen(QPixmap("splash.png"));
splash->show();
```

Supported classes:

- `Dialog` (rounded Fluent popup / fade-in animation)
- `MessageBox`
- `MessageBoxBase`
- `OpacityAniStackedWidget`
- `PopUpAniStackedWidget`
- `TransitionStackedWidget`
- `EntranceTransitionStackedWidget`
- `DrillInTransitionStackedWidget`
- `FluentTitleBar`
- `FluentWidget`
- `SplashScreen` (fade-in/out animation)
- `NavigationInterface`
- `FluentWindow`
- `MSFluentWindow`
- `SplitFluentWindow`

`Dialog`, `MessageBox`, and `MessageBoxBase` expose the Python reference's low-level message-box structure: `view()`, `widget()`, `viewLayout()`, `titleLabel()`, `contentLabel()`, `buttonGroup()`, `yesButton()`, `cancelButton()`, `vBoxLayout()`, `textLayout()`, `buttonLayout()`, `yesSignal`, `cancelSignal`, `setContentCopyable()`, `hideYesButton()`, and `hideCancelButton()`. `Dialog::validate()` is virtual and is called before accepting, matching the Python `MessageBoxBase` customization point. The popup remains frameless and translucent while the inner view owns the rounded background and bottom button group. `FluentWidget` mirrors the Python frameless base window with a `FluentTitleBar`, custom light/dark background colors, a Mica-compatible API, and Windows edge hit-testing for resize. `SplitFluentWindow` mirrors the Python split-window layout by keeping the title bar full-width and leaving page content to reserve title-bar space. The stacked widget family mirrors the Python reference's page transition helpers: fade-in/out, pop-up position animation, entrance transition, and drill-in transition with snapshots. These classes are intended for navigation surfaces and gallery page transitions instead of raw `QStackedWidget` when animated route changes are needed.
