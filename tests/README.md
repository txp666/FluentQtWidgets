# Tests

Qt Test suites, one executable per area of the Gallery, built with `-DFQW_BUILD_TESTS=ON`.

| Suite | Covers |
|---|---|
| `tst_basic_input` | buttons, check boxes, radio buttons, switches, sliders |
| `tst_combo_box` | combo boxes, editable and model combo boxes |
| `tst_text` | labels, line edits, text edits, spin boxes, completers |
| `tst_status_info` | info bars, badges, tooltips, state tooltips, progress |
| `tst_dialog_flyout` | dialogs, message boxes, colour and folder dialogs, flyouts, teaching tips |
| `tst_menu` | round menus, checkable and tray menus, command bars |
| `tst_navigation` | navigation interface, panel and bar, pivot, segmented, breadcrumb, tab bar |
| `tst_view` | list, table and tree views, flip views, cards |
| `tst_scroll` | scroll areas, overlay scroll bars, smooth scrolling, pips pagers |
| `tst_layout` | flow, expand and v-box layouts, separators |
| `tst_settings` | setting cards and groups |
| `tst_window` | Fluent windows, title bars, frameless windows, splash screen, stacked widgets |
| `tst_datetime`, `tst_charts`, `tst_material`, `tst_icon` | the Gallery pages of the same name |
| `tst_media` | media play bars, the volume flyout, the media player and the video widget (only with Qt Multimedia) |
| `tst_theme`, `tst_qss`, `tst_config` | theme and accent colour, style sheet resources, configuration |
| `tst_fonts` | the Fluent fonts the widgets set themselves, under a non-Fluent application font |
| `tst_gallery_pages`, `tst_gallery_translation` | whole Gallery pages, and their translations |
| `tst_python_parity` | sizes of every shared widget against PyQt-Fluent-Widgets |

Helpers shared by the suites live in `TestSupport.h`: rendering a widget (`FqwTest::render`), sampling its
pixels (`countPixels`, `averageGray`), finding pop-ups (`findVisibleTopLevel`) and mask dialogs, which are child
overlays (`findVisibleWidget`, `findVisibleWidgetByRole`), and waiting for animations (`waitForAnimations`). The
widget suites restore the theme and the configuration after each test with `FqwTest::GlobalState`, so a test
sets what it needs and leaves the restore to it; the Gallery suites also point the configuration at a temporary
file, since the Gallery saves it.

A test that needs something the platform may not give skips with a message: `FQW_REQUIRE_PLATFORM("windows")`
for native APIs, `FQW_REQUIRE_MAXIMIZE()` for a real maximize, `FQW_REQUIRE_SCREEN_ROOM(w, h)` for placement.
Wait on the condition (`QTRY_*`, `waitForAnimations`) rather than a fixed time; a fixed `qWait` is only for
checking that nothing happens later.

## Running

Each test carries the label of its area (`text`, `scroll`, `gallery`, ...), plus:

- `smoke`: the fast whole-library checks (`tst_python_parity`, `tst_qss`, `tst_gallery_pages`);
- `native`: tests that need a real windowing system (window activation, a movable pointer, Windows' own
  hit tests). Offscreen they skip with a message saying why.

```
QT_QPA_PLATFORM=offscreen ctest --test-dir build -j4         # everything
QT_QPA_PLATFORM=offscreen ctest --test-dir build -L smoke    # a quick confidence run
QT_QPA_PLATFORM=offscreen ctest --test-dir build -L scroll   # one area
ctest --test-dir build -L native                             # on a desktop, nothing else in front
build/tests/tst_text multilineFocusLineSpansTheBox           # one test function
```

Run suites offscreen by default. Natively, popups close and activation fails when another application
takes the focus, so native runs should be serial and left alone.

## The parity test

`tst_python_parity` builds every widget class shared with PyQt-Fluent-Widgets 1.11.3 in a few profiles (no
arguments, with text, with an icon, with items) and compares size, sizeHint, minimumSize and maximumSize with
`data/python_sizes.json`, which `tools/parity/measure_python.py` records from the Python library. Both sides
run offscreen with Fusion, the light theme and a fixed application font, so a size check under the real style
or after interaction still belongs in its area's suite. Deliberate differences are listed with their reason in
the test's `kIntended` table. See [tools/parity/README.md](../tools/parity/README.md) for regenerating the data.
