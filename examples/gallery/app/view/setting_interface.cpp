#include "../../GalleryWindow.h"

#include "../update/GalleryUpdateChecker.h"
#include "GalleryViewHelpers.h"

#include <QtCore/QSignalBlocker>
#include <QtWidgets/QFileDialog>

using namespace FluentQt;

QWidget *GalleryWindow::createSettingsPage()
{
    auto *page = new GalleryInterface(settingTx("Settings"), QStringLiteral("Personalization, layout and project options"),
                                      this);
    auto *layout = page->contentLayout();

    // ---- Music on this PC ----
    auto *musicGroup = new SettingCardGroup(settingTx("Music on this PC"));
    const QString musicPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    auto *musicFolderCard =
        new FolderListSettingCard(QStringList{musicPath}, FluentIcon::Folder, settingTx("Local music library"),
                                  QString(), musicPath.isEmpty() ? QDir::homePath() : musicPath);
    auto *downloadFolderCard = new PushSettingCard(settingTx("Choose folder"), FluentIcon::Download,
                                                   settingTx("Download directory"),
                                                   FluentConfig::instance()->downloadFolder());
    musicGroup->addSettingCards({musicFolderCard, downloadFolderCard});
    layout->addWidget(musicGroup);

    // ---- Personalization ----
    auto *personalization = new SettingCardGroup(settingTx("Personalization"));

    // Mica/Acrylic effect switch
    auto *micaCard = new SwitchSettingCard(FluentIcon::Transparent, settingTx("Mica effect"),
                                           settingTx("Apply semi transparent to windows and surfaces"));
    auto *micaWindow = qobject_cast<FluentWindow *>(window());
    micaCard->setEnabled(isMicaEffectAvailable());
    if (micaWindow && isMicaEffectAvailable()) {
        micaWindow->setMicaEffectEnabled(FluentConfig::instance()->isMicaEnabled());
        micaCard->setChecked(micaWindow->isMicaEffectEnabled());
        connect(micaCard, &SwitchSettingCard::checkedChanged, page, [micaWindow, micaCard](bool checked) {
            micaWindow->setMicaEffectEnabled(checked);
            const bool actual = micaWindow->isMicaEffectEnabled();
            FluentConfig::instance()->setMicaEnabled(actual);
            FluentConfig::instance()->save();
            if (micaCard->isChecked() != actual) {
                micaCard->setChecked(actual);
            }
        });
    } else {
        micaCard->setChecked(false);
        micaCard->setEnabled(false);
    }

    // Application theme
    auto *themeMode = new ComboBoxSettingCard(
        QStringList{settingTx("Light"), settingTx("Dark"), settingTx("Use system setting")}, FluentIcon::Brush,
        settingTx("Application theme"), settingTx("Change the appearance of your application"));

    // Theme color
    auto *themeColorCard =
        new CustomColorSettingCard(ThemeManager::instance()->accentColor(), QColor(QStringLiteral("#c239b3")),
                                   FluentIcon::Palette, settingTx("Theme color"),
                                   settingTx("Change the theme color of you application"));
    themeColorCard->setCustomColorEnabled(false);

    // Interface zoom
    const QStringList zoomTexts = {QStringLiteral("100%"), QStringLiteral("125%"), QStringLiteral("150%"),
                                   QStringLiteral("175%"), QStringLiteral("200%"),
                                   settingTx("Use system setting")};
    const QVariantList zoomValues = {QStringLiteral("1"), QStringLiteral("1.25"), QStringLiteral("1.5"),
                                     QStringLiteral("1.75"), QStringLiteral("2"), QStringLiteral("Auto")};
    auto *zoomCard = new OptionsSettingCard(zoomTexts, zoomValues, FluentIcon::Zoom,
                                            settingTx("Interface zoom"),
                                            settingTx("Change the size of widgets and fonts"));

    // Language
    auto *language = new ComboBoxSettingCard(
        QStringList{QStringLiteral("简体中文"), QStringLiteral("繁體中文"), QStringLiteral("English"),
                    settingTx("Use system setting")},
        FluentIcon::Language, settingTx("Language"), settingTx("Set your preferred language for UI"));

    // Config items
    const auto themeToIndex = [](Theme theme) {
        switch (theme) {
        case Theme::Light:
            return 0;
        case Theme::Dark:
            return 1;
        case Theme::Auto:
            return 2;
        }
        return 0;
    };
    const auto indexToTheme = [](int index) {
        if (index == 1) {
            return Theme::Dark;
        }
        if (index == 2) {
            return Theme::Auto;
        }
        return Theme::Light;
    };

    auto *themeModeItem = new OptionsConfigItem(
        QStringLiteral("Gallery"), QStringLiteral("ThemeMode"), static_cast<int>(FluentConfig::instance()->themeMode()),
        {static_cast<int>(Theme::Light), static_cast<int>(Theme::Dark), static_cast<int>(Theme::Auto)}, page);
    auto *accentColorItem =
        new ColorConfigItem(QStringLiteral("Gallery"), QStringLiteral("AccentColor"),
                            FluentConfig::instance()->themeColor(), page);
    auto *dpiScaleItem = new OptionsConfigItem(QStringLiteral("Gallery"), QStringLiteral("DpiScale"),
                                               FluentConfig::instance()->dpiScale(), zoomValues, page);
    dpiScaleItem->setRestartRequired(true);

    themeMode->setCurrentIndex(themeToIndex(FluentConfig::instance()->themeMode()));
    language->setCurrentIndex(GalleryTranslation::languageIndexForName(FluentConfig::instance()->localeName()));

    bindComboBoxSettingCard(
        themeMode, themeModeItem, [indexToTheme](int index) { return static_cast<int>(indexToTheme(index)); },
        [themeToIndex](const QVariant &value) { return themeToIndex(static_cast<Theme>(value.toInt())); });
    bindCustomColorSettingCard(themeColorCard, accentColorItem);
    bindOptionsSettingCard(zoomCard, dpiScaleItem);

    connect(themeModeItem, &ConfigItem::valueChanged, page, [](const QVariant &value) {
        const Theme theme = static_cast<Theme>(value.toInt());
        FluentConfig::instance()->setThemeMode(theme);
        ThemeManager::instance()->setTheme(theme);
        FluentConfig::instance()->save();
    });
    connect(ThemeManager::instance(), &ThemeManager::themeChanged, page, [themeModeItem](Theme theme) {
        themeModeItem->setValue(static_cast<int>(theme));
    });
    connect(accentColorItem, &ConfigItem::valueChanged, page, [](const QVariant &value) {
        const QColor color = value.value<QColor>();
        FluentConfig::instance()->setThemeColor(color);
        ThemeManager::instance()->setAccentColor(color);
        FluentConfig::instance()->save();
    });
    connect(dpiScaleItem, &ConfigItem::valueChanged, page, [page](const QVariant &value) {
        FluentConfig::instance()->setDpiScale(value.toString());
        FluentConfig::instance()->save();
        InfoBar::success(settingTx("Updated successfully"),
                         settingTx("Configuration takes effect after restart"),
                         Qt::Horizontal, true, 1500, InfoBarPosition::Top, page);
    });
    connect(downloadFolderCard, &PushSettingCard::clicked, page, [page, downloadFolderCard]() {
        const QString folder = QFileDialog::getExistingDirectory(page, settingTx("Choose folder"),
                                                                 QStringLiteral("./"));
        if (folder.isEmpty() || FluentConfig::instance()->downloadFolder() == folder) {
            return;
        }

        FluentConfig::instance()->setDownloadFolder(folder);
        FluentConfig::instance()->save();
        downloadFolderCard->setContent(folder);
    });
    connect(language, &ComboBoxSettingCard::currentIndexChanged, page, [this](int index) {
        FluentConfig::instance()->setLocaleName(GalleryTranslation::languageNameForIndex(index));
        FluentConfig::instance()->save();
        reloadForLanguageChange();
    });

    personalization->addSettingCards({micaCard, themeMode, themeColorCard, zoomCard, language});
    layout->addWidget(personalization);

    // ---- Material ----
    auto *materialGroup = new SettingCardGroup(settingTx("Material"));
    auto *blurRadiusCard =
        new RangeSettingCard(0, 40, FluentConfig::instance()->acrylicBlurRadius(),
                             FluentIcon::Album, settingTx("Acrylic blur radius"),
                             settingTx("The greater the radius, the more blurred the image"));
    connect(blurRadiusCard, &RangeSettingCard::valueChanged, page, [](int value) {
        FluentConfig::instance()->setAcrylicBlurRadius(value);
        FluentConfig::instance()->save();
    });
    materialGroup->addSettingCards({blurRadiusCard});
    layout->addWidget(materialGroup);

    // ---- Software update ----
    auto *updateGroup = new SettingCardGroup(settingTx("Software update"));
    auto *autoUpdateCard =
        new SwitchSettingCard(FluentIcon::Update, settingTx("Check for updates when the application starts"),
                             settingTx("The new version will be more stable and have more features"));
    auto *manualUpdateCard =
        new PrimaryPushSettingCard(settingTx("Check update"), FluentIcon::Update, settingTx("Software update"),
                                  settingTx("Check latest version from GitHub releases"));
    autoUpdateCard->setChecked(FluentConfig::instance()->isAutoUpdateEnabled());
    connect(autoUpdateCard, &SwitchSettingCard::checkedChanged, page, [](bool checked) {
        FluentConfig::instance()->setAutoUpdateEnabled(checked);
        FluentConfig::instance()->save();
    });
    connect(manualUpdateCard, &PushSettingCard::clicked, page, [page]() {
        checkGalleryUpdate(page, true, false);
    });
    updateGroup->addSettingCards({autoUpdateCard, manualUpdateCard});
    layout->addWidget(updateGroup);

    // ---- About ----
    auto *aboutGroup = new SettingCardGroup(settingTx("About"));
    auto *helpCard =
        new HyperlinkCard(QStringLiteral(FQW_REPOSITORY_URL "/tree/main/docs"), settingTx("Open help page"),
                          FluentIcon::Help, settingTx("Help"),
                          settingTx("Discover new features and learn useful tips about FluentQtWidgets"));
    auto *feedbackCard =
        new PrimaryPushSettingCard(settingTx("Provide feedback"), FluentIcon::Feedback,
                                   settingTx("Provide feedback"),
                                   settingTx("Help us improve FluentQtWidgets by providing feedback"));
    auto *aboutCard = new PrimaryPushSettingCard(
        settingTx("Version"), FluentIcon::Info, settingTx("About"),
        QString(QStringLiteral("© FluentQtWidgets ") + settingTx("Version") + QStringLiteral(" ")
                + FluentQt::libraryVersion()));

    aboutGroup->addSettingCards({helpCard, feedbackCard, aboutCard});
    layout->addWidget(aboutGroup);

    return page;
}
