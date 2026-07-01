# Third Party Notices

## PyQt-Fluent-Widgets Reference

Upstream project: [PyQt-Fluent-Widgets](https://github.com/zhiyiYo/PyQt-Fluent-Widgets).

This repository is a C++/Qt Widgets port of the upstream Python project. The upstream implementation is the reference for API inventory, behavior, examples, resource layout, Gallery structure, QSS, and translation parity. The upstream checkout is intentionally not committed to this repository.

The reference project is GPLv3 licensed. This C++ project follows GPL-compatible licensing because the implementation and selected resources are derivative of the Python reference rather than a clean-room rewrite.

The C++ port copies selected SVG icon resources from the reference project into `resources/images/icons` so the themed icon engine can resolve `:/qfluentwidgets/images/icons/*_{black,white}.svg` paths. These copied SVG files remain under the reference project's GPL license terms.

Gallery image resources copied from the reference gallery into `examples/gallery/resource/images` are included under the reference project's GPL license terms to improve visual parity with the original example gallery.

## Qt

Qt is provided under GPL, LGPL, and commercial license options depending on modules and distribution model. Users are responsible for complying with the Qt license that applies to their build.

## Apache ECharts

ChartWidget bundles Apache ECharts from https://echarts.apache.org/ in `resources/chart/echarts.min.js`.

ECharts is licensed under the Apache License 2.0.
