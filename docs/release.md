# Release and OTA

## Version Source

The release version is defined by `project(... VERSION ...)` in the root `CMakeLists.txt`.

That version is used by:

- `FluentQt::Version`
- Gallery About/version display
- Gallery update comparisons
- Release asset names
- CMake package version metadata

Keep `include/FluentQtWidgets/Version.h` and `vcpkg.json` on the same release number so installed headers and package manifests remain consistent.

## Release Tags

Release tags use the `vX.Y.Z` format. The current release line is `v0.1.6`.

Create a matching release-notes file before tagging:

```text
docs/release-notes/vX.Y.Z.md
```

```bash
git tag v0.1.6
git push origin main v0.1.6
```

Pushing a `v*` tag runs the GitHub Actions release job after the CI build matrix passes.
The release job uses `docs/release-notes/${GITHUB_REF_NAME}.md` as the GitHub Release description.
Release CI installs Qt Multimedia explicitly. `ChartWidget` is rendered natively, so Gallery
release archives do not need Qt WebEngine unless a standalone WebEngine example is packaged
separately.

## Release Assets

The release workflow builds `FluentQtWidgets` and `FluentQtWidgetsGallery` on:

- `ubuntu-latest`
- `windows-latest`
- `macos-15`

It uploads one Gallery archive per platform:

- `FluentQtWidgetsGallery-ubuntu-latest-vX.Y.Z.tar.gz`
- `FluentQtWidgetsGallery-windows-latest-vX.Y.Z.zip`
- `FluentQtWidgetsGallery-macos-15-vX.Y.Z.tar.gz`

Each archive includes the Gallery output, `LICENSE`, `CHANGELOG.md`, and `THIRD_PARTY_NOTICES.md`. Windows archives include the deployed runtime directory produced by `windeployqt`.

## Gallery OTA

Gallery uses the default update channel:

```text
https://github.com/txp666/FluentQtWidgets/releases/latest
```

The update checker compares the current CMake project version with the latest GitHub Release tag. A newer tag is reported as an available update. The release assets are the downloadable OTA payloads.

## Verification

Run the local checks before moving a release tag:

```bash
cmake --preset ninja-debug
cmake --build --preset debug --parallel
ctest --preset debug --output-on-failure
python3 scripts/verify_example_parity.py
```

`scripts/verify_example_parity.py` confirms that every Python reference example directory has a C++ counterpart. C++-only examples are allowed when they demonstrate Qt/C++ specific APIs or composed controls.

On Windows MinGW, use the matching preset instead:

```powershell
cmake --preset mingw-debug
cmake --build --preset mingw-debug --parallel
ctest --preset mingw-debug --output-on-failure
python scripts/verify_example_parity.py
```

Before tagging, confirm `git status --short` is clean and that local reference checkouts such as `PyQt-Fluent-Widgets/` are not tracked.

For GitHub release validation, confirm the tag workflow succeeds and that the release contains all three platform archives.
