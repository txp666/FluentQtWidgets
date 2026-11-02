"""Measure the sizes of PyQt-Fluent-Widgets widgets for the C++ parity test.

Every widget class that qfluentwidgets 1.11.3 exports and FluentQtWidgets also has is built in one or more
profiles (no-argument construction, with text, with an icon, ...), laid out top-left in a plain container,
shown, and measured: size, sizeHint, minimumSize, maximumSize. The result goes to
tests/data/python_sizes.json, which tests/tst_python_parity.cpp builds the same widgets against.

Run it with the Python runtime that has PyQt5 and qfluentwidgets 1.11.3 (see tools/parity/README.md):

    <python> -I tools/parity/measure_python.py [output.json]

The profiles here and the factories in tst_python_parity.cpp must stay in step: a row the C++ test has no
factory for fails there.
"""

import json
import os
import sys
from pathlib import Path

# Always offscreen: the same screen on every system (800x600 at 100 logical DPI in Qt 5.15; macOS reports 72
# DPI natively, which changes Fusion's DPI-scaled metrics), at 1x. The C++ test gives Qt 6's offscreen
# plugin the same screen (tests/data/parity_offscreen.json) and checks the DPI recorded here.
os.environ["QT_QPA_PLATFORM"] = "offscreen"
os.environ.pop("QT_SCALE_FACTOR", None)

from PyQt5.QtCore import PYQT_VERSION_STR, QSize, Qt, qVersion
from PyQt5.QtGui import QColor, QFont, QFontInfo
from PyQt5.QtTest import QTest
from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QApplication, QVBoxLayout, QWidget

QApplication.setAttribute(Qt.AA_EnableHighDpiScaling, False)
app = QApplication(sys.argv[:1])
# The style sizes scroll bars, sliders and check indicators, and the default one depends on the platform and
# the Qt version. Fusion, here and in the C++ test.
STYLE = "Fusion"
app.setStyle(STYLE)

import qfluentwidgets as qfw  # noqa: E402  (needs the QApplication's attributes set first)
from qfluentwidgets import FluentIcon as FIF  # noqa: E402
from qfluentwidgets import (  # noqa: E402
    ColorConfigItem,
    ConfigItem,
    FolderListValidator,
    OptionsConfigItem,
    OptionsValidator,
    RangeConfigItem,
    RangeValidator,
    Theme,
    setTheme,
)

QWIDGETSIZE_MAX = 16777215

# The application font, fixed so that widgets which don't set their own font measure the same everywhere.
# The families are the Fluent defaults (qconfig.fontFamilies); the C++ test sets the same.
APP_FONT_FAMILIES = ["Segoe UI", "Microsoft YaHei", "PingFang SC"]
APP_FONT_PIXEL_SIZE = 14

# The text a profile uses, and a longer text: a row whose width changes with the text is marked
# text-dependent, and the C++ test compares its width with a tolerance for Qt 5 vs Qt 6 text metrics.
LONGER = "Longer"


def items_of(t):
    return [t, t + "2"]


# ---------------------------------------------------------------------------------------------------------
# Profiles: class name -> {profile: factory(text) -> QWidget}. Keep in step with tst_python_parity.cpp.
# ---------------------------------------------------------------------------------------------------------

PROFILES = {}


def profile(cls, name, factory):
    PROFILES.setdefault(cls, {})[name] = factory


def push_button(cls):
    c = getattr(qfw, cls)
    profile(cls, "default", lambda t: c())
    profile(cls, "text", lambda t: c(t))
    profile(cls, "icon+text", lambda t: c(FIF.MAIL, t))


def tool_button(cls):
    c = getattr(qfw, cls)
    profile(cls, "default", lambda t: c())
    profile(cls, "icon", lambda t: c(FIF.MAIL))


def default_only(cls):
    c = getattr(qfw, cls)
    profile(cls, "default", lambda t: c())


def label(cls):
    c = getattr(qfw, cls)
    profile(cls, "default", lambda t: c())
    profile(cls, "text", lambda t: c(t))


# Buttons
for name in ["PushButton", "PrimaryPushButton", "TransparentPushButton", "ToggleButton", "TogglePushButton",
             "TransparentTogglePushButton", "PillPushButton", "DropDownPushButton", "PrimaryDropDownPushButton",
             "TransparentDropDownPushButton", "SplitPushButton", "PrimarySplitPushButton"]:
    push_button(name)
for name in ["ToolButton", "PrimaryToolButton", "TransparentToolButton", "ToggleToolButton",
             "TransparentToggleToolButton", "PillToolButton", "DropDownToolButton", "PrimaryDropDownToolButton",
             "TransparentDropDownToolButton", "SplitToolButton", "PrimarySplitToolButton", "CommandButton"]:
    tool_button(name)
profile("HyperlinkButton", "default", lambda t: qfw.HyperlinkButton())
profile("HyperlinkButton", "text", lambda t: qfw.HyperlinkButton("https://example.com", t))
profile("HyperlinkButton", "icon+text", lambda t: qfw.HyperlinkButton(FIF.MAIL, "https://example.com", t))
profile("LineEditButton", "icon", lambda t: qfw.LineEditButton(FIF.MAIL))
profile("ColorPickerButton", "default", lambda t: qfw.ColorPickerButton(QColor("#ff0000"), "Color"))

# Selection
label("CheckBox")
label("RadioButton")
label("SwitchButton")
default_only("Slider")
profile("Slider", "horizontal", lambda t: qfw.Slider(Qt.Horizontal))
default_only("ClickableSlider")

# Combo boxes
for name in ["ComboBox", "ModelComboBox", "EditableComboBox", "EditableModelComboBox"]:
    default_only(name)


def _combo_items(cls):
    def make(t):
        w = getattr(qfw, cls)()
        w.addItems(items_of(t))
        return w
    return make


for name in ["ComboBox", "EditableComboBox"]:
    profile(name, "items", _combo_items(name))

# Text input
for name in ["LineEdit", "SearchLineEdit", "PasswordLineEdit", "TextEdit", "PlainTextEdit", "TextBrowser"]:
    default_only(name)

# Spin boxes and date/time
for name in ["SpinBox", "DoubleSpinBox", "TimeEdit", "DateEdit", "DateTimeEdit", "CompactSpinBox",
             "CompactDoubleSpinBox", "CompactTimeEdit", "CompactDateEdit", "CompactDateTimeEdit",
             "DatePicker", "ZhDatePicker", "TimePicker", "AMTimePicker", "CalendarPicker", "FastCalendarPicker"]:
    default_only(name)
profile("CycleListWidget", "items", lambda t: qfw.CycleListWidget(items_of(t), QSize(80, 37)))

# Labels and images
for name in ["BodyLabel", "CaptionLabel", "StrongBodyLabel", "SubtitleLabel", "TitleLabel", "LargeTitleLabel",
             "DisplayLabel", "HyperlinkLabel"]:
    label(name)
for name in ["ImageLabel", "AvatarWidget", "PixmapLabel"]:
    default_only(name)
default_only("IconWidget")
profile("IconWidget", "icon", lambda t: qfw.IconWidget(FIF.MAIL))

# Status and info
for name in ["ProgressBar", "ProgressRing", "IndeterminateProgressBar", "IndeterminateProgressRing"]:
    default_only(name)
default_only("InfoBadge")
profile("InfoBadge", "text", lambda t: qfw.InfoBadge("8"))
default_only("DotInfoBadge")
default_only("IconInfoBadge")
profile("IconInfoBadge", "icon", lambda t: qfw.IconInfoBadge(FIF.MAIL))

# Paging, separators, scrolling, views
for name in ["PipsPager", "HorizontalPipsPager", "VerticalPipsPager", "FlipView", "HorizontalFlipView",
             "VerticalFlipView", "HorizontalSeparator", "VerticalSeparator", "ScrollArea", "SmoothScrollArea",
             "SingleDirectionScrollArea", "ListView", "ListWidget", "TableView", "TableWidget", "TreeView",
             "TreeWidget", "OpacityAniStackedWidget", "PopUpAniStackedWidget", "DrillInTransitionStackedWidget",
             "EntranceTransitionStackedWidget"]:
    default_only(name)


def _pips(cls):
    def make(t):
        w = getattr(qfw, cls)()
        w.setPageNumber(5)
        return w
    return make


profile("HorizontalPipsPager", "items", _pips("HorizontalPipsPager"))
profile("VerticalPipsPager", "items", _pips("VerticalPipsPager"))

# Cards
for name in ["CardWidget", "SimpleCardWidget", "ElevatedCardWidget", "HeaderCardWidget", "GroupHeaderCardWidget"]:
    default_only(name)
profile("HeaderCardWidget", "text", lambda t: qfw.HeaderCardWidget(t))
profile("CardGroupWidget", "icon+text", lambda t: qfw.CardGroupWidget(FIF.MAIL, t, "Content"))

# Command bar
default_only("CommandBar")
default_only("CommandBarView")


def _command_bar(t):
    w = qfw.CommandBar()
    w.addAction(qfw.Action(FIF.MAIL, t))
    w.addAction(qfw.Action(FIF.SAVE, t + "2"))
    return w


profile("CommandBar", "items", _command_bar)

# Navigation
for name in ["Pivot", "SegmentedWidget", "SegmentedToggleToolWidget", "TabBar", "TabWidget", "BreadcrumbBar",
             "NavigationBar", "NavigationPanel", "NavigationInterface", "NavigationUserCard"]:
    default_only(name)


def _pivot(cls):
    def make(t):
        w = getattr(qfw, cls)()
        for i, text in enumerate(items_of(t)):
            w.addItem("k%d" % i, text)
        return w
    return make


profile("Pivot", "items", _pivot("Pivot"))
profile("SegmentedWidget", "items", _pivot("SegmentedWidget"))


def _segmented_tools(t):
    w = qfw.SegmentedToggleToolWidget()
    w.addItem("k0", FIF.MAIL)
    w.addItem("k1", FIF.SAVE)
    return w


def _tab_bar(t):
    w = qfw.TabBar()
    for i, text in enumerate(items_of(t)):
        w.addTab("k%d" % i, text)
    return w


def _breadcrumb(t):
    w = qfw.BreadcrumbBar()
    for i, text in enumerate(items_of(t)):
        w.addItem("k%d" % i, text)
    return w


profile("SegmentedToggleToolWidget", "items", _segmented_tools)
profile("TabBar", "items", _tab_bar)
profile("BreadcrumbBar", "items", _breadcrumb)


def _breadcrumb_item(t):
    # A non-root item, which paints the chevron in front of its text. The bar sizes its items
    # (setFont/setSpacing call setText); the constructor alone leaves them 0 x 0.
    w = qfw.BreadcrumbItem("k1", t, 1)
    w.setText(t)
    return w


profile("BreadcrumbItem", "text", _breadcrumb_item)
profile("PivotItem", "text", lambda t: qfw.PivotItem(t))
profile("SegmentedItem", "text", lambda t: qfw.SegmentedItem(t))
profile("TabItem", "text", lambda t: qfw.TabItem(t))
profile("NavigationToolButton", "icon", lambda t: qfw.NavigationToolButton(FIF.MAIL))
profile("NavigationBarPushButton", "icon+text", lambda t: qfw.NavigationBarPushButton(FIF.MAIL, t, True))
# Python's tree item reaches for its parent's layout in its constructor, so it is built in the container.
profile("NavigationTreeWidget", "icon+text",
        lambda t: qfw.NavigationTreeWidget(FIF.MAIL, t, True, CURRENT_CONTAINER[0]))
profile("NavigationItemHeader", "text", lambda t: qfw.NavigationItemHeader(t))
profile("NavigationAvatarWidget", "text", lambda t: qfw.NavigationAvatarWidget(t))

# Flyout contents (embeddable views; the Flyout/TeachingTip windows themselves are skipped)
profile("FlyoutView", "text", lambda t: qfw.FlyoutView(t, "Content"))
profile("TeachingTipView", "text", lambda t: qfw.TeachingTipView(t, "Content"))

# Setting cards
profile("SettingCard", "icon+text", lambda t: qfw.SettingCard(FIF.MAIL, t, "Content"))
profile("SwitchSettingCard", "icon+text", lambda t: qfw.SwitchSettingCard(FIF.MAIL, t, "Content"))
profile("PushSettingCard", "icon+text", lambda t: qfw.PushSettingCard("Button", FIF.MAIL, t, "Content"))
profile("PrimaryPushSettingCard", "icon+text",
        lambda t: qfw.PrimaryPushSettingCard("Button", FIF.MAIL, t, "Content"))
profile("HyperlinkCard", "icon+text",
        lambda t: qfw.HyperlinkCard("https://example.com", "Button", FIF.MAIL, t, "Content"))
profile("RangeSettingCard", "icon+text", lambda t: qfw.RangeSettingCard(
    RangeConfigItem("Parity", "Range", 50, RangeValidator(0, 100)), FIF.MAIL, t, "Content"))
profile("OptionsSettingCard", "icon+text", lambda t: qfw.OptionsSettingCard(
    OptionsConfigItem("Parity", "Options", "a", OptionsValidator(["a", "b"])), FIF.MAIL, t, "Content",
    ["Alpha", "Beta"]))
profile("ComboBoxSettingCard", "icon+text", lambda t: qfw.ComboBoxSettingCard(
    OptionsConfigItem("Parity", "Combo", "a", OptionsValidator(["a", "b"])), FIF.MAIL, t, "Content",
    ["Alpha", "Beta"]))
profile("ColorSettingCard", "icon+text", lambda t: qfw.ColorSettingCard(
    ColorConfigItem("Parity", "Color", QColor("#ff0000")), FIF.MAIL, t, "Content"))
profile("CustomColorSettingCard", "icon+text", lambda t: qfw.CustomColorSettingCard(
    ColorConfigItem("Parity", "CustomColor", QColor("#ff0000")), FIF.MAIL, t, "Content"))
profile("FolderListSettingCard", "icon+text", lambda t: qfw.FolderListSettingCard(
    ConfigItem("Parity", "Folders", [], FolderListValidator()), t, "Content"))
for name in ["ExpandSettingCard", "ExpandGroupSettingCard", "SimpleExpandGroupSettingCard"]:
    profile(name, "icon+text", (lambda c: lambda t: c(FIF.MAIL, t, "Content"))(getattr(qfw, name)))
profile("SettingCardGroup", "text", lambda t: qfw.SettingCardGroup(t))

# The word each class's text profile uses (default "Button").
WORDS = {
    "CheckBox": "Option", "RadioButton": "Option", "SwitchButton": "On",
    "ComboBox": "Alpha", "EditableComboBox": "Alpha", "CycleListWidget": "Alpha",
    "HeaderCardWidget": "Title", "CardGroupWidget": "Title", "CommandBar": "Mail",
    "Pivot": "Alpha", "SegmentedWidget": "Alpha", "TabBar": "Alpha", "BreadcrumbBar": "Alpha",
    "PivotItem": "Item", "SegmentedItem": "Item", "TabItem": "Tab",
    "NavigationBarPushButton": "Home", "NavigationTreeWidget": "Home", "NavigationItemHeader": "Header",
    "NavigationAvatarWidget": "Ada", "FlyoutView": "Title", "TeachingTipView": "Title",
    "SettingCardGroup": "Title",
}
for name in ["BodyLabel", "CaptionLabel", "StrongBodyLabel", "SubtitleLabel", "TitleLabel", "LargeTitleLabel",
             "DisplayLabel", "HyperlinkLabel"]:
    WORDS[name] = "Label"
for name in PROFILES:
    if name.endswith("Card"):
        WORDS[name] = "Title"

# Exported widget classes that FluentQtWidgets also has, but that are not measured.
SKIPPED = {
    # Top-level windows and dialogs: their size comes from the screen and the window system.
    "FluentWindow": "top-level window", "MSFluentWindow": "top-level window",
    "SplitFluentWindow": "top-level window", "FluentWidget": "top-level window",
    "FluentTitleBar": "part of a frameless window, needs one as its parent",
    "SplashScreen": "top-level splash screen",
    "Dialog": "top-level dialog", "MessageBox": "top-level dialog", "MessageDialog": "top-level dialog",
    "MessageBoxBase": "top-level dialog", "MaskDialogBase": "top-level dialog base",
    "ColorDialog": "top-level dialog", "FolderListDialog": "top-level dialog",
    # Pop-ups.
    "RoundMenu": "pop-up menu", "CheckableMenu": "pop-up menu", "SystemTrayMenu": "pop-up menu",
    "LineEditMenu": "pop-up menu", "Flyout": "pop-up flyout", "TeachingTip": "pop-up flyout",
    "PopupTeachingTip": "pop-up flyout", "ToolTip": "pop-up tool tip",
    "StateToolTip": "floating tool tip, placed by its parent",
    "InfoBar": "floating info bar, placed by its manager",
    "PickerPanel": "the pop-up panel of the pickers",
    # Abstract and base classes.
    "FluentLabelBase": "base class of the labels", "SplitWidgetBase": "base class of the split buttons",
    "FlyoutViewBase": "base class of the flyout views", "NavigationWidget": "base class, paints nothing",
    "TransitionStackedWidget": "base class of the transition stacked widgets",
    # Attached to another widget.
    "ScrollBar": "attached to a scroll area, sized by it",
    "SmoothScrollBar": "attached to a scroll area, sized by it",
    # Not the same kind of thing.
}


def size_list(s):
    return [s.width(), s.height()]


def max_list(s):
    return [("max" if v >= QWIDGETSIZE_MAX else v) for v in (s.width(), s.height())]


CURRENT_CONTAINER = [None]


def measure(factory, text):
    container = QWidget()
    layout = QVBoxLayout(container)
    CURRENT_CONTAINER[0] = container
    layout.setContentsMargins(0, 0, 0, 0)
    widget = factory(text)
    layout.addWidget(widget, 0, Qt.AlignLeft | Qt.AlignTop)
    container.resize(1200, 900)
    container.show()
    QTest.qWaitForWindowExposed(container)
    QTest.qWait(50)
    result = {
        "size": size_list(widget.size()),
        "sizeHint": size_list(widget.sizeHint()),
        "minimumSize": size_list(widget.minimumSize()),
        "maximumSize": max_list(widget.maximumSize()),
    }
    container.hide()
    container.deleteLater()
    QApplication.sendPostedEvents(None, 0)
    app.processEvents()
    return result


# Plain QLabels in the Fluent font, no qfluentwidgets code involved: the C++ test measures the same, and the
# differences are what Qt 5 vs Qt 6 text layout alone accounts for. They calibrate the text tolerance.
CONTROL_TEXTS = ["Button", "Option", "Label", "Title", "ButtonLonger", "Settings and preferences"]
CONTROL_PIXEL_SIZES = [12, 14, 20, 28, 40, 68]
# Qt 5 weights (QFont.Normal, QFont.DemiBold); the C++ test maps them to Qt 6's.
CONTROL_WEIGHTS = [50, 63]


def measure_controls():
    from PyQt5.QtWidgets import QLabel
    controls = {}
    for weight in CONTROL_WEIGHTS:
        for px in CONTROL_PIXEL_SIZES:
            for text in CONTROL_TEXTS:
                label = QLabel(text)
                label.setFont(qfw.getFont(px, weight))
                controls["QLabel/%d/%d/%s" % (weight, px, text)] = size_list(label.sizeHint())
                label.deleteLater()
    return controls


# Plain Qt widgets, no qfluentwidgets code: where these differ between the runtimes, Qt 5 vs Qt 6 itself
# does, and a Fluent row built on that class may differ for the same reason. The C++ test prints them.
QT_BASELINE = {
    "QSpinBox": lambda: QtWidgets.QSpinBox(),
    "QDoubleSpinBox": lambda: QtWidgets.QDoubleSpinBox(),
    "QDateEdit": lambda: QtWidgets.QDateEdit(),
    "QTimeEdit": lambda: QtWidgets.QTimeEdit(),
    "QDateTimeEdit": lambda: QtWidgets.QDateTimeEdit(),
    "QLineEdit": lambda: QtWidgets.QLineEdit(),
    "QScrollArea": lambda: QtWidgets.QScrollArea(),
    "QListWidget": lambda: QtWidgets.QListWidget(),
    "QSlider": lambda: QtWidgets.QSlider(),
    "QProgressBar": lambda: QtWidgets.QProgressBar(),
    "QPushButton": lambda: QtWidgets.QPushButton("Button"),
    "QToolButton": lambda: QtWidgets.QToolButton(),
    "QCheckBox": lambda: QtWidgets.QCheckBox("Option"),
    "QRadioButton": lambda: QtWidgets.QRadioButton("Option"),
}


def measure_qt_baseline():
    return {name: size_list(make().sizeHint()) for name, make in sorted(QT_BASELINE.items())}


def main():
    out = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).resolve().parents[2] / "tests/data/python_sizes.json"

    font = QFont()
    font.setFamilies(APP_FONT_FAMILIES)
    font.setPixelSize(APP_FONT_PIXEL_SIZE)
    QApplication.setFont(font)
    setTheme(Theme.LIGHT)

    exported = {n for n in dir(qfw) if isinstance(getattr(qfw, n), type) and issubclass(getattr(qfw, n), QWidget)}
    missing = sorted(set(PROFILES) - exported)
    if missing:
        sys.exit("not exported by qfluentwidgets: %s" % ", ".join(missing))

    rows = {}
    for cls in sorted(PROFILES):
        word = WORDS.get(cls, "Button")
        for name, factory in sorted(PROFILES[cls].items()):
            row = measure(factory, word)
            longer = measure(factory, word + LONGER)
            row["textDependentWidth"] = longer["sizeHint"][0] != row["sizeHint"][0] or longer["size"][0] != row["size"][0]
            row["textDependentHeight"] = longer["sizeHint"][1] != row["sizeHint"][1] or longer["size"][1] != row["size"][1]
            row["text"] = word
            rows["%s/%s" % (cls, name)] = row

    data = {
        "meta": {
            "qfluentwidgets": qfw.__version__,
            "pyqt": PYQT_VERSION_STR,
            "qt": qVersion(),
            "platform": sys.platform,
            "qpa": QApplication.platformName(),
            "logicalDpi": round(app.primaryScreen().logicalDotsPerInch()),
            "style": STYLE,
            "appFont": {"families": APP_FONT_FAMILIES, "pixelSize": APP_FONT_PIXEL_SIZE,
                        "resolvedFamily": QFontInfo(font).family()},
            "fluentFontResolvedFamily": QFontInfo(qfw.getFont(14)).family(),
            "theme": "light",
            "longerTextSuffix": LONGER,
        },
        "rows": rows,
        "controls": measure_controls(),
        "qtBaseline": measure_qt_baseline(),
        "skipped": SKIPPED,
    }
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(json.dumps(data, indent=1, sort_keys=True) + "\n", encoding="utf-8")
    print("wrote %d rows to %s" % (len(rows), out))


if __name__ == "__main__":
    main()
