// Size parity with PyQt-Fluent-Widgets 1.11.3.
//
// tools/parity/measure_python.py builds every widget class the two libraries share, in a few profiles
// (no arguments, with text, with an icon, ...), and records its size, sizeHint, minimumSize and
// maximumSize in tests/data/python_sizes.json. This test builds the same widgets the same way and
// compares. See tools/parity/README.md.

#include <FluentQtWidgets/Config.h>
#include <FluentQtWidgets/DateTime/CalendarPicker.h>
#include <FluentQtWidgets/DateTime/DatePicker.h>
#include <FluentQtWidgets/DateTime/TimePicker.h>
#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Navigation/BreadcrumbBar.h>
#include <FluentQtWidgets/Navigation/NavigationBar.h>
#include <FluentQtWidgets/Navigation/NavigationInterface.h>
#include <FluentQtWidgets/Navigation/NavigationPanel.h>
#include <FluentQtWidgets/Navigation/NavigationTreeWidget.h>
#include <FluentQtWidgets/Navigation/NavigationWidget.h>
#include <FluentQtWidgets/Navigation/Pivot.h>
#include <FluentQtWidgets/Navigation/SegmentedWidget.h>
#include <FluentQtWidgets/Settings/SettingCard.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Views/ItemViews.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/CardWidget.h>
#include <FluentQtWidgets/Widgets/ComboBox.h>
#include <FluentQtWidgets/Widgets/CommandBar.h>
#include <FluentQtWidgets/Widgets/CycleListWidget.h>
#include <FluentQtWidgets/Widgets/FlipView.h>
#include <FluentQtWidgets/Widgets/Flyout.h>
#include <FluentQtWidgets/Widgets/IconWidget.h>
#include <FluentQtWidgets/Widgets/InfoBadge.h>
#include <FluentQtWidgets/Widgets/Label.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>
#include <FluentQtWidgets/Widgets/PipsPager.h>
#include <FluentQtWidgets/Widgets/ProgressBar.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>
#include <FluentQtWidgets/Widgets/Selection.h>
#include <FluentQtWidgets/Widgets/Separator.h>
#include <FluentQtWidgets/Widgets/Slider.h>
#include <FluentQtWidgets/Widgets/SpinBox.h>
#include <FluentQtWidgets/Widgets/StackedWidget.h>
#include <FluentQtWidgets/Widgets/SwitchButton.h>
#include <FluentQtWidgets/Widgets/TabView.h>
#include <FluentQtWidgets/Widgets/TeachingTip.h>
#include <QtCore/QFile>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtGui/QFontInfo>
#include <QtGui/QGuiApplication>
#include <QtGui/QScreen>
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStyleFactory>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <algorithm>
#include <functional>
#include <map>
#include <optional>
#include <vector>

using namespace FluentQt;

namespace {

// ---------------------------------------------------------------------------------------------------------
// The two tables. One line per row: {"Class/profile", "metrics", "reason"}. Metrics are space-separated:
// size.w size.h sizeHint.w sizeHint.h minimumSize.w minimumSize.h maximumSize.w maximumSize.h, or "*".
// ---------------------------------------------------------------------------------------------------------

struct Expectation
{
    const char *row;
    const char *metrics;
    const char *reason;
};

// Deliberate differences, for a Qt/C++/platform reason. The listed metrics are not compared.
const std::vector<Expectation> kIntended = {
    // clang-format off
    // Date and time pickers stay QFrames (a QPushButton base would change the public API).
    {"DatePicker/default", "sizeHint.w sizeHint.h", "a QFrame reporting its fixed size; Python's QPushButton hint (38x22) is unused under the same fixed size"},
    {"ZhDatePicker/default", "sizeHint.w sizeHint.h", "a QFrame reporting its fixed size; Python's QPushButton hint (38x22) is unused under the same fixed size"},
    {"TimePicker/default", "sizeHint.w sizeHint.h", "a QFrame reporting its fixed size; Python's QPushButton hint (38x22) is unused under the same fixed size"},
    {"AMTimePicker/default", "sizeHint.w sizeHint.h", "a QFrame reporting its fixed size; Python's QPushButton hint (38x22) is unused under the same fixed size"},
    // Spin boxes: Qt 6 lays out a style-sheeted QAbstractSpinBox 8 px wider and hints 5 px taller.
    {"SpinBox/default", "size.w sizeHint.w sizeHint.h", "Qt 6 style sheet layout: a plain QSpinBox (NoButtons) given only the sheet's padding 0 80 0 10 and 1 px border hints 115x24 in Qt 5.15, 123x29 in Qt 6.11 (Fusion, 100 DPI); no Fluent code involved. Unstyled, the two agree (31x31 without buttons)"},
    {"DoubleSpinBox/default", "size.w sizeHint.w sizeHint.h", "Qt 6 style sheet layout: a plain QSpinBox (NoButtons) given only the sheet's padding 0 80 0 10 and 1 px border hints 115x24 in Qt 5.15, 123x29 in Qt 6.11 (Fusion, 100 DPI); no Fluent code involved. Unstyled, the two agree (31x31 without buttons)"},
    {"TimeEdit/default", "size.w sizeHint.w sizeHint.h", "Qt 6 style sheet layout: a plain QSpinBox (NoButtons) given only the sheet's padding 0 80 0 10 and 1 px border hints 115x24 in Qt 5.15, 123x29 in Qt 6.11 (Fusion, 100 DPI); no Fluent code involved. Unstyled, the two agree (31x31 without buttons)"},
    {"DateEdit/default", "size.w sizeHint.w sizeHint.h", "Qt 6 style sheet layout: a plain QSpinBox (NoButtons) given only the sheet's padding 0 80 0 10 and 1 px border hints 115x24 in Qt 5.15, 123x29 in Qt 6.11 (Fusion, 100 DPI); no Fluent code involved. Unstyled, the two agree (31x31 without buttons)"},
    {"DateTimeEdit/default", "size.w sizeHint.w sizeHint.h", "Qt 6 style sheet layout: a plain QSpinBox (NoButtons) given only the sheet's padding 0 80 0 10 and 1 px border hints 115x24 in Qt 5.15, 123x29 in Qt 6.11 (Fusion, 100 DPI); no Fluent code involved. Unstyled, the two agree (31x31 without buttons)"},
    {"CompactSpinBox/default", "size.w sizeHint.w sizeHint.h", "Qt 6 style sheet layout: a plain QSpinBox (NoButtons) given only the sheet's padding 0 80 0 10 and 1 px border hints 115x24 in Qt 5.15, 123x29 in Qt 6.11 (Fusion, 100 DPI); no Fluent code involved. Unstyled, the two agree (31x31 without buttons)"},
    {"CompactDoubleSpinBox/default", "size.w sizeHint.w sizeHint.h", "Qt 6 style sheet layout: a plain QSpinBox (NoButtons) given only the sheet's padding 0 80 0 10 and 1 px border hints 115x24 in Qt 5.15, 123x29 in Qt 6.11 (Fusion, 100 DPI); no Fluent code involved. Unstyled, the two agree (31x31 without buttons)"},
    {"CompactTimeEdit/default", "size.w sizeHint.w sizeHint.h", "Qt 6 style sheet layout: a plain QSpinBox (NoButtons) given only the sheet's padding 0 80 0 10 and 1 px border hints 115x24 in Qt 5.15, 123x29 in Qt 6.11 (Fusion, 100 DPI); no Fluent code involved. Unstyled, the two agree (31x31 without buttons)"},
    {"CompactDateEdit/default", "size.w sizeHint.w sizeHint.h", "Qt 6 style sheet layout: a plain QSpinBox (NoButtons) given only the sheet's padding 0 80 0 10 and 1 px border hints 115x24 in Qt 5.15, 123x29 in Qt 6.11 (Fusion, 100 DPI); no Fluent code involved. Unstyled, the two agree (31x31 without buttons)"},
    {"CompactDateTimeEdit/default", "size.w sizeHint.w sizeHint.h", "Qt 6 style sheet layout: a plain QSpinBox (NoButtons) given only the sheet's padding 0 80 0 10 and 1 px border hints 115x24 in Qt 5.15, 123x29 in Qt 6.11 (Fusion, 100 DPI); no Fluent code involved. Unstyled, the two agree (31x31 without buttons)"},
    // clang-format on
};

// Differences that look like port bugs, not fixed yet. Expected to fail (QEXPECT_FAIL, Continue): when
// a fix makes the metric match, the row reports XPASS and fails, and the entry must go.
const std::vector<Expectation> kPending = {
    // clang-format off
    // Buttons
    {"ColorPickerButton/default", "sizeHint.w sizeHint.h", "hint 27x27, Python 24x23 (both fixed in size)"},
    // Selection
    // Combo boxes
    // Labels and images
    // Status and info
    {"ProgressRing/default", "sizeHint.w sizeHint.h", "hint 100x100, Python QProgressBar's 95x28 (fixed size matches)"},
    {"InfoBadge/default", "size.w sizeHint.w", "18 px wide without text, Python 11"},
    {"DotInfoBadge/default", "sizeHint.w sizeHint.h", "hint 16x16, Python 11x18 (fixed size matches)"},
    {"IconInfoBadge/default", "sizeHint.h", "hint 16 px tall, Python 18 (fixed size matches)"},
    {"IconInfoBadge/icon", "sizeHint.h", "hint 16 px tall, Python 18 (fixed size matches)"},
    // Paging, scrolling, views
    {"PipsPager/default", "size.w sizeHint.w sizeHint.h minimumSize.h maximumSize.h", "90 px long where Python keeps QListWidget's 256x192 hint; Python fixes the 12 px thickness"},
    {"HorizontalPipsPager/default", "size.w sizeHint.w sizeHint.h minimumSize.h maximumSize.h", "90 px long where Python keeps QListWidget's 256x192 hint; Python fixes the 12 px thickness"},
    {"HorizontalPipsPager/items", "sizeHint.w sizeHint.h minimumSize.w minimumSize.h maximumSize.w maximumSize.h", "no fixed size (Python fixes 90x12 for 5 pages); hint 90x12 vs QListWidget's 256x192"},
    {"VerticalPipsPager/default", "size.h sizeHint.w sizeHint.h minimumSize.w maximumSize.w", "90 px long where Python keeps QListWidget's 256x192 hint; Python fixes the 12 px thickness"},
    {"VerticalPipsPager/items", "sizeHint.w sizeHint.h minimumSize.w minimumSize.h maximumSize.w maximumSize.h", "no fixed size (Python fixes 90x12 for 5 pages); hint 90x12 vs QListWidget's 256x192"},
    {"FlipView/default", "sizeHint.w sizeHint.h", "hint 480x270, Python QListWidget's 256x192"},
    {"HorizontalFlipView/default", "sizeHint.w sizeHint.h", "hint 480x270, Python QListWidget's 256x192"},
    {"VerticalFlipView/default", "sizeHint.w sizeHint.h", "hint 480x270, Python QListWidget's 256x192"},
    {"ScrollArea/default", "size.w size.h sizeHint.w sizeHint.h", "240x160 without the frame (setFrameShape NoFrame); Python keeps QScrollArea's 242x162"},
    {"SmoothScrollArea/default", "size.w size.h sizeHint.w sizeHint.h", "240x160 without the frame (setFrameShape NoFrame); Python keeps QScrollArea's 242x162"},
    {"SingleDirectionScrollArea/default", "size.w size.h sizeHint.w sizeHint.h", "240x160 without the frame (setFrameShape NoFrame); Python keeps QScrollArea's 242x162"},
    // Cards
    // Command bar
    {"CommandBar/default", "size.w size.h", "laid out at 0 px wide (0x0 empty) though its hint matches; Python 100 px"},
    {"CommandBar/items", "size.w", "laid out at 0 px wide (0x0 empty) though its hint matches; Python 100 px"},
    {"CommandBarView/default", "size.w size.h sizeHint.w sizeHint.h minimumSize.w minimumSize.h maximumSize.h", "12x12 and fixed 12 px tall, Python 112x42"},
    // Navigation
    {"TabBar/default", "sizeHint.w sizeHint.h", "hint 93x93, Python 45x24; the size matches"},
    {"TabBar/items", "sizeHint.h", "hint 93 px tall, Python 46; the fixed 46 px height matches"},
    {"BreadcrumbBar/items", "size.w size.h sizeHint.w sizeHint.h minimumSize.h maximumSize.h", "hint 149x32 and free height; Python has no hint and fixes the height at 20"},
    {"NavigationBar/default", "size.w minimumSize.w maximumSize.w", "fixed 72 px wide, Python free (93)"},
    {"NavigationInterface/default", "sizeHint.w sizeHint.h minimumSize.w maximumSize.w", "free width, Python fixed at 48 when collapsed; hint 49x1, Python none"},
    {"NavigationUserCard/default", "sizeHint.w sizeHint.h", "has a hint, Python none (fixed size matches)"},
    {"NavigationToolButton/icon", "sizeHint.w sizeHint.h", "has a hint, Python none (fixed size matches)"},
    {"NavigationBarPushButton/icon+text", "sizeHint.w sizeHint.h", "has a hint, Python none (fixed size matches)"},
    {"NavigationItemHeader/text", "sizeHint.w sizeHint.h", "has a hint, Python none (fixed size matches)"},
    {"NavigationAvatarWidget/text", "sizeHint.w sizeHint.h", "has a hint, Python none (fixed size matches)"},
    // Flyout contents
    // Setting cards
    {"SettingCard/icon+text", "size.w sizeHint.w", "16 px wider than Python (121 vs 105); the fixed height matches"},
    {"OptionsSettingCard/icon+text", "sizeHint.h", "hint 105 px tall, Python 208 (collapsed size matches)"},
    {"CustomColorSettingCard/icon+text", "size.w sizeHint.w sizeHint.h", "27 px narrower, hint 179 px tall vs 353 (collapsed height matches)"},
    {"ExpandGroupSettingCard/icon+text", "sizeHint.h", "hint 20 px tall, Python 3 (collapsed size matches)"},
    {"SimpleExpandGroupSettingCard/icon+text", "sizeHint.h", "hint 20 px tall, Python 3 (collapsed size matches)"},
    // clang-format on
};

// Plain Qt classes whose size hint Qt 6 changed (qtBaseline). Not a table of the port's differences: a
// hint to whoever classifies a row of a Fluent class built on one of them.
const std::vector<Expectation> kQtBaselineDifferences = {
    // clang-format off
    {"QSpinBox",       "sizeHint", "Qt 6 hint 6 px narrower"},
    {"QDoubleSpinBox", "sizeHint", "Qt 6 hint 5 px narrower"},
    {"QTimeEdit",      "sizeHint", "Qt 6 hint 5 px narrower"},
    {"QDateEdit",      "sizeHint", "Qt 6 hint 2 px narrower"},
    {"QDateTimeEdit",  "sizeHint", "Qt 6 hint 2 px narrower"},
    {"QToolButton",    "sizeHint", "Qt 6 hint 6 px larger each way (30x29 vs 24x23)"},
    // clang-format on
};

// Text-dependent widths (the Python run marks a row so when a longer text makes it wider) may differ by
// this much, for Qt 5.15 vs Qt 6 text layout. Calibrated with the plain QLabel controls
// (textMetricsControls): in the Fluent font at 12-68 px, normal and demibold, Qt 5.15.19 and 6.11 agree
// to the pixel, so this is the 1 px rounding allowed everywhere plus 1 px for strings outside the controls.
constexpr int kTextWidthTolerance = 2;
constexpr int kTolerance = 1;
constexpr int kWidgetSizeMax = 16777215; // QWIDGETSIZE_MAX

const QStringList kMetrics = {QStringLiteral("size"), QStringLiteral("sizeHint"), QStringLiteral("minimumSize"),
                              QStringLiteral("maximumSize")};

// ---------------------------------------------------------------------------------------------------------
// The widgets, built as tools/parity/measure_python.py builds them.
// ---------------------------------------------------------------------------------------------------------

using Factory = std::function<QWidget *(const QString &text, QWidget *container)>;
using Factories = std::map<QString, Factory>;

QIcon mail() { return icon(FluentIcon::Mail); }

QStringList itemsOf(const QString &text) { return {text, text + QLatin1Char('2')}; }

template <typename T> void pushButton(Factories &f, const char *name)
{
    const QString n = QLatin1String(name);
    f[n + "/default"] = [](const QString &, QWidget *) { return new T; };
    f[n + "/text"] = [](const QString &t, QWidget *) { return new T(t); };
    f[n + "/icon+text"] = [](const QString &t, QWidget *) { return new T(mail(), t); };
}

template <typename T> void toolButton(Factories &f, const char *name)
{
    const QString n = QLatin1String(name);
    f[n + "/default"] = [](const QString &, QWidget *) { return new T; };
    f[n + "/icon"] = [](const QString &, QWidget *) { return new T(mail()); };
}

template <typename T> void defaultOnly(Factories &f, const char *name)
{
    f[QLatin1String(name) + "/default"] = [](const QString &, QWidget *) { return new T; };
}

template <typename T> void label(Factories &f, const char *name)
{
    const QString n = QLatin1String(name);
    f[n + "/default"] = [](const QString &, QWidget *) { return new T; };
    f[n + "/text"] = [](const QString &t, QWidget *) { return new T(t); };
}

const Factories &factories()
{
    static const Factories table = [] {
        Factories f;
        const QString content = QStringLiteral("Content");
        const QString url = QStringLiteral("https://example.com");

        // Buttons
        pushButton<PushButton>(f, "PushButton");
        pushButton<PrimaryPushButton>(f, "PrimaryPushButton");
        pushButton<TransparentPushButton>(f, "TransparentPushButton");
        pushButton<ToggleButton>(f, "ToggleButton");
        pushButton<TogglePushButton>(f, "TogglePushButton");
        pushButton<TransparentTogglePushButton>(f, "TransparentTogglePushButton");
        pushButton<PillPushButton>(f, "PillPushButton");
        pushButton<DropDownPushButton>(f, "DropDownPushButton");
        pushButton<PrimaryDropDownPushButton>(f, "PrimaryDropDownPushButton");
        pushButton<TransparentDropDownPushButton>(f, "TransparentDropDownPushButton");
        pushButton<SplitPushButton>(f, "SplitPushButton");
        pushButton<PrimarySplitPushButton>(f, "PrimarySplitPushButton");
        toolButton<ToolButton>(f, "ToolButton");
        toolButton<PrimaryToolButton>(f, "PrimaryToolButton");
        toolButton<TransparentToolButton>(f, "TransparentToolButton");
        toolButton<ToggleToolButton>(f, "ToggleToolButton");
        toolButton<TransparentToggleToolButton>(f, "TransparentToggleToolButton");
        toolButton<PillToolButton>(f, "PillToolButton");
        toolButton<DropDownToolButton>(f, "DropDownToolButton");
        toolButton<PrimaryDropDownToolButton>(f, "PrimaryDropDownToolButton");
        toolButton<TransparentDropDownToolButton>(f, "TransparentDropDownToolButton");
        toolButton<SplitToolButton>(f, "SplitToolButton");
        toolButton<PrimarySplitToolButton>(f, "PrimarySplitToolButton");
        toolButton<CommandButton>(f, "CommandButton");
        f["HyperlinkButton/default"] = [](const QString &, QWidget *) { return new HyperlinkButton; };
        f["HyperlinkButton/text"] = [=](const QString &t, QWidget *) { return new HyperlinkButton(url, t); };
        f["HyperlinkButton/icon+text"] = [=](const QString &t, QWidget *) {
            return new HyperlinkButton(mail(), url, t);
        };
        f["LineEditButton/icon"] = [](const QString &, QWidget *) { return new LineEditButton(FluentIcon::Mail); };
        f["ColorPickerButton/default"] = [](const QString &, QWidget *) {
            return new ColorPickerButton(QColor(QStringLiteral("#ff0000")), QStringLiteral("Color"));
        };

        // Selection
        label<CheckBox>(f, "CheckBox");
        label<RadioButton>(f, "RadioButton");
        label<SwitchButton>(f, "SwitchButton");
        defaultOnly<Slider>(f, "Slider");
        f["Slider/horizontal"] = [](const QString &, QWidget *) { return new Slider(Qt::Horizontal); };
        defaultOnly<ClickableSlider>(f, "ClickableSlider");

        // Combo boxes
        defaultOnly<ComboBox>(f, "ComboBox");
        defaultOnly<ModelComboBox>(f, "ModelComboBox");
        defaultOnly<EditableComboBox>(f, "EditableComboBox");
        defaultOnly<EditableModelComboBox>(f, "EditableModelComboBox");
        f["ComboBox/items"] = [](const QString &t, QWidget *) {
            auto *w = new ComboBox;
            w->addItems(itemsOf(t));
            return w;
        };
        f["EditableComboBox/items"] = [](const QString &t, QWidget *) {
            auto *w = new EditableComboBox;
            w->addItems(itemsOf(t));
            return w;
        };

        // Text input
        defaultOnly<LineEdit>(f, "LineEdit");
        defaultOnly<SearchLineEdit>(f, "SearchLineEdit");
        defaultOnly<PasswordLineEdit>(f, "PasswordLineEdit");
        defaultOnly<TextEdit>(f, "TextEdit");
        defaultOnly<PlainTextEdit>(f, "PlainTextEdit");
        defaultOnly<TextBrowser>(f, "TextBrowser");

        // Spin boxes and date/time
        defaultOnly<SpinBox>(f, "SpinBox");
        defaultOnly<DoubleSpinBox>(f, "DoubleSpinBox");
        defaultOnly<TimeEdit>(f, "TimeEdit");
        defaultOnly<DateEdit>(f, "DateEdit");
        defaultOnly<DateTimeEdit>(f, "DateTimeEdit");
        defaultOnly<CompactSpinBox>(f, "CompactSpinBox");
        defaultOnly<CompactDoubleSpinBox>(f, "CompactDoubleSpinBox");
        defaultOnly<CompactTimeEdit>(f, "CompactTimeEdit");
        defaultOnly<CompactDateEdit>(f, "CompactDateEdit");
        defaultOnly<CompactDateTimeEdit>(f, "CompactDateTimeEdit");
        defaultOnly<DatePicker>(f, "DatePicker");
        defaultOnly<ZhDatePicker>(f, "ZhDatePicker");
        defaultOnly<TimePicker>(f, "TimePicker");
        defaultOnly<AMTimePicker>(f, "AMTimePicker");
        defaultOnly<CalendarPicker>(f, "CalendarPicker");
        defaultOnly<FastCalendarPicker>(f, "FastCalendarPicker");
        f["CycleListWidget/items"] = [](const QString &t, QWidget *) {
            return new CycleListWidget(itemsOf(t), QSize(80, 37));
        };

        // Labels and images
        label<BodyLabel>(f, "BodyLabel");
        label<CaptionLabel>(f, "CaptionLabel");
        label<StrongBodyLabel>(f, "StrongBodyLabel");
        label<SubtitleLabel>(f, "SubtitleLabel");
        label<TitleLabel>(f, "TitleLabel");
        label<LargeTitleLabel>(f, "LargeTitleLabel");
        label<DisplayLabel>(f, "DisplayLabel");
        label<HyperlinkLabel>(f, "HyperlinkLabel");
        defaultOnly<ImageLabel>(f, "ImageLabel");
        defaultOnly<AvatarWidget>(f, "AvatarWidget");
        defaultOnly<PixmapLabel>(f, "PixmapLabel");
        defaultOnly<IconWidget>(f, "IconWidget");
        f["IconWidget/icon"] = [](const QString &, QWidget *) { return new IconWidget(FluentIcon::Mail); };

        // Status and info
        defaultOnly<ProgressBar>(f, "ProgressBar");
        defaultOnly<ProgressRing>(f, "ProgressRing");
        defaultOnly<IndeterminateProgressBar>(f, "IndeterminateProgressBar");
        defaultOnly<IndeterminateProgressRing>(f, "IndeterminateProgressRing");
        defaultOnly<InfoBadge>(f, "InfoBadge");
        f["InfoBadge/text"] = [](const QString &, QWidget *) { return new InfoBadge(QStringLiteral("8")); };
        defaultOnly<DotInfoBadge>(f, "DotInfoBadge");
        defaultOnly<IconInfoBadge>(f, "IconInfoBadge");
        f["IconInfoBadge/icon"] = [](const QString &, QWidget *) { return new IconInfoBadge(FluentIcon::Mail); };

        // Paging, separators, scrolling, views
        defaultOnly<PipsPager>(f, "PipsPager");
        defaultOnly<HorizontalPipsPager>(f, "HorizontalPipsPager");
        defaultOnly<VerticalPipsPager>(f, "VerticalPipsPager");
        f["HorizontalPipsPager/items"] = [](const QString &, QWidget *) {
            auto *w = new HorizontalPipsPager;
            w->setPageNumber(5);
            return w;
        };
        f["VerticalPipsPager/items"] = [](const QString &, QWidget *) {
            auto *w = new VerticalPipsPager;
            w->setPageNumber(5);
            return w;
        };
        defaultOnly<FlipView>(f, "FlipView");
        defaultOnly<HorizontalFlipView>(f, "HorizontalFlipView");
        defaultOnly<VerticalFlipView>(f, "VerticalFlipView");
        defaultOnly<HorizontalSeparator>(f, "HorizontalSeparator");
        defaultOnly<VerticalSeparator>(f, "VerticalSeparator");
        defaultOnly<ScrollArea>(f, "ScrollArea");
        defaultOnly<SmoothScrollArea>(f, "SmoothScrollArea");
        defaultOnly<SingleDirectionScrollArea>(f, "SingleDirectionScrollArea");
        defaultOnly<ListView>(f, "ListView");
        defaultOnly<ListWidget>(f, "ListWidget");
        defaultOnly<TableView>(f, "TableView");
        defaultOnly<TableWidget>(f, "TableWidget");
        defaultOnly<TreeView>(f, "TreeView");
        defaultOnly<TreeWidget>(f, "TreeWidget");
        defaultOnly<OpacityAniStackedWidget>(f, "OpacityAniStackedWidget");
        defaultOnly<PopUpAniStackedWidget>(f, "PopUpAniStackedWidget");
        defaultOnly<DrillInTransitionStackedWidget>(f, "DrillInTransitionStackedWidget");
        defaultOnly<EntranceTransitionStackedWidget>(f, "EntranceTransitionStackedWidget");

        // Cards
        defaultOnly<CardWidget>(f, "CardWidget");
        defaultOnly<SimpleCardWidget>(f, "SimpleCardWidget");
        defaultOnly<ElevatedCardWidget>(f, "ElevatedCardWidget");
        defaultOnly<HeaderCardWidget>(f, "HeaderCardWidget");
        defaultOnly<GroupHeaderCardWidget>(f, "GroupHeaderCardWidget");
        f["HeaderCardWidget/text"] = [](const QString &t, QWidget *) { return new HeaderCardWidget(t); };
        f["CardGroupWidget/icon+text"] = [=](const QString &t, QWidget *) {
            return new CardGroupWidget(mail(), t, content);
        };

        // Command bar
        defaultOnly<CommandBar>(f, "CommandBar");
        defaultOnly<CommandBarView>(f, "CommandBarView");
        f["CommandBar/items"] = [](const QString &t, QWidget *) {
            auto *w = new CommandBar;
            w->addAction(FluentIcon::Mail, t);
            w->addAction(FluentIcon::Save, t + QLatin1Char('2'));
            return w;
        };

        // Navigation
        defaultOnly<Pivot>(f, "Pivot");
        defaultOnly<SegmentedWidget>(f, "SegmentedWidget");
        defaultOnly<SegmentedToggleToolWidget>(f, "SegmentedToggleToolWidget");
        defaultOnly<TabBar>(f, "TabBar");
        defaultOnly<TabWidget>(f, "TabWidget");
        defaultOnly<BreadcrumbBar>(f, "BreadcrumbBar");
        defaultOnly<NavigationBar>(f, "NavigationBar");
        defaultOnly<NavigationPanel>(f, "NavigationPanel");
        defaultOnly<NavigationInterface>(f, "NavigationInterface");
        defaultOnly<NavigationUserCard>(f, "NavigationUserCard");
        f["Pivot/items"] = [](const QString &t, QWidget *) {
            auto *w = new Pivot;
            const QStringList items = itemsOf(t);
            for (int i = 0; i < items.size(); ++i) {
                w->addItem(QStringLiteral("k%1").arg(i), items[i]);
            }
            return w;
        };
        f["SegmentedWidget/items"] = [](const QString &t, QWidget *) {
            auto *w = new SegmentedWidget;
            const QStringList items = itemsOf(t);
            for (int i = 0; i < items.size(); ++i) {
                w->addItem(QStringLiteral("k%1").arg(i), items[i]);
            }
            return w;
        };
        f["SegmentedToggleToolWidget/items"] = [](const QString &, QWidget *) {
            auto *w = new SegmentedToggleToolWidget;
            w->addItem(QStringLiteral("k0"), icon(FluentIcon::Mail));
            w->addItem(QStringLiteral("k1"), icon(FluentIcon::Save));
            return w;
        };
        f["TabBar/items"] = [](const QString &t, QWidget *) {
            auto *w = new TabBar;
            const QStringList items = itemsOf(t);
            for (int i = 0; i < items.size(); ++i) {
                w->addTab(QStringLiteral("k%1").arg(i), items[i]);
            }
            return w;
        };
        f["BreadcrumbBar/items"] = [](const QString &t, QWidget *) {
            auto *w = new BreadcrumbBar;
            const QStringList items = itemsOf(t);
            for (int i = 0; i < items.size(); ++i) {
                w->addItem(QStringLiteral("k%1").arg(i), items[i]);
            }
            return w;
        };
        f["PivotItem/text"] = [](const QString &t, QWidget *) { return new PivotItem(t.toLower(), t); };
        f["SegmentedItem/text"] = [](const QString &t, QWidget *) { return new SegmentedItem(t.toLower(), t); };
        f["TabItem/text"] = [](const QString &t, QWidget *) { return new TabItem(t.toLower(), t); };
        f["NavigationToolButton/icon"] = [](const QString &, QWidget *) { return new NavigationToolButton(mail()); };
        f["NavigationBarPushButton/icon+text"] = [](const QString &t, QWidget *) {
            return new NavigationBarPushButton(mail(), t, true);
        };
        f["NavigationTreeWidget/icon+text"] = [](const QString &t, QWidget *container) {
            return new NavigationTreeWidget(mail(), t, true, container);
        };
        f["NavigationItemHeader/text"] = [](const QString &t, QWidget *) { return new NavigationItemHeader(t); };
        f["NavigationAvatarWidget/text"] = [](const QString &t, QWidget *) { return new NavigationAvatarWidget(t); };

        // Flyout contents
        f["FlyoutView/text"] = [=](const QString &t, QWidget *) { return new FlyoutView(t, content); };
        f["TeachingTipView/text"] = [=](const QString &t, QWidget *) { return new TeachingTipView(t, content); };

        // Setting cards
        const QString button = QStringLiteral("Button");
        const QStringList options = {QStringLiteral("Alpha"), QStringLiteral("Beta")};
        const QColor red(QStringLiteral("#ff0000"));
        f["SettingCard/icon+text"] = [=](const QString &t, QWidget *) {
            return new SettingCard(FluentIcon::Mail, t, content);
        };
        f["SwitchSettingCard/icon+text"] = [=](const QString &t, QWidget *) {
            return new SwitchSettingCard(FluentIcon::Mail, t, content);
        };
        f["PushSettingCard/icon+text"] = [=](const QString &t, QWidget *) {
            return new PushSettingCard(button, FluentIcon::Mail, t, content);
        };
        f["PrimaryPushSettingCard/icon+text"] = [=](const QString &t, QWidget *) {
            return new PrimaryPushSettingCard(button, FluentIcon::Mail, t, content);
        };
        f["HyperlinkCard/icon+text"] = [=](const QString &t, QWidget *) {
            return new HyperlinkCard(url, button, FluentIcon::Mail, t, content);
        };
        f["RangeSettingCard/icon+text"] = [=](const QString &t, QWidget *) {
            return new RangeSettingCard(0, 100, 50, FluentIcon::Mail, t, content);
        };
        f["OptionsSettingCard/icon+text"] = [=](const QString &t, QWidget *) {
            return new OptionsSettingCard(options, {QStringLiteral("a"), QStringLiteral("b")}, FluentIcon::Mail, t,
                                          content);
        };
        f["ComboBoxSettingCard/icon+text"] = [=](const QString &t, QWidget *) {
            return new ComboBoxSettingCard(options, FluentIcon::Mail, t, content);
        };
        f["ColorSettingCard/icon+text"] = [=](const QString &t, QWidget *) {
            return new ColorSettingCard(red, FluentIcon::Mail, t, content);
        };
        f["CustomColorSettingCard/icon+text"] = [=](const QString &t, QWidget *) {
            return new CustomColorSettingCard(red, red, FluentIcon::Mail, t, content);
        };
        // Python's FolderListSettingCard takes no icon and always shows FluentIcon.FOLDER.
        f["FolderListSettingCard/icon+text"] = [=](const QString &t, QWidget *) {
            return new FolderListSettingCard({}, FluentIcon::Folder, t, content);
        };
        f["ExpandSettingCard/icon+text"] = [=](const QString &t, QWidget *) {
            return new ExpandSettingCard(FluentIcon::Mail, t, content);
        };
        f["ExpandGroupSettingCard/icon+text"] = [=](const QString &t, QWidget *) {
            return new ExpandGroupSettingCard(FluentIcon::Mail, t, content);
        };
        f["SimpleExpandGroupSettingCard/icon+text"] = [=](const QString &t, QWidget *) {
            return new SimpleExpandGroupSettingCard(FluentIcon::Mail, t, content);
        };
        f["SettingCardGroup/text"] = [](const QString &t, QWidget *) { return new SettingCardGroup(t); };
        return f;
    }();
    return table;
}

// ---------------------------------------------------------------------------------------------------------

QJsonObject pythonData()
{
    static const QJsonObject data = [] {
        QFile file(QStringLiteral(FQW_TEST_DATA_DIR "/python_sizes.json"));
        if (!file.open(QIODevice::ReadOnly)) {
            return QJsonObject();
        }
        return QJsonDocument::fromJson(file.readAll()).object();
    }();
    return data;
}

QStringList metricsOf(const Expectation &e)
{
    const QString text = QLatin1String(e.metrics);
    if (text == QLatin1String("*")) {
        QStringList all;
        for (const QString &m : kMetrics) {
            all << m + QStringLiteral(".w") << m + QStringLiteral(".h");
        }
        return all;
    }
    return text.split(QLatin1Char(' '), Qt::SkipEmptyParts);
}

const Expectation *find(const std::vector<Expectation> &table, const QString &row, const QString &metric)
{
    for (const Expectation &e : table) {
        if (QLatin1String(e.row) == row && metricsOf(e).contains(metric)) {
            return &e;
        }
    }
    return nullptr;
}

int jsonValue(const QJsonValue &value) { return value.isString() ? kWidgetSizeMax : value.toInt(); }

QString valueText(int value) { return value >= kWidgetSizeMax ? QStringLiteral("max") : QString::number(value); }

struct Measured
{
    QSize size;
    QSize sizeHint;
    QSize minimumSize;
    QSize maximumSize;

    QSize metric(const QString &name) const
    {
        if (name == QLatin1String("size")) {
            return size;
        }
        if (name == QLatin1String("sizeHint")) {
            return sizeHint;
        }
        if (name == QLatin1String("minimumSize")) {
            return minimumSize;
        }
        return maximumSize;
    }
};

// As measure() in measure_python.py: top-left in a large container, shown, events settled.
Measured measure(const Factory &factory, const QString &text)
{
    QWidget container;
    auto *layout = new QVBoxLayout(&container);
    layout->setContentsMargins(0, 0, 0, 0);
    QWidget *widget = factory(text, &container);
    layout->addWidget(widget, 0, Qt::AlignLeft | Qt::AlignTop);
    container.resize(1200, 900);
    container.show();
    (void)QTest::qWaitForWindowExposed(&container);
    QTest::qWait(50);
    return {widget->size(), widget->sizeHint(), widget->minimumSize(), widget->maximumSize()};
}

} // namespace

class PythonParityTest : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase()
    {
        QVERIFY2(!pythonData().isEmpty(), "tests/data/python_sizes.json is missing or not valid JSON");
        const QJsonObject meta = pythonData().value(QStringLiteral("meta")).toObject();

        // The same setup as measure_python.py: Fusion (the platform style sizes scroll bars, sliders and
        // check indicators differently natively and offscreen), light theme, a fixed application font.
        QApplication::setStyle(QStyleFactory::create(meta.value(QStringLiteral("style")).toString()));
        ThemeManager::instance()->setTheme(Theme::Light);
        // After the theme: setting the theme gives the application font the Fluent families.
        const QJsonObject appFont = meta.value(QStringLiteral("appFont")).toObject();
        QFont font;
        QStringList families;
        for (const QJsonValue &family : appFont.value(QStringLiteral("families")).toArray()) {
            families << family.toString();
        }
        font.setFamilies(families);
        font.setPixelSize(appFont.value(QStringLiteral("pixelSize")).toInt());
        QApplication::setFont(font);

        const int dpi = qRound(QGuiApplication::primaryScreen()->logicalDotsPerInch());
        QCOMPARE(dpi, meta.value(QStringLiteral("logicalDpi")).toInt());

        // Text widths are only comparable in the font the data was measured in.
        const QString resolved = QFontInfo(font).family();
        const QString expected = appFont.value(QStringLiteral("resolvedFamily")).toString();
        if (resolved != expected) {
            QSKIP(qPrintable(QStringLiteral("The data was measured with the font %1, this system resolves %2: "
                                            "regenerate it here (tools/parity/README.md).")
                                 .arg(expected, resolved)));
        }
    }

    // Plain QLabels in the Fluent font, measured by both: how far Qt 5 and Qt 6 text layout alone
    // differ. Within kTextWidthTolerance, or the tolerance for text-dependent widths is out of date.
    void textMetricsControls()
    {
        const QJsonObject controls = pythonData().value(QStringLiteral("controls")).toObject();
        QVERIFY(!controls.isEmpty());
        int widest = 0;
        for (auto it = controls.begin(); it != controls.end(); ++it) {
            const QStringList parts = it.key().split(QLatin1Char('/'));
            QCOMPARE(parts.size(), 4);
            QLabel label(parts[3]);
            QFont font;
            font.setFamilies(
                {QStringLiteral("Segoe UI"), QStringLiteral("Microsoft YaHei"), QStringLiteral("PingFang SC")});
            // Qt 5's QFont::Normal and DemiBold are 50 and 63.
            font.setWeight(parts[1].toInt() == 63 ? QFont::DemiBold : QFont::Normal);
            font.setPixelSize(parts[2].toInt());
            label.setFont(font);
            const QJsonArray python = it.value().toArray();
            const int dw = label.sizeHint().width() - python[0].toInt();
            const int dh = label.sizeHint().height() - python[1].toInt();
            widest = qMax(widest, qAbs(dw));
            QVERIFY2(qAbs(dh) <= kTolerance, qPrintable(QStringLiteral("%1: height %2 vs Python %3")
                                                            .arg(it.key())
                                                            .arg(label.sizeHint().height())
                                                            .arg(python[1].toInt())));
        }
        qInfo("widest Qt 5 vs Qt 6 text width difference: %d px", widest);
        QVERIFY2(widest + kTolerance <= kTextWidthTolerance,
                 qPrintable(QStringLiteral("text widths differ by up to %1 px").arg(widest)));
    }

    // Plain Qt widgets, no Fluent code, measured by both. Where they differ, Qt 5.15 vs Qt 6 does, and a
    // Fluent class built on the same Qt class may differ for that reason alone: kQtBaselineDifferences
    // lists them, and a difference missing there, or listed but gone, fails here.
    void qtBaseline()
    {
        const QJsonObject python = pythonData().value(QStringLiteral("qtBaseline")).toObject();
        const std::map<QString, std::function<QWidget *()>> widgets = {
            {QStringLiteral("QSpinBox"), [] { return new QSpinBox; }},
            {QStringLiteral("QDoubleSpinBox"), [] { return new QDoubleSpinBox; }},
            {QStringLiteral("QDateEdit"), [] { return new QDateEdit; }},
            {QStringLiteral("QTimeEdit"), [] { return new QTimeEdit; }},
            {QStringLiteral("QDateTimeEdit"), [] { return new QDateTimeEdit; }},
            {QStringLiteral("QLineEdit"), [] { return new QLineEdit; }},
            {QStringLiteral("QScrollArea"), [] { return new QScrollArea; }},
            {QStringLiteral("QListWidget"), [] { return new QListWidget; }},
            {QStringLiteral("QSlider"), [] { return new QSlider; }},
            {QStringLiteral("QProgressBar"), [] { return new QProgressBar; }},
            {QStringLiteral("QPushButton"), [] { return new QPushButton(QStringLiteral("Button")); }},
            {QStringLiteral("QToolButton"), [] { return new QToolButton; }},
            {QStringLiteral("QCheckBox"), [] { return new QCheckBox(QStringLiteral("Option")); }},
            {QStringLiteral("QRadioButton"), [] { return new QRadioButton(QStringLiteral("Option")); }},
        };
        QCOMPARE(python.size(), int(widgets.size()));
        for (const auto &entry : widgets) {
            const QString &name = entry.first;
            QScopedPointer<QWidget> widget(entry.second());
            const QJsonArray want = python.value(name).toArray();
            const QSize got = widget->sizeHint();
            const bool same =
                qAbs(got.width() - want[0].toInt()) <= kTolerance && qAbs(got.height() - want[1].toInt()) <= kTolerance;
            const QString message = QStringLiteral("%1 sizeHint: Qt 6 %2x%3, Qt 5 %4x%5")
                                        .arg(name)
                                        .arg(got.width())
                                        .arg(got.height())
                                        .arg(want[0].toInt())
                                        .arg(want[1].toInt());
            if (const auto known = std::find_if(kQtBaselineDifferences.begin(), kQtBaselineDifferences.end(),
                                                [&](const Expectation &e) { return name == QLatin1String(e.row); });
                known != kQtBaselineDifferences.end()) {
                qInfo("%s; %s", qPrintable(message), known->reason);
                QEXPECT_FAIL("", known->reason, Continue);
            }
            QVERIFY2(same, qPrintable(message));
        }
    }

    // Every Python row has a C++ factory, every factory a Python row, every table entry a row and metrics.
    void tablesAndFactoriesAreCurrent()
    {
        const QJsonObject rows = pythonData().value(QStringLiteral("rows")).toObject();
        for (auto it = rows.begin(); it != rows.end(); ++it) {
            QVERIFY2(factories().count(it.key()), qPrintable(it.key() + QStringLiteral(": no C++ factory")));
        }
        for (const auto &entry : factories()) {
            QVERIFY2(rows.contains(entry.first), qPrintable(entry.first + QStringLiteral(": no Python data")));
        }
        QStringList valid;
        for (const QString &m : kMetrics) {
            valid << m + QStringLiteral(".w") << m + QStringLiteral(".h");
        }
        for (const auto *table : {&kIntended, &kPending}) {
            for (const Expectation &e : *table) {
                QVERIFY2(rows.contains(QLatin1String(e.row)), e.row);
                QVERIFY2(e.reason && *e.reason, e.row);
                for (const QString &m : metricsOf(e)) {
                    QVERIFY2(valid.contains(m), qPrintable(QLatin1String(e.row) + QStringLiteral(": ") + m));
                }
            }
        }
        for (const Expectation &e : kIntended) {
            for (const Expectation &p : kPending) {
                if (QLatin1String(e.row) == QLatin1String(p.row)) {
                    for (const QString &m : metricsOf(e)) {
                        QVERIFY2(!metricsOf(p).contains(m), qPrintable(QLatin1String(e.row) + QStringLiteral(" ") + m +
                                                                       QStringLiteral(" is in both tables")));
                    }
                }
            }
        }
    }

    void matchesPython_data()
    {
        QTest::addColumn<QString>("row");
        const QJsonObject rows = pythonData().value(QStringLiteral("rows")).toObject();
        for (auto it = rows.begin(); it != rows.end(); ++it) {
            QTest::newRow(qPrintable(it.key())) << it.key();
        }
    }

    void matchesPython()
    {
        QFETCH(QString, row);
        const auto factory = factories().find(row);
        if (factory == factories().end()) {
            QFAIL("no C++ factory");
        }
        const QJsonObject python = pythonData().value(QStringLiteral("rows")).toObject().value(row).toObject();
        const Measured measured = measure(factory->second, python.value(QStringLiteral("text")).toString());
        const bool textWidth = python.value(QStringLiteral("textDependentWidth")).toBool();
        const bool textHeight = python.value(QStringLiteral("textDependentHeight")).toBool();

        if (m_dump) {
            QJsonObject out;
            for (const QString &metric : kMetrics) {
                const QSize v = measured.metric(metric);
                out[metric] = QJsonArray{valueText(v.width()), valueText(v.height())};
            }
            (*m_dump)[row] = out;
        }

        // Pending metrics first, each its own expected failure; then every other mismatch in one failure,
        // so that a row lists all its differences at once.
        QStringList unexpected;
        for (const QString &metric : kMetrics) {
            const QJsonArray expected = python.value(metric).toArray();
            const QSize actual = measured.metric(metric);
            for (int axis = 0; axis < 2; ++axis) {
                const QString name = metric + (axis == 0 ? QStringLiteral(".w") : QStringLiteral(".h"));
                const int want = jsonValue(expected[axis]);
                const int got = axis == 0 ? actual.width() : actual.height();
                const bool textDependent = axis == 0 ? textWidth : textHeight;
                const int tolerance =
                    (textDependent && metric != QLatin1String("maximumSize")) ? kTextWidthTolerance : kTolerance;
                const bool same =
                    (want >= kWidgetSizeMax || got >= kWidgetSizeMax) ? want == got : qAbs(got - want) <= tolerance;
                const QString message = QStringLiteral("%1: C++ %2, Python %3 (tolerance %4)")
                                            .arg(name, valueText(got), valueText(want))
                                            .arg(tolerance);
                if (const Expectation *e = find(kIntended, row, name)) {
                    if (!same) {
                        qInfo("intended: %s %s; %s", qPrintable(row), qPrintable(message), e->reason);
                    }
                    continue;
                }
                if (const Expectation *e = find(kPending, row, name)) {
                    QEXPECT_FAIL("", qPrintable(QStringLiteral("pending: ") + QLatin1String(e->reason)), Continue);
                    QVERIFY2(same, qPrintable(message));
                    continue;
                }
                if (!same) {
                    unexpected << message;
                }
            }
        }
        QVERIFY2(unexpected.isEmpty(), qPrintable(unexpected.join(QStringLiteral("; "))));
    }

    void cleanupTestCase()
    {
        if (m_dump) {
            QFile file(qEnvironmentVariable("FQW_PARITY_DUMP"));
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.write(QJsonDocument(*m_dump).toJson());
        }
    }

  private:
    // FQW_PARITY_DUMP=<file> writes every C++ measurement there, for updating the tables.
    std::optional<QJsonObject> m_dump =
        qEnvironmentVariableIsSet("FQW_PARITY_DUMP") ? std::optional<QJsonObject>(QJsonObject()) : std::nullopt;
};

// Always offscreen, on a screen like Python's, also when ctest runs natively. Both runtimes have to
// measure at the same logical DPI, since Fusion scales its scroll bar extent, spin box buttons and frames
// by it, and a font sized in points follows it. PyQt5's Qt 5.15 offscreen screen is 800x600 at 100 DPI
// and cannot be configured; Qt 6's offscreen plugin can, and tests/data/parity_offscreen.json gives it
// the same screen. Natively macOS reports 72 DPI: scroll areas and navigation panels come out 20 px
// narrower, spin boxes 2 px, point-sized text a quarter smaller (hiding HeaderCardWidget's title bug), and
// AA_Use96Dpi is ignored by the styles on macOS, so there is no native setup that matches.
int main(int argc, char *argv[])
{
    qputenv("QT_QPA_PLATFORM", "offscreen:configfile=" FQW_TEST_DATA_DIR "/parity_offscreen.json");
    qunsetenv("QT_SCALE_FACTOR");
    QApplication app(argc, argv);
    PythonParityTest test;
    QTEST_SET_MAIN_SOURCE_PATH
    return QTest::qExec(&test, argc, argv);
}
#include "tst_python_parity.moc"
