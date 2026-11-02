// The fonts the widgets give themselves, under an application font that is not a Fluent one.
//
// Python's setFont(widget, size, weight) and getFont() take the configured Fluent families
// (qconfig.fontFamilies), whatever the application font. Taking font() and changing its size instead
// hides behind the application font as long as that is a Fluent one, as it is in the parity test; so
// every check here runs with a serif application font of another size and weight.

#include <FluentQtWidgets/FluentQtWidgets.h>
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStyleFactory>

#include "../src/core/FontUtils_p.h"
#include "TestSupport.h"

#include <functional>
#include <memory>
#include <optional>

using namespace FluentQt;

namespace {

using WidgetFactory = std::function<QWidget *(QWidget *host)>;

template <typename T> WidgetFactory make()
{
    return [](QWidget *host) -> QWidget * { return new T(host); };
}

} // namespace

Q_DECLARE_METATYPE(WidgetFactory)

class FontsTest : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase()
    {
        // Python's metrics are measured in Fusion (see tst_python_parity).
        QApplication::setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
        ThemeManager::instance()->setTheme(Theme::Light);
        // After the theme, which gives the application font the Fluent families.
        QFont appFont;
        appFont.setFamilies({QStringLiteral("Courier New"), QStringLiteral("Courier"),
                             QStringLiteral("Times New Roman"), QStringLiteral("DejaVu Serif"),
                             QStringLiteral("serif")});
        appFont.setPixelSize(11);
        appFont.setWeight(QFont::Light);
        QApplication::setFont(appFont);
        if (QFontInfo(appFont).family() == QFontInfo(Private::fluentFont(14)).family()) {
            QSKIP("This system resolves the test's application font to the Fluent font.");
        }
    }

    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void widgetsTakeTheFluentFont_data()
    {
        QTest::addColumn<WidgetFactory>("factory");
        QTest::addColumn<int>("pixelSize");
        QTest::addColumn<int>("weight");

        const int normal = QFont::Normal;
        const int demiBold = QFont::DemiBold;
        // button.py: setFont(self).
        QTest::newRow("PushButton") << make<PushButton>() << 14 << normal;
        QTest::newRow("PrimaryPushButton") << make<PrimaryPushButton>() << 14 << normal;
        QTest::newRow("TransparentPushButton") << make<TransparentPushButton>() << 14 << normal;
        QTest::newRow("ToggleButton") << make<ToggleButton>() << 14 << normal;
        QTest::newRow("HyperlinkButton") << make<HyperlinkButton>() << 14 << normal;
        QTest::newRow("ToolButton") << make<ToolButton>() << 14 << normal;
        QTest::newRow("PrimaryToolButton") << make<PrimaryToolButton>() << 14 << normal;
        QTest::newRow("CheckBox") << make<CheckBox>() << 14 << normal;
        QTest::newRow("ComboBox") << make<ComboBox>() << 14 << normal;
        QTest::newRow("ModelComboBox") << make<ModelComboBox>() << 14 << normal;
        // line_edit.py, spin_box.py: setFont(self).
        QTest::newRow("LineEdit") << make<LineEdit>() << 14 << normal;
        QTest::newRow("SearchLineEdit") << make<SearchLineEdit>() << 14 << normal;
        QTest::newRow("EditableComboBox") << make<EditableComboBox>() << 14 << normal;
        QTest::newRow("TextEdit") << make<TextEdit>() << 14 << normal;
        QTest::newRow("PlainTextEdit") << make<PlainTextEdit>() << 14 << normal;
        QTest::newRow("TextBrowser") << make<TextBrowser>() << 14 << normal;
        QTest::newRow("SpinBox") << make<SpinBox>() << 14 << normal;
        QTest::newRow("DoubleSpinBox") << make<DoubleSpinBox>() << 14 << normal;
        // label.py: getFont(size, weight) per role, setFont(self, 14) for HyperlinkLabel.
        QTest::newRow("CaptionLabel") << make<CaptionLabel>() << 12 << normal;
        QTest::newRow("BodyLabel") << make<BodyLabel>() << 14 << normal;
        QTest::newRow("StrongBodyLabel") << make<StrongBodyLabel>() << 14 << demiBold;
        QTest::newRow("SubtitleLabel") << make<SubtitleLabel>() << 20 << demiBold;
        QTest::newRow("TitleLabel") << make<TitleLabel>() << 28 << demiBold;
        QTest::newRow("LargeTitleLabel") << make<LargeTitleLabel>() << 40 << demiBold;
        QTest::newRow("DisplayLabel") << make<DisplayLabel>() << 68 << demiBold;
        QTest::newRow("HyperlinkLabel") << make<HyperlinkLabel>() << 14 << normal;
        QTest::newRow("AvatarWidget") << make<AvatarWidget>() << 48 << normal;
        // command_bar.py, info_badge.py, progress_ring.py, tab_view.py.
        QTest::newRow("CommandButton") << make<CommandButton>() << 12 << normal;
        QTest::newRow("CommandBar") << make<CommandBar>() << 12 << normal;
        QTest::newRow("InfoBadge") << make<InfoBadge>() << 11 << normal;
        QTest::newRow("ProgressRing") << make<ProgressRing>() << 14 << normal;
        QTest::newRow("TabItem") << make<TabItem>() << 12 << normal;
        // pivot.py, segmented_widget.py, breadcrumb.py, navigation_bar.py, navigation_widget.py.
        QTest::newRow("PivotItem") << make<PivotItem>() << 18 << normal;
        QTest::newRow("SegmentedItem") << make<SegmentedItem>() << 14 << normal;
        QTest::newRow("BreadcrumbBar") << make<BreadcrumbBar>() << 14 << normal;
        QTest::newRow("NavigationBarPushButton") << WidgetFactory([](QWidget *host) -> QWidget * {
            return new NavigationBarPushButton(QIcon(), QStringLiteral("Home"), true, QIcon(), host);
        }) << 11 << normal;
        QTest::newRow("NavigationItemHeader") << WidgetFactory([](QWidget *host) -> QWidget * {
            return new NavigationItemHeader(QStringLiteral("Header"), host);
        }) << 12 << normal;
        QTest::newRow("NavigationTreeWidget") << WidgetFactory([](QWidget *host) -> QWidget * {
            return new NavigationTreeWidget(QIcon(), QStringLiteral("Home"), true, host);
        }) << 14 << normal;
        QTest::newRow("NavigationToolButton")
            << WidgetFactory([](QWidget *host) -> QWidget * { return new NavigationToolButton(QIcon(), host); }) << 14
            << normal;
        QTest::newRow("NavigationAvatarWidget") << WidgetFactory([](QWidget *host) -> QWidget * {
            return new NavigationAvatarWidget(QStringLiteral("zhiyiYo"), host);
        }) << 14 << normal;
        // setting_card_group.py: setFont(self.titleLabel, 20); card_widget.py: 15 px DemiBold header.
        QTest::newRow("SettingCardGroup title") << WidgetFactory([](QWidget *host) -> QWidget * {
            return (new SettingCardGroup(QStringLiteral("Group"), host))->titleLabel();
        }) << 20 << normal;
        QTest::newRow("HeaderCardWidget title") << WidgetFactory([](QWidget *host) -> QWidget * {
            return (new HeaderCardWidget(QStringLiteral("Header"), host))->headerLabel();
        }) << 15 << demiBold;
    }

    void widgetsTakeTheFluentFont()
    {
        QFETCH(WidgetFactory, factory);
        QFETCH(int, pixelSize);
        QFETCH(int, weight);

        QWidget host;
        QWidget *widget = factory(&host);
        widget->ensurePolished();

        const QFont expected = Private::fluentFont(pixelSize, QFont::Weight(weight));
        QCOMPARE(widget->font().families(), expected.families());
        QCOMPARE(widget->font().pixelSize(), pixelSize);
        QCOMPARE(int(widget->font().weight()), weight);
    }

    // The symptom in the Gallery: push buttons 2 px shorter than Python's (30 px instead of 32
    // natively) under the Gallery's application font. Their height must not depend on it.
    void pushButtonHeightIgnoresTheApplicationFont()
    {
        PushButton push(QStringLiteral("Standard push button"));
        PrimaryPushButton primary(QStringLiteral("Accent style button"));
        const int pushHeight = push.sizeHint().height();
        const int primaryHeight = primary.sizeHint().height();

        const QFont appFont = QApplication::font();
        QApplication::setFont(Private::fluentFont(14));
        PushButton fluentPush(QStringLiteral("Standard push button"));
        PrimaryPushButton fluentPrimary(QStringLiteral("Accent style button"));
        const int fluentPushHeight = fluentPush.sizeHint().height();
        const int fluentPrimaryHeight = fluentPrimary.sizeHint().height();
        QApplication::setFont(appFont);

        QCOMPARE(pushHeight, fluentPushHeight);
        QCOMPARE(primaryHeight, fluentPrimaryHeight);
    }

    // The tool tip's label takes `font: 12px --FontFamilies` from tool_tip.qss, as in Python; a
    // `font-size` alone left it in the application's family.
    void toolTipLabelTakesTheFluentFamilies()
    {
        for (ToolTip *tip : std::initializer_list<ToolTip *>{new ToolTip(QStringLiteral("Tip")),
                                                             new AcrylicToolTip(QStringLiteral("Tip"))}) {
            std::unique_ptr<ToolTip> owner(tip);
            auto *label = tip->findChild<QLabel *>(QStringLiteral("contentLabel"));
            QVERIFY(label);
            tip->ensurePolished();
            label->ensurePolished();
            QCOMPARE(label->font().families().value(0), Private::fluentFont(12).families().value(0));
            QCOMPARE(label->font().pixelSize(), 12);
        }
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(FontsTest)
#include "tst_fonts.moc"
