// The bundled Fluent style sheets: the selectors and values the widgets rely on.

#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtTest/QtTest>

#include "TestSupport.h"

using namespace FluentQt;

namespace {

// A row per theme, so a failure names the file it is about.
void addStyleSheetRows(const char *name, const QString &file, const QStringList &themes, const QStringList &present,
                       const QStringList &absent = {})
{
    for (const QString &theme : themes) {
        QTest::newRow(qPrintable(QStringLiteral("%1 (%2)").arg(QLatin1String(name), theme)))
            << QStringLiteral(":/qfluentwidgets/qss/%1/%2").arg(theme, file) << present << absent;
    }
}

const QStringList kBothThemes = {QStringLiteral("light"), QStringLiteral("dark")};

} // namespace

class QssTest : public QObject
{
    Q_OBJECT

  private slots:
    void styleSheetsContainWhatTheWidgetsUse_data()
    {
        QTest::addColumn<QString>("path");
        QTest::addColumn<QStringList>("present");
        QTest::addColumn<QStringList>("absent");

        // C++ role selectors instead of the Python class names.
        addStyleSheetRows("spin box", QStringLiteral("spin_box.qss"), kBothThemes,
                          {QStringLiteral("QToolButton[fqw=\"SpinButton\"]"),
                           QStringLiteral("QToolButton[fqw=\"CompactSpinButton\"]"),
                           QStringLiteral("QSpinBox[fqw=\"SpinBox\"][symbolVisible=\"false\"]")},
                          {QStringLiteral("\nSpinButton {"), QStringLiteral("\nSpinButton:hover"),
                           QStringLiteral("\nSpinButton:pressed")});
        addStyleSheetRows("line edit", QStringLiteral("line_edit.qss"), kBothThemes,
                          {QStringLiteral("QTextEdit[fqw=\"TextEdit\"],"), QStringLiteral("font: 14px --FontFamilies;"),
                           QStringLiteral("padding: 2px 3px 2px 8px;"), QStringLiteral("#lineEditButton"),
                           QStringLiteral("#lineEditButton:hover"), QStringLiteral("#lineEditButton:pressed"),
                           QStringLiteral("QLineEdit[fqw=\"LineEdit\"]:disabled"),
                           // Python's editable combo boxes are line edits, styled by line_edit.qss.
                           QStringLiteral("QLineEdit[fqw=\"EditableComboBox\"]:disabled"),
                           QStringLiteral("QLineEdit[fqw=\"AcrylicEditableComboBox\"]:hover"),
                           QStringLiteral("QLineEdit[fqw=\"EditableModelComboBox\"],")});
        addStyleSheetRows("line edit focus", QStringLiteral("line_edit.qss"), {QStringLiteral("dark")},
                          {QStringLiteral("selection-color: black;"),
                           QStringLiteral("QLineEdit[fqw=\"LineEdit\"][transparent=\"true\"]:focus"),
                           QStringLiteral("QLineEdit[fqw=\"LineEdit\"][transparent=\"false\"]:focus")});
        addStyleSheetRows("combo box", QStringLiteral("combo_box.qss"), kBothThemes,
                          {QStringLiteral("QPushButton[fqw=\"ComboBox\"]"),
                           QStringLiteral("QPushButton[fqw=\"ModelComboBox\"]"),
                           QStringLiteral("[isPlaceholderText=\"true\"]")},
                          // Python sets the font in code; a sheet font would override setFont().
                          {QStringLiteral("QLineEdit"), QStringLiteral("font:")});
        addStyleSheetRows("calendar picker", QStringLiteral("calendar_picker.qss"), kBothThemes,
                          {QStringLiteral("QFrame[fqw=\"CalendarView\"]"),
                           QStringLiteral("QPushButton[calendarTitle=\"true\"]"),
                           QStringLiteral("QPushButton[calendarItem=\"true\"][selected=\"true\"]"),
                           QStringLiteral("QPushButton#monthButton[calendarItem=\"true\"]"),
                           QStringLiteral("QPushButton#yearButton[calendarItem=\"true\"]")});
        // Python metrics; radio buttons are styled with the buttons, not the check boxes.
        addStyleSheetRows("check box", QStringLiteral("check_box.qss"), kBothThemes,
                          {QStringLiteral("QCheckBox[fqw=\"CheckBox\"] {"), QStringLiteral("min-width: 28px;"),
                           QStringLiteral("min-height: 22px;"), QStringLiteral("margin-left: 1px;"),
                           QStringLiteral("QCheckBox[fqw=\"CheckBox\"]:disabled")},
                          {QStringLiteral("QRadioButton[fqw=\"RadioButton\"]"), QStringLiteral("font:")});
        addStyleSheetRows("radio button", QStringLiteral("button.qss"), kBothThemes,
                          {QStringLiteral("QRadioButton[fqw=\"RadioButton\"] {"),
                           QStringLiteral("font: 14px --FontFamilies;"), QStringLiteral("max-height: 24px;"),
                           QStringLiteral("QRadioButton[fqw=\"RadioButton\"]::indicator")});
        // Python applies slider.qss to no widget; the port's Slider paints itself like Python's, so
        // only ClickableSlider uses it, and the Slider selectors were dead.
        addStyleSheetRows("slider", QStringLiteral("slider.qss"), kBothThemes,
                          {QStringLiteral("QSlider[fqw=\"ClickableSlider\"]:horizontal"),
                           QStringLiteral("QSlider[fqw=\"ClickableSlider\"]::sub-page:horizontal"),
                           QStringLiteral("QSlider[fqw=\"ClickableSlider\"]::handle:horizontal:hover"),
                           QStringLiteral("QSlider[fqw=\"ClickableSlider\"]::handle:horizontal:pressed"),
                           QStringLiteral("QSlider[fqw=\"ClickableSlider\"]:vertical"),
                           QStringLiteral("QSlider[fqw=\"ClickableSlider\"]::add-page:vertical"),
                           QStringLiteral("QSlider[fqw=\"ClickableSlider\"]::handle:vertical:hover"),
                           QStringLiteral("qradialgradient")},
                          {QStringLiteral("QSlider[fqw=\"Slider\"]")});
        addStyleSheetRows("slider disabled", QStringLiteral("slider.qss"), {QStringLiteral("light")},
                          {QStringLiteral("QSlider[fqw=\"ClickableSlider\"]::groove:horizontal:disabled"),
                           QStringLiteral("QSlider[fqw=\"ClickableSlider\"]::handle:vertical:disabled")});
        addStyleSheetRows("buttons", QStringLiteral("button.qss"), kBothThemes,
                          {QStringLiteral("QToolButton[fqw=\"DropDownToolButton\"]"),
                           QStringLiteral("QToolButton[fqw=\"PrimaryDropDownToolButton\"]"),
                           QStringLiteral("QPushButton[fqw=\"PrimaryDropDownPushButton\"]"),
                           QStringLiteral("QPushButton[fqw=\"TransparentDropDownPushButton\"]"),
                           QStringLiteral("QToolButton[fqw=\"TransparentDropDownToolButton\"]"),
                           QStringLiteral("QToolButton[fqw=\"CommandButton\"]"),
                           QStringLiteral("QToolButton[fqw=\"MoreActionsButton\"]"),
                           QStringLiteral("QToolButton[fqw=\"CommandButton\"]:checked"),
                           QStringLiteral("QPushButton[fqw=\"TransparentTogglePushButton\"]:checked"),
                           QStringLiteral("QToolButton[fqw=\"TransparentToggleToolButton\"]:checked"),
                           QStringLiteral("QPushButton[fqw=\"HyperlinkButton\"]:disabled"),
                           QStringLiteral("QPushButton[fqw=\"PillPushButton\"]:disabled:checked"),
                           QStringLiteral("QPushButton[fqw=\"PushButton\"]:disabled"),
                           QStringLiteral("QToolButton[fqw=\"DropDownToolButton\"]:disabled")});
        addStyleSheetRows("switch button", QStringLiteral("switch_button.qss"), kBothThemes,
                          {QStringLiteral("QWidget[fqw=\"SwitchButton\"] QLabel"),
                           QStringLiteral("font: 14px --FontFamilies;"),
                           QStringLiteral("QWidget[fqw=\"SwitchButton\"] QLabel:disabled")});
        // The Python Gallery's look.
        addStyleSheetRows("state tooltip", QStringLiteral("state_tool_tip.qss"), {QStringLiteral("light")},
                          {QStringLiteral("background-color: --ThemeColorDark1"), QStringLiteral("color: white")},
                          {QStringLiteral("@accentColor")});
        addStyleSheetRows("state tooltip", QStringLiteral("state_tool_tip.qss"), {QStringLiteral("dark")},
                          {QStringLiteral("background-color: --ThemeColorDark1"), QStringLiteral("color: black")},
                          {QStringLiteral("@accentColor")});
        addStyleSheetRows("info badge", QStringLiteral("info_badge.qss"), {QStringLiteral("light")},
                          {QStringLiteral("padding: 1px 3px 1px 3px")}, {QStringLiteral("min-width: 20px")});
        // Painted by the widgets themselves; the style sheets must not draw over them.
        addStyleSheetRows("pips pager", QStringLiteral("pips_pager.qss"), kBothThemes,
                          {QStringLiteral("QToolButton[fqw=\"PipsPagerDot\"]"),
                           QStringLiteral("QToolButton[fqw=\"PipsPagerScrollButton\"]")},
                          {QStringLiteral("background: rgba")});
        addStyleSheetRows("scroll bar", QStringLiteral("scroll_bar.qss"), kBothThemes,
                          {QStringLiteral("QWidget[fqw=\"ScrollBar\"]")},
                          {QStringLiteral("QScrollBar[fqw=\"ScrollBar\"]"), QStringLiteral("::handle")});
    }

    void styleSheetsContainWhatTheWidgetsUse()
    {
        QFETCH(QString, path);
        QFETCH(QStringList, present);
        QFETCH(QStringList, absent);

        const QString qss = FqwTest::readText(path);
        QVERIFY2(!qss.isEmpty(), qPrintable(path));
        for (const QString &text : present) {
            QVERIFY2(qss.contains(text), qPrintable(text));
        }
        for (const QString &text : absent) {
            QVERIFY2(!qss.contains(text), qPrintable(text));
        }
    }

    void rolesUseTheirStyleSheet_data()
    {
        QTest::addColumn<QString>("role");
        QTest::addColumn<int>("source");

        const auto addRow = [](const char *role, FluentStyleSheetSource source) {
            QTest::newRow(role) << QString::fromLatin1(role) << static_cast<int>(source);
        };
        addRow("ComboBox", FluentStyleSheetSource::ComboBox);
        addRow("EditableComboBox", FluentStyleSheetSource::LineEdit);
        addRow("AcrylicEditableComboBox", FluentStyleSheetSource::LineEdit);
        addRow("ModelComboBox", FluentStyleSheetSource::ComboBox);
        addRow("EditableModelComboBox", FluentStyleSheetSource::LineEdit);
        addRow("ComboBoxMenu", FluentStyleSheetSource::Menu);
        addRow("CalendarView", FluentStyleSheetSource::CalendarPicker);
        addRow("FastCalendarView", FluentStyleSheetSource::CalendarPicker);
        addRow("PickerPanel", FluentStyleSheetSource::TimePicker);
        addRow("SeparatorWidget", FluentStyleSheetSource::TimePicker);
        addRow("ItemMaskWidget", FluentStyleSheetSource::TimePicker);
        addRow("RadioButton", FluentStyleSheetSource::Button);
        addRow("CommandButton", FluentStyleSheetSource::Button);
        addRow("MoreActionsButton", FluentStyleSheetSource::Button);
        // These paint themselves, and Python gives them no style sheet; progress_bar.qss is gone.
        addRow("ProgressBar", FluentStyleSheetSource::Global);
        addRow("IndeterminateProgressRing", FluentStyleSheetSource::Global);
        // Python leaves Slider unstyled; the port sets the role only as a marker.
        addRow("Slider", FluentStyleSheetSource::Global);
        addRow("ClickableSlider", FluentStyleSheetSource::Slider);
    }

    // Python's class-name selectors (`RoundMenu`, `FlipView`) never match the port's classes, which
    // live in the FluentQt namespace; the sheets select by role instead.
    void styleSheetsUseNoPythonClassSelectors()
    {
        static const QRegularExpression comment(QStringLiteral("/\\*.*?\\*/"),
                                                QRegularExpression::DotMatchesEverythingOption);
        static const QRegularExpression ruleStart(QStringLiteral("([^{}]*)\\{"));
        static const QRegularExpression notTypes(QStringLiteral("\\[[^\\]]*\\]|::?[\\w-]+|#[\\w-]+"));
        static const QRegularExpression pythonClass(QStringLiteral("\\b(?!Q)[A-Z]\\w*"));
        for (const Theme theme : {Theme::Light, Theme::Dark}) {
            for (int i = 0; i <= static_cast<int>(FluentStyleSheetSource::ExpandSettingCard); ++i) {
                const auto source = static_cast<FluentStyleSheetSource>(i);
                QString qss = FluentStyleSheet::loadSource(source, theme);
                qss.remove(comment);
                for (const QRegularExpressionMatch &rule : ruleStart.globalMatch(qss)) {
                    QString selectors = rule.captured(1);
                    selectors.remove(notTypes);
                    QVERIFY2(!pythonClass.match(selectors).hasMatch(),
                             qPrintable(styleSheetSourceName(source) + QStringLiteral(": ") + rule.captured(1)));
                }
            }
        }
    }

    void rolesUseTheirStyleSheet()
    {
        QFETCH(QString, role);
        QFETCH(int, source);
        QCOMPARE(static_cast<int>(styleSheetSourceForRole(role)), source);
    }

    // Python's sheets use only --ThemeColor* tokens, all derived per theme by ThemeColor.color().
    void styleSheetsOnlyUseThemeColorTokens()
    {
        for (const Theme theme : {Theme::Light, Theme::Dark}) {
            for (int i = 0; i <= static_cast<int>(FluentStyleSheetSource::ExpandSettingCard); ++i) {
                const auto source = static_cast<FluentStyleSheetSource>(i);
                const QString rendered = FluentStyleSheet::render(FluentStyleSheet::loadSource(source, theme), theme);
                const QString name = styleSheetSourceName(source);
                QVERIFY2(!rendered.contains(QLatin1Char('@')), qPrintable(name));
                QVERIFY2(!rendered.contains(QStringLiteral("--ThemeColor")), qPrintable(name));
            }
        }
    }

    // Python's sheets colour hyperlinks, radio indicators, checked command buttons, focused text
    // edits, selections and the calendar's today/selected items with --ThemeColorPrimary (lightened
    // in dark mode: #009faa becomes #29f1ff), not the raw accent colour.
    void accentedRulesUseThePrimaryThemeColor()
    {
        for (const Theme theme : {Theme::Light, Theme::Dark}) {
            for (const FluentStyleSheetSource source :
                 {FluentStyleSheetSource::LineEdit, FluentStyleSheetSource::SpinBox, FluentStyleSheetSource::ComboBox,
                  FluentStyleSheetSource::CalendarPicker}) {
                const QString qss = FluentStyleSheet::loadSource(source, theme);
                QVERIFY(!qss.contains(QStringLiteral("@accentColor")));
            }
            const QString buttons = FluentStyleSheet::loadSource(FluentStyleSheetSource::Button, theme);
            for (const char *selector :
                 {"QPushButton[fqw=\"HyperlinkButton\"] {", "QPushButton[fqw=\"HyperlinkButton\"]:hover {",
                  "QToolButton[fqw=\"CommandButton\"]:checked {"}) {
                const int start = buttons.indexOf(QLatin1String(selector));
                QVERIFY2(start >= 0, selector);
                const QString rule = buttons.mid(start, buttons.indexOf(QLatin1Char('}'), start) - start);
                QVERIFY2(rule.contains(QStringLiteral("--ThemeColorPrimary")), selector);
            }
        }
    }

    void navigationLightQssPinsTextColor()
    {
        const QString qss = FluentStyleSheet::loadSource(FluentStyleSheetSource::Pivot, Theme::Light);
        QVERIFY(qss.contains(QStringLiteral("QPushButton[fqw=\"PivotItem\"]")));
        QVERIFY(qss.contains(QStringLiteral("QPushButton[fqw=\"SegmentedItem\"]")));
        QVERIFY(qss.contains(QStringLiteral("QToolButton[fqw=\"SegmentedToolItem\"]")));
        QVERIFY(qss.contains(QStringLiteral("QWidget[fqw=\"SegmentedToolWidget\"]")));
        QVERIFY(qss.contains(QStringLiteral("QPushButton[fqw=\"BreadcrumbItem\"]")));
        QVERIFY(qss.contains(QStringLiteral("color: black;")));
    }
};

QTEST_MAIN(QssTest)
#include "tst_qss.moc"
