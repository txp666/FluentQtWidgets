# FluentQtWidgets

FluentQtWidgets 是一个面向 C++/Qt Widgets 的 Fluent Design 风格控件库，包含完整的组件实现与 Gallery 示例应用。

本项目是 Python 版 [PyQt-Fluent-Widgets](https://github.com/zhiyiYo/PyQt-Fluent-Widgets) 的 **C++/Qt Widgets 移植版本**。Python 原版是本仓库的行为与视觉基准：控件状态、间距、QSS 常量、示例内容、资源路径、Gallery 页面结构和翻译均按 Python 原版对齐；只有 Qt/C++ 平台差异不可避免时才保留差异说明。

当前发布线：**0.1.6**。本地上游参考源码、构建目录和开发 agent 元数据不会进入源码发布包。


## 环境要求

- CMake 3.21+
- C++17
- Qt 6.5+（Core/Gui/Widgets/Svg/Network），可选：
  - Multimedia + MultimediaWidgets
  - WebEngineWidgets（仅独立 WebEngine 示例需要）

## 构建

```bash
cmake -S . -B build -DFQW_BUILD_EXAMPLES=ON -DFQW_BUILD_TESTS=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

### 安装 / 消费

```bash
cmake --install build --prefix <install-prefix>
```

```cmake
find_package(FluentQtWidgets CONFIG REQUIRED)
target_link_libraries(my_app PRIVATE FluentQtWidgets::Widgets)
```

### Windows Qt kit 选择

CMake 使用的生成器和编译器必须与已安装的 Qt kit 匹配。MinGW Qt kit 应使用 MinGW 构建，
MSVC Qt kit 应使用 MSVC 构建。如果 CMake 提示找不到 `Qt6Config.cmake`，请通过
`CMAKE_PREFIX_PATH` 指向 Qt kit 根目录，或把 `Qt6_DIR` 设置到包含 `Qt6Config.cmake` 的目录。

仓库内的 `CMakePresets.json` 不写死本机 Qt、编译器或调试器路径。机器相关路径应放在环境变量、
IDE kit 选择，或被忽略的 `CMakeUserPresets.json` 中。仓库提供了 `CMakeUserPresets.json.example`
作为本地配置模板。

使用本机 Qt MinGW kit 时，先让 Qt kit、MinGW 编译器和 Ninja 对当前终端可见，再使用仓库内的
`mingw-debug` preset：

```powershell
$env:CMAKE_PREFIX_PATH = "<path-to-qt-mingw-kit>"
$env:Path = "<path-to-mingw-bin>;<path-to-ninja-dir>;<path-to-qt-bin>;$env:Path"

cmake --preset mingw-debug
cmake --build --preset mingw-debug --parallel
ctest --preset mingw-debug
```

VS Code 任务会继承启动 VS Code 时的环境。要使用一键构建，可以从已经设置好上述变量的终端启动
VS Code，设置等价的用户环境变量，或复制 `CMakeUserPresets.json.example` 为被忽略的
`CMakeUserPresets.json` 并填入本机路径。Windows 下的 `Gallery: Debug` 使用 `gdb.exe`，
因此启动调试前 VS Code 必须能从 PATH 找到 MinGW 的 `bin` 目录。在 Windows 上重新构建前请先关闭
正在运行的 Gallery 窗口，否则链接器无法覆盖正在运行的 `.exe`。

如果看到 `ninja: error: loading 'build.ninja': The system cannot find the file specified.`，请先运行
`cmake --preset mingw-debug` 重新生成 `build/mingw/build.ninja`，再执行构建。如果 PowerShell
显示重复的 `PATH`/`Path` 环境变量，请先启动干净终端或修正用户环境；否则 MinGW 工具可能没有编译器诊断就失败。

如果要使用 Visual Studio/MSVC，请安装匹配的 Qt MSVC kit，并将 `CMAKE_PREFIX_PATH` 或 `Qt6_DIR`
指向该 kit。不要提交本机的 `CMakeUserPresets.json`。

### 启动 Gallery

使用仓库 preset 时，请选择当前平台对应的 preset。不同平台使用不同构建目录，是因为 CMake 会把
生成器、编译器和 Qt kit 缓存在构建目录里，不建议让 MinGW、MSVC、macOS 共用同一个 cache。

```bash
# macOS/Linux
cmake --build --preset debug --target FluentQtWidgetsGallery --parallel
./build/debug/examples/gallery/FluentQtWidgetsGallery
# macOS:
# open ./build/debug/examples/gallery/FluentQtWidgetsGallery.app
```

```powershell
# Windows MinGW
cmake --build --preset mingw-debug --target FluentQtWidgetsGallery --parallel
.\build\mingw\examples\gallery\FluentQtWidgetsGallery.exe
```

## 示例程序

macOS/Linux 使用 `--preset debug`；Windows MinGW 使用 `--preset mingw-debug`：

```bash
cmake --build --preset debug --target fqw_basic_input_button_demo --parallel
cmake --build --preset debug --target fqw_basic_input_check_box_demo --parallel
cmake --build --preset debug --target fqw_basic_input_combo_box_demo --parallel
cmake --build --preset debug --target fqw_basic_input_model_combo_box_demo --parallel
cmake --build --preset debug --target fqw_basic_input_radio_button_demo --parallel
cmake --build --preset debug --target fqw_basic_input_slider_demo --parallel
cmake --build --preset debug --target fqw_basic_input_switch_button_demo --parallel
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
