// Config items, their registry and the Fluent configuration file.

#include <FluentQtWidgets/Config.h>
#include <FluentQtWidgets/ConfigItem.h>
#include <FluentQtWidgets/Dialogs/Dialog.h>
#include <FluentQtWidgets/FluentQtWidgets.h>
#include <FluentQtWidgets/Layout/FlowLayout.h>
#include <FluentQtWidgets/Layout/VBoxLayout.h>
#include <FluentQtWidgets/Navigation/NavigationHistory.h>
#include <FluentQtWidgets/Navigation/NavigationPanel.h>
#include <FluentQtWidgets/Navigation/Pivot.h>
#include <FluentQtWidgets/Settings/SettingCard.h>
#include <FluentQtWidgets/Views/ItemViews.h>
#include <FluentQtWidgets/Widgets/ComboBox.h>
#include <FluentQtWidgets/Widgets/CommandBar.h>
#include <FluentQtWidgets/Widgets/Flyout.h>
#include <FluentQtWidgets/Widgets/InfoBadge.h>
#include <FluentQtWidgets/Widgets/InfoBadgeManager.h>
#include <FluentQtWidgets/Widgets/InfoBar.h>
#include <FluentQtWidgets/Widgets/InfoBarManager.h>
#include <FluentQtWidgets/Widgets/Label.h>
#include <FluentQtWidgets/Widgets/LineEdit.h>
#include <FluentQtWidgets/Widgets/Menu.h>
#include <FluentQtWidgets/Widgets/PipsPager.h>
#include <FluentQtWidgets/Widgets/ProgressBar.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>
#include <FluentQtWidgets/Widgets/Slider.h>
#include <FluentQtWidgets/Widgets/StackedWidget.h>
#include <FluentQtWidgets/Widgets/StateToolTip.h>
#include <FluentQtWidgets/Widgets/SwitchButton.h>
#include <FluentQtWidgets/Widgets/TabView.h>
#include <FluentQtWidgets/Widgets/TeachingTip.h>
#include <FluentQtWidgets/Widgets/ToolTip.h>
#include <FluentQtWidgets/Window/FluentTitleBar.h>
#include <FluentQtWidgets/Window/FluentWindow.h>
#include <FluentQtWidgets/Window/SplashScreen.h>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QJsonObject>
#include <QtCore/QPointer>
#include <QtCore/QScopedPointer>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QStringListModel>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QPixmap>
#include <QtGui/QStandardItemModel>
#include <QtGui/QTextDocument>
#include <QtGui/QWheelEvent>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>
#include <QtTest/QtTest>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QAbstractScrollArea>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QCompleter>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QStyle>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

class ConfigTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void optionsItemCorrectsInvalidValue()
    {
        OptionsConfigItem item(QStringLiteral("Test"), QStringLiteral("Mode"), QStringLiteral("light"),
                               {QStringLiteral("light"), QStringLiteral("dark")});
        item.setValue(QStringLiteral("invalid"));
        QCOMPARE(item.value().toString(), QStringLiteral("light"));
    }

    void rangeItemClampsValue()
    {
        RangeConfigItem item(QStringLiteral("Test"), QStringLiteral("Scale"), 100, 80, 140);
        item.setValue(200);
        QCOMPARE(item.value().toInt(), 140);
    }

    void configItemChangeIsReportedOnce()
    {
        ConfigItem item(QStringLiteral("Regressions"), QStringLiteral("Once"), 1);
        QSignalSpy spy(ConfigItemRegistry::instance(), &ConfigItemRegistry::itemValueChanged);
        item.setValue(2);
        QCOMPARE(spy.count(), 1);
    }

    void deletedConfigItemLeavesRegistry()
    {
        auto *item = new ConfigItem(QStringLiteral("Lifetime"), QStringLiteral("Dangling"), 1);
        const QString key = item->key();
        QCOMPARE(ConfigItemRegistry::instance()->item(key), item);

        delete item;
        QVERIFY(!ConfigItemRegistry::instance()->item(key));
        QVERIFY(!ConfigItemRegistry::instance()->items().contains(item));
        QVERIFY(!ConfigItemRegistry::instance()->toJson().contains(QStringLiteral("Lifetime")));
    }

    void deletingReplacedConfigItemKeepsNewOne()
    {
        auto *first = new ConfigItem(QStringLiteral("Lifetime"), QStringLiteral("Shared"), 1);
        ConfigItem second(QStringLiteral("Lifetime"), QStringLiteral("Shared"), 2);
        delete first;
        QCOMPARE(ConfigItemRegistry::instance()->item(second.key()), &second);
    }

    void bindSwitchSettingCardUpdatesBothWays()
    {
        ConfigItem item(QStringLiteral("Test"), QStringLiteral("Enabled"), false);
        SwitchSettingCard card(FluentIcon::Setting, QStringLiteral("Switch"), QString());
        bindSwitchSettingCard(&card, &item);

        card.setChecked(true);
        QCOMPARE(item.value().toBool(), true);

        item.setValue(false);
        QCOMPARE(card.isChecked(), false);
    }

    void configPersistsCoreValues()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        const QString fileName = directory.filePath(QStringLiteral("config.json"));
        auto *config = FluentQt::FluentConfig::instance();
        config->reset();
        config->setThemeMode(FluentQt::Theme::Dark);
        config->setThemeColor(QColor(QStringLiteral("#112233")));
        config->setFontFamilies({QStringLiteral("Inter"), QStringLiteral("Segoe UI")});
        config->setLocaleName(QStringLiteral("zh_HK"));

        QVERIFY(config->save(fileName));

        config->reset();
        QVERIFY(config->load(fileName));
        QCOMPARE(config->themeMode(), FluentQt::Theme::Dark);
        QCOMPARE(config->themeColor(), QColor(QStringLiteral("#112233")));
        QCOMPARE(config->fontFamilies(), QStringList({QStringLiteral("Inter"), QStringLiteral("Segoe UI")}));
        QCOMPARE(config->localeName(), QStringLiteral("zh_HK"));
    }

    void fluentConfigPersistsDpiScale()
    {
        auto *config = FluentConfig::instance();
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        config->setFileName(dir.filePath(QStringLiteral("config.json")));

        config->setDpiScale(QStringLiteral("125%"));
        QCOMPARE(config->dpiScale(), QStringLiteral("1.25"));
        QVERIFY(config->save());

        config->setDpiScale(QStringLiteral("Auto"));
        QCOMPARE(config->dpiScale(), QStringLiteral("Auto"));
        QVERIFY(config->load());
        QCOMPARE(config->dpiScale(), QStringLiteral("1.25"));

        config->setDpiScale(QStringLiteral("175%"));
        QCOMPARE(config->dpiScale(), QStringLiteral("1.75"));
        config->setDpiScale(QStringLiteral("invalid"));
        QCOMPARE(config->dpiScale(), QStringLiteral("Auto"));
    }

    void fluentConfigMatchesPythonDefaults()
    {
        auto *config = FluentConfig::instance();
        config->reset();
        QCOMPARE(config->themeColor(), QColor(QStringLiteral("#009faa")));
        QCOMPARE(config->isMicaEnabled(), isMicaEffectAvailable());
        QCOMPARE(config->acrylicBlurRadius(), 15);
    }

    void fluentConfigReadsPythonGroups()
    {
        auto *config = FluentConfig::instance();
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        config->setFileName(dir.filePath(QStringLiteral("python-config.json")));

        QFile pythonConfig(config->fileName());
        QVERIFY(pythonConfig.open(QIODevice::WriteOnly | QIODevice::Text));
        pythonConfig.write(QByteArrayLiteral("{\n"
                                             "  \"QFluentWidgets\": {\"ThemeMode\": \"Dark\", "
                                             "\"ThemeColor\": \"#ff009faa\", "
                                             "\"FontFamilies\": [\"Inter\", \"PingFang SC\"]},\n"
                                             "  \"MainWindow\": {\"MicaEnabled\": true, \"DpiScale\": 1.5},\n"
                                             "  \"Material\": {\"AcrylicBlurRadius\": 28},\n"
                                             "  \"Folders\": {\"Download\": \"app/python-download\"}\n"
                                             "}"));
        pythonConfig.close();

        QVERIFY(config->load());
        QCOMPARE(config->themeMode(), Theme::Dark);
        QCOMPARE(config->themeColor(), QColor(QStringLiteral("#009faa")));
        QCOMPARE(config->fontFamilies(), QStringList({QStringLiteral("Inter"), QStringLiteral("PingFang SC")}));
        QCOMPARE(config->dpiScale(), QStringLiteral("1.5"));
        QCOMPARE(config->isMicaEnabled(), isMicaEffectAvailable());
        QCOMPARE(config->acrylicBlurRadius(), 28);
        QCOMPARE(config->downloadFolder(), QStringLiteral("app/python-download"));
    }

    void fluentConfigPersistsDownloadFolder()
    {
        auto *config = FluentConfig::instance();
        QTemporaryDir dir;
        QVERIFY(dir.isValid());
        config->setFileName(dir.filePath(QStringLiteral("config.json")));

        const QString selectedFolder = dir.filePath(QStringLiteral("downloads"));
        config->setDownloadFolder(selectedFolder);
        QCOMPARE(config->downloadFolder(), selectedFolder);
        QVERIFY(config->save());

        config->setDownloadFolder(QStringLiteral("app/download"));
        QVERIFY(config->load());
        QCOMPARE(config->downloadFolder(), selectedFolder);

        QFile pythonConfig(config->fileName());
        QVERIFY(pythonConfig.open(QIODevice::WriteOnly | QIODevice::Text));
        pythonConfig.write(R"({"Folders":{"Download":"python/download"}})");
        pythonConfig.close();
        QVERIFY(config->load());
        QCOMPARE(config->downloadFolder(), QStringLiteral("python/download"));

        config->setDownloadFolder(QString());
        QCOMPARE(config->downloadFolder(), QStringLiteral("app/download"));
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(ConfigTest)
#include "tst_config.moc"
