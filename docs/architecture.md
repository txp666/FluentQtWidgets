# Architecture

FluentQtWidgets is organized as a native C++ QWidget library. It is not a Python binding layer.

## Layers

1. Core

   `ThemeManager`, `FluentStyleSheet`, `FluentIcon`, configuration, translator services, resource lookup, and theme listeners.

2. Controls

   Small widgets built on Qt primitives: buttons, labels, cards, inputs, sliders, switches, progress controls, InfoBar.

3. Composition

   Dialogs, navigation, settings cards, date-time pickers, tab/flyout/menu systems, and window shells.

4. Applications

   Gallery and examples that only consume public headers and exported CMake targets.

## CMake Targets

The project exposes one public target:

```cmake
FluentQtWidgets::Widgets
```

An alias target is also provided for compatibility:

```cmake
FluentQtWidgets::FluentQtWidgets
```

Optional modules are tracked as deferred release targets:

- `FluentQtWidgets::Multimedia`
- `FluentQtWidgets::DesignerPlugin`
- `FluentQtWidgets::WindowEffects`

## Resources

Resources use the `:/qfluentwidgets` prefix for compatibility with the Python reference resource layout. QSS files are split into per-control files:

```text
:/qfluentwidgets/qss/light/*.qss
:/qfluentwidgets/qss/dark/*.qss
```

Each control (e.g. `button.qss`, `line_edit.qss`, `navigation_interface.qss`) has its own QSS file, matching the Python reference layout.

## Theme

`ThemeManager` owns:

- requested theme: light, dark, auto
- resolved effective theme
- accent color
- application or widget style application

Qt 6 uses `QStyleHints::colorSchemeChanged` where available for system theme updates. The Qt 5 source path is legacy and is not part of the release CI matrix.

## Style Management

`StyleSheetManager` registers Fluent widgets when `FluentStyleSheet::setRole()` or `FluentStyleSheet::apply()` is called. Registered widgets are repolished when the theme or accent color changes.

Custom per-widget QSS can be attached with:

```cpp
FluentQt::FluentStyleSheet::setCustomStyleSheet(
    widget,
    "QWidget { color: --ThemeColorPrimary; }",
    "QWidget { color: --ThemeColorPrimary; background: #202020; }");
```

The manager stores separate light and dark snippets, renders theme tokens, and switches between them when the theme changes; set them once, as setting the same snippets again does nothing.

On a theme change, visible widgets restyle at once and hidden ones when they are shown. Code that draws a hidden widget, for example with `grab()`, calls `StyleSheetManager::ensureUpdated()` on it first.

Setting a style sheet that differs from the previous one makes Qt restyle the widget and everything inside it. For a container that holds many widgets, such as a window or a page stack, keep one style sheet for both themes and select the theme in its rules: the manager sets the `fqwTheme` property to `light` or `dark` on every widget it styles.

```css
QStackedWidget[fqw="NavigationStack"][fqwTheme="light"] { background-color: rgba(255, 255, 255, 128); }
QStackedWidget[fqw="NavigationStack"][fqwTheme="dark"] { background-color: rgba(255, 255, 255, 8); }
```

## API Style

The current C++ API keeps short class names inside the `FluentQt` namespace:

```cpp
FluentQt::PushButton
FluentQt::NavigationInterface
FluentQt::FluentWindow
```

This avoids verbose call sites while still separating Fluent classes from Qt classes.

## Release Packages

Release tags run the GitHub Actions matrix for Linux, Windows, and macOS. The release job builds `FluentQtWidgets` and `FluentQtWidgetsGallery`, packages the Gallery output with `LICENSE`, `CHANGELOG.md`, and `THIRD_PARTY_NOTICES.md`, and uploads the archives to the matching GitHub Release for Gallery OTA detection.
