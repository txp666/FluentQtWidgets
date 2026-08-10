#include "../../GalleryWindow.h"

#include "../update/GalleryUpdateChecker.h"
#include "GalleryViewHelpers.h"

#include <FluentQtWidgets/Layout/FlowLayout.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>

#include <QtCore/QUrl>
#include <QtGui/QDesktopServices>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLabel>

using namespace FluentQt;

QWidget *GalleryWindow::createSettingsPage()
{
    auto *page = new ScrollArea(this);
    page->resize(1000, 800);
    page->setObjectName(QStringLiteral("settingInterface"));
    page->setAttribute(Qt::WA_StyledBackground);
    page->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (page->horizontalFluentScrollBar()) {
        page->horizontalFluentScrollBar()->setForceHidden(true);
    }
    page->enableTransparentBackground(true);
    page->setViewportMargins(0, 80, 0, 20);
    page->setWidgetResizable(true);

    auto *scrollWidget = new QWidget(page);
    scrollWidget->setObjectName(QStringLiteral("scrollWidget"));
    scrollWidget->setAttribute(Qt::WA_StyledBackground);
    page->setWidget(scrollWidget);

    auto *settingLabel = new QLabel(settingTx("Settings"), page);
    settingLabel->setObjectName(QStringLiteral("settingLabel"));
    settingLabel->move(36, 30);

    auto *layout = new ExpandLayout(scrollWidget);
    layout->setSpacing(28);
    layout->setContentsMargins(36, 10, 36, 0);

    applyGalleryViewStyle(page, QStringLiteral("setting_interface"));

    auto *musicGroup = new SettingCardGroup(settingTx("Music on this PC"), scrollWidget);
    const QString musicPath = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    auto *musicFolderCard =
        new FolderListSettingCard(QStringList{musicPath}, FluentIcon::Folder, settingTx("Local music library"),
                                  QString(), musicPath.isEmpty() ? QDir::homePath() : musicPath, musicGroup);
    auto *downloadFolderCard = new PushSettingCard(settingTx("Choose folder"), FluentIcon::Download,
                                                   settingTx("Download directory"),
                                                   FluentConfig::instance()->downloadFolder(), musicGroup);
    musicGroup->addSettingCards({musicFolderCard, downloadFolderCard});
    layout->addWidget(musicGroup);

    auto *personalization = new SettingCardGroup(settingTx("Personalization"), scrollWidget);
    auto *micaCard = new SwitchSettingCard(FluentIcon::Transparent, settingTx("Mica effect"),
                                           settingTx("Apply semi transparent to windows and surfaces"),
                                           personalization);
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

    auto *themeMode = new ComboBoxSettingCard(
        QStringList{settingTx("Light"), settingTx("Dark"), settingTx("Use system setting")}, FluentIcon::Brush,
        settingTx("Application theme"), settingTx("Change the appearance of your application"), personalization);
    const QColor defaultThemeColor(QStringLiteral("#009faa"));
    const QColor currentThemeColor = FluentConfig::instance()->themeColor();
    auto *themeColorCard =
        new CustomColorSettingCard(defaultThemeColor, currentThemeColor, FluentIcon::Palette,
                                   settingTx("Theme color"),
                                   settingTx("Change the theme color of you application"), personalization);

    const QStringList zoomTexts = {QStringLiteral("100%"), QStringLiteral("125%"), QStringLiteral("150%"),
                                   QStringLiteral("175%"), QStringLiteral("200%"),
                                   settingTx("Use system setting")};
    const QVariantList zoomValues = {QStringLiteral("1"), QStringLiteral("1.25"), QStringLiteral("1.5"),
                                     QStringLiteral("1.75"), QStringLiteral("2"), QStringLiteral("Auto")};
    auto *zoomCard = new OptionsSettingCard(zoomTexts, zoomValues, FluentIcon::Zoom,
                                            settingTx("Interface zoom"),
                                            settingTx("Change the size of widgets and fonts"),
                                            personalization);
    auto *language = new ComboBoxSettingCard(
        QStringList{QString::fromUtf8("\xE7\xAE\x80\xE4\xBD\x93\xE4\xB8\xAD\xE6\x96\x87"),
                    QString::fromUtf8("\xE7\xB9\x81\xE9\xAB\x94\xE4\xB8\xAD\xE6\x96\x87"),
                    QStringLiteral("English"), settingTx("Use system setting")},
        FluentIcon::Language, settingTx("Language"), settingTx("Set your preferred language for UI"),
        personalization);

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
        new ColorConfigItem(QStringLiteral("Gallery"), QStringLiteral("AccentColor"), defaultThemeColor, page);
    accentColorItem->setValue(currentThemeColor);
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

    auto *materialGroup = new SettingCardGroup(settingTx("Material"), scrollWidget);
    auto *blurRadiusCard =
        new RangeSettingCard(0, 40, FluentConfig::instance()->acrylicBlurRadius(),
                             FluentIcon::Album, settingTx("Acrylic blur radius"),
                             settingTx("The greater the radius, the more blurred the image"), materialGroup);
    connect(blurRadiusCard, &RangeSettingCard::valueChanged, page, [](int value) {
        FluentConfig::instance()->setAcrylicBlurRadius(value);
        FluentConfig::instance()->save();
    });
    materialGroup->addSettingCards({blurRadiusCard});
    layout->addWidget(materialGroup);

    auto *updateGroup = new SettingCardGroup(settingTx("Software update"), scrollWidget);
    auto *autoUpdateCard =
        new SwitchSettingCard(FluentIcon::Update, settingTx("Check for updates when the application starts"),
                             settingTx("The new version will be more stable and have more features"),
                             updateGroup);
    autoUpdateCard->setChecked(FluentConfig::instance()->isAutoUpdateEnabled());
    connect(autoUpdateCard, &SwitchSettingCard::checkedChanged, page, [](bool checked) {
        FluentConfig::instance()->setAutoUpdateEnabled(checked);
        FluentConfig::instance()->save();
    });
    updateGroup->addSettingCards({autoUpdateCard});
    layout->addWidget(updateGroup);

    auto *aboutGroup = new SettingCardGroup(settingTx("About"), scrollWidget);
    auto *helpCard =
        new HyperlinkCard(QStringLiteral(FQW_REPOSITORY_URL "/tree/main/docs"), settingTx("Open help page"),
                          FluentIcon::Help, settingTx("Help"),
                          settingTx("Discover new features and learn useful tips about FluentQtWidgets"),
                          aboutGroup);
    auto *feedbackCard =
        new PrimaryPushSettingCard(settingTx("Provide feedback"), FluentIcon::Feedback,
                                   settingTx("Provide feedback"),
                                   settingTx("Help us improve FluentQtWidgets by providing feedback"),
                                   aboutGroup);
    auto *aboutCard = new PrimaryPushSettingCard(
        settingTx("Check update"), FluentIcon::Info, settingTx("About"),
        QString(QString::fromUtf8("\xC2\xA9 ") + settingTx("Copyright")
                + QStringLiteral(" 2026, FluentQtWidgets. ") + settingTx("Version")
                + QStringLiteral(" ") + FluentQt::libraryVersion()),
        aboutGroup);
    connect(feedbackCard, &PushSettingCard::clicked, page, []() {
        QDesktopServices::openUrl(QUrl(QStringLiteral(FQW_REPOSITORY_URL "/issues")));
    });
    connect(aboutCard, &PushSettingCard::clicked, page, [page]() {
        checkGalleryUpdate(page, true, false);
    });

    aboutGroup->addSettingCards({helpCard, feedbackCard, aboutCard});
    layout->addWidget(aboutGroup);

    return page;
}
