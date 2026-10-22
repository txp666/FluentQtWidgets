// The Gallery's languages and translators.

#include "components/GalleryComponents.h"
#include "GalleryTranslator.h"
#include "GalleryWindow.h"

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
#include <QtCore/QCoreApplication>
#include <QtCore/QLocale>
#include <QtCore/QPointer>
#include <QtTest/QtTest>
#include <QtWidgets/QAbstractSpinBox>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QFrame>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QTreeWidgetItemIterator>
#include <QtWidgets/QVBoxLayout>

#include "TestSupport.h"

#include <optional>

class GalleryTranslationTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        const auto widgets = QApplication::topLevelWidgets();
        for (QWidget *widget : widgets) {
            if (auto *window = qobject_cast<GalleryWindow *>(widget)) {
                window->close();
            }
        }
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
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
        QCOMPARE(QCoreApplication::translate("HomeInterface", "Chart samples"), QStringLiteral("图表示例"));
        QCOMPARE(QCoreApplication::translate("HomeInterface", "AudioWaveformWidget"), QStringLiteral("AudioWaveformWidget"));
        QCOMPARE(QCoreApplication::translate("HomeInterface", "RealtimePlotWidget"), QStringLiteral("RealtimePlotWidget"));
        QCOMPARE(QCoreApplication::translate("Translator", "Charts"), QStringLiteral("图表"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Audio waveform widget"), QStringLiteral("音频波形控件"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Realtime multi-series plot"),
                 QStringLiteral("实时多曲线图"));
        QCOMPARE(QCoreApplication::translate(
                     "ChartInterface",
                     "RealtimePlotWidget is optimized for streaming telemetry and dense multi-series data."),
                 QStringLiteral("RealtimePlotWidget 针对流式遥测和高密度多曲线数据进行了优化。"));
        QCOMPARE(QCoreApplication::translate(
                     "ChartInterface",
                     "Pixel peak downsampling and cached range blocks keep tens of thousands of points smooth."),
                 QStringLiteral("像素峰值降采样和范围块缓存让几万点数据依然流畅。"));
        QCOMPARE(QCoreApplication::translate(
                     "ChartInterface",
                     "Use capacity(0) for unlimited chunked history, with CSV and PNG export built in."),
                 QStringLiteral("可用 capacity(0) 启用无限分块历史，并内置 CSV 与 PNG 导出。"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Memory"), QStringLiteral("内存"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Native mixed bar and line chart"),
                 QStringLiteral("原生柱状折线混合图"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Mon"), QStringLiteral("周一"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Jan"), QStringLiteral("1月"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Search"), QStringLiteral("搜索"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Direct"), QStringLiteral("直接访问"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Web"), QStringLiteral("Web"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "View All"), QStringLiteral("查看全部"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "Maximum visible points"),
                 QStringLiteral("最大显示点数"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "Auto-scroll X"),
                 QStringLiteral("X 轴自动滚动"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "Fill under curves"),
                 QStringLiteral("曲线下方填充"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "Export CSV"),
                 QStringLiteral("导出 CSV"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "Export image"),
                 QStringLiteral("导出图片"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "Save CSV"),
                 QStringLiteral("保存 CSV"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "CSV files (*.csv)"),
                 QStringLiteral("CSV 文件 (*.csv)"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "Save image"),
                 QStringLiteral("保存图片"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "PNG images (*.png)"),
                 QStringLiteral("PNG 图片 (*.png)"));
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
        QCOMPARE(QCoreApplication::translate("HomeInterface", "Chart samples"), QStringLiteral("圖表示例"));
        QCOMPARE(QCoreApplication::translate("HomeInterface", "AudioWaveformWidget"), QStringLiteral("AudioWaveformWidget"));
        QCOMPARE(QCoreApplication::translate("HomeInterface", "RealtimePlotWidget"), QStringLiteral("RealtimePlotWidget"));
        QCOMPARE(QCoreApplication::translate("Translator", "Charts"), QStringLiteral("圖表"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Audio waveform widget"), QStringLiteral("音頻波形控件"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Realtime multi-series plot"),
                 QStringLiteral("實時多曲線圖"));
        QCOMPARE(QCoreApplication::translate(
                     "ChartInterface",
                     "RealtimePlotWidget is optimized for streaming telemetry and dense multi-series data."),
                 QStringLiteral("RealtimePlotWidget 針對串流遙測和高密度多曲線資料進行了最佳化。"));
        QCOMPARE(QCoreApplication::translate(
                     "ChartInterface",
                     "Pixel peak downsampling and cached range blocks keep tens of thousands of points smooth."),
                 QStringLiteral("像素峰值降採樣和範圍區塊快取讓幾萬點資料依然流暢。"));
        QCOMPARE(QCoreApplication::translate(
                     "ChartInterface",
                     "Use capacity(0) for unlimited chunked history, with CSV and PNG export built in."),
                 QStringLiteral("可用 capacity(0) 啟用無限分塊歷史，並內建 CSV 與 PNG 匯出。"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Memory"), QStringLiteral("記憶體"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Native mixed bar and line chart"),
                 QStringLiteral("原生柱狀折線混合圖"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Mon"), QStringLiteral("週一"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Jan"), QStringLiteral("1月"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Search"), QStringLiteral("搜尋"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Direct"), QStringLiteral("直接訪問"));
        QCOMPARE(QCoreApplication::translate("ChartInterface", "Web"), QStringLiteral("Web"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "View All"), QStringLiteral("查看全部"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "Maximum visible points"),
                 QStringLiteral("最大顯示點數"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "Auto-scroll X"),
                 QStringLiteral("X 軸自動滾動"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "Fill under curves"),
                 QStringLiteral("曲線下方填充"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "Export CSV"),
                 QStringLiteral("匯出 CSV"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "Export image"),
                 QStringLiteral("匯出圖片"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "Save CSV"),
                 QStringLiteral("儲存 CSV"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "CSV files (*.csv)"),
                 QStringLiteral("CSV 檔案 (*.csv)"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "Save image"),
                 QStringLiteral("儲存圖片"));
        QCOMPARE(QCoreApplication::translate("FluentQt::RealtimePlotWidget", "PNG images (*.png)"),
                 QStringLiteral("PNG 圖片 (*.png)"));
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

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(GalleryTranslationTest)
#include "tst_gallery_translation.moc"
