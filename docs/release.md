# Release and OTA

## Version Source

The release version is defined by `project(... VERSION ...)` in the root `CMakeLists.txt`.

That version is used by:

- `FluentQt::Version`
- Gallery About/version display
- Gallery update comparisons
- Release asset names

## Release Tags

Release tags use the `vX.Y.Z` format. The current first official release line is `v0.1.2`.

```bash
git tag v0.1.2
git push origin main v0.1.2
```

Pushing a `v*` tag runs the GitHub Actions release job after the CI build matrix passes.

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
cmake --build build/debug --parallel
ctest --test-dir build/debug --output-on-failure
python3 scripts/verify_example_parity.py
```

For GitHub release validation, confirm the tag workflow succeeds and that the release contains all three platform archives.
