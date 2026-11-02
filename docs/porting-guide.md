# Python to C++ Porting Guide

FluentQtWidgets ports PyQt-Fluent-Widgets (`qfluentwidgets` 1.11.3) to C++/Qt Widgets. The Python library is the
reference for behaviour, looks, sizes, examples, resources, Gallery pages and translations. This guide is for
moving code from the Python library to the port, and for contributors who port or fix a widget.

## Names

| Python | C++ |
| --- | --- |
| `from qfluentwidgets import PushButton` | `#include <FluentQtWidgets/FluentQtWidgets.h>`, `FluentQt::PushButton` |
| `setTheme(Theme.DARK)` | `FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Dark)` |
| `setThemeColor("#0078d4")` | `FluentQt::ThemeManager::instance()->setAccentColor(QColor("#0078d4"))` |
| `themeColor()` | `FluentQt::themeColor()` |
| `FluentIcon.HOME` | `FluentQt::FluentIcon::Home`; `FluentQt::icon(FluentIcon::Home)` for a `QIcon` |
| `FluentStyleSheet.BUTTON.apply(w)` | `FluentQt::FluentStyleSheet::apply(w, FluentQt::FluentStyleSheetSource::Button)` |
| `setCustomStyleSheet(w, light, dark)` | `FluentQt::FluentStyleSheet::setCustomStyleSheet(w, light, dark)` |
| `qconfig`, `QConfig` | `FluentQt::FluentConfig`, `ConfigItem` and its subclasses |
| an attribute, `w.hBoxLayout` | an accessor, `w->hBoxLayout()` |
| enum members, `FlyoutAnimationType.SLIDE_LEFT` | `enum class` members, `FluentQt::FlyoutAnimationType::SlideLeft` |

Classes keep their Python names and constructor arguments; Python keyword arguments become positional ones in the
same order. A few classes and methods exist only in the port, such as the `Filled*`, `Outlined*`, `Text*`, `Round*`
and `LuminaPushButton` buttons, `Chip`, `Tag`, `AdaptiveFlowLayout`, the charts, and `RoundMenu::execAndWait()`.
[Controls](controls.md) marks them.

## Differences that follow from C++ and Qt

- Python mixins become small C++ base classes or composition.
- Python style sheets select some widgets by class name. The port's classes live in the `FluentQt` namespace, so a
  bare class name never matches; widgets set an `fqw` role property instead, and the rules select on it.
- After changing a property that a style sheet selects on, unpolish and polish the widget.
- Buttons keep their icon themselves and paint it, as Python's `_icon` does; the Qt base-class icon stays empty so
  the platform style cannot draw a second one.
- `ComboBox` is a `QPushButton` with a `RoundMenu`, as in Python. `PipsPager` and `FlipView` are `QWidget`s, not
  `QListWidget`s, so `currentIndex()` is a plain integer.
- `RoundMenu::exec()` returns at once, as in Python; use `execAndWait()` where C++ code expects `QMenu::exec()`.
- Ownership follows Qt's parent-child model: give every widget a parent or a layout.
- A Python class that is two Qt classes at once becomes one that owns the other: Python's `MediaPlayer` is a
  `QMediaPlayer`, the port's implements `MediaPlayerBase` and hands out its `QMediaPlayer` and `QAudioOutput`
  (`mediaPlayer()`, `audioOutput()`).
- A Python method that subclasses override to supply a part becomes a protected constructor argument: Python's
  `ToolTip._createContainer()` is the `ToolTip` constructor that takes the container, as `AcrylicToolTip` uses.
- Python class attributes that set every instance become per-object settings: `NavigationWidget.EXPAND_WIDTH` is
  `NavigationWidget::setExpandWidth()`, which `NavigationPanel::setExpandWidth()` calls on its items.
- Worker threads hand back a `QImage`, since a `QPixmap` cannot be made off the GUI thread: `BlurCoverThread`
  emits one where Python's emits a `QPixmap`.
- Widgets build their fonts from the Fluent families at Python's pixel sizes, as Python's `setFont()` and
  `getFont()` do, so an application font doesn't change their size.
- Resources keep the `:/qfluentwidgets` prefix and the Python file layout.
- Gallery route keys keep the Python object names (`basicInputInterface`, `dateTimeInterface`,
  `navigationViewInterface`, ...), and the Gallery starts as Python's does: splash screen, pages and navigation,
  then the splash closes.

## Checking a change against Python

Compare with the Python original, not with the port's earlier behaviour: when they differ, Python wins unless Qt,
C++ or the platform gives a reason. Run the Python Gallery or example beside the C++ one.

- **Sizes.** `tst_python_parity` compares the size, size hint and minimum and maximum size of every widget the port
  shares with qfluentwidgets 1.11.3 with measurements taken from Python. A widget that changes size fails it. See
  [tools/parity/README.md](../tools/parity/README.md) to read a failure, record a deliberate difference, or measure
  a new widget.
- **Examples.** `python3 scripts/verify_example_parity.py` checks that every Python example directory has a C++
  counterpart. It reads a `PyQt-Fluent-Widgets/` checkout at the repository root, which git ignores.
- **Behaviour.** Add a regression test to the suite of the widget's area; [tests/README.md](../tests/README.md)
  describes the suites.

## Resources

Copy a missing Gallery or widget resource from PyQt-Fluent-Widgets when the Python behaviour uses it, with its qrc
entry and, for icons, both the black and the white file. Don't add a C++-only resource where a Python one exists.
Large binary assets need a licence check first.

## Licence

A line-by-line translation of the GPL Python implementation is a derivative work. The port stays under a
GPL-compatible licence unless code and resources are rewritten clean-room.
