#include "GalleryWindow.h"
#include "GalleryTranslator.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QLocale>
#include <QtCore/QPointer>
#include <QtTest/QtTest>
#include <QtWidgets/QAbstractSpinBox>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QTreeWidgetItemIterator>
#include <QtWidgets/QVBoxLayout>

#include <FluentQtWidgets/Config.h>
#include <FluentQtWidgets/DateTime/CalendarPicker.h>
#include <FluentQtWidgets/Layout/FlowLayout.h>
#include <FluentQtWidgets/Settings/SettingCard.h>
#include <FluentQtWidgets/Views/ItemViews.h>
#include <FluentQtWidgets/Widgets/AcrylicLabel.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/FlipView.h>
#include <FluentQtWidgets/Widgets/InfoBadge.h>
#include <FluentQtWidgets/Widgets/InfoBar.h>
#include <FluentQtWidgets/Widgets/Label.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>
#include <FluentQtWidgets/Widgets/Menu.h>
#include <FluentQtWidgets/Widgets/PipsPager.h>
#include <FluentQtWidgets/Widgets/ProgressBar.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>
#include <FluentQtWidgets/Widgets/Slider.h>
#include <FluentQtWidgets/Widgets/SpinBox.h>
#include <FluentQtWidgets/Widgets/ToolTip.h>

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
        QCOMPARE(QCoreApplication::translate("FluentQt::CalendarView", "Mo"), QStringLiteral("一"));
        QCOMPARE(QCoreApplication::translate("FluentQt::FastCalendarView", "Su"), QStringLiteral("日"));
        QCOMPARE(QCoreApplication::translate("SettingInterface", "Language"), QStringLiteral("语言"));
        QCOMPARE(QCoreApplication::translate("SettingInterface", "Help us improve FluentQtWidgets by providing feedback"),
                 QStringLiteral("通过提供反馈帮助我们改进 FluentQtWidgets"));
        QCOMPARE(QCoreApplication::translate("DialogInterface", "Show dialog"), QStringLiteral("显示对话框"));
        QCOMPARE(QCoreApplication::translate("DialogInterface", "Teaching tips with different tail positions"),
                 QStringLiteral("不同尾巴位置的气泡弹窗"));
        QCOMPARE(QCoreApplication::translate("DialogInterface", "Action"), QStringLiteral("操作"));
        QCOMPARE(QCoreApplication::translate("HomeInterface", "Basic input samples"), QStringLiteral("基本输入示例"));
        QCOMPARE(QCoreApplication::translate("HomeInterface", "Help us improve FluentQtWidgets by providing feedback."),
                 QStringLiteral("通过提供反馈帮助我们改进 FluentQtWidgets。"));
        QCOMPARE(QCoreApplication::translate("IconInterface", "Icon name"), QStringLiteral("图标名字"));
        QCOMPARE(QCoreApplication::translate("IconInterface", "Browse all icons in the FluentIcon enum"),
                 QStringLiteral("浏览 FluentIcon 枚举中的所有图标"));
        QCOMPARE(QCoreApplication::translate("IconCardView", "Fluent Icons Library"), QStringLiteral("流畅图标库"));
        QCOMPARE(QCoreApplication::translate("LineEdit", "Search icons"), QStringLiteral("搜索图标"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A simple CalendarPicker"),
                 QStringLiteral("日历选择器"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A fast CalendarPicker"),
                 QStringLiteral("快速日历选择器"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A CalendarPicker in another format"),
                 QStringLiteral("自定义格式的日历选择器"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A simple DatePicker"),
                 QStringLiteral("日期选择器"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A DatePicker in another format"),
                 QStringLiteral("另一种格式的日期选择器"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A simple TimePicker"),
                 QStringLiteral("时间选择器"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A TimePicker using a 24-hour clock"),
                 QStringLiteral("24 小时制的时间选择器"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A TimePicker with seconds column"),
                 QStringLiteral("显示秒的时间选择器"));
        QCOMPARE(QCoreApplication::translate("LayoutInterface", "Flow layout without animation"),
                 QStringLiteral("不带动画效果的流式布局"));
        QCOMPARE(QCoreApplication::translate("NavigationViewInterface", "%1 Interface").arg(QStringLiteral("歌曲")),
                 QStringLiteral("歌曲 界面"));
        QCOMPARE(QCoreApplication::translate("TabInterface", "Shoko is super kawaii x%1").arg(1),
                 QStringLiteral("硝子酱一级棒卡哇伊×1"));
        QCOMPARE(QCoreApplication::translate("TextInterface", "LineEdit, SpinBox, TimeEdit and TextEdit"),
                 QStringLiteral("LineEdit、SpinBox、TimeEdit 和 TextEdit"));
        QCOMPARE(QCoreApplication::translate("ScrollInterface", "Pips pager"), QStringLiteral("圆点分页组件"));
        QCOMPARE(QCoreApplication::translate("ExampleCard", "Source code"), QStringLiteral("源代码"));
        QCOMPARE(QCoreApplication::translate("TreeFrame", "JoJo 1 - Phantom Blood"),
                 QStringLiteral("JoJo 1 - 幻影之血"));
        QCOMPARE(QCoreApplication::translate("SettingInterface", "Check latest version from GitHub releases"),
                 QStringLiteral("从 GitHub Releases 检查最新版本"));
        QCOMPARE(QCoreApplication::translate("Translator", "A simple ListView"), QStringLiteral("简单的列表组件"));
        QCOMPARE(QCoreApplication::translate("GalleryProfile", "Logout"), QStringLiteral("注销"));
        QCOMPARE(QCoreApplication::translate("GalleryTray", "Show Gallery"), QStringLiteral("显示 Gallery"));
        QCOMPARE(QCoreApplication::translate("GalleryTray", "Quit"), QStringLiteral("退出"));

        QVERIFY(GalleryTranslation::installTranslators(app, QStringLiteral("zh_HK")));
        QCOMPARE(QCoreApplication::translate("FluentQt::CalendarPicker", "Pick a date"),
                 QStringLiteral("選擇日期"));
        QCOMPARE(QCoreApplication::translate("FluentQt::CalendarView", "Mo"), QStringLiteral("一"));
        QCOMPARE(QCoreApplication::translate("FluentQt::FastCalendarView", "Su"), QStringLiteral("日"));
        QCOMPARE(QCoreApplication::translate("SettingInterface", "Language"), QStringLiteral("語言"));
        QCOMPARE(QCoreApplication::translate("SettingInterface", "Help us improve FluentQtWidgets by providing feedback"),
                 QStringLiteral("通過提供反饋幫助我們改進 FluentQtWidgets"));
        QCOMPARE(QCoreApplication::translate("DialogInterface", "Show dialog"), QStringLiteral("顯示對話框"));
        QCOMPARE(QCoreApplication::translate("DialogInterface", "Teaching tips with different tail positions"),
                 QStringLiteral("不同尾巴位置的氣泡彈窗"));
        QCOMPARE(QCoreApplication::translate("DialogInterface", "Action"), QStringLiteral("操作"));
        QCOMPARE(QCoreApplication::translate("HomeInterface", "Basic input samples"), QStringLiteral("基本輸入示例"));
        QCOMPARE(QCoreApplication::translate("HomeInterface", "Help us improve FluentQtWidgets by providing feedback."),
                 QStringLiteral("通過提供反饋幫助我們改進 FluentQtWidgets。"));
        QCOMPARE(QCoreApplication::translate("IconInterface", "Icon name"), QStringLiteral("圖標名字"));
        QCOMPARE(QCoreApplication::translate("IconInterface", "Browse all icons in the FluentIcon enum"),
                 QStringLiteral("瀏覽 FluentIcon 枚舉中的所有圖標"));
        QCOMPARE(QCoreApplication::translate("IconCardView", "Fluent Icons Library"), QStringLiteral("流暢圖標庫"));
        QCOMPARE(QCoreApplication::translate("LineEdit", "Search icons"), QStringLiteral("蒐索圖標"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A simple CalendarPicker"),
                 QStringLiteral("日曆選擇器"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A fast CalendarPicker"),
                 QStringLiteral("快速日曆選擇器"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A CalendarPicker in another format"),
                 QStringLiteral("自定義格式的日曆選擇器"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A simple DatePicker"),
                 QStringLiteral("日期選擇器"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A DatePicker in another format"),
                 QStringLiteral("另一種格式的日期選擇器"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A simple TimePicker"),
                 QStringLiteral("時間選擇器"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A TimePicker using a 24-hour clock"),
                 QStringLiteral("24 小時製的時間選擇器"));
        QCOMPARE(QCoreApplication::translate("DateTimeInterface", "A TimePicker with seconds column"),
                 QStringLiteral("顯示秒的時間選擇器"));
        QCOMPARE(QCoreApplication::translate("LayoutInterface", "Flow layout without animation"),
                 QStringLiteral("不帶動畫效果的流式佈局"));
        QCOMPARE(QCoreApplication::translate("NavigationViewInterface", "%1 Interface").arg(QStringLiteral("歌曲")),
                 QStringLiteral("歌曲 介面"));
        QCOMPARE(QCoreApplication::translate("TabInterface", "Shoko is super kawaii x%1").arg(1),
                 QStringLiteral("硝子醬一級棒可愛×1"));
        QCOMPARE(QCoreApplication::translate("TextInterface", "LineEdit, SpinBox, TimeEdit and TextEdit"),
                 QStringLiteral("LineEdit、SpinBox、TimeEdit 和 TextEdit"));
        QCOMPARE(QCoreApplication::translate("ScrollInterface", "Pips pager"), QStringLiteral("圓點分頁組件"));
        QCOMPARE(QCoreApplication::translate("ExampleCard", "Source code"), QStringLiteral("源代碼"));
        QCOMPARE(QCoreApplication::translate("TreeFrame", "JoJo 1 - Phantom Blood"),
                 QStringLiteral("JoJo 1 - 幻影之血"));
        QCOMPARE(QCoreApplication::translate("SettingInterface", "Check latest version from GitHub releases"),
                 QStringLiteral("從 GitHub Releases 檢查最新版本"));
        QCOMPARE(QCoreApplication::translate("Translator", "A simple ListView"), QStringLiteral("簡單的列表組件"));
        QCOMPARE(QCoreApplication::translate("GalleryProfile", "Logout"), QStringLiteral("註銷"));
        QCOMPARE(QCoreApplication::translate("GalleryTray", "Show Gallery"), QStringLiteral("顯示 Gallery"));
        QCOMPARE(QCoreApplication::translate("GalleryTray", "Quit"), QStringLiteral("退出"));

        GalleryTranslation::installTranslators(app, QStringLiteral("en"));
        QCOMPARE(QCoreApplication::translate("FluentQt::CalendarPicker", "Pick a date"),
                 QStringLiteral("Pick a date"));
        QCOMPARE(QCoreApplication::translate("FluentQt::CalendarView", "Mo"), QStringLiteral("Mo"));
        QCOMPARE(QCoreApplication::translate("FluentQt::FastCalendarView", "Su"), QStringLiteral("Su"));
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
        QVERIFY(window.switchTo(QStringLiteral("settingInterface")));

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
        QCOMPARE(newWindow->currentRouteKey(), QStringLiteral("settingInterface"));

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

    void basicInputPageTitleUsesNavigationTranslator()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("zh_CN"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("basicInputInterface")));

        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);

        bool foundTranslatedTitle = false;
        const auto labels = page->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() == QStringLiteral("基本输入")) {
                foundTranslatedTitle = true;
                break;
            }
        }
        QVERIFY(foundTranslatedTitle);
    }

    void layoutPageMatchesPythonGalleryAndUsesTranslations()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("zh_CN"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("layoutInterface")));

        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);
        QCOMPARE(page->objectName(), QStringLiteral("layoutInterface"));

        const QString withoutAnimation =
            QCoreApplication::translate("LayoutInterface", "Flow layout without animation");
        const QString withAnimation = QCoreApplication::translate("LayoutInterface", "Flow layout with animation");
        const QString starPlatinum = QCoreApplication::translate("LayoutInterface", "Star Platinum");
        const QString d4c = QCoreApplication::translate("LayoutInterface", "Dirty Deeds Done Dirt Cheap");
        QVERIFY(withoutAnimation != QStringLiteral("Flow layout without animation"));
        QVERIFY(withAnimation != QStringLiteral("Flow layout with animation"));
        QVERIFY(starPlatinum != QStringLiteral("Star Platinum"));
        QVERIFY(d4c != QStringLiteral("Dirty Deeds Done Dirt Cheap"));

        bool foundSubtitle = false;
        bool foundWithoutAnimation = false;
        bool foundWithAnimation = false;
        const auto labels = page->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() == QStringLiteral("qfluentwidgets.components.layout")) {
                foundSubtitle = true;
            } else if (label->text() == withoutAnimation) {
                foundWithoutAnimation = true;
            } else if (label->text() == withAnimation) {
                foundWithAnimation = true;
            }
        }
        QVERIFY(foundSubtitle);
        QVERIFY(foundWithoutAnimation);
        QVERIFY(foundWithAnimation);

        bool foundStarPlatinum = false;
        bool foundD4C = false;
        const auto buttons = page->findChildren<FluentQt::PushButton *>();
        for (FluentQt::PushButton *button : buttons) {
            if (button->text() == starPlatinum) {
                foundStarPlatinum = true;
            } else if (button->text() == d4c) {
                foundD4C = true;
            }
        }
        QVERIFY(foundStarPlatinum);
        QVERIFY(foundD4C);
    }

    void textPageMatchesPythonGallery()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("textInterface")));

        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);
        QCOMPARE(page->objectName(), QStringLiteral("textInterface"));

        bool foundSubtitle = false;
        const auto labels = page->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() == QStringLiteral("qfluentwidgets.components.widgets")) {
                foundSubtitle = true;
                break;
            }
        }
        QVERIFY(foundSubtitle);

        FluentQt::LineEdit *plainLineEdit = nullptr;
        const auto lineEdits = page->findChildren<FluentQt::LineEdit *>();
        for (FluentQt::LineEdit *edit : lineEdits) {
            if (!qobject_cast<FluentQt::SearchLineEdit *>(edit)
                && !qobject_cast<FluentQt::PasswordLineEdit *>(edit)
                && edit->text() == QStringLiteral("ko no dio da！")) {
                plainLineEdit = edit;
                break;
            }
        }
        QVERIFY(plainLineEdit != nullptr);
        QVERIFY(plainLineEdit->isClearButtonEnabled());
        QVERIFY(plainLineEdit->maximumWidth() > 260);

        auto *searchEdit = page->findChild<FluentQt::SearchLineEdit *>();
        QVERIFY(searchEdit != nullptr);
        QCOMPARE(searchEdit->placeholderText(), QStringLiteral("Type a stand name"));
        QVERIFY(searchEdit->isClearButtonEnabled());
        QCOMPARE(searchEdit->minimumWidth(), 230);
        QCOMPARE(searchEdit->maximumWidth(), 230);
        QVERIFY(searchEdit->completer() != nullptr);
        QCOMPARE(searchEdit->completer()->caseSensitivity(), Qt::CaseInsensitive);
        QCOMPARE(searchEdit->completer()->maxVisibleItems(), 10);
        auto *completionModel = searchEdit->completer()->model();
        QVERIFY(completionModel != nullptr);
        QCOMPARE(completionModel->rowCount(), 36);
        QCOMPARE(completionModel->index(2, 0).data().toString(), QStringLiteral("Made in Haven"));
        QCOMPARE(completionModel->index(5, 0).data().toString(), QStringLiteral("Crazy diamond"));
        QCOMPARE(completionModel->index(26, 0).data().toString(), QStringLiteral("D4C • Love Train"));

        auto *passwordEdit = page->findChild<FluentQt::PasswordLineEdit *>();
        QVERIFY(passwordEdit != nullptr);
        QCOMPARE(passwordEdit->placeholderText(), QStringLiteral("Enter your password"));
        QCOMPARE(passwordEdit->minimumWidth(), 230);
        QCOMPARE(passwordEdit->maximumWidth(), 230);

        auto verifyInlineSpinBox = [](auto *spin, const QString &role) {
            QVERIFY(spin != nullptr);
            QCOMPARE(spin->property("fqw").toString(), role);
            QCOMPARE(spin->buttonSymbols(), QAbstractSpinBox::NoButtons);
            QCOMPARE(spin->property("transparent").toBool(), true);
            QCOMPARE(spin->property("symbolVisible").toBool(), true);
            QCOMPARE(spin->height(), 33);
            QVERIFY(spin->upButton() != nullptr);
            QVERIFY(spin->downButton() != nullptr);
            QCOMPARE(spin->upButton()->property("fqw").toString(), QStringLiteral("SpinButton"));
            QCOMPARE(spin->downButton()->property("fqw").toString(), QStringLiteral("SpinButton"));
            QCOMPARE(spin->upButton()->size(), QSize(31, 23));
            QCOMPARE(spin->downButton()->size(), QSize(31, 23));
        };
        verifyInlineSpinBox(page->findChild<FluentQt::SpinBox *>(), QStringLiteral("SpinBox"));
        verifyInlineSpinBox(page->findChild<FluentQt::DoubleSpinBox *>(), QStringLiteral("DoubleSpinBox"));
        verifyInlineSpinBox(page->findChild<FluentQt::DateEdit *>(), QStringLiteral("DateEdit"));
        verifyInlineSpinBox(page->findChild<FluentQt::TimeEdit *>(), QStringLiteral("TimeEdit"));
        verifyInlineSpinBox(page->findChild<FluentQt::DateTimeEdit *>(), QStringLiteral("DateTimeEdit"));

        const auto textEdits = page->findChildren<FluentQt::TextEdit *>();
        QCOMPARE(textEdits.size(), 1);
        auto *textEdit = textEdits.constFirst();
        QCOMPARE(textEdit->objectName(), QStringLiteral("galleryTextEdit"));
        QTRY_COMPARE(textEdit->height(), 150);
        QVERIFY(textEdit->toPlainText().contains(QStringLiteral("Johnny Joestar")));
        QVERIFY(textEdit->toPlainText().contains(QStringLiteral("Gyro Zeppeli")));
    }

    void viewPageMatchesPythonGallery()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("viewInterface")));

        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);
        QCOMPARE(page->objectName(), QStringLiteral("viewInterface"));

        bool foundSubtitle = false;
        const auto labels = page->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() == QStringLiteral("qfluentwidgets.components.widgets")) {
                foundSubtitle = true;
                break;
            }
        }
        QVERIFY(foundSubtitle);

        QList<QFrame *> viewFrames;
        const auto frames = page->findChildren<QFrame *>(QStringLiteral("frame"));
        for (QFrame *frame : frames) {
            if (frame->minimumSize() == QSize(300, 380) && frame->maximumSize() == QSize(300, 380)) {
                viewFrames.append(frame);
            }
        }
        QCOMPARE(viewFrames.size(), 3);
        for (QFrame *frame : viewFrames) {
            QVERIFY(qobject_cast<QHBoxLayout *>(frame->layout()) != nullptr);
            QCOMPARE(frame->layout()->contentsMargins(), QMargins(0, 8, 0, 0));
        }

        auto *list = page->findChild<FluentQt::ListWidget *>();
        QVERIFY(list != nullptr);
        QCOMPARE(list->count(), 36);
        QCOMPARE(list->selectionMode(), QAbstractItemView::SingleSelection);
        QCOMPARE(list->item(0)->text(), QStringLiteral("Star Platinum"));
        QCOMPARE(list->item(27)->text(), QStringLiteral("Born This Way"));
        QVERIFY(list->scrollDelegate() != nullptr);

        auto *table = page->findChild<FluentQt::TableWidget *>();
        QVERIFY(table != nullptr);
        QCOMPARE(table->rowCount(), 60);
        QCOMPARE(table->columnCount(), 5);
        QCOMPARE(table->selectionMode(), QAbstractItemView::SingleSelection);
        QCOMPARE(table->selectionBehavior(), QAbstractItemView::SelectRows);
        QVERIFY(table->alternatingRowColors());
        QCOMPARE(table->horizontalHeader()->sectionResizeMode(0), QHeaderView::Interactive);
        QVERIFY(table->verticalHeader()->isHidden());
        QVERIFY(table->isBorderVisible());
        QCOMPARE(table->minimumSize(), QSize(625, 440));
        QCOMPARE(table->maximumSize(), QSize(625, 440));
        QCOMPARE(table->item(20, 4)->text(), QStringLiteral("aiko"));

        const auto trees = page->findChildren<FluentQt::TreeWidget *>();
        QCOMPARE(trees.size(), 2);
        FluentQt::TreeWidget *checkableTree = nullptr;
        int checkableItems = 0;
        for (FluentQt::TreeWidget *tree : trees) {
            QVERIFY(tree->isHeaderHidden());
            QCOMPARE(tree->iconSize(), QSize(16, 16));
            QCOMPARE(tree->selectionMode(), QAbstractItemView::SingleSelection);
            QVERIFY(!tree->isBorderVisible());
            QCOMPARE(tree->topLevelItemCount(), 2);
            QCOMPARE(tree->topLevelItem(0)->text(0), QStringLiteral("JoJo 1 - Phantom Blood"));

            int currentCheckableItems = 0;
            QTreeWidgetItemIterator it(tree);
            while (*it) {
                if ((*it)->data(0, Qt::CheckStateRole).isValid()) {
                    ++currentCheckableItems;
                }
                ++it;
            }
            if (currentCheckableItems > 0) {
                checkableTree = tree;
                checkableItems = currentCheckableItems;
            }
        }
        QVERIFY(checkableTree != nullptr);
        QCOMPARE(checkableItems, 11);

        auto *flip = page->findChild<FluentQt::HorizontalFlipView *>();
        QVERIFY(flip != nullptr);
        QCOMPARE(flip->count(), 4);
        QCOMPARE(flip->itemSize(), QSize(480, 270));
        QCOMPARE(flip->currentIndex(), 0);
    }

    void materialPageMatchesPythonGalleryAndUsesTranslations()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("zh_CN"));

        auto *config = FluentQt::FluentConfig::instance();
        const int previousAcrylicBlurRadius = config->acrylicBlurRadius();
        config->setAcrylicBlurRadius(23);

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("materialInterface")));

        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);
        QCOMPARE(page->objectName(), QStringLiteral("materialInterface"));

        const QString acrylicLabelTitle = QCoreApplication::translate("MaterialInterface", "Acrylic label");
        QVERIFY(acrylicLabelTitle != QStringLiteral("Acrylic label"));

        bool foundSubtitle = false;
        bool foundTitle = false;
        const auto labels = page->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() == QStringLiteral("qfluentwidgets.components.widgets")) {
                foundSubtitle = true;
            } else if (label->text() == acrylicLabelTitle) {
                foundTitle = true;
            }
        }
        QVERIFY(foundSubtitle);
        QVERIFY(foundTitle);

        const auto acrylicLabels = page->findChildren<FluentQt::AcrylicLabel *>();
        QCOMPARE(acrylicLabels.size(), 1);
        auto *acrylicLabel = acrylicLabels.constFirst();
        QCOMPARE(acrylicLabel->minimumSize(), QSize(197, 145));
        QCOMPARE(acrylicLabel->maximumSize(), QSize(787, 579));
        QCOMPARE(acrylicLabel->blurRadius(), 23);
        QVERIFY(!acrylicLabel->isNull());

        config->setAcrylicBlurRadius(31);
        QCOMPARE(acrylicLabel->blurRadius(), 31);
        QVERIFY(!acrylicLabel->isNull());

        config->setAcrylicBlurRadius(previousAcrylicBlurRadius);
    }

    void scrollPageMatchesPythonGallery()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("scrollInterface")));

        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);
        QCOMPARE(page->objectName(), QStringLiteral("scrollInterface"));

        bool foundSubtitle = false;
        const auto labels = page->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() == QStringLiteral("qfluentwidgets.components.widgets")) {
                foundSubtitle = true;
                break;
            }
        }
        QVERIFY(foundSubtitle);

        const auto pagers = page->findChildren<FluentQt::HorizontalPipsPager *>();
        QCOMPARE(pagers.size(), 1);
        auto *pager = pagers.constFirst();
        QCOMPARE(pager->pageNumber(), 15);
        QCOMPARE(pager->previousButtonDisplayMode(), FluentQt::PipsScrollButtonDisplayMode::Always);
        QCOMPARE(pager->nextButtonDisplayMode(), FluentQt::PipsScrollButtonDisplayMode::Always);

        QWidget *pipsCard = pager;
        while (pipsCard && !pipsCard->findChild<QFrame *>(QStringLiteral("card"), Qt::FindDirectChildrenOnly)) {
            pipsCard = pipsCard->parentWidget();
        }
        QVERIFY(pipsCard != nullptr);
        auto *innerCard = pipsCard->findChild<QFrame *>(QStringLiteral("card"), Qt::FindDirectChildrenOnly);
        QVERIFY(innerCard != nullptr);
        auto *cardLayout = qobject_cast<QVBoxLayout *>(innerCard->layout());
        QVERIFY(cardLayout != nullptr);
        QLayoutItem *topItem = cardLayout->itemAt(0);
        QVERIFY(topItem != nullptr);
        QVERIFY(topItem->layout() != nullptr);
        QCOMPARE(topItem->layout()->contentsMargins(), QMargins(12, 20, 12, 20));

        const auto cards = page->findChildren<QFrame *>(QStringLiteral("card"));
        QVERIFY(cards.size() >= 3);
        int timedToolTipCards = 0;
        for (QFrame *card : cards) {
            if (card->toolTipDuration() == 2000) {
                ++timedToolTipCards;
            }
        }
        QCOMPARE(timedToolTipCards, 3);
    }

    void statusInfoPageMatchesPythonGallery()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("statusInfoInterface")));

        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);
        QCOMPARE(page->objectName(), QStringLiteral("statusInfoInterface"));

        bool foundSubtitle = false;
        const auto labels = page->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() == QStringLiteral("qfluentwidgets.components.widgets")) {
                foundSubtitle = true;
                break;
            }
        }
        QVERIFY(foundSubtitle);

        FluentQt::PushButton *toolTipButton = nullptr;
        const auto buttons = page->findChildren<FluentQt::PushButton *>();
        for (FluentQt::PushButton *button : buttons) {
            if (button->text() == QStringLiteral("Button with a simple ToolTip")) {
                toolTipButton = button;
                break;
            }
        }
        QVERIFY(toolTipButton != nullptr);
        QCOMPARE(toolTipButton->toolTip(), QStringLiteral("Simple ToolTip"));
        QVERIFY(toolTipButton->findChild<FluentQt::ToolTipFilter *>() != nullptr);
        QCOMPARE(page->findChildren<FluentQt::PixmapLabel *>().size(), 1);

        QStringList badgeTexts;
        QWidget *badgeRow = nullptr;
        const auto badges = page->findChildren<FluentQt::InfoBadge *>();
        for (FluentQt::InfoBadge *badge : badges) {
            if (badge->property("fqw").toString() == QStringLiteral("InfoBadge")) {
                badgeTexts.append(badge->text());
                if (badge->text() == QStringLiteral("1w+")) {
                    badgeRow = badge->parentWidget();
                }
            }
        }
        for (const QString &text : {QStringLiteral("1"), QStringLiteral("10"), QStringLiteral("100"),
                                   QStringLiteral("1000"), QStringLiteral("10000"), QStringLiteral("1w+")}) {
            QVERIFY2(badgeTexts.contains(text), qPrintable(QStringLiteral("Missing badge %1").arg(text)));
        }
        QVERIFY(!badgeTexts.contains(QStringLiteral("99+")));
        QVERIFY(badgeRow != nullptr);
        auto *badgeLayout = qobject_cast<QHBoxLayout *>(badgeRow->layout());
        QVERIFY(badgeLayout != nullptr);
        QCOMPARE(badgeLayout->contentsMargins(), QMargins(0, 10, 0, 10));
        QCOMPARE(badgeLayout->spacing(), 20);

        FluentQt::PushButton *topRightButton = nullptr;
        for (FluentQt::PushButton *button : buttons) {
            if (button->text() == QStringLiteral("Top right")) {
                topRightButton = button;
                break;
            }
        }
        QVERIFY(topRightButton != nullptr);
        auto *positionLayout = qobject_cast<QHBoxLayout *>(topRightButton->parentWidget()->layout());
        QVERIFY(positionLayout != nullptr);
        QCOMPARE(positionLayout->spacing(), 15);

        const auto infoBars = page->findChildren<FluentQt::InfoBar *>();
        QCOMPARE(infoBars.size(), 3);
        bool foundVerticalWarning = false;
        bool foundCustomInfoBar = false;
        for (FluentQt::InfoBar *bar : infoBars) {
            if (bar->title() == QStringLiteral("Warning") && bar->orient() == Qt::Vertical) {
                foundVerticalWarning = true;
            } else if (bar->title() == QStringLiteral("GitHub")) {
                foundCustomInfoBar = true;
                QVERIFY(!bar->iconWidget()->customIcon().isNull());
            }
        }
        QVERIFY(foundVerticalWarning);
        QVERIFY(foundCustomInfoBar);

        const auto spinBoxes = page->findChildren<FluentQt::SpinBox *>();
        QCOMPARE(spinBoxes.size(), 2);
        for (FluentQt::SpinBox *spinBox : spinBoxes) {
            QCOMPARE(spinBox->value(), 0);
        }

        const auto indeterminateBars = page->findChildren<FluentQt::IndeterminateProgressBar *>();
        QCOMPARE(indeterminateBars.size(), 1);
        QWidget *progressCard = indeterminateBars.constFirst();
        while (progressCard && !progressCard->findChild<QFrame *>(QStringLiteral("card"), Qt::FindDirectChildrenOnly)) {
            progressCard = progressCard->parentWidget();
        }
        QVERIFY(progressCard != nullptr);
        auto *innerCard = progressCard->findChild<QFrame *>(QStringLiteral("card"), Qt::FindDirectChildrenOnly);
        QVERIFY(innerCard != nullptr);
        auto *cardLayout = qobject_cast<QVBoxLayout *>(innerCard->layout());
        QVERIFY(cardLayout != nullptr);
        QLayoutItem *topItem = cardLayout->itemAt(0);
        QVERIFY(topItem != nullptr);
        QVERIFY(topItem->layout() != nullptr);
        QCOMPARE(topItem->layout()->contentsMargins(), QMargins(12, 24, 12, 24));
    }

    void basicInputTrailingTransparentTogglesStayCheckable()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("basicInputInterface")));
        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);

        auto *transparentTogglePush = page->findChild<FluentQt::TransparentTogglePushButton *>();
        QVERIFY(transparentTogglePush != nullptr);
        QCOMPARE(transparentTogglePush->text(), QStringLiteral("Start practicing"));
        QVERIFY(transparentTogglePush->isCheckable());
        QVERIFY(!transparentTogglePush->isChecked());
        transparentTogglePush->click();
        QVERIFY(transparentTogglePush->isChecked());

        auto *transparentToggleTool = page->findChild<FluentQt::TransparentToggleToolButton *>();
        QVERIFY(transparentToggleTool != nullptr);
        QVERIFY(transparentToggleTool->isCheckable());
        QVERIFY(!transparentToggleTool->isChecked());
        transparentToggleTool->click();
        QVERIFY(transparentToggleTool->isChecked());
    }

    void dateTimePageMatchesPythonGalleryDefaults()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(window.switchTo(QStringLiteral("dateTimeInterface")));
        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);

        bool foundPythonSubtitle = false;
        const auto labels = page->findChildren<QLabel *>();
        for (QLabel *label : labels) {
            if (label->text() == QStringLiteral("qfluentwidgets.components.time_picker")) {
                foundPythonSubtitle = true;
                break;
            }
        }
        QVERIFY(foundPythonSubtitle);

        const auto calendarPickers = page->findChildren<FluentQt::CalendarPicker *>();
        QCOMPARE(calendarPickers.size(), 3);
        for (FluentQt::CalendarPicker *picker : calendarPickers) {
            QVERIFY(!picker->date().isValid());
            QCOMPARE(picker->text(), QStringLiteral("Pick a date"));
        }
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
        QVERIFY(window.switchTo(QStringLiteral("settingInterface")));

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

    void settingsPageMatchesPythonGallery()
    {
        GalleryTranslation::installTranslators(qApp, QStringLiteral("en"));

        auto *config = FluentQt::FluentConfig::instance();
        auto *manager = FluentQt::ThemeManager::instance();
        const QColor previousThemeColor = config->themeColor();
        const QColor previousAccentColor = manager->accentColor();
        const QColor defaultThemeColor(QStringLiteral("#009faa"));
        const QColor currentThemeColor(QStringLiteral("#112233"));
        config->setThemeColor(currentThemeColor);

        GalleryWindow window;
        window.resize(1040, 760);
        window.show();
        QVERIFY(QTest::qWaitForWindowExposed(&window));
        QVERIFY(!window.switchTo(QStringLiteral("settings")));
        QVERIFY(window.switchTo(QStringLiteral("settingInterface")));

        QWidget *page = window.currentInterface();
        QVERIFY(page != nullptr);
        QCOMPARE(page->objectName(), QStringLiteral("settingInterface"));

        auto *scrollArea = qobject_cast<FluentQt::ScrollArea *>(page);
        QVERIFY(scrollArea != nullptr);
        QCOMPARE(scrollArea->horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QCOMPARE(scrollArea->viewportMargins(), QMargins(0, 80, 0, 20));
        QVERIFY(scrollArea->widgetResizable());

        QWidget *scrollWidget = scrollArea->widget();
        QVERIFY(scrollWidget != nullptr);
        QCOMPARE(scrollWidget->objectName(), QStringLiteral("scrollWidget"));

        auto *layout = qobject_cast<FluentQt::ExpandLayout *>(scrollWidget->layout());
        QVERIFY(layout != nullptr);
        QCOMPARE(layout->spacing(), 28);
        QCOMPARE(layout->contentsMargins(), QMargins(36, 10, 36, 0));
        QCOMPARE(layout->count(), 5);

        auto *settingLabel = page->findChild<QLabel *>(QStringLiteral("settingLabel"));
        QVERIFY(settingLabel != nullptr);
        QCOMPARE(settingLabel->text(), QStringLiteral("Settings"));
        QCOMPARE(settingLabel->pos(), QPoint(36, 30));
        QCOMPARE(settingLabel->parentWidget(), page);

        QList<FluentQt::SettingCardGroup *> groups;
        for (int i = 0; i < layout->count(); ++i) {
            auto *group = qobject_cast<FluentQt::SettingCardGroup *>(layout->itemAt(i)->widget());
            QVERIFY(group != nullptr);
            groups.append(group);
        }

        QCOMPARE(groups.at(0)->title(), QStringLiteral("Music on this PC"));
        QCOMPARE(groups.at(1)->title(), QStringLiteral("Personalization"));
        QCOMPARE(groups.at(2)->title(), QStringLiteral("Material"));
        QCOMPARE(groups.at(3)->title(), QStringLiteral("Software update"));
        QCOMPARE(groups.at(4)->title(), QStringLiteral("About"));

        QCOMPARE(groups.at(0)->cards().size(), 2);
        QCOMPARE(groups.at(1)->cards().size(), 5);
        QCOMPARE(groups.at(2)->cards().size(), 1);
        QCOMPARE(groups.at(3)->cards().size(), 1);
        QCOMPARE(groups.at(4)->cards().size(), 3);
        for (FluentQt::SettingCardGroup *group : std::as_const(groups)) {
            QVERIFY(group->cardLayout() != nullptr);
            QCOMPARE(group->height(), group->cardLayout()->heightForWidth(group->width()) + 46);
        }
        for (int i = 1; i < groups.size(); ++i) {
            QVERIFY2(groups.at(i - 1)->geometry().bottom() < groups.at(i)->geometry().top(),
                     qPrintable(QStringLiteral("%1 overlaps %2")
                                    .arg(groups.at(i - 1)->title(), groups.at(i)->title())));
        }

        auto *updateCard = qobject_cast<FluentQt::SwitchSettingCard *>(groups.at(3)->cards().constFirst());
        QVERIFY(updateCard != nullptr);
        QCOMPARE(updateCard->title(), QStringLiteral("Check for updates when the application starts"));

        auto *themeColorCard = qobject_cast<FluentQt::CustomColorSettingCard *>(groups.at(1)->cards().at(2));
        QVERIFY(themeColorCard != nullptr);
        QCOMPARE(themeColorCard->defaultColor(), defaultThemeColor);
        QCOMPARE(themeColorCard->customColor(), currentThemeColor);
        QVERIFY(themeColorCard->isCustomColorEnabled());
        QCOMPARE(themeColorCard->color(), currentThemeColor);
        QVERIFY(themeColorCard->choiceLabel() != nullptr);
        QCOMPARE(themeColorCard->choiceLabel()->text(), QStringLiteral("Custom color"));

        auto *blurRadiusCard = qobject_cast<FluentQt::RangeSettingCard *>(groups.at(2)->cards().constFirst());
        QVERIFY(blurRadiusCard != nullptr);
        QVERIFY(blurRadiusCard->slider() != nullptr);
        QCOMPARE(blurRadiusCard->slider()->minimumWidth(), 268);
        QCOMPARE(blurRadiusCard->actionLayout()->spacing(), 6);

        auto *aboutCard = qobject_cast<FluentQt::PrimaryPushSettingCard *>(groups.at(4)->cards().at(2));
        QVERIFY(aboutCard != nullptr);
        QCOMPARE(aboutCard->title(), QStringLiteral("About"));
        QVERIFY(aboutCard->button() != nullptr);
        QCOMPARE(aboutCard->button()->text(), QStringLiteral("Check update"));
        QVERIFY(aboutCard->content().contains(QStringLiteral("Copyright 2026, FluentQtWidgets. Version ")));

        config->setThemeColor(previousThemeColor);
        manager->setAccentColor(previousAccentColor);
        config->save();
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
        QVERIFY(window.switchTo(QStringLiteral("settingInterface")));

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
        QVERIFY(window.switchTo(QStringLiteral("settingInterface")));

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
        QVERIFY(window.switchTo(QStringLiteral("settingInterface")));

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
