// Theme manager, colours, style sheet manager and translations.

#include <FluentQtWidgets/FluentQtWidgets.h>
#include <QtGui/QColor>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtGui/QPalette>
#include <QtGui/QPixmap>
#include <QtGui/QScreen>
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>
#include <QtWidgets/QProxyStyle>
#include <QtWidgets/QWidget>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

class ThemeTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void themeNameMatchesResourceDirectory()
    {
        QCOMPARE(FluentQt::themeName(FluentQt::Theme::Light), QStringLiteral("light"));
        QCOMPARE(FluentQt::themeName(FluentQt::Theme::Dark), QStringLiteral("dark"));
        QCOMPARE(FluentQt::themeName(FluentQt::Theme::Auto), QStringLiteral("auto"));
    }

    // Painted accents use the per-theme colour, as Python's themeColor() does: in dark mode the
    // accent is lightened (#009faa becomes #29f1ff), never painted raw.
    void paintedAccentsUseThePerThemeColor()
    {
        auto *manager = FluentQt::ThemeManager::instance();
        manager->setAccentColor(QColor(QStringLiteral("#009faa")));
        manager->setTheme(FluentQt::Theme::Dark);
        const QColor primary = FluentQt::themeColor();
        const QColor raw = manager->accentColor();
        QVERIFY(primary != raw);

        const auto pixelsNear = [](const QImage &image, const QColor &color) {
            int count = 0;
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    const QColor pixel = image.pixelColor(x, y);
                    if (pixel.alpha() > 200 && qAbs(pixel.red() - color.red()) <= 6 &&
                        qAbs(pixel.green() - color.green()) <= 6 && qAbs(pixel.blue() - color.blue()) <= 6) {
                        ++count;
                    }
                }
            }
            return count;
        };

        FluentQt::SwitchButton switchButton;
        switchButton.setChecked(true);
        FluentQt::InfoBadge badge(QStringLiteral("8"), FluentQt::InfoLevel::Attention);
        for (QWidget *widget : {static_cast<QWidget *>(&switchButton), static_cast<QWidget *>(&badge)}) {
            // Away from the offscreen pointer at (0, 0), which would paint the hover colour instead.
            widget->move(QGuiApplication::primaryScreen()->availableGeometry().topLeft() + QPoint(300, 300));
            widget->show();
            QVERIFY(QTest::qWaitForWindowExposed(widget));
            QVERIFY(FqwTest::waitForAnimations(widget));
            const QImage image = widget->grab().toImage();
            QVERIFY2(pixelsNear(image, primary) > 20, widget->metaObject()->className());
            QVERIFY2(pixelsNear(image, raw) == 0, widget->metaObject()->className());
        }
    }

    void settingCurrentThemeStillAppliesApplicationPalette()
    {
        auto *app = qobject_cast<QApplication *>(QCoreApplication::instance());
        QVERIFY(app != nullptr);

        auto *manager = FluentQt::ThemeManager::instance();
        const QPalette previousPalette = app->palette();

        manager->setTheme(FluentQt::Theme::Light);

        QPalette wrongPalette = app->palette();
        wrongPalette.setColor(QPalette::ButtonText, QColor(Qt::white));
        wrongPalette.setColor(QPalette::WindowText, QColor(Qt::white));
        app->setPalette(wrongPalette);

        QSignalSpy finishedSpy(manager, &FluentQt::ThemeManager::themeChangedFinished);
        manager->setTheme(FluentQt::Theme::Light);
        QCOMPARE(finishedSpy.count(), 1);
        QCOMPARE(app->palette().color(QPalette::ButtonText), QColor(32, 32, 32));
        QCOMPARE(app->palette().color(QPalette::WindowText), QColor(32, 32, 32));

        app->setPalette(previousPalette);
    }

    void autoThemeUsesSystemThemeListener()
    {
        auto *app = qobject_cast<QApplication *>(QCoreApplication::instance());
        QVERIFY(app != nullptr);

        auto *manager = FluentQt::ThemeManager::instance();
        const QPalette previousPalette = app->palette();

        QPalette testPalette = app->palette();
        testPalette.setColor(QPalette::Window, QColor(16, 16, 16));
        app->setPalette(testPalette);
        manager->systemThemeListener()->refresh();
        const FluentQt::Theme listenerTheme = manager->systemThemeListener()->systemTheme();

        manager->setTheme(FluentQt::Theme::Auto);
        QCOMPARE(manager->effectiveTheme(), listenerTheme);

        app->setPalette(previousPalette);
        manager->systemThemeListener()->refresh();
    }

    void themeColorPaletteMatchesReference()
    {
        QCOMPARE(FluentQt::color(FluentQt::FluentThemeColor::DefaultBlue), QColor(QStringLiteral("#0078D7")));
        QCOMPARE(FluentQt::color(FluentQt::FluentSystemColor::SuccessForeground, FluentQt::Theme::Light),
                 QColor(QStringLiteral("#0f7b0f")));
        QCOMPARE(FluentQt::color(FluentQt::FluentSystemColor::SuccessForeground, FluentQt::Theme::Dark),
                 QColor(QStringLiteral("#6ccb5f")));
    }

    void themeColorIsDerivedPerThemeLikePython_data()
    {
        QTest::addColumn<QString>("accent");
        QTest::addColumn<FluentQt::Theme>("theme");
        QTest::addColumn<QStringList>("expected"); // Primary, Dark1-3, Light1-3

        // Measured with qfluentwidgets 1.11.3: ThemeColor.<X>.color().name() after setThemeColor()/setTheme().
        QTest::newRow("#009faa light") << QStringLiteral("#009faa") << FluentQt::Theme::Light
                                       << QStringList{"#009faa", "#007780", "#005055", "#004044",
                                                      "#00a7b3", "#2daab3", "#3eabb3"};
        QTest::newRow("#009faa dark") << QStringLiteral("#009faa") << FluentQt::Theme::Dark
                                      << QStringList{"#29f1ff", "#25d9e6", "#25c6d1", "#24a9b3",
                                                     "#3af2ff", "#58f4ff", "#74f6ff"};
        QTest::newRow("#0078d7 dark") << QStringLiteral("#0078d7") << FluentQt::Theme::Dark
                                      << QStringList{"#29a0ff", "#2590e6", "#2585d1", "#2474b3",
                                                     "#3aa8ff", "#58b5ff", "#74c1ff"};
    }

    void themeColorIsDerivedPerThemeLikePython()
    {
        QFETCH(QString, accent);
        QFETCH(FluentQt::Theme, theme);
        QFETCH(QStringList, expected);

        auto *manager = FluentQt::ThemeManager::instance();
        manager->setAccentColor(QColor(accent));
        manager->setTheme(theme);

        const FluentQt::ThemeColor types[] = {FluentQt::ThemeColor::Primary, FluentQt::ThemeColor::Dark1,
                                              FluentQt::ThemeColor::Dark2,   FluentQt::ThemeColor::Dark3,
                                              FluentQt::ThemeColor::Light1,  FluentQt::ThemeColor::Light2,
                                              FluentQt::ThemeColor::Light3};
        for (int i = 0; i < 7; ++i) {
            QCOMPARE(FluentQt::derivedThemeColor(QColor(accent), types[i], theme).name(), expected.at(i));
        }
        // Python's themeColor() is ThemeColor.PRIMARY.color(): lighter than the accent in dark mode.
        QCOMPARE(FluentQt::themeColor().name(), expected.at(0));

        // The style sheets substitute the same colour.
        QCOMPARE(FluentQt::FluentStyleSheet::render(QStringLiteral("--ThemeColorPrimary"), theme), expected.at(0));
    }

    void styleSheetManagerRegistersAndRendersCustomQss()
    {
        QWidget widget;
        FluentQt::ThemeManager::instance()->setAccentColor(QColor(QStringLiteral("#445566")));

        FluentQt::FluentStyleSheet::setRole(&widget, QStringLiteral("UnitTestWidget"));
        QVERIFY(FluentQt::StyleSheetManager::instance()->isRegistered(&widget));
        QCOMPARE(widget.property("fqw").toString(), QStringLiteral("UnitTestWidget"));

        FluentQt::FluentStyleSheet::setCustomStyleSheet(
            &widget, QStringLiteral("QWidget { color: --ThemeColorPrimary; }"),
            QStringLiteral("QWidget { color: --ThemeColorPrimary; background: #202020; }"));
        QVERIFY(widget.styleSheet().contains(QStringLiteral("#445566")));

        FluentQt::ThemeManager::instance()->setAccentColor(QColor(QStringLiteral("#112233")));
        FluentQt::FluentStyleSheet::updateAll();
        QVERIFY(widget.styleSheet().contains(QStringLiteral("#112233")));
    }

    void themeChangeRestylesHiddenWidgetsWhenTheyAreShown()
    {
        // Restyling every hidden page of a large window took seconds on each theme change: visible
        // widgets restyle at once, the others when they are shown or drawn.
        auto *manager = FluentQt::ThemeManager::instance();
        manager->setTheme(FluentQt::Theme::Light);
        const QString light = QStringLiteral("QWidget { color: #010101; }");
        const QString dark = QStringLiteral("QWidget { color: #fefefe; }");
        QWidget visible;
        QWidget shownLater;
        QWidget drawnHidden;
        for (QWidget *widget : {&visible, &shownLater, &drawnHidden}) {
            FluentQt::FluentStyleSheet::setCustomStyleSheet(widget, light, dark);
            QVERIFY(widget->styleSheet().contains(QStringLiteral("#010101")));
        }
        visible.show();
        QVERIFY(QTest::qWaitForWindowExposed(&visible));

        manager->setTheme(FluentQt::Theme::Dark);
        QVERIFY(visible.styleSheet().contains(QStringLiteral("#fefefe")));
        QVERIFY(shownLater.styleSheet().contains(QStringLiteral("#010101")));
        // Applications set their sheets again on theme changes; the same sheets change nothing.
        FluentQt::FluentStyleSheet::setCustomStyleSheet(&shownLater, light, dark);
        QVERIFY(shownLater.styleSheet().contains(QStringLiteral("#010101")));

        shownLater.show();
        QVERIFY(shownLater.styleSheet().contains(QStringLiteral("#fefefe")));

        FluentQt::StyleSheetManager::instance()->ensureUpdated(&drawnHidden);
        QVERIFY(drawnHidden.styleSheet().contains(QStringLiteral("#fefefe")));
    }

    void styleSheetManagerSurvivesStylesThatSetProperties()
    {
        // Qt 6.8's Windows 11 style sets dynamic properties on a widget as it polishes and
        // unpolishes it; repolishing on each of them recursed until the stack overflowed.
        class PropertySettingStyle : public QProxyStyle
        {
          public:
            using QProxyStyle::polish;
            using QProxyStyle::unpolish;
            void polish(QWidget *widget) override
            {
                ++polished;
                if (active) {
                    widget->setProperty("styleState", polished);
                    widget->setProperty("_q_styleState", polished);
                }
                QProxyStyle::polish(widget);
            }
            void unpolish(QWidget *widget) override
            {
                if (active) {
                    widget->setProperty("styleState", QVariant());
                    widget->setProperty("_q_styleState", QVariant());
                }
                QProxyStyle::unpolish(widget);
            }
            int polished = 0;
            bool active = true;
        };

        PropertySettingStyle style;
        QWidget widget;
        widget.setStyle(&style);
        FluentQt::FluentStyleSheet::setRole(&widget, QStringLiteral("UnitTestWidget"));

        // A property the application sets repolishes once, whatever the style sets meanwhile.
        int before = style.polished;
        widget.setProperty("state", QStringLiteral("hover"));
        QCOMPARE(style.polished - before, 1);

        // Qt's own properties do not change how a widget looks.
        before = style.polished;
        widget.setProperty("_q_internal", 1);
        QCOMPARE(style.polished, before);
        style.active = false;
    }

    // The strings of the folder list dialog, colour card, title bar and navigation buttons, media
    // bars, switch button and dialog buttons must be in the library's .ts files; they once were not.
    void fluentTranslatorCoversTheLibrarysStrings_data()
    {
        QTest::addColumn<QString>("locale");
        QTest::addColumn<QByteArray>("context");
        QTest::addColumn<QByteArray>("source");
        QTest::addColumn<QString>("translation");
        const auto row = [](const char *locale, const char *context, const char *source, const char *translation) {
            QTest::addRow("%s %s %s", locale, context, source) << QString::fromLatin1(locale) << QByteArray(context)
                                                               << QByteArray(source) << QString::fromUtf8(translation);
        };
        row("zh_CN", "FluentQt::FolderListDialog", "Choose folder", "选择文件夹");
        row("zh_CN", "FluentQt::FolderListSettingCard", "Are you sure you want to delete the folder?",
            "确定要删除此文件夹吗？");
        row("zh_CN", "FluentQt::CustomColorSettingCard", "Choose color", "选择颜色");
        row("zh_CN", "FluentQt::FluentTitleBar", "Maximize", "最大化");
        row("zh_CN", "FluentQt::NavigationPanel", "Open Navigation", "打开导航");
        row("zh_CN", "FluentQt::PlayButton", "Pause", "暂停");
        row("zh_CN", "FluentQt::VolumeView", "Mute", "静音");
        row("zh_CN", "FluentQt::SwitchButton", "Off", "关");
        row("zh_CN", "FluentQt::MessageBoxBase", "Cancel", "取消");
        row("zh_HK", "FluentQt::FolderListDialog", "Choose folder", "選擇資料夾");
        row("zh_HK", "FluentQt::FluentTitleBar", "Close", "關閉");
        row("zh_HK", "FluentQt::VolumeView", "Unmute", "取消靜音");
        row("zh_HK", "FluentQt::MessageDialog", "OK", "確定");
    }

    void fluentTranslatorCoversTheLibrarysStrings()
    {
        QFETCH(QString, locale);
        QFETCH(QByteArray, context);
        QFETCH(QByteArray, source);
        QFETCH(QString, translation);
        FluentQt::FluentTranslator translator{QLocale(locale)};
        QCOMPARE(translator.translate(context.constData(), source.constData()), translation);
    }

    void fluentTranslatorOnlyLoadsMatchingLanguages()
    {
        FluentQt::FluentTranslator english(QLocale(QStringLiteral("en")));
        QVERIFY(english.isEmpty());

        FluentQt::FluentTranslator chinese(QLocale(QStringLiteral("zh_CN")));
        QVERIFY(!chinese.isEmpty());
        QCOMPARE(chinese.translate("FluentQt::CalendarPicker", "Pick a date"), QStringLiteral("选择日期"));

        FluentQt::FluentTranslator traditionalChinese(QLocale(QStringLiteral("zh_HK")));
        QVERIFY(!traditionalChinese.isEmpty());
        QCOMPARE(traditionalChinese.translate("FluentQt::CalendarPicker", "Pick a date"),
                 QStringLiteral("選擇日期"));

        FluentQt::FluentTranslator taiwanChinese(QLocale(QStringLiteral("zh_TW")));
        QVERIFY(!taiwanChinese.isEmpty());
        QCOMPARE(taiwanChinese.translate("FluentQt::CalendarPicker", "Pick a date"),
                 QStringLiteral("選擇日期"));
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(ThemeTest)
#include "tst_theme.moc"
