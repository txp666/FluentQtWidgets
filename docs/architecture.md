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

   The Gallery and the examples, which use only the public headers and the exported CMake target.

## CMake Targets

The installed package exports `FluentQtWidgets::FluentQtWidgets`. Link the alias the package config adds for it:

```cmake
target_link_libraries(my_app PRIVATE FluentQtWidgets::Widgets)
```

The library includes the media widgets when Qt Multimedia and MultimediaWidgets are found (`FQW_HAS_MULTIMEDIA`).

## Resources

Resources use the `:/qfluentwidgets` prefix for compatibility with the Python reference resource layout. QSS files are split into per-control files:

```text
:/qfluentwidgets/qss/light/*.qss
:/qfluentwidgets/qss/dark/*.qss
```

The files and their rules follow the Python reference (`button.qss`, `line_edit.qss`, `navigation_interface.qss`, ...). Rules select widgets by their `fqw` role property rather than by class name, because the port's classes live in the `FluentQt` namespace.

## Theme

`ThemeManager` owns:

- the requested theme: light, dark or auto
- the effective theme (auto follows the system through `SystemThemeListener` and `QStyleHints::colorSchemeChanged`)
- the accent colour

`themeColor()` returns the accent for the effective theme, as Python's does: in dark mode it is lighter (saturation
× 0.84, value 1, so `#009faa` becomes `#29f1ff`). `derivedThemeColor()` gives the other shades. Widgets that paint
the accent themselves call `themeColor()`; style sheets use the tokens `--ThemeColorPrimary`, `--ThemeColorDark1`
… `--ThemeColorDark3` and `--ThemeColorLight1` … `--ThemeColorLight3`, which the manager replaces when it renders a
sheet.

Qt 6.12, the version CI builds with, is the supported toolchain. The sources also build against Qt 5.15 (`-DFQW_QT_MAJOR_VERSION=5`,
without the media widgets), but CI does not test that.

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

Release tags run the GitHub Actions matrix for Linux, Windows, and macOS. The release job builds `FluentQtWidgets` and `FluentQtWidgetsGallery`, packages the Gallery output with `README.md`, `README_EN.md`, `LICENSE`, `CHANGELOG.md`, and `THIRD_PARTY_NOTICES.md`, and uploads the archives to the matching GitHub Release for Gallery OTA detection.
