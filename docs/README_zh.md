# FluentQtWidgets

FluentQtWidgets 是一个面向 C++/Qt Widgets 的 Fluent Design 风格控件库，包含完整的组件实现与 Gallery 示例应用。

本项目为 **Codex 从 Python 版 [PyQt-Fluent-Widgets](https://github.com/zhiyiYo/PyQt-Fluent-Widgets) 移植而来的 C++/Qt Widgets 版本**。Python 原版是本仓库的行为与视觉基准：控件状态、间距、QSS 常量、示例内容、资源路径、Gallery 页面结构和翻译均按 Python 原版对齐；只有 Qt/C++ 平台差异不可避免时才保留差异说明。

## 发行版本

- **当前版本**：`0.1.2`
- **发布页**：<https://github.com/txp666/FluentQtWidgets/releases>
- **仓库**：<https://github.com/txp666/FluentQtWidgets>
- **OTA 源**：<https://github.com/txp666/FluentQtWidgets/releases/latest>

Gallery 已内置更新检查能力：

- 手动检查：设置页 → 软件更新 → 检查更新。
- 启动检查：设置页 → 软件更新 → 开机自动检查（可关闭）。
- OTA 产物：CI 在 tag 构建后上传 Linux、Windows、macOS 的 Gallery 压缩包到同一个 GitHub Release。

## 环境要求

- CMake 3.21+
- C++17
- Qt 6.5+（Core/Gui/Widgets/Svg/Network），可选：
  - Multimedia + MultimediaWidgets
  - WebEngineWidgets

## 构建

```bash
cmake -S . -B build -DFQW_BUILD_EXAMPLES=ON
cmake --build build --parallel
```

### 启动 Gallery

```bash
cmake --build build --target FluentQtWidgetsGallery
./build/examples/gallery/FluentQtWidgetsGallery
# macOS:
# ./build/examples/gallery/FluentQtWidgetsGallery.app/Contents/MacOS/FluentQtWidgetsGallery
```

## 示例程序

```bash
cmake --build build --target fqw_basic_input_button_demo
cmake --build build --target fqw_basic_input_check_box_demo
cmake --build build --target fqw_basic_input_combo_box_demo
cmake --build build --target fqw_basic_input_model_combo_box_demo
cmake --build build --target fqw_basic_input_radio_button_demo
cmake --build build --target fqw_basic_input_slider_demo
cmake --build build --target fqw_basic_input_switch_button_demo
```

（全部示例目标可从各子目录 `CMakeLists.txt` 查看）

## 文档

- [English README](../README.md)
- [快速开始](quick-start.md)
- [发布与 OTA](release.md)
- [架构设计](architecture.md)
- [基础控件文档](controls-basic.md)
- [移植说明](porting-guide.md)
- [API 路线图](api-roadmap.md)
- [贡献指引](../CONTRIBUTING.md)

## 发布说明

- 版本号通过 `project(... VERSION ...)` 统一定义。
- 版本号用于 About 页显示、Gallery 更新检查和 release 资产命名。
- 发布规则见 [发布与 OTA](release.md)。

## 许可

本仓库以 **GPL-3.0-or-later** 发布。上游来源与资源授权见 [Third Party Notices](../THIRD_PARTY_NOTICES.md)。
