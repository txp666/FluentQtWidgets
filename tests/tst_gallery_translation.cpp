#include "GalleryWindow.h"
#include "GalleryTranslator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QLocale>
#include <QtCore/QPointer>
#include <QtTest/QtTest>
#include <QtWidgets/QApplication>

#include <FluentQtWidgets/Config.h>
#include <FluentQtWidgets/Settings/SettingCard.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/Menu.h>

class GalleryTranslationTest : public QObject
{
    Q_OBJECT

  private slots:
    void cleanup()
    {
        const auto widgets = QApplication::topLevelWidgets();
        for (QWidget *widget : widgets) {
            if (auto *window = qobject_cast<GalleryWindow *>(widget)) {
                window->close();
            }
        }
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    void languageIndexAndNameStayStable()
    {
        QCOMPARE(GalleryTranslation::languageNameForIndex(0), QStringLiteral("zh_CN"));
        QCOMPARE(GalleryTranslation::languageNameForIndex(1), QStringLiteral("zh_HK"));
        QCOMPARE(GalleryTranslation::languageNameForIndex(2), QStringLiteral("en"));
        QCOMPARE(GalleryTranslation::languageNameForIndex(3), QStringLiteral("Auto"));
        QCOMPARE(GalleryTranslation::languageNameForIndex(99), QStringLiteral("Auto"));

        QCOMPARE(GalleryTranslation::languageIndexForName(QStringLiteral("zh_CN")), 0);
        QCOMPARE(GalleryTranslation::languageIndexForName(QStringLiteral("zh_Hans")), 0);
        QCOMPARE(GalleryTranslation::languageIndexForName(QStringLiteral("zh")), 0);
        QCOMPARE(GalleryTranslation::languageIndexForName(QStringLiteral("zh_HK")), 1);
        QCOMPARE(GalleryTranslation::languageIndexForName(QStringLiteral("zh_Hant")), 1);
        QCOMPARE(GalleryTranslation::languageIndexForName(QStringLiteral("zh_TW")), 1);
        QCOMPARE(GalleryTranslation::languageIndexForName(QStringLiteral("zh_MO")), 1);
        QCOMPARE(GalleryTranslation::languageIndexForName(QStringLiteral("en")), 2);
        QCOMPARE(GalleryTranslation::languageIndexForName(QStringLiteral("en_US")), 2);
        QCOMPARE(GalleryTranslation::languageIndexForName(QStringLiteral("en_GB")), 2);
        QCOMPARE(GalleryTranslation::languageIndexForName(QStringLiteral("Auto")), 3);
        QCOMPARE(GalleryTranslation::languageIndexForName(QString()), 3);
        QCOMPARE(GalleryTranslation::languageIndexForName(QStringLiteral("fr_FR")), 3);
    }

    void localeForNameNormalizesSupportedAliases()
    {
        QCOMPARE(GalleryTranslation::localeForName(QStringLiteral("zh_Hans")).name(), QStringLiteral("zh_CN"));
        QCOMPARE(GalleryTranslation::localeForName(QStringLiteral("zh_TW")).name(), QStringLiteral("zh_HK"));
        QCOMPARE(GalleryTranslation::localeForName(QStringLiteral("en_US")).name(), QStringLiteral("en_US"));
    }

    void installTranslatorsLoadsGalleryResources()
    {
        auto *app = QCoreApplication::instance();
        QVERIFY(app != nullptr);

        QVERIFY(GalleryTranslation::installTranslators(app, QStringLiteral("zh_CN")));
        QCOMPARE(QCoreApplication::translate("FluentQt::CalendarPicker", "Pick a date"),
                 QStringLiteral("选择日期"));
        QCOMPARE(QCoreApplication::translate("SettingInterface", "Language"), QStringLiteral("语言"));
        QCOMPARE(QCoreApplication::translate("SettingInterface", "Help us improve FluentQtWidgets by providing feedback"),
                 QStringLiteral("通过提供反馈帮助我们改进 FluentQtWidgets"));
        QCOMPARE(QCoreApplication::translate("DialogInterface", "Show dialog"), QStringLiteral("显示对话框"));
        QCOMPARE(QCoreApplication::translate("DialogInterface", "Teaching tips with different tail positions"),
                 QStringLiteral("不同尾巴位置的气泡弹窗"));
        QCOMPARE(QCoreApplication::translate("DialogInterface", "Action"), QStringLiteral("操作"));
        QCOMPARE(QCoreApplication::translate("GalleryTray", "Show Gallery"), QStringLiteral("显示 Gallery"));
        QCOMPARE(QCoreApplication::translate("GalleryTray", "Quit"), QStringLiteral("退出"));

        QVERIFY(GalleryTranslation::installTranslators(app, QStringLiteral("zh_HK")));
        QCOMPARE(QCoreApplication::translate("FluentQt::CalendarPicker", "Pick a date"),
                 QStringLiteral("選擇日期"));
        QCOMPARE(QCoreApplication::translate("SettingInterface", "Language"), QStringLiteral("語言"));
        QCOMPARE(QCoreApplication::translate("SettingInterface", "Help us improve FluentQtWidgets by providing feedback"),
                 QStringLiteral("通過提供反饋幫助我們改進 FluentQtWidgets"));
        QCOMPARE(QCoreApplication::translate("DialogInterface", "Show dialog"), QStringLiteral("顯示對話框"));
        QCOMPARE(QCoreApplication::translate("DialogInterface", "Teaching tips with different tail positions"),
                 QStringLiteral("不同尾巴位置的氣泡彈窗"));
        QCOMPARE(QCoreApplication::translate("DialogInterface", "Action"), QStringLiteral("操作"));
        QCOMPARE(QCoreApplication::translate("GalleryTray", "Show Gallery"), QStringLiteral("顯示 Gallery"));
        QCOMPARE(QCoreApplication::translate("GalleryTray", "Quit"), QStringLiteral("退出"));

        GalleryTranslation::installTranslators(app, QStringLiteral("en"));
        QCOMPARE(QCoreApplication::translate("FluentQt::CalendarPicker", "Pick a date"),
                 QStringLiteral("Pick a date"));
        QCOMPARE(QCoreApplication::translate("SettingInterface", "Language"), QStringLiteral("Language"));
        QCOMPARE(QCoreApplication::translate("GalleryTray", "Show Gallery"), QStringLiteral("Show Gallery"));
        QCOMPARE(QCoreApplication::translate("GalleryTray", "Quit"), QStringLiteral("Quit"));
    }

    void languageReloadDoesNotDeleteStackWindow()
    {
        auto *app = qobject_cast<QApplication *>(QCoreApplication::instance());
        QVERIFY(app != nullptr);

        const bool previousQuitOnLastWindowClosed = QApplication::quitOnLastWindowClosed();
        QApplication::setQuitOnLastWindowClosed(false);
        const QString previousLocale = FluentQt::FluentConfig::instance()->localeName();

        FluentQt::FluentConfig::instance()->setLocaleName(QStringLiteral("en"));
        GalleryTranslation::installTranslators(app, QStringLiteral("en"));

        GalleryWindow window;
        QPointer<GalleryWindow> oldWindow(&window);
        window.resize(900, 640);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("settings")));

        FluentQt::ComboBoxSettingCard *languageCard = nullptr;
        const auto cards = window.findChildren<FluentQt::ComboBoxSettingCard *>();
        for (FluentQt::ComboBoxSettingCard *card : cards) {
            if (card->title() == QStringLiteral("Language")) {
                languageCard = card;
                break;
            }
        }
        QVERIFY(languageCard != nullptr);

        languageCard->setCurrentIndex(0);

        GalleryWindow *newWindow = nullptr;
        QTRY_VERIFY([&]() {
            const auto widgets = QApplication::topLevelWidgets();
            for (QWidget *widget : widgets) {
                auto *candidate = qobject_cast<GalleryWindow *>(widget);
                if (candidate && candidate != &window && candidate->isVisible()) {
                    newWindow = candidate;
                    return true;
                }
            }
            return false;
        }());

        QVERIFY(oldWindow);
        QVERIFY(!window.isVisible());
        QVERIFY(newWindow->testAttribute(Qt::WA_DeleteOnClose));
        QCOMPARE(newWindow->currentRouteKey(), QStringLiteral("settings"));

        newWindow->close();
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
        FluentQt::FluentConfig::instance()->setLocaleName(previousLocale);
        GalleryTranslation::installTranslators(app, previousLocale);
        QApplication::setQuitOnLastWindowClosed(previousQuitOnLastWindowClosed);
    }

    void basicInputSplitPushMenusUpdateVisibleText()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("basicInputInterface")));

        FluentQt::SplitPushButton *splitPush = nullptr;
        const auto splitPushes = window.findChildren<FluentQt::SplitPushButton *>();
        for (FluentQt::SplitPushButton *candidate : splitPushes) {
            if (candidate->text() == QStringLiteral("Choose your stand")) {
                splitPush = candidate;
                break;
            }
        }
        QVERIFY(splitPush != nullptr);
        QVERIFY(splitPush->dropDownMenu() != nullptr);
        splitPush->showFlyout();
        QTRY_VERIFY(splitPush->dropDownMenu()->isVisible());
        QListWidgetItem *normalItem = splitPush->dropDownMenu()->view()->item(1);
        QVERIFY(normalItem != nullptr);
        QTest::mouseClick(splitPush->dropDownMenu()->view()->viewport(), Qt::LeftButton, Qt::NoModifier,
                          splitPush->dropDownMenu()->view()->visualItemRect(normalItem).center());
        QTRY_VERIFY(!splitPush->dropDownMenu()->isVisible());
        QCOMPARE(splitPush->text(), QStringLiteral("Crazy Diamond"));

        FluentQt::PrimarySplitPushButton *primarySplitPush = nullptr;
        const auto primarySplitPushes = window.findChildren<FluentQt::PrimarySplitPushButton *>();
        for (FluentQt::PrimarySplitPushButton *candidate : primarySplitPushes) {
            if (candidate->text() == QStringLiteral("Choose your stand")) {
                primarySplitPush = candidate;
                break;
            }
        }
        QVERIFY(primarySplitPush != nullptr);
        QVERIFY(primarySplitPush->dropDownMenu() != nullptr);
        primarySplitPush->showFlyout();
        QTRY_VERIFY(primarySplitPush->dropDownMenu()->isVisible());
        QListWidgetItem *primaryItem = primarySplitPush->dropDownMenu()->view()->item(2);
        QVERIFY(primaryItem != nullptr);
        QTest::mouseClick(primarySplitPush->dropDownMenu()->view()->viewport(), Qt::LeftButton, Qt::NoModifier,
                          primarySplitPush->dropDownMenu()->view()->visualItemRect(primaryItem).center());
        QTRY_VERIFY(!primarySplitPush->dropDownMenu()->isVisible());
        QCOMPARE(primarySplitPush->text(), QStringLiteral("Gold Experience"));
    }

    void settingsThemeCardsFollowGlobalThemeChanges()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        auto *config = FluentQt::FluentConfig::instance();
        auto *manager = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousConfigTheme = config->themeMode();
        const FluentQt::Theme previousManagerTheme = manager->theme();

        config->setThemeMode(FluentQt::Theme::Light);
        manager->setTheme(FluentQt::Theme::Light);

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("settings")));

        FluentQt::ComboBoxSettingCard *themeModeCard = nullptr;
        const auto comboCards = window.findChildren<FluentQt::ComboBoxSettingCard *>();
        for (FluentQt::ComboBoxSettingCard *card : comboCards) {
            if (card->title() == QStringLiteral("Application theme")) {
                themeModeCard = card;
                break;
            }
        }
        QVERIFY(themeModeCard != nullptr);

        QCOMPARE(themeModeCard->currentIndex(), 0);

        manager->setTheme(FluentQt::Theme::Dark);
        QCOMPARE(config->themeMode(), FluentQt::Theme::Dark);
        QCOMPARE(themeModeCard->currentIndex(), 1);

        manager->setTheme(FluentQt::Theme::Auto);
        QCOMPARE(config->themeMode(), FluentQt::Theme::Auto);
        QCOMPARE(themeModeCard->currentIndex(), 2);

        config->setThemeMode(previousConfigTheme);
        config->save();
        manager->setTheme(previousManagerTheme);
    }

    void settingsZoomCardPersistsRestartScale()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        auto *config = FluentQt::FluentConfig::instance();
        const QString previousDpiScale = config->dpiScale();

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("settings")));

        FluentQt::OptionsSettingCard *zoomCard = nullptr;
        const auto optionCards = window.findChildren<FluentQt::OptionsSettingCard *>();
        for (FluentQt::OptionsSettingCard *card : optionCards) {
            if (card->card() && card->card()->title() == QStringLiteral("Interface zoom")) {
                zoomCard = card;
                break;
            }
        }
        QVERIFY(zoomCard != nullptr);
        QCOMPARE(zoomCard->value().toString(), previousDpiScale);

        const QString nextDpiScale = previousDpiScale == QStringLiteral("1.25") ? QStringLiteral("1.5")
                                                                                : QStringLiteral("1.25");
        zoomCard->setValue(nextDpiScale);
        QCOMPARE(config->dpiScale(), nextDpiScale);

        config->setDpiScale(previousDpiScale);
        config->save();
    }

    void settingsDownloadFolderCardUsesConfigValue()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        auto *config = FluentQt::FluentConfig::instance();
        const QString previousDownloadFolder = config->downloadFolder();
        const QString expectedDownloadFolder = QStringLiteral("app/test-download");
        config->setDownloadFolder(expectedDownloadFolder);

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("settings")));

        FluentQt::PushSettingCard *downloadCard = nullptr;
        const auto pushCards = window.findChildren<FluentQt::PushSettingCard *>();
        for (FluentQt::PushSettingCard *card : pushCards) {
            if (card->title() == QStringLiteral("Download directory")) {
                downloadCard = card;
                break;
            }
        }
        QVERIFY(downloadCard != nullptr);
        QCOMPARE(downloadCard->content(), expectedDownloadFolder);

        config->setDownloadFolder(previousDownloadFolder);
        config->save();
    }

    void settingsMicaAndAcrylicCardsFollowConfig()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        auto *config = FluentQt::FluentConfig::instance();
        const bool previousMicaEnabled = config->isMicaEnabled();
        const int previousAcrylicBlurRadius = config->acrylicBlurRadius();
        config->setAcrylicBlurRadius(23);

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("settings")));

        FluentQt::SwitchSettingCard *micaCard = nullptr;
        const auto switchCards = window.findChildren<FluentQt::SwitchSettingCard *>();
        for (FluentQt::SwitchSettingCard *card : switchCards) {
            if (card->title() == QStringLiteral("Mica effect")) {
                micaCard = card;
                break;
            }
        }
        QVERIFY(micaCard != nullptr);
        QCOMPARE(micaCard->isEnabled(), FluentQt::isMicaEffectAvailable());
        QCOMPARE(micaCard->isChecked(), FluentQt::isMicaEffectAvailable() && config->isMicaEnabled());

        FluentQt::RangeSettingCard *blurRadiusCard = nullptr;
        const auto rangeCards = window.findChildren<FluentQt::RangeSettingCard *>();
        for (FluentQt::RangeSettingCard *card : rangeCards) {
            if (card->title() == QStringLiteral("Acrylic blur radius")) {
                blurRadiusCard = card;
                break;
            }
        }
        QVERIFY(blurRadiusCard != nullptr);
        QCOMPARE(blurRadiusCard->value(), 23);

        blurRadiusCard->setValue(31);
        QCOMPARE(config->acrylicBlurRadius(), 31);

        config->setMicaEnabled(previousMicaEnabled);
        config->setAcrylicBlurRadius(previousAcrylicBlurRadius);
        config->save();
    }
};

QTEST_MAIN(GalleryTranslationTest)

#include "tst_gallery_translation.moc"
