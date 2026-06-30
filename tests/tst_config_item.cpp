#include <FluentQtWidgets/ConfigItem.h>
#include <FluentQtWidgets/Config.h>
#include <FluentQtWidgets/Settings/SettingCard.h>
#include <FluentQtWidgets/Window/FluentWindow.h>

#include <QtCore/QFile>
#include <QtCore/QTemporaryDir>
#include <QtTest/QSignalSpy>
#include <QtTest/QTest>

using namespace FluentQt;

class ConfigItemTest : public QObject
{
    Q_OBJECT

  private slots:
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

    void fluentConfigPersistsDpiScale()
    {
        auto *config = FluentConfig::instance();
        const QString previousFileName = config->fileName();
        const QString previousDpiScale = config->dpiScale();

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

        config->setFileName(previousFileName);
        config->setDpiScale(previousDpiScale);
    }

    void fluentConfigMatchesPythonDefaults()
    {
        auto *config = FluentConfig::instance();
        const QString previousFileName = config->fileName();
        const Theme previousThemeMode = config->themeMode();
        const QColor previousThemeColor = config->themeColor();
        const QStringList previousFontFamilies = config->fontFamilies();
        const QString previousLocaleName = config->localeName();
        const QString previousDpiScale = config->dpiScale();
        const QString previousDownloadFolder = config->downloadFolder();
        const bool previousMicaEnabled = config->isMicaEnabled();
        const int previousAcrylicBlurRadius = config->acrylicBlurRadius();

        config->reset();
        QCOMPARE(config->themeColor(), QColor(QStringLiteral("#009faa")));
        QCOMPARE(config->isMicaEnabled(), isMicaEffectAvailable());
        QCOMPARE(config->acrylicBlurRadius(), 15);

        config->setFileName(previousFileName);
        config->setThemeMode(previousThemeMode);
        config->setThemeColor(previousThemeColor);
        config->setFontFamilies(previousFontFamilies);
        config->setLocaleName(previousLocaleName);
        config->setDpiScale(previousDpiScale);
        config->setDownloadFolder(previousDownloadFolder);
        config->setMicaEnabled(previousMicaEnabled);
        config->setAcrylicBlurRadius(previousAcrylicBlurRadius);
    }

    void fluentConfigReadsPythonGroups()
    {
        auto *config = FluentConfig::instance();
        const QString previousFileName = config->fileName();
        const Theme previousThemeMode = config->themeMode();
        const QColor previousThemeColor = config->themeColor();
        const QStringList previousFontFamilies = config->fontFamilies();
        const QString previousLocaleName = config->localeName();
        const QString previousDpiScale = config->dpiScale();
        const QString previousDownloadFolder = config->downloadFolder();
        const bool previousMicaEnabled = config->isMicaEnabled();
        const int previousAcrylicBlurRadius = config->acrylicBlurRadius();

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

        config->setFileName(previousFileName);
        config->setThemeMode(previousThemeMode);
        config->setThemeColor(previousThemeColor);
        config->setFontFamilies(previousFontFamilies);
        config->setLocaleName(previousLocaleName);
        config->setDpiScale(previousDpiScale);
        config->setDownloadFolder(previousDownloadFolder);
        config->setMicaEnabled(previousMicaEnabled);
        config->setAcrylicBlurRadius(previousAcrylicBlurRadius);
    }

    void fluentConfigPersistsDownloadFolder()
    {
        auto *config = FluentConfig::instance();
        const QString previousFileName = config->fileName();
        const QString previousDownloadFolder = config->downloadFolder();

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

        config->setFileName(previousFileName);
        config->setDownloadFolder(previousDownloadFolder);
    }
};

QTEST_MAIN(ConfigItemTest)
#include "tst_config_item.moc"
