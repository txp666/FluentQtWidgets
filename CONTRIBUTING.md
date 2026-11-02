# Contributing

FluentQtWidgets is a C++/Qt Widgets port of [PyQt-Fluent-Widgets](https://github.com/zhiyiYo/PyQt-Fluent-Widgets)
1.11.3. The Python implementation is the reference for public behaviour, looks, sizes, Gallery content, resources
and translations; the [porting guide](docs/porting-guide.md) explains how the port maps it.

## Development Setup

Use CMake and a Qt 6 kit. On macOS and Linux:

```bash
cmake --preset ninja-debug
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
```

On Windows, see [Quick start](docs/quick-start.md#windows) for the `mingw-debug` preset and Qt kit setup.

[tests/README.md](tests/README.md) describes the test suites and where a new test goes.

## Code Style

- C++17, with Qt parent ownership for widgets and `QObject` trees
- Public headers under `include/FluentQtWidgets`, implementation under the matching `src` folders
- Examples use only the public headers
- Unpolish and polish a widget after changing a property its style sheet selects on
- Follow the Python behaviour rather than keeping older C++ behaviour around

Format the lines you change with clang-format; the repository's `.clang-format` holds the style:

```bash
git clang-format
```

## Adding A Control

1. Add the public header and the implementation, and list both in `CMakeLists.txt`.
2. Include the header from `FluentQtWidgets.h` if it is public.
3. Port the Python style sheet rules for the light and dark themes, selecting on the widget's `fqw` role.
4. Add a standalone example when the Python reference has one, and update the Gallery page.
5. Add QtTest coverage when the control owns painting, popup geometry, resource lookup or model state.
6. If the control exists in Python, add it to the size parity test (see below).
7. Update [Controls](docs/controls.md) and `CHANGELOG.md`.

## Checking A Change Against Python

- Run the Python Gallery or example beside the C++ one and compare states, spacing and animations.
- `tst_python_parity` compares every shared widget's size, size hint and minimum and maximum size with
  measurements of the Python original, so a change that makes a widget a different size from Python fails it.
  [tools/parity/README.md](tools/parity/README.md) explains how to read a failure, how to record a difference that
  has a Qt, C++ or platform reason, and how to measure a new widget.
- `python3 scripts/verify_example_parity.py` checks that every Python example has a C++ counterpart (it needs a
  `PyQt-Fluent-Widgets/` checkout at the repository root).

## Resources

Only add assets with clear licensing. Resources copied from PyQt-Fluent-Widgets keep the upstream GPL licensing
recorded in `THIRD_PARTY_NOTICES.md`. Do not import demo music, photos, fonts or third-party screenshots without a
licence note.

The README screenshots and the Gallery tour GIF are rendered by `tools/screenshots`
([README](tools/screenshots/README.md)); render them again there when a change alters what they show.

## Validation Checklist

- Builds on at least one Qt 6 kit, and the tests pass (`ctest --preset debug --output-on-failure`)
- The Gallery still runs
- New style sheet rules cover the light and dark themes
- The change matches Python (see above)
- Documentation and `CHANGELOG.md` are updated
- For a release: the version agrees in `CMakeLists.txt`, `include/FluentQtWidgets/Version.h`, `vcpkg.json` and the
  [release docs](docs/release.md)

## Conduct

Keep discussion technical, specific and respectful. Issues and patches include reproduction steps, the Python
reference behaviour, the actual C++ behaviour, platform details, and screenshots when the look is involved.
