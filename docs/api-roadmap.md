# API Status

## P0 Core

- Theme manager with light, dark, and auto modes; auto resolves through `SystemThemeListener`, and Gallery theme entry points keep `FluentConfig` plus settings-page controls synchronized
- Accent color tokens and derived theme colors
- QSS loading, per-control source registry, and token replacement
- SVG icon engine with theme-aware bundled assets and optional tint recoloring
- Translator loading from `:/qfluentwidgets/i18n`; Gallery locale changes rebuild the Gallery window in-process and preserve the current route/window state
- Config items and JSON persistence

Status: complete for the P0-P3 first official release scope. Open release-track items: broader i18n coverage and automated `lrelease` in CI.

## P1 Basic Controls

- Buttons: PushButton, PrimaryPushButton, TransparentPushButton, PillPushButton, PillToolButton, HyperlinkButton, SplitDropButton, PrimarySplitDropButton, SplitWidgetBase, SplitPushButton, SplitToolButton, PrimarySplitPushButton, PrimarySplitToolButton, PrimaryDropDownPushButton, PrimaryDropDownToolButton, TransparentDropDownPushButton, TransparentDropDownToolButton, TogglePushButton, ToggleToolButton, TransparentTogglePushButton, TransparentToggleToolButton, ToolButton, PrimaryToolButton, TransparentToolButton, DropDownPushButton (with arrow rotation animation)
- Inputs: LineEdit (ClearButton / Completer / error state / focus underline / Fluent edit context menu with icons), AcrylicLineEdit / AcrylicSearchLineEdit (acrylic completer + context menu), SearchLineEdit, PasswordLineEdit, TextEditMenu, LineEditMenu, ComboBox / EditableComboBox (item-list Fluent popup + arrow animation + plain indicator delegate + exposed `dropMenu()`), AcrylicComboBox / AcrylicEditableComboBox (acrylic popup + completer/context menu parity), ModelComboBox / EditableModelComboBox (model-backed Fluent popup), Slider (custom paint + HollowHandleStyle + hover/press animation + press-start drag guard), ClickableSlider, SpinBox family (inline/compact variants + popup-mode Fluent flyout + Fluent edit menu + exposed spin buttons/flyout + error state), DoubleSpinBox
- Selection: CheckBox, RadioButton (self-painted Fluent indicators / Python-style color and state properties), SwitchButton (text label / IndicatorPosition / fixed disabled color)
- Display: labels (textColor / HyperlinkLabel / PixmapLabel / multi-radius), IconWidget, ImageLabel, AvatarWidget (Python-style image path constructor + GIF paths + remote http(s) loading), AcrylicBrush (screen/image blur + tint/luminosity/noise texture + clip path), AcrylicLabel (blur/tint material + Python-style image path overload), CardWidget (background animation + clicked signal), SimpleCardWidget, HeaderCardWidget (Python-style child widget/layout accessors), ElevatedCardWidget, CardGroupWidget, GroupHeaderCardWidget, CardSeparator, HorizontalSeparator, VerticalSeparator, CycleListWidget (picker column / circular scrolling / hover scroll buttons)
- Layout: FlowLayout, AdaptiveFlowLayout (responsive card widths), ExpandLayout (variable-height vertical expansion), VBoxLayout (tracked vertical widget list)
- Scroll: ArrowButton, ScrollBarGroove, ScrollBarHandle, ScrollBar, SmoothScrollBar, ScrollArea, SingleDirectionScrollArea, SmoothScrollArea, SmoothScrollDelegate (including Python-style `vScrollBar` / `hScrollBar` / `scrollDelagate` / `delegate` accessors)
- Item Views: ListView, ListWidget, TableView, TableWidget, TreeView, TreeWidget, delegates, tooltip delegates, Python-style `delegate` / `scrollDelegate` / `scrollDelagate` accessors
- Feedback: ProgressBar (value animation / pause / error / custom color), ProgressRing, IndeterminateProgressBar, IndeterminateProgressRing, InfoBar + InfoBarManager (7 positions / slide-in stacking / SVG icons), InfoBadge, InfoBadgeManager, ToolTip, AcrylicToolTip / AcrylicToolTipFilter, StateToolTip (loading spinner / done/cancel / fade-out), Flyout (SLIDE_LEFT/RIGHT animation / optional popupMode), AcrylicFlyout / AcrylicFlyoutView / AcrylicFlyoutViewBase (AcrylicBrush-backed popup material), FlyoutView (icon/image/closeButton/shadow/isDeleteOnClose), TeachingTip (12 tail positions / fade-in/out animation / shadow / isDeleteOnClose)
- Menus: RoundMenu (QListWidget-backed / sub-menu / animation / edge detection / shadow / separators / shortcuts / embedded widgets), MenuItemDelegate / ShortcutMenuItemDelegate / IndicatorMenuItemDelegate, AcrylicMenu / AcrylicCheckableMenu / AcrylicSystemTrayMenu (acrylic menu view and embedded widget menu examples), CheckableMenu (Radio/Check indicators), SystemTrayMenu, LabelContextMenu, CommandButton, CommandToolTipFilter, MoreActionsButton, CommandSeparator, CommandMenu, CommandBar, CommandViewMenu, CommandViewBar, CommandBarView
- Settings: full SettingCard family including HyperlinkCard, HeaderSettingCard, ExpandGroupSettingCard, ConfigItem two-way binding, Fluent color dialog, and Fluent folder picker

Status: complete for the P0-P3 first official release scope. Open release-track item: richer menu animations.

## P2 Application Shell

- NavigationInterface, NavigationPanel (QPropertyAnimation expand/collapse, separators, nested parent route items, custom navigation widgets, item headers, user cards), NavigationBar, NavigationBarPushButton, NavigationWidget, NavigationAvatarWidget, NavigationUserCard, NavigationItemHeader, NavigationTreeWidget (QPropertyAnimation expand/collapse), NavigationHistory, StackedWidget transition family (opacity / pop-up / entrance / drill-in)
- Pivot (QPropertyAnimation indicator slide), SegmentedWidget, BreadcrumbBar (Python-style current item/index, spacing, pop, and item text APIs)
- Dialog / MessageBoxBase (rounded Fluent popup + QGraphicsOpacityEffect + QPropertyAnimation fade-in + overridable validation), MessageBox, FluentTitleBar, FluentWidget (frameless base window + custom background + Mica-compatible API), SplashScreen (fade-in/out animation), FluentWindow with back navigation, MSFluentWindow with NavigationBar-backed Microsoft Store layout, SplitFluentWindow with full-width split title bar layout
- Gallery app aligned with Python structure (14 pages: Home banner, LinkCard, SampleCard, ExampleCard, ToolBar)

Status: complete for the P0-P3 first official release scope. Open release-track items: acrylic platform backends and frameless integration refinements.

## P3 Rich Controls

- Picker foundation (SeparatorWidget / ItemMaskWidget / PickerColumnButton / PickerPanel), CalendarView / FastCalendarView (stacked day/month/year fast-selection grids), CalendarPicker / FastCalendarPicker (rounded popup + QGraphicsOpacityEffect + QPropertyAnimation fade-in / grid lines), DatePicker / TimePicker / AMTimePicker (shared list-column `PickerPanel` popup)
- TeachingTip (12 tail positions / fade-in/out animation), Flyout (SLIDE_LEFT/RIGHT animation / outside-click dismissal)
- Tab/Pips/TeachingTip/SmoothScroll refinements shipped with QtTest coverage, including `TabBar::currentTab()` and `TabWidget::setMovable()`

Status: complete for the P0-P3 first official release scope.

## P4 Platform And Optional Modules

- Frameless window integration
- Mica/Acrylic effects with platform-specific backends
- Multimedia widgets
- Qt Designer plugin
- Packaging with CPack, vcpkg, Conan

Status: deferred outside the first official release scope.
