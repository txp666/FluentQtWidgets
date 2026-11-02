# Size parity with PyQt-Fluent-Widgets

`tests/tst_python_parity.cpp` checks that FluentQtWidgets' widgets come out the same size as their
counterparts in PyQt-Fluent-Widgets 1.11.3, the baseline the port follows.

## What it checks

`measure_python.py` builds every widget class that `qfluentwidgets` exports and FluentQtWidgets also has, in
one or more profiles: `default` (no arguments), and where the constructor takes them `text`, `icon` (Mail),
`icon+text`, plus `items`/`horizontal` for a few containers. Each one goes top-left into a large container,
is shown, and has its `size`, `sizeHint`, `minimumSize` and `maximumSize` recorded in
`tests/data/python_sizes.json`. The test builds the same widgets the same way in C++ and compares:

- heights within 1 px; widths within 1 px, or 2 px when the Python run found the width follows the text
  (a longer text made it wider). The 2 px is calibrated by `textMetricsControls`: plain QLabels in the
  Fluent font measure the same in Qt 5.15 and Qt 6.11.
- Both runtimes use Fusion, the light theme, a 14 px application font in the Fluent families, and the same
  offscreen screen (800x600 at 100 logical DPI; `tests/data/parity_offscreen.json` configures Qt 6's).
  The test always runs offscreen, also from a native `ctest`: natively macOS reports 72 DPI, and Fusion's
  metrics and point-sized fonts follow the DPI.
- `qtBaseline` measures plain Qt widgets in both runtimes and lists where Qt 6 itself differs from Qt 5.
  That helps when you sort a row into one of the two tables.

The media classes (`qfluentwidgets.multimedia`) need Qt Multimedia, which the default build doesn't have, so
they are not in the JSON: `measure_python_media.py` prints their sizes and child geometries, and
`tests/tst_media.cpp` checks those values.

The script lists the skipped classes, with reasons, under `skipped` in the JSON: windows, dialogs, pop-ups,
base classes, and widgets attached to another widget.

## Regenerating the Python data

You need a Python environment with PyQt5 5.15 and qfluentwidgets 1.11.3 (for example a virtual environment with
`pip install PyQt5==5.15.* PyQt-Fluent-Widgets==1.11.3`). From the repository root, run the script with that
environment's interpreter, in isolated mode so nothing from the current directory is imported:

```
<venv>/bin/python -I tools/parity/measure_python.py
```

It sets the platform itself and writes `tests/data/python_sizes.json` (sorted keys, so two runs on one machine
give the same file). Text widths depend on the installed fonts. The test skips itself when the data's
resolved font family differs from the one the system resolves, so regenerate on the platform the test runs
on. The checked-in data comes from macOS, which resolves the families to PingFang SC.

To add a class or profile, add it to `PROFILES` in the script and to `factories()` in the test. A row
without a factory, or a factory without a row, fails `tablesAndFactoriesAreCurrent`.

## Reading failures

A row is `Class/profile`. A failing row lists every metric that differs:

```
FAIL!  : PythonParityTest::matchesPython(LineEdit/default) ... (sizeHint.h: C++ 34, Python 24 (tolerance 1))
```

Metrics are `size`, `sizeHint`, `minimumSize` and `maximumSize`, each `.w` or `.h`; `max` is
QWIDGETSIZE_MAX. To see the C++ values for every row, set `FQW_PARITY_DUMP=/tmp/cpp.json`.

## The two tables

Near the top of `tst_python_parity.cpp`, one line per row, grouped by area:
`{"Class/profile", "metrics", "reason"}`, with the metrics separated by spaces, or `"*"` for all of them.

- `kPending`: differences that look like port bugs. Each listed metric is a `QEXPECT_FAIL`, so the suite
  passes and still prints them (`XFAIL`). When a fix makes a metric match, the row reports `XPASS` and
  fails. Remove that metric from the entry, or the whole line once none are left.
- `kIntended`: deliberate differences, for a reason in Qt, C++ or the platform. These metrics are not
  compared. Keep the list short, and give each entry a reason that holds up. A difference you can't justify
  belongs in `kPending`.

A new difference fails as an ordinary failure. Fix it, or add it to one of the tables with a reason.
