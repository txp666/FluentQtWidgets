# Contributing

FluentQtWidgets is a Codex C++ migration of [PyQt-Fluent-Widgets](https://github.com/zhiyiYo/PyQt-Fluent-Widgets). Contributions must treat the Python implementation as the reference for public behavior, visual spacing, Gallery content, resources, and translations.

## Development Setup

Use CMake and a Qt 6 kit:

```powershell
cmake -S . -B build -G Ninja -DFQW_BUILD_EXAMPLES=ON -DFQW_BUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Code Style

- C++17
- Qt parent ownership for widgets and QObject trees
- Public headers under `include/FluentQtWidgets`
- Implementation under matching `src` folders
- Keep examples consuming only public headers
- Re-polish widgets after changing dynamic QSS properties
- Prefer Python-compatible behavior over legacy C++ compatibility branches

Run clang-format before sending a change:

```powershell
$files = Get-ChildItem include,src,examples,tests -Recurse -Include *.h,*.cpp
clang-format -i $files.FullName
```

## Adding A Control

1. Add the public header.
2. Add the implementation source.
3. Register it in `CMakeLists.txt`.
4. Include it from `FluentQtWidgets.h` if it is public.
5. Port or map the Python reference QSS for light and dark themes.
6. Add a standalone example when the Python reference has one.
7. Add or update the Gallery example.
8. Add focused QtTest coverage for low-level behavior when the control owns painting, popup geometry, resource lookup, or model state.
9. Update `docs/api-roadmap.md`, `docs/controls-basic.md`, or the relevant guide.

## Resources

Only add assets with clear licensing. Resources copied from PyQt-Fluent-Widgets must retain the upstream GPL-compatible licensing record in `THIRD_PARTY_NOTICES.md`. Do not import demo music, photos, fonts, or third-party screenshots without a license note.

## Validation Checklist

- Builds on at least one Qt 6 kit
- Public headers compile from a consumer target
- Gallery still runs
- New QSS has light and dark coverage
- Documentation is updated
- Python parity is checked for the touched demo or Gallery page
- `ctest --test-dir build --output-on-failure` passes when tests are enabled

## Conduct

Keep discussion technical, specific, and respectful. Issues and patches include reproduction steps, expected Python reference behavior, actual C++ behavior, platform details, and screenshots when visual parity is involved.
