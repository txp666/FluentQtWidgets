# C++ 版本移植状态

## 目标

基于 Python 版 [PyQt-Fluent-Widgets](https://github.com/zhiyiYo/PyQt-Fluent-Widgets)，建设一个原生 C++/Qt Widgets 开源控件库。C++ 版不是 Python 绑定层，而是面向 CMake、Qt Widgets 桌面应用开发者的独立库。

## 技术栈

- UI 框架：Qt Widgets
- 主线 Qt：Qt 6.5+
- 发布验证 Qt：Qt 6.8.0（GitHub Actions）
- Qt 5.15：保留源码选项，不进入首个正式版发布矩阵
- C++ 标准：C++17
- 构建：CMake 3.21+
- 包管理：CMake package 为主，vcpkg manifest 辅助
- 测试：QtTest + CTest
- 文档：Markdown；API reference 作为独立里程碑接入 Doxygen + Sphinx/Breathe
- CI：GitHub Actions，Windows/macOS/Linux 矩阵

## 架构分层

1. Core

   主题、颜色、配置、样式表、图标、翻译、字体和资源路径。对应 Python 版 `qfluentwidgets/common`。

2. Widgets

   低依赖基础控件：按钮、输入框、选择控件、卡片、标签、进度、提示。对应 Python 版 `components/widgets`。

3. Composition

   导航、对话框、设置卡片、日期时间、菜单、浮层、页签、窗口。对应 Python 版 `components/navigation`、`dialog_box`、`settings`、`date_time`、`window`。

4. Applications

   Gallery、独立示例、文档截图和人工验收场景。示例必须只使用 public headers 和 `FluentQtWidgets::Widgets`。

## 移植原则

- Python 原版是唯一行为和视觉基准。
- 与 Python 冲突的旧 C++ 兼容行为不作为保留目标。
- 资源路径优先保持 `:/qfluentwidgets`。
- Gallery、独立 demo、翻译和 QSS 按 Python 页面结构维护。
- 资源从 Python 原版补齐时同步记录授权来源。

## 首版交付范围

- API/控件映射：确定 `common/components/window/multimedia` 的 public surface 和迁移优先级。
- 资源/QSS：保留 `:/qfluentwidgets` 前缀，迁移 light/dark QSS、图标、i18n。
- 工程化：CMake target、install/export、CI、测试、Gallery target。
- 文档/示例：README、中英文快速开始、迁移指南、Gallery 分类、许可证风险。

这些内容已合并到首个正式版：`Color/Config/Theme/StyleSheet/Translator`、基础控件、组合控件、Gallery、CI、release 资产和文档。

## 迁移阶段

### P0 Core

- 已完成：`ThemeManager`、`SystemThemeListener`、`FluentConfig`、`FluentStyleSheet::render()`、`StyleSheetManager`、`FluentStyleSheetSource` per-control 注册器与 compose 重刷、`FluentThemeColor`、`FluentSystemColor`、`FluentTranslator`、`:/qfluentwidgets/i18n` 翻译资源、动态 SVG 图标路径解析、`QIconEngine` 与 accent 着色 API
- 开放项：更完整的 i18n 字符串迁移与 `lrelease` 自动化 target

### P1 Basic Widgets

- 已完成：按钮（PushButton / PrimaryPushButton / TransparentPushButton / PillPushButton / PillToolButton / HyperlinkButton / SplitDropButton / PrimarySplitDropButton / SplitWidgetBase / SplitPushButton / SplitToolButton / PrimarySplitPushButton / PrimarySplitToolButton / PrimaryDropDownPushButton / PrimaryDropDownToolButton / TransparentDropDownPushButton / TransparentDropDownToolButton / TogglePushButton / ToggleToolButton / TransparentTogglePushButton / TransparentToggleToolButton / DropDownPushButton 等全部变体，下拉按钮含箭头旋转动画）、布局（FlowLayout / AdaptiveFlowLayout / ExpandLayout / VBoxLayout）、标签（textColor / HyperlinkLabel / PixmapLabel / 多角 radius / LabelContextMenu）、DisplayLabel、ImageLabel、IconWidget、AvatarWidget（含 QMovie 本地 GIF / 远程 http(s) 图片加载）、卡片（CardWidget 背景动画 / clicked 信号 / HeaderCardWidget Python 同名子控件和布局访问器 / ElevatedCardWidget 阴影抬升 / CardGroupWidget / GroupHeaderCardWidget / CardSeparator）、设置卡片（含 HyperlinkCard / ExpandButton / HeaderSettingCard / GroupWidget / ExpandGroupSettingCard / SimpleExpandGroupSettingCard）、`ConfigItem` 适配器与 SettingCard 双向绑定、FolderListSettingCard、CustomColorSettingCard、输入框（LineEdit 含 ClearButton / Completer / 错误态 / Focus 下划线 / SearchLineEdit / PasswordLineEdit 视图按钮）、选择控件、ComboBox / EditableComboBox（item-list Fluent 弹出列表 + 箭头旋转动画 + `dropMenu()`）、ModelComboBox / EditableModelComboBox（model-backed Fluent 弹出列表）、滑块（自定义绘制 + HollowHandleStyle + hover/press 动画）、SpinBox（双模式普通/Compact + CompactSpinBox + 自定义按钮 + 错误态）、开关（文本标签 / IndicatorPosition / 禁用态颜色修复）、页签（拖拽/溢出/阴影）、分页点（像素动画）、滚动区域（overlay ScrollBar）、`SmoothScrollDelegate`、`ItemViewToolTipDelegate`、列表/表格/树视图、进度条/进度环（数值动画 / 暂停 / 错误 / 自定义色 / IndeterminateProgressBar / IndeterminateProgressRing）、InfoBar + InfoBarManager（7 种位置 / 滑入堆叠动画 / 图标 SVG）、InfoBadge、`InfoBadgeManager`、list-backed RoundMenu（QListWidget 底层 / 子菜单 / 动画 / 边缘检测 / 可勾选指示器 / 阴影 / 分隔线 / 快捷键 / 动画遮罩）、CheckableMenu（Radio/Check 两种指示器类型）、ToolTip、StateToolTip（加载旋转动画 / 完成/取消状态 / 淡出动画）、Flyout（SLIDE_LEFT/RIGHT 动画 / FlyoutView 增强 / icon/image/closeButton / 阴影 / isDeleteOnClose）、TeachingTip（12 尾部位置 / 淡入淡出动画 / 阴影 / isDeleteOnClose）
- 开放项：RoundMenu 动画细节继续对齐 Python 原版

### P2 Shell

- 已完成：`Dialog`（圆角 Fluent 弹窗 / QGraphicsOpacityEffect + QPropertyAnimation 淡入）、`MessageBox`、`NavigationInterface`、`NavigationPanel`（QPropertyAnimation 展开/折叠、分隔线、嵌套导航项、自定义导航控件、分组标题、用户卡片）、`NavigationBar`、`NavigationBarPushButton`、`NavigationWidget`、`NavigationAvatarWidget`、`NavigationUserCard`、`NavigationItemHeader`、`NavigationTreeWidget`（QPropertyAnimation 展开/折叠）、`NavigationHistory` 返回/历史、`Pivot`（QPropertyAnimation 指示器滑动）、`SegmentedWidget`、`BreadcrumbBar`、`FluentTitleBar`、`FluentWidget`（Python 同名无边框基础窗口 / 自定义背景 / Mica 兼容 API）、`SplashScreen`（QGraphicsOpacityEffect + QPropertyAnimation 淡入/淡出）、`FluentWindow`（含 `goBack()`/`canGoBack()`）、`MSFluentWindow`（NavigationBar 支撑的 Microsoft Store 风格窗口）、`SplitFluentWindow`（全宽标题栏 split 布局）
- 开放项：NavigationPanel acrylic 属性继续对齐 Python 原版

### P3 Rich Controls

- 已完成：`TabBar`/`TabWidget` 拖拽排序、溢出滚动与阴影、`PipsPager` 像素动画、`TeachingTip` 完整 12 尾部位置与淡入淡出动画、`SmoothScrollDelegate`、Picker 底层（SeparatorWidget / ItemMaskWidget / PickerColumnButton / PickerPanel）、`CalendarView` / `FastCalendarView`、`CalendarPicker`（公开底层日历弹窗 / QGraphicsOpacityEffect + QPropertyAnimation 淡入 / 网格线显示）、`DatePicker` / `TimePicker` / `AMTimePicker`（共享 list-column `PickerPanel` 弹窗）及 QtTest
- 开放项：TeachingTip 动画细节继续对齐 Python 原版

### P4 Platform（首版外里程碑）

- 首版外：更完整的 frameless window 后端、Mica/Acrylic 平台后端、multimedia 扩展、Qt Designer 插件、CPack/vcpkg/conan 发布流水线

## 命名策略

当前 C++ API 使用 `FluentQt` 命名空间并保留 Python 常用类名：

```cpp
FluentQt::PushButton
FluentQt::PrimaryPushButton
FluentQt::NavigationInterface
FluentQt::FluentWindow
```

这样调用更短，也能通过命名空间避免和 Qt 原生类混淆。

## 资源策略

资源前缀保持：

```text
:/qfluentwidgets
```

原因是 Python 参考版的 QSS、图标和翻译路径都围绕这个前缀组织。C++ 版可内部拆成多个 `.qrc`，但导出的资源路径不应轻易改变。

## 许可证策略

当前实现参考 GPL Python 项目推进，根目录采用 GPL-3.0-or-later。若未来希望换成 MIT、Apache-2.0 或 LGPL，需要重做 clean-room 实现并审计资源授权。
