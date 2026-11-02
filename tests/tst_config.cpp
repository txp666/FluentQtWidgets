// Config items, their registry and the Fluent configuration file.

#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>
#include <QtTest/QtTest>

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

        config->setDownloadFolder(QString());
        QCOMPARE(config->downloadFolder(), QStringLiteral("app/download"));
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(ConfigTest)
#include "tst_config.moc"
