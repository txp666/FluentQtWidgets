#include <FluentQtWidgets/FluentQtWidgets.h>

#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QPointer>
#include <QtCore/QStringListModel>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QPixmap>
#include <QtTest/QtTest>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QWidget>

class ThemeTest : public QObject
{
    Q_OBJECT

  private slots:
    void themeNameMatchesResourceDirectory()
    {
        QCOMPARE(FluentQt::themeName(FluentQt::Theme::Light), QStringLiteral("light"));
        QCOMPARE(FluentQt::themeName(FluentQt::Theme::Dark), QStringLiteral("dark"));
        QCOMPARE(FluentQt::themeName(FluentQt::Theme::Auto), QStringLiteral("auto"));
    }

    void accentColorCanChange()
    {
        auto *manager = FluentQt::ThemeManager::instance();
        manager->setAccentColor(QColor(10, 20, 30));
        QCOMPARE(manager->accentColor(), QColor(10, 20, 30));
    }

    void settingCurrentThemeStillAppliesApplicationPalette()
    {
        auto *app = qobject_cast<QApplication *>(QCoreApplication::instance());
        QVERIFY(app != nullptr);

        auto *manager = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = manager->theme();
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

        manager->setTheme(previousTheme);
        app->setPalette(previousPalette);
    }

    void autoThemeUsesSystemThemeListener()
    {
        auto *app = qobject_cast<QApplication *>(QCoreApplication::instance());
        QVERIFY(app != nullptr);

        auto *manager = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = manager->theme();
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
        manager->setTheme(previousTheme);
    }

    void themeColorPaletteMatchesReference()
    {
        QCOMPARE(FluentQt::color(FluentQt::FluentThemeColor::DefaultBlue), QColor(QStringLiteral("#0078D7")));
        QCOMPARE(FluentQt::color(FluentQt::FluentSystemColor::SuccessForeground, FluentQt::Theme::Light),
                 QColor(QStringLiteral("#0f7b0f")));
        QCOMPARE(FluentQt::color(FluentQt::FluentSystemColor::SuccessForeground, FluentQt::Theme::Dark),
                 QColor(QStringLiteral("#6ccb5f")));
    }

    void iconPathUsesFluentResourceNames()
    {
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::Setting), QStringLiteral("Setting"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::Return), QStringLiteral("Return"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::LeftArrow), QStringLiteral("LeftArrow"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::Accept), QStringLiteral("Accept"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::Checkbox), QStringLiteral("CheckBox"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::DateTime), QStringLiteral("DateTime"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::Message), QStringLiteral("Message"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::Layout), QStringLiteral("Layout"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::Chat), QStringLiteral("Chat"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::Copy), QStringLiteral("Copy"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::Paste), QStringLiteral("Paste"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::Video), QStringLiteral("Video"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::Music), QStringLiteral("Music"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::FolderAdd), QStringLiteral("FolderAdd"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::Cancel), QStringLiteral("Cancel"));
        QCOMPARE(FluentQt::iconPath(FluentQt::FluentIcon::Setting, FluentQt::Theme::Light),
                 QStringLiteral(":/qfluentwidgets/images/icons/Setting_black.svg"));
        QCOMPARE(FluentQt::iconPath(FluentQt::FluentIcon::Setting, FluentQt::Theme::Dark),
                 QStringLiteral(":/qfluentwidgets/images/icons/Setting_white.svg"));
    }

    void navigationLightQssPinsTextColor()
    {
        const QString qss = FluentQt::FluentStyleSheet::loadSource(FluentQt::FluentStyleSheetSource::Pivot,
                                                                   FluentQt::Theme::Light);
        QVERIFY(qss.contains(QStringLiteral("QPushButton[fqw=\"PivotItem\"]")));
        QVERIFY(qss.contains(QStringLiteral("QPushButton[fqw=\"SegmentedItem\"]")));
        QVERIFY(qss.contains(QStringLiteral("QToolButton[fqw=\"SegmentedToolItem\"]")));
        QVERIFY(qss.contains(QStringLiteral("QWidget[fqw=\"SegmentedToolWidget\"]")));
        QVERIFY(qss.contains(QStringLiteral("QPushButton[fqw=\"BreadcrumbItem\"]")));
        QVERIFY(qss.contains(QStringLiteral("color: black;")));
    }

    void fluentIconsUseBundledSvgResources()
    {
        const QList<FluentQt::FluentIcon> icons = {
            FluentQt::FluentIcon::Up, FluentQt::FluentIcon::Add, FluentQt::FluentIcon::Bus,
            FluentQt::FluentIcon::Car, FluentQt::FluentIcon::Cut, FluentQt::FluentIcon::IOT,
            FluentQt::FluentIcon::Pin, FluentQt::FluentIcon::Tag, FluentQt::FluentIcon::VPN,
            FluentQt::FluentIcon::Cafe, FluentQt::FluentIcon::Chat, FluentQt::FluentIcon::Copy,
            FluentQt::FluentIcon::Code, FluentQt::FluentIcon::Down, FluentQt::FluentIcon::Edit,
            FluentQt::FluentIcon::Flag, FluentQt::FluentIcon::Font, FluentQt::FluentIcon::Game,
            FluentQt::FluentIcon::Help, FluentQt::FluentIcon::Hide, FluentQt::FluentIcon::Home,
            FluentQt::FluentIcon::Info, FluentQt::FluentIcon::Leaf, FluentQt::FluentIcon::Link,
            FluentQt::FluentIcon::Mail, FluentQt::FluentIcon::Menu, FluentQt::FluentIcon::Mute,
            FluentQt::FluentIcon::More, FluentQt::FluentIcon::Move, FluentQt::FluentIcon::Play,
            FluentQt::FluentIcon::Save, FluentQt::FluentIcon::Send, FluentQt::FluentIcon::Sync,
            FluentQt::FluentIcon::Unit, FluentQt::FluentIcon::View, FluentQt::FluentIcon::Wifi,
            FluentQt::FluentIcon::Zoom, FluentQt::FluentIcon::Album, FluentQt::FluentIcon::Brush,
            FluentQt::FluentIcon::Broom, FluentQt::FluentIcon::Close, FluentQt::FluentIcon::Cloud,
            FluentQt::FluentIcon::Embed, FluentQt::FluentIcon::Globe, FluentQt::FluentIcon::Heart,
            FluentQt::FluentIcon::Label, FluentQt::FluentIcon::Media, FluentQt::FluentIcon::Movie,
            FluentQt::FluentIcon::Music, FluentQt::FluentIcon::Robot, FluentQt::FluentIcon::Pause,
            FluentQt::FluentIcon::Paste, FluentQt::FluentIcon::Photo, FluentQt::FluentIcon::Phone,
            FluentQt::FluentIcon::Print, FluentQt::FluentIcon::Share, FluentQt::FluentIcon::Tiles,
            FluentQt::FluentIcon::Unpin, FluentQt::FluentIcon::Video, FluentQt::FluentIcon::Train,
            FluentQt::FluentIcon::AddTo, FluentQt::FluentIcon::Accept, FluentQt::FluentIcon::Camera,
            FluentQt::FluentIcon::Cancel, FluentQt::FluentIcon::Delete, FluentQt::FluentIcon::Folder,
            FluentQt::FluentIcon::Filter, FluentQt::FluentIcon::Market, FluentQt::FluentIcon::Scroll,
            FluentQt::FluentIcon::Layout, FluentQt::FluentIcon::GitHub, FluentQt::FluentIcon::Update,
            FluentQt::FluentIcon::Remove, FluentQt::FluentIcon::Return, FluentQt::FluentIcon::People,
            FluentQt::FluentIcon::QRCode, FluentQt::FluentIcon::Ringer, FluentQt::FluentIcon::Rotate,
            FluentQt::FluentIcon::Search, FluentQt::FluentIcon::Volume, FluentQt::FluentIcon::Frigid,
            FluentQt::FluentIcon::SaveAs, FluentQt::FluentIcon::ZoomIn, FluentQt::FluentIcon::Connect,
            FluentQt::FluentIcon::History, FluentQt::FluentIcon::Setting, FluentQt::FluentIcon::Palette,
            FluentQt::FluentIcon::Message, FluentQt::FluentIcon::FitPage, FluentQt::FluentIcon::ZoomOut,
            FluentQt::FluentIcon::Airplane, FluentQt::FluentIcon::Asterisk, FluentQt::FluentIcon::Calories,
            FluentQt::FluentIcon::Calendar, FluentQt::FluentIcon::Feedback, FluentQt::FluentIcon::Library,
            FluentQt::FluentIcon::Minimize, FluentQt::FluentIcon::Checkbox, FluentQt::FluentIcon::Document,
            FluentQt::FluentIcon::Language, FluentQt::FluentIcon::Download, FluentQt::FluentIcon::Question,
            FluentQt::FluentIcon::Speakers, FluentQt::FluentIcon::DateTime, FluentQt::FluentIcon::FontSize,
            FluentQt::FluentIcon::HomeFill, FluentQt::FluentIcon::PageLeft, FluentQt::FluentIcon::SaveCopy,
            FluentQt::FluentIcon::SendFill, FluentQt::FluentIcon::SkipBack, FluentQt::FluentIcon::SpeedOff,
            FluentQt::FluentIcon::Alignment, FluentQt::FluentIcon::Bluetooth, FluentQt::FluentIcon::Completed,
            FluentQt::FluentIcon::Constract, FluentQt::FluentIcon::Headphone, FluentQt::FluentIcon::Megaphone,
            FluentQt::FluentIcon::Projector, FluentQt::FluentIcon::Education, FluentQt::FluentIcon::LeftArrow,
            FluentQt::FluentIcon::EraseTool, FluentQt::FluentIcon::PageRight, FluentQt::FluentIcon::PlaySolid,
            FluentQt::FluentIcon::BookShelf, FluentQt::FluentIcon::Hightlight, FluentQt::FluentIcon::FolderAdd,
            FluentQt::FluentIcon::PauseBold, FluentQt::FluentIcon::PencilInk, FluentQt::FluentIcon::PieSingle,
            FluentQt::FluentIcon::QuickNote, FluentQt::FluentIcon::SpeedHigh, FluentQt::FluentIcon::StopWatch,
            FluentQt::FluentIcon::ZipFolder, FluentQt::FluentIcon::Basketball, FluentQt::FluentIcon::Brightness,
            FluentQt::FluentIcon::Dictionary, FluentQt::FluentIcon::Microphone, FluentQt::FluentIcon::ArrowDown,
            FluentQt::FluentIcon::FullScreen, FluentQt::FluentIcon::MixVolumes, FluentQt::FluentIcon::RemoveFrom,
            FluentQt::FluentIcon::RightArrow, FluentQt::FluentIcon::QuietHours, FluentQt::FluentIcon::Fingerprint,
            FluentQt::FluentIcon::Application, FluentQt::FluentIcon::Certificate, FluentQt::FluentIcon::Transparent,
            FluentQt::FluentIcon::ImageExport, FluentQt::FluentIcon::SpeedMedium, FluentQt::FluentIcon::LibraryFill,
            FluentQt::FluentIcon::MusicFolder, FluentQt::FluentIcon::PowerButton, FluentQt::FluentIcon::SkipForward,
            FluentQt::FluentIcon::CareUpSolid, FluentQt::FluentIcon::AcceptMedium, FluentQt::FluentIcon::CancelMedium,
            FluentQt::FluentIcon::ChevronRight, FluentQt::FluentIcon::ClippingTool, FluentQt::FluentIcon::SearchMirror,
            FluentQt::FluentIcon::ShoppingCart, FluentQt::FluentIcon::FontIncrease, FluentQt::FluentIcon::BackToWindow,
            FluentQt::FluentIcon::CommandPrompt, FluentQt::FluentIcon::CloudDownload, FluentQt::FluentIcon::DictionaryAdd,
            FluentQt::FluentIcon::CareDownSolid, FluentQt::FluentIcon::CareLeftSolid, FluentQt::FluentIcon::ClearSelection,
            FluentQt::FluentIcon::DeveloperTools, FluentQt::FluentIcon::BackgroundFill, FluentQt::FluentIcon::CareRightSolid,
            FluentQt::FluentIcon::ChevronDownMed, FluentQt::FluentIcon::ChevronRightMed, FluentQt::FluentIcon::EmojiTabSymbols,
            FluentQt::FluentIcon::ExpressiveInputEntry,
        };

        for (const FluentQt::FluentIcon iconType : icons) {
            const QString lightPath = FluentQt::iconPath(iconType, FluentQt::Theme::Light);
            const QString darkPath = FluentQt::iconPath(iconType, FluentQt::Theme::Dark);
            QVERIFY2(QFile::exists(lightPath), qPrintable(lightPath));
            QVERIFY2(QFile::exists(darkPath), qPrintable(darkPath));
            QVERIFY2(!FluentQt::icon(iconType, FluentQt::Theme::Light).pixmap(QSize(16, 16)).isNull(),
                     qPrintable(lightPath));
            QVERIFY2(!FluentQt::icon(iconType, FluentQt::Theme::Dark).pixmap(QSize(16, 16)).isNull(),
                     qPrintable(darkPath));
        }
    }

    void fluentIconPixmapKeepsRequestedDevicePixelRatio()
    {
        const QSize logicalSize(16, 16);
        const qreal requestedDpr = 2.0;
        const QPixmap pixmap =
            FluentQt::icon(FluentQt::FluentIcon::Home, FluentQt::Theme::Light)
                .pixmap(logicalSize, requestedDpr, QIcon::Normal, QIcon::Off);

        QVERIFY(!pixmap.isNull());
        QCOMPARE(qRound(pixmap.devicePixelRatioF() * 100), 200);
        QCOMPARE(QSize(qRound(pixmap.width() / pixmap.devicePixelRatioF()),
                      qRound(pixmap.height() / pixmap.devicePixelRatioF())),
                 logicalSize);
        QVERIFY(pixmap.width() >= logicalSize.width() * requestedDpr);
        QVERIFY(pixmap.height() >= logicalSize.height() * requestedDpr);
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

    void repositoryLinksUseConfiguredProjectUrl()
    {
        QCOMPARE(FluentQt::repositoryUrl(), QStringLiteral(FQW_REPOSITORY_URL));
        QCOMPARE(FluentQt::repositoryIssuesUrl(), QStringLiteral(FQW_REPOSITORY_URL "/issues"));
        QCOMPARE(FluentQt::repositoryDocsUrl(), QStringLiteral(FQW_REPOSITORY_URL "/tree/main/docs"));
        QCOMPARE(FluentQt::repositoryExampleUrl(), QStringLiteral(FQW_REPOSITORY_URL "/tree/main/examples"));
        QVERIFY(!FluentQt::repositoryUrl().contains(QStringLiteral("github.com/FluentQtWidgets/FluentQtWidgets")));
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

    void spinBoxQssUsesCppSelectors()
    {
        const QStringList paths = {
            QStringLiteral(":/qfluentwidgets/qss/light/spin_box.qss"),
            QStringLiteral(":/qfluentwidgets/qss/dark/spin_box.qss"),
        };

        for (const QString &path : paths) {
            QFile file(path);
            QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
            const QString qss = QString::fromUtf8(file.readAll());

            QVERIFY2(qss.contains(QStringLiteral("QToolButton[fqw=\"SpinButton\"]")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QToolButton[fqw=\"CompactSpinButton\"]")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QSpinBox[fqw=\"SpinBox\"][symbolVisible=\"false\"]")),
                     qPrintable(path));
            QVERIFY2(!qss.contains(QStringLiteral("\nSpinButton {")), qPrintable(path));
            QVERIFY2(!qss.contains(QStringLiteral("\nSpinButton:hover")), qPrintable(path));
            QVERIFY2(!qss.contains(QStringLiteral("\nSpinButton:pressed")), qPrintable(path));
        }
    }

    void lineEditQssKeepsPythonParityStates()
    {
        const QStringList paths = {
            QStringLiteral(":/qfluentwidgets/qss/light/line_edit.qss"),
            QStringLiteral(":/qfluentwidgets/qss/dark/line_edit.qss"),
        };

        for (const QString &path : paths) {
            QFile file(path);
            QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
            const QString qss = QString::fromUtf8(file.readAll());

            QVERIFY2(qss.contains(QStringLiteral("QTextEdit[fqw=\"TextEdit\"],")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("font: 14px --FontFamilies;")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("padding: 2px 3px 2px 8px;")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("#lineEditButton")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("#lineEditButton:hover")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("#lineEditButton:pressed")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QLineEdit[fqw=\"LineEdit\"]:disabled")), qPrintable(path));
        }

        const QStringList darkPaths = {
            QStringLiteral(":/qfluentwidgets/qss/dark/line_edit.qss"),
        };
        for (const QString &path : darkPaths) {
            QFile file(path);
            QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
            const QString qss = QString::fromUtf8(file.readAll());
            QVERIFY2(qss.contains(QStringLiteral("selection-color: black;")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QLineEdit[fqw=\"LineEdit\"][transparent=\"true\"]:focus")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QLineEdit[fqw=\"LineEdit\"][transparent=\"false\"]:focus")),
                     qPrintable(path));
        }
    }

    void comboBoxQssCoversAllCppRoles()
    {
        QCOMPARE(FluentQt::styleSheetSourceForRole(QStringLiteral("ComboBox")),
                 FluentQt::FluentStyleSheetSource::ComboBox);
        QCOMPARE(FluentQt::styleSheetSourceForRole(QStringLiteral("EditableComboBox")),
                 FluentQt::FluentStyleSheetSource::ComboBox);
        QCOMPARE(FluentQt::styleSheetSourceForRole(QStringLiteral("ModelComboBox")),
                 FluentQt::FluentStyleSheetSource::ComboBox);
        QCOMPARE(FluentQt::styleSheetSourceForRole(QStringLiteral("EditableModelComboBox")),
                 FluentQt::FluentStyleSheetSource::ComboBox);
        QCOMPARE(FluentQt::styleSheetSourceForRole(QStringLiteral("ComboBoxMenu")),
                 FluentQt::FluentStyleSheetSource::Menu);

        const QStringList paths = {
            QStringLiteral(":/qfluentwidgets/qss/light/combo_box.qss"),
            QStringLiteral(":/qfluentwidgets/qss/dark/combo_box.qss"),
        };

        for (const QString &path : paths) {
            QFile file(path);
            QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
            const QString qss = QString::fromUtf8(file.readAll());

            QVERIFY2(qss.contains(QStringLiteral("QPushButton[fqw=\"ComboBox\"]")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QPushButton[fqw=\"ModelComboBox\"]")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QLineEdit[fqw=\"EditableComboBox\"]")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QLineEdit[fqw=\"EditableModelComboBox\"]")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("font: 14px --FontFamilies;")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("[isPlaceholderText=\"true\"]")), qPrintable(path));
        }
    }

    void dateTimeQssCoversCalendarViewRoles()
    {
        QCOMPARE(FluentQt::styleSheetSourceForRole(QStringLiteral("CalendarView")),
                 FluentQt::FluentStyleSheetSource::CalendarPicker);
        QCOMPARE(FluentQt::styleSheetSourceForRole(QStringLiteral("FastCalendarView")),
                 FluentQt::FluentStyleSheetSource::CalendarPicker);
        QCOMPARE(FluentQt::styleSheetSourceForRole(QStringLiteral("PickerPanel")),
                 FluentQt::FluentStyleSheetSource::TimePicker);
        QCOMPARE(FluentQt::styleSheetSourceForRole(QStringLiteral("SeparatorWidget")),
                 FluentQt::FluentStyleSheetSource::TimePicker);
        QCOMPARE(FluentQt::styleSheetSourceForRole(QStringLiteral("ItemMaskWidget")),
                 FluentQt::FluentStyleSheetSource::TimePicker);

        const QStringList paths = {
            QStringLiteral(":/qfluentwidgets/qss/light/calendar_picker.qss"),
            QStringLiteral(":/qfluentwidgets/qss/dark/calendar_picker.qss"),
        };

        for (const QString &path : paths) {
            QFile file(path);
            QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
            const QString qss = QString::fromUtf8(file.readAll());

            QVERIFY2(qss.contains(QStringLiteral("QFrame[fqw=\"CalendarView\"]")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QPushButton[calendarTitle=\"true\"]")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QPushButton[calendarItem=\"true\"][selected=\"true\"]")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QPushButton#monthButton[calendarItem=\"true\"]")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QPushButton#yearButton[calendarItem=\"true\"]")),
                     qPrintable(path));
        }
    }

    void selectionControlQssKeepsPythonMetrics()
    {
        QCOMPARE(FluentQt::styleSheetSourceForRole(QStringLiteral("RadioButton")),
                 FluentQt::FluentStyleSheetSource::Button);

        const QStringList paths = {
            QStringLiteral(":/qfluentwidgets/qss/light/check_box.qss"),
            QStringLiteral(":/qfluentwidgets/qss/dark/check_box.qss"),
        };

        for (const QString &path : paths) {
            QFile file(path);
            QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
            const QString qss = QString::fromUtf8(file.readAll());

            QVERIFY2(qss.contains(QStringLiteral("QCheckBox[fqw=\"CheckBox\"] {")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("min-width: 28px;")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("min-height: 22px;")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("font: 14px --FontFamilies;")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("margin-left: 1px;")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QCheckBox[fqw=\"CheckBox\"]:disabled")), qPrintable(path));
            QVERIFY2(!qss.contains(QStringLiteral("QRadioButton[fqw=\"RadioButton\"]")), qPrintable(path));
        }

        const QStringList buttonPaths = {
            QStringLiteral(":/qfluentwidgets/qss/light/button.qss"),
            QStringLiteral(":/qfluentwidgets/qss/dark/button.qss"),
        };
        for (const QString &path : buttonPaths) {
            QFile file(path);
            QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
            const QString qss = QString::fromUtf8(file.readAll());
            QVERIFY2(qss.contains(QStringLiteral("QRadioButton[fqw=\"RadioButton\"] {")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("font: 14px --FontFamilies;")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("max-height: 24px;")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QRadioButton[fqw=\"RadioButton\"]::indicator")),
                     qPrintable(path));
        }
    }

    void sliderQssKeepsPythonParityStates()
    {
        const QStringList paths = {
            QStringLiteral(":/qfluentwidgets/qss/light/slider.qss"),
            QStringLiteral(":/qfluentwidgets/qss/dark/slider.qss"),
        };

        for (const QString &path : paths) {
            QFile file(path);
            QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
            const QString qss = QString::fromUtf8(file.readAll());

            QVERIFY2(qss.contains(QStringLiteral("QSlider[fqw=\"Slider\"]:horizontal")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QSlider[fqw=\"Slider\"]::sub-page:horizontal")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QSlider[fqw=\"Slider\"]::handle:horizontal:hover")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QSlider[fqw=\"Slider\"]::handle:horizontal:pressed")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QSlider[fqw=\"Slider\"]:vertical")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QSlider[fqw=\"Slider\"]::add-page:vertical")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QSlider[fqw=\"Slider\"]::handle:vertical:hover")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("qradialgradient")), qPrintable(path));
        }

        QFile lightFile(QStringLiteral(":/qfluentwidgets/qss/light/slider.qss"));
        QVERIFY(lightFile.open(QIODevice::ReadOnly | QIODevice::Text));
        const QString lightQss = QString::fromUtf8(lightFile.readAll());
        QVERIFY(lightQss.contains(QStringLiteral("QSlider[fqw=\"Slider\"]::groove:horizontal:disabled")));
        QVERIFY(lightQss.contains(QStringLiteral("QSlider[fqw=\"Slider\"]::handle:vertical:disabled")));
    }

    void buttonQssKeepsPythonParityStates()
    {
        QCOMPARE(FluentQt::styleSheetSourceForRole(QStringLiteral("CommandButton")),
                 FluentQt::FluentStyleSheetSource::Button);
        QCOMPARE(FluentQt::styleSheetSourceForRole(QStringLiteral("MoreActionsButton")),
                 FluentQt::FluentStyleSheetSource::Button);

        const QStringList paths = {
            QStringLiteral(":/qfluentwidgets/qss/light/button.qss"),
            QStringLiteral(":/qfluentwidgets/qss/dark/button.qss"),
        };

        for (const QString &path : paths) {
            QFile file(path);
            QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
            const QString qss = QString::fromUtf8(file.readAll());

            QVERIFY2(qss.contains(QStringLiteral("QToolButton[fqw=\"DropDownToolButton\"]")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QToolButton[fqw=\"PrimaryDropDownToolButton\"]")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QPushButton[fqw=\"PrimaryDropDownPushButton\"]")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QPushButton[fqw=\"TransparentDropDownPushButton\"]")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QToolButton[fqw=\"TransparentDropDownToolButton\"]")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QToolButton[fqw=\"CommandButton\"]")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QToolButton[fqw=\"MoreActionsButton\"]")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QToolButton[fqw=\"CommandButton\"]:checked")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QPushButton[fqw=\"TransparentTogglePushButton\"]:checked")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QToolButton[fqw=\"TransparentToggleToolButton\"]:checked")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QPushButton[fqw=\"HyperlinkButton\"]:disabled")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QPushButton[fqw=\"PillPushButton\"]:disabled:checked")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QToolButton[fqw=\"TabCloseButton\"]:hover")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QPushButton[fqw=\"PushButton\"]:disabled")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QToolButton[fqw=\"DropDownToolButton\"]:disabled")),
                     qPrintable(path));
        }
    }

    void switchButtonQssKeepsPythonFontSource()
    {
        const QStringList paths = {
            QStringLiteral(":/qfluentwidgets/qss/light/switch_button.qss"),
            QStringLiteral(":/qfluentwidgets/qss/dark/switch_button.qss"),
        };

        for (const QString &path : paths) {
            QFile file(path);
            QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
            const QString qss = QString::fromUtf8(file.readAll());

            QVERIFY2(qss.contains(QStringLiteral("QWidget[fqw=\"SwitchButton\"] QLabel")),
                     qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("font: 14px --FontFamilies;")), qPrintable(path));
            QVERIFY2(qss.contains(QStringLiteral("QWidget[fqw=\"SwitchButton\"] QLabel:disabled")),
                     qPrintable(path));
        }
    }

    void navigationControlsTrackCurrentItems()
    {
        FluentQt::Pivot pivot;
        QVERIFY(pivot.addItem(QStringLiteral("home"), QStringLiteral("Home")) != nullptr);
        QVERIFY(pivot.addItem(QStringLiteral("settings"), QStringLiteral("Settings")) != nullptr);
        QVERIFY(pivot.contains(QStringLiteral("home")));
        QCOMPARE(pivot.currentItem(), QStringLiteral("home"));

        pivot.setCurrentItem(QStringLiteral("settings"));
        QCOMPARE(pivot.currentItem(), QStringLiteral("settings"));
        QVERIFY(pivot.item(QStringLiteral("settings"))->isChecked());

        FluentQt::SegmentedWidget segmented;
        segmented.addItem(QStringLiteral("day"), QStringLiteral("Day"));
        segmented.addItem(QStringLiteral("week"), QStringLiteral("Week"));
        segmented.setCurrentItem(QStringLiteral("week"));
        QCOMPARE(segmented.currentItem(), QStringLiteral("week"));
        QCOMPARE(segmented.item(QStringLiteral("day"))->font().pixelSize(), 14);
        QCOMPARE(segmented.item(QStringLiteral("week"))->property("isSelected").toBool(), true);

        FluentQt::BreadcrumbBar breadcrumb;
        QSignalSpy breadcrumbItemSpy(&breadcrumb, &FluentQt::BreadcrumbBar::currentItemChanged);
        QSignalSpy breadcrumbIndexSpy(&breadcrumb, &FluentQt::BreadcrumbBar::currentIndexChanged);
        breadcrumb.addItem(QStringLiteral("root"), QStringLiteral("Root"));
        breadcrumb.addItem(QStringLiteral("child"), QStringLiteral("Child"));
        breadcrumb.addItem(QStringLiteral("leaf"), QStringLiteral("Leaf"));
        QCOMPARE(breadcrumb.count(), 3);
        QCOMPARE(breadcrumb.items().size(), 3);
        QCOMPARE(breadcrumb.currentIndex(), 2);
        QCOMPARE(breadcrumb.currentItem(), QStringLiteral("leaf"));
        QCOMPARE(breadcrumb.itemAt(1).key, QStringLiteral("child"));
        QCOMPARE(breadcrumb.item(QStringLiteral("leaf")).text, QStringLiteral("Leaf"));

        breadcrumb.setSpacing(20);
        QCOMPARE(breadcrumb.spacing(), 20);
        breadcrumb.setItemText(QStringLiteral("child"), QStringLiteral("Documents"));
        QCOMPARE(breadcrumb.item(QStringLiteral("child")).text, QStringLiteral("Documents"));

        breadcrumb.setCurrentItem(QStringLiteral("child"));
        QCOMPARE(breadcrumb.count(), 2);
        QCOMPARE(breadcrumb.currentIndex(), 1);
        QCOMPARE(breadcrumb.currentItem(), QStringLiteral("child"));
        QCOMPARE(breadcrumbItemSpy.takeLast().at(0).toString(), QStringLiteral("child"));
        QCOMPARE(breadcrumbIndexSpy.takeLast().at(0).toInt(), 1);

        breadcrumb.popItem();
        QCOMPARE(breadcrumb.currentItem(), QStringLiteral("root"));
        QCOMPARE(breadcrumb.count(), 1);
    }

    void settingCardsExposeComposedControls()
    {
        FluentQt::SettingCardGroup group(QStringLiteral("General"));

        auto *switchCard =
            new FluentQt::SwitchSettingCard(FluentQt::FluentIcon::Setting, QStringLiteral("Dark theme"));
        auto *rangeCard =
            new FluentQt::RangeSettingCard(0, 10, 4, FluentQt::FluentIcon::Volume, QStringLiteral("Scale"));
        auto *comboCard =
            new FluentQt::ComboBoxSettingCard(QStringList{QStringLiteral("Light"), QStringLiteral("Dark")},
                                              FluentQt::FluentIcon::Setting, QStringLiteral("Theme"));
        auto *colorCard = new FluentQt::ColorSettingCard(QColor(QStringLiteral("#112233")), FluentQt::FluentIcon::Heart,
                                                         QStringLiteral("Accent"));
        auto *optionsCard = new FluentQt::OptionsSettingCard(
            QStringList{QStringLiteral("Compact"), QStringLiteral("Comfortable")},
            QVariantList{QVariant(QStringLiteral("compact")), QVariant(QStringLiteral("comfortable"))},
            FluentQt::FluentIcon::Setting, QStringLiteral("Density"));
        auto *folderCard = new FluentQt::FolderListSettingCard(
            QStringList{QStringLiteral("C:/Projects")}, FluentQt::FluentIcon::Folder, QStringLiteral("Folders"));
        auto *customColorCard =
            new FluentQt::CustomColorSettingCard(QColor(QStringLiteral("#005fb8")), QColor(QStringLiteral("#112233")),
                                                 FluentQt::FluentIcon::Heart, QStringLiteral("Custom accent"));
        auto *hyperlinkCard =
            new FluentQt::HyperlinkCard(QStringLiteral(FQW_REPOSITORY_URL),
                                        QStringLiteral("Open"), FluentQt::FluentIcon::Link,
                                        QStringLiteral("Repository"), QStringLiteral("Project source"));

        group.addSettingCards(
            {switchCard, rangeCard, comboCard, colorCard, optionsCard, folderCard, customColorCard, hyperlinkCard});
        QCOMPARE(group.cards().size(), 8);
        QVERIFY(group.cardLayout() != nullptr);
        QCOMPARE(group.height(), group.cardLayout()->heightForWidth(group.width()) + 46);
        QVERIFY(group.height() > 0);

        QSignalSpy switchSpy(switchCard, &FluentQt::SwitchSettingCard::checkedChanged);
        switchCard->setChecked(true);
        QCOMPARE(switchCard->isChecked(), true);
        QCOMPARE(switchSpy.count(), 1);

        QSignalSpy rangeSpy(rangeCard, &FluentQt::RangeSettingCard::valueChanged);
        rangeCard->setValue(8);
        QCOMPARE(rangeCard->value(), 8);
        QCOMPARE(rangeSpy.count(), 1);
        QCOMPARE(rangeCard->valueLabel()->text(), QStringLiteral("8"));

        comboCard->setCurrentIndex(1);
        QCOMPARE(comboCard->currentText(), QStringLiteral("Dark"));

        colorCard->setColor(QColor(QStringLiteral("#445566")));
        QCOMPARE(colorCard->color(), QColor(QStringLiteral("#445566")));

        optionsCard->setValue(QStringLiteral("comfortable"));
        QCOMPARE(optionsCard->value().toString(), QStringLiteral("comfortable"));
        QCOMPARE(optionsCard->choiceLabel()->text(), QStringLiteral("Comfortable"));

        QSignalSpy folderAddedSpy(folderCard, &FluentQt::FolderListSettingCard::folderAdded);
        QSignalSpy folderRemovedSpy(folderCard, &FluentQt::FolderListSettingCard::folderRemoved);
        QSignalSpy foldersSpy(folderCard, &FluentQt::FolderListSettingCard::foldersChanged);
        QVERIFY(folderCard->addFolder(QStringLiteral("D:/Assets")));
        QVERIFY(!folderCard->addFolder(QStringLiteral("D:/Assets")));
        QCOMPARE(folderCard->folders(),
                 QStringList({QStringLiteral("C:/Projects"), QStringLiteral("D:/Assets")}));
        QCOMPARE(folderCard->folderItems().size(), 2);
        folderCard->folderItems().first()->removeButton()->click();
        QCOMPARE(folderCard->folders(), QStringList({QStringLiteral("D:/Assets")}));
        QCOMPARE(folderAddedSpy.count(), 1);
        QCOMPARE(folderRemovedSpy.count(), 1);
        QCOMPARE(foldersSpy.count(), 2);

        QSignalSpy customColorSpy(customColorCard, &FluentQt::CustomColorSettingCard::colorChanged);
        QVERIFY(customColorCard->isCustomColorEnabled());
        QCOMPARE(customColorCard->color(), QColor(QStringLiteral("#112233")));
        customColorCard->setCustomColorEnabled(false);
        QCOMPARE(customColorCard->color(), QColor(QStringLiteral("#005fb8")));
        QCOMPARE(customColorCard->choiceLabel()->text(), QStringLiteral("Default color"));
        customColorCard->setColor(QColor(QStringLiteral("#445566")));
        QVERIFY(customColorCard->isCustomColorEnabled());
        QCOMPARE(customColorCard->customColor(), QColor(QStringLiteral("#445566")));
        QCOMPARE(customColorCard->color(), QColor(QStringLiteral("#445566")));
        customColorCard->setAlphaEnabled(true);
        QVERIFY(customColorCard->colorPicker()->isAlphaEnabled());
        QCOMPARE(customColorSpy.count(), 2);

        QVERIFY(hyperlinkCard->linkButton() != nullptr);
        QCOMPARE(hyperlinkCard->linkButton()->text(), QStringLiteral("Open"));
        QCOMPARE(hyperlinkCard->url(), QUrl(QStringLiteral(FQW_REPOSITORY_URL)));
        hyperlinkCard->setUrl(QStringLiteral(FQW_REPOSITORY_URL "/issues"));
        QCOMPARE(hyperlinkCard->linkButton()->url(),
                 QUrl(QStringLiteral(FQW_REPOSITORY_URL "/issues")));

        FluentQt::ExpandButton expandButton;
        QCOMPARE(expandButton.size(), QSize(30, 30));
        expandButton.setAngle(90);
        QCOMPARE(expandButton.angle(), 90.0);
        expandButton.setHover(true);
        expandButton.setPressed(true);
        QVERIFY(expandButton.isHover());
        QVERIFY(expandButton.isPressed());

        FluentQt::HeaderSettingCard headerCard(FluentQt::FluentIcon::Setting, QStringLiteral("Advanced"),
                                               QStringLiteral("Header content"));
        QVERIFY(headerCard.expandButton() != nullptr);
        auto *headerAction = new FluentQt::PushButton(QStringLiteral("Action"));
        headerCard.addWidget(headerAction);
        QVERIFY(headerCard.actionLayout()->indexOf(headerAction) >= 0);

        auto *groupAction = new FluentQt::PushButton(QStringLiteral("Configure"));
        FluentQt::GroupWidget groupWidget(FluentQt::FluentIcon::Setting, QStringLiteral("Group"),
                                          QStringLiteral("Group content"), groupAction);
        QCOMPARE(groupWidget.title(), QStringLiteral("Group"));
        QCOMPARE(groupWidget.content(), QStringLiteral("Group content"));
        QCOMPARE(groupWidget.widget(), groupAction);
        QVERIFY(groupWidget.iconWidget() != nullptr);
        QVERIFY(groupWidget.hBoxLayout() != nullptr);
        groupWidget.setContent(QString());
        QVERIFY(!groupWidget.contentLabel()->isVisible());

        FluentQt::ExpandGroupSettingCard expandGroup(FluentQt::FluentIcon::Setting, QStringLiteral("Grouped"),
                                                     QStringLiteral("Grouped content"));
        auto *firstGroup = expandGroup.addGroup(FluentQt::FluentIcon::Info, QStringLiteral("One"),
                                                QStringLiteral("First"), new FluentQt::SwitchButton);
        auto *secondGroup = expandGroup.addGroup(FluentQt::FluentIcon::Heart, QStringLiteral("Two"),
                                                 QStringLiteral("Second"), new FluentQt::PushButton(QStringLiteral("Go")));
        QCOMPARE(expandGroup.groupCount(), 2);
        QCOMPARE(expandGroup.groupWidgets().at(0), firstGroup);
        QCOMPARE(expandGroup.groupWidgets().at(1), secondGroup);
        QCOMPARE(expandGroup.viewLayout()->count(), 3);
        QVERIFY(qobject_cast<FluentQt::GroupSeparator *>(expandGroup.viewLayout()->itemAt(1)->widget()) != nullptr);
        expandGroup.removeGroupWidget(firstGroup);
        QCOMPARE(expandGroup.groupCount(), 1);
        QCOMPARE(expandGroup.viewLayout()->count(), 1);

        FluentQt::SimpleExpandGroupSettingCard simpleGroup(FluentQt::FluentIcon::Setting,
                                                           QStringLiteral("Simple grouped"));
        QCOMPARE(simpleGroup.property("fqw").toString(), QStringLiteral("SimpleExpandGroupSettingCard"));
    }

    void colorPickerButtonUsesFluentPopup()
    {
        FluentQt::ColorDialog alphaDialog(QColor(QStringLiteral("#5012aaa2")),
                                          QStringLiteral("Background Color"), nullptr, true);
        QCOMPARE(alphaDialog.property("fqw").toString(), QStringLiteral("ColorDialogPopup"));
        QVERIFY(alphaDialog.testAttribute(Qt::WA_TranslucentBackground));
        QVERIFY(alphaDialog.windowFlags() & Qt::FramelessWindowHint);
        QVERIFY(alphaDialog.windowFlags() & Qt::NoDropShadowWindowHint);
        QVERIFY(alphaDialog.widget() != nullptr);
        QCOMPARE(alphaDialog.widget()->property("fqw").toString(), QStringLiteral("ColorDialog"));
        QCOMPARE(alphaDialog.widget()->size(), QSize(488, 736));
        QCOMPARE(alphaDialog.widget()->minimumSize(), QSize(488, 736));
        QCOMPARE(alphaDialog.widget()->maximumSize(), QSize(488, 736));
        QCOMPARE(alphaDialog.scrollWidget()->size(), QSize(440, 600));
        QCOMPARE(alphaDialog.huePanel()->size(), QSize(256, 256));
        QVERIFY(alphaDialog.brightnessSlider() != nullptr);
        QCOMPARE(alphaDialog.hexLineEdit()->text(), QStringLiteral("5012aaa2"));
        QVERIFY(!alphaDialog.opacityLineEdit()->isHidden());
        QVERIFY(alphaDialog.findChildren<QColorDialog *>().isEmpty());

        QSignalSpy dialogSpy(&alphaDialog, &FluentQt::ColorDialog::colorChanged);
        alphaDialog.setColor(QColor(QStringLiteral("#ff445566")));
        alphaDialog.cancelButton()->click();
        QCOMPARE(dialogSpy.count(), 0);

        FluentQt::ColorDialog acceptDialog(QColor(QStringLiteral("#112233")),
                                           QStringLiteral("Background Color"));
        QSignalSpy acceptSpy(&acceptDialog, &FluentQt::ColorDialog::colorChanged);
        acceptDialog.setColor(QColor(QStringLiteral("#445566")));
        acceptDialog.yesButton()->click();
        QCOMPARE(acceptSpy.count(), 1);
        QCOMPARE(acceptSpy.takeFirst().at(0).value<QColor>(), QColor(QStringLiteral("#445566")));
        QCOMPARE(acceptDialog.widget()->size(), QSize(488, 696));
        QCOMPARE(acceptDialog.widget()->minimumSize(), QSize(488, 696));
        QCOMPARE(acceptDialog.widget()->maximumSize(), QSize(488, 696));
        QCOMPARE(acceptDialog.scrollWidget()->size(), QSize(440, 560));
        QVERIFY(acceptDialog.opacityLineEdit()->isHidden());

        FluentQt::ColorSettingCard card(QColor(QStringLiteral("#112233")), FluentQt::FluentIcon::Heart,
                                        QStringLiteral("Accent"));
        FluentQt::ColorPickerButton picker(QColor(QStringLiteral("#5012aaa2")),
                                           QStringLiteral("Background Color"), nullptr, true);
        QCOMPARE(picker.title(), QStringLiteral("Background Color"));
        QVERIFY(picker.isAlphaEnabled());
        QCOMPARE(picker.toolTip(), QStringLiteral("Choose Background Color"));

        card.show();
        QVERIFY(QTest::qWaitForWindowExposed(&card));

        QSignalSpy colorSpy(&card, &FluentQt::ColorSettingCard::colorChanged);
        bool dialogInspected = false;
        QPointer<FluentQt::ColorDialog> dialogGuard;
        QTimer::singleShot(0, &card, [&card]() { card.colorPicker()->click(); });
        QTimer::singleShot(100, &card, [&]() {
            auto *popup = findVisibleTopLevelByRole(QStringLiteral("ColorDialogPopup"));
            auto *dialog = qobject_cast<FluentQt::ColorDialog *>(popup);
            if (!dialog) {
                return;
            }

            dialogInspected = true;
            dialogGuard = dialog;
            QVERIFY(dialog->testAttribute(Qt::WA_TranslucentBackground));
            QVERIFY(dialog->windowFlags() & Qt::NoDropShadowWindowHint);
            QCOMPARE(dialog->widget()->property("fqw").toString(), QStringLiteral("ColorDialog"));
            QCOMPARE(dialog->widget()->size(), QSize(488, 696));
            QCOMPARE(dialog->scrollWidget()->size(), QSize(440, 560));
            QCOMPARE(dialog->buttonGroup()->size(), QSize(486, 81));
            QCOMPARE(dialog->scrollArea()->verticalScrollBar()->maximum(), 0);
            QCOMPARE(dialog->findChildren<FluentQt::BrightnessSlider *>().size(), 1);
            QVERIFY(dialog->findChildren<QColorDialog *>().isEmpty());

            dialog->redLineEdit()->setFocus();
            dialog->redLineEdit()->clear();
            QTest::keyClicks(dialog->redLineEdit(), QStringLiteral("68"));
            QCOMPARE(card.color(), QColor(QStringLiteral("#112233")));
            QCOMPARE(colorSpy.count(), 0);
            dialog->yesButton()->click();
        });
        QTRY_COMPARE(colorSpy.count(), 1);
        QVERIFY(dialogInspected);
        QCOMPARE(card.color().red(), 68);
        QVERIFY(dialogGuard.isNull() || !dialogGuard->isVisible());
        QCOMPARE(colorSpy.count(), 1);

        FluentQt::CustomColorSettingCard customCard(QColor(QStringLiteral("#005fb8")), QColor(QStringLiteral("#112233")),
                                                    FluentQt::FluentIcon::Heart, QStringLiteral("Custom accent"));
        customCard.setAlphaEnabled(true);
        customCard.show();
        QVERIFY(QTest::qWaitForWindowExposed(&customCard));
        bool alphaDialogInspected = false;
        QTimer::singleShot(0, &customCard, [&customCard]() { customCard.colorPicker()->click(); });
        QTimer::singleShot(100, &customCard, [&]() {
            auto *alphaPopup = findVisibleTopLevelByRole(QStringLiteral("ColorDialogPopup"));
            auto *alphaPopupDialog = qobject_cast<FluentQt::ColorDialog *>(alphaPopup);
            if (!alphaPopupDialog) {
                return;
            }

            alphaDialogInspected = true;
            QCOMPARE(alphaPopupDialog->widget()->size(), QSize(488, 736));
            QCOMPARE(alphaPopupDialog->scrollWidget()->size(), QSize(440, 600));
            QCOMPARE(alphaPopupDialog->buttonGroup()->size(), QSize(486, 81));
            QCOMPARE(alphaPopupDialog->scrollArea()->verticalScrollBar()->maximum(), 0);
            QVERIFY(!alphaPopupDialog->opacityLineEdit()->isHidden());
            alphaPopupDialog->cancelButton()->click();
        });
        QTRY_VERIFY(alphaDialogInspected);
    }

    void dialogsUseFluentFramelessButtons()
    {
        FluentQt::Dialog dialog(QStringLiteral("Title"), QStringLiteral("Message"));
        QCOMPARE(dialog.property("fqw").toString(), QStringLiteral("DialogPopup"));
        QVERIFY(dialog.testAttribute(Qt::WA_TranslucentBackground));
        QVERIFY(dialog.windowFlags() & Qt::FramelessWindowHint);
        QVERIFY(dialog.windowFlags() & Qt::NoDropShadowWindowHint);
        QVERIFY(dialog.findChildren<QDialogButtonBox *>().isEmpty());

        auto *view = dialog.findChild<QFrame *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(view != nullptr);
        QCOMPARE(view->property("fqw").toString(), QStringLiteral("Dialog"));
        QCOMPARE(dialog.view(), view);
        QCOMPARE(dialog.property("view").value<QFrame *>(), dialog.view());
        QCOMPARE(dialog.widget(), dialog.view());
        QCOMPARE(dialog.property("widget").value<QFrame *>(), dialog.widget());
        QVERIFY(qobject_cast<QGraphicsDropShadowEffect *>(view->graphicsEffect()) != nullptr);
        QVERIFY(dialog.titleLabel() != nullptr);
        QVERIFY(dialog.windowTitleLabel() != nullptr);
        QVERIFY(dialog.contentLabel() != nullptr);
        QCOMPARE(dialog.messageLabel(), dialog.contentLabel());
        QCOMPARE(dialog.windowTitleLabel()->objectName(), QStringLiteral("windowTitleLabel"));
        QCOMPARE(dialog.titleLabel()->objectName(), QStringLiteral("titleLabel"));
        QCOMPARE(dialog.contentLabel()->objectName(), QStringLiteral("contentLabel"));
        QCOMPARE(dialog.windowTitleLabel()->text(), QStringLiteral("Title"));
        QCOMPARE(dialog.titleLabel()->text(), QStringLiteral("Title"));
        QCOMPARE(dialog.contentLabel()->text(), QStringLiteral("Message"));
        QVERIFY(!dialog.contentLabel()->wordWrap());
        QCOMPARE(dialog.property("windowTitleLabel").value<QLabel *>(), dialog.windowTitleLabel());
        dialog.setTitleBarVisible(false);
        QVERIFY(dialog.windowTitleLabel()->isHidden());
        dialog.setTitleBarVisible(true);
        QVERIFY(!dialog.windowTitleLabel()->isHidden());
        QVERIFY(dialog.buttonGroup() != nullptr);
        QCOMPARE(dialog.buttonGroup()->objectName(), QStringLiteral("buttonGroup"));
        QCOMPARE(dialog.buttonGroup()->parentWidget(), dialog.view());
        QCOMPARE(dialog.buttonGroup()->height(), 81);
        QVERIFY(dialog.vBoxLayout() != nullptr);
        QVERIFY(dialog.viewLayout() != nullptr);
        QVERIFY(dialog.textLayout() != nullptr);
        QCOMPARE(dialog.viewLayout(), dialog.textLayout());
        QVERIFY(dialog.buttonLayout() != nullptr);
        QCOMPARE(dialog.textLayout()->contentsMargins(), QMargins(24, 24, 24, 24));
        QCOMPARE(dialog.buttonLayout()->contentsMargins(), QMargins(24, 24, 24, 24));
        QCOMPARE(dialog.property("titleLabel").value<QLabel *>(), dialog.titleLabel());
        QCOMPARE(dialog.property("contentLabel").value<QLabel *>(), dialog.contentLabel());
        QCOMPARE(dialog.property("buttonGroup").value<QFrame *>(), dialog.buttonGroup());
        QCOMPARE(dialog.property("vBoxLayout").value<QVBoxLayout *>(), dialog.vBoxLayout());
        QCOMPARE(dialog.property("viewLayout").value<QVBoxLayout *>(), dialog.viewLayout());
        QCOMPARE(dialog.property("textLayout").value<QVBoxLayout *>(), dialog.textLayout());
        QCOMPARE(dialog.property("buttonLayout").value<QHBoxLayout *>(), dialog.buttonLayout());
        QVERIFY(dialog.acceptButton() != nullptr);
        QVERIFY(dialog.cancelButton() != nullptr);
        QCOMPARE(dialog.yesButton(), dialog.acceptButton());
        QCOMPARE(dialog.property("yesButton").value<QPushButton *>(), dialog.yesButton());
        QCOMPARE(dialog.property("cancelButton").value<QPushButton *>(), dialog.cancelButton());
        QCOMPARE(dialog.cancelButton()->objectName(), QStringLiteral("cancelButton"));
        QCOMPARE(dialog.acceptButton()->property("fqw").toString(), QStringLiteral("PrimaryPushButton"));
        QCOMPARE(dialog.cancelButton()->property("fqw").toString(), QStringLiteral("PushButton"));
        dialog.setContentCopyable(true);
        QVERIFY(dialog.contentLabel()->textInteractionFlags() & Qt::TextSelectableByMouse);
        dialog.setContentCopyable(false);
        QCOMPARE(dialog.contentLabel()->textInteractionFlags(), Qt::NoTextInteraction);

        QSignalSpy acceptedSpy(&dialog, &QDialog::accepted);
        QSignalSpy yesSpy(&dialog, &FluentQt::Dialog::yesSignal);
        dialog.acceptButton()->click();
        QCOMPARE(acceptedSpy.count(), 1);
        QCOMPARE(yesSpy.count(), 1);

        FluentQt::Dialog hiddenButtonDialog(QStringLiteral("Title"), QStringLiteral("Message"));
        hiddenButtonDialog.hideYesButton();
        QVERIFY(!hiddenButtonDialog.yesButton()->isVisible());
        hiddenButtonDialog.hideCancelButton();
        QVERIFY(!hiddenButtonDialog.cancelButton()->isVisible());

        FluentQt::MessageBoxBase customBase;
        customBase.viewLayout()->addWidget(new FluentQt::LineEdit(customBase.widget()));
        QCOMPARE(customBase.widget(), customBase.view());
        QVERIFY(customBase.windowMask() != nullptr);
        QCOMPARE(customBase.windowMask()->objectName(), QStringLiteral("windowMask"));

        class ValidatingMessageBox : public FluentQt::MessageBoxBase
        {
          public:
            using FluentQt::MessageBoxBase::MessageBoxBase;
            void setValid(bool valid) { m_valid = valid; }

          protected:
            bool validate() const override { return m_valid; }

          private:
            bool m_valid = false;
        };

        ValidatingMessageBox validatingDialog;
        QSignalSpy validatingAcceptedSpy(&validatingDialog, &QDialog::accepted);
        QSignalSpy validatingYesSpy(&validatingDialog, &FluentQt::MessageBoxBase::yesSignal);
        validatingDialog.yesButton()->click();
        QCOMPARE(validatingAcceptedSpy.count(), 0);
        QCOMPARE(validatingYesSpy.count(), 0);
        validatingDialog.setValid(true);
        validatingDialog.yesButton()->click();
        QTRY_COMPARE(validatingAcceptedSpy.count(), 1);
        QCOMPARE(validatingYesSpy.count(), 1);

        FluentQt::MessageBox messageBox(QStringLiteral("Title"), QStringLiteral("Message"));
        QCOMPARE(messageBox.property("fqw").toString(), QStringLiteral("MessageBoxPopup"));
        messageBox.setClosableOnMaskClicked(true);
        messageBox.setDraggable(true);
        QVERIFY(messageBox.isClosableOnMaskClicked());
        QVERIFY(messageBox.isDraggable());
        QVERIFY(messageBox.windowMask() != nullptr);
        QCOMPARE(messageBox.windowMask()->objectName(), QStringLiteral("windowMask"));
        QCOMPARE(messageBox.maskColor(), QColor(0, 0, 0, 76));
        auto *messageView = messageBox.findChild<QFrame *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(messageView != nullptr);
        QCOMPARE(messageView->property("fqw").toString(), QStringLiteral("MessageBox"));
        QCOMPARE(messageView->objectName(), QStringLiteral("centerWidget"));
        QCOMPARE(messageBox.buttonGroup()->parentWidget(), messageBox.view());
        QCOMPARE(messageBox.titleLabel()->text(), QStringLiteral("Title"));
        QCOMPARE(messageBox.contentLabel()->text(), QStringLiteral("Message"));
        QVERIFY(!messageBox.contentLabel()->wordWrap());

        QWidget dialogParent;
        dialogParent.resize(1000, 640);
        dialogParent.move(80, 90);
        dialogParent.show();
        QVERIFY(QTest::qWaitForWindowExposed(&dialogParent));
        FluentQt::Dialog galleryDialog(
            QStringLiteral("This is a frameless message dialog"),
            QStringLiteral("If the content of the message box is veeeeeeeeeeeeeeeeeeeeeeeeeery long, it will "
                           "automatically wrap like this."),
            &dialogParent);
        galleryDialog.widget()->setFixedSize(240, 192);
        QVERIFY(!galleryDialog.contentLabel()->wordWrap());
        QVERIFY(!galleryDialog.contentLabel()->text().contains(QLatin1Char('\n')));
        galleryDialog.show();
        QVERIFY(QTest::qWaitForWindowExposed(&galleryDialog));
        const QPoint viewCenter = galleryDialog.widget()->mapToGlobal(galleryDialog.widget()->rect().center());
        const QPoint parentCenter = dialogParent.frameGeometry().center();
        QVERIFY(qAbs(viewCenter.x() - parentCenter.x()) <= 2);
        QVERIFY(qAbs(viewCenter.y() - parentCenter.y()) <= 2);
        galleryDialog.close();

        messageBox.show();
        QVERIFY(QTest::qWaitForWindowExposed(&messageBox));
        QSignalSpy messageAcceptedSpy(&messageBox, &QDialog::accepted);
        QSignalSpy messageYesSpy(&messageBox, &FluentQt::MessageBoxBase::yesSignal);
        QSignalSpy rejectedSpy(&messageBox, &QDialog::rejected);
        QSignalSpy cancelSpy(&messageBox, &FluentQt::MessageBoxBase::cancelSignal);
        QTest::mouseMove(messageBox.acceptButton(), messageBox.acceptButton()->rect().center());
        QTest::mouseMove(messageBox.cancelButton(), messageBox.cancelButton()->rect().center());
        QCOMPARE(messageAcceptedSpy.count(), 0);
        QCOMPARE(messageYesSpy.count(), 0);
        QCOMPARE(rejectedSpy.count(), 0);
        QCOMPARE(cancelSpy.count(), 0);
        QTest::mouseClick(messageBox.cancelButton(), Qt::LeftButton);
        QTRY_COMPARE(rejectedSpy.count(), 1);
        QCOMPARE(cancelSpy.count(), 1);

        QWidget maskParent;
        maskParent.resize(520, 360);
        maskParent.show();
        QVERIFY(QTest::qWaitForWindowExposed(&maskParent));

        FluentQt::MessageBox maskedBox(QStringLiteral("Mask"), QStringLiteral("Parent overlay"), &maskParent);
        maskedBox.setClosableOnMaskClicked(true);
        maskedBox.show();
        QVERIFY(QTest::qWaitForWindowExposed(&maskedBox));
        QCOMPARE(maskedBox.geometry(), maskParent.geometry());
        QCOMPARE(maskedBox.windowMask()->size(), maskedBox.size());
        QVERIFY(maskedBox.widget()->geometry().center().x() > 0);
        QVERIFY(maskedBox.widget()->geometry().center().y() > 0);

        maskParent.resize(640, 420);
        QCoreApplication::processEvents();
        QTRY_COMPARE(maskedBox.geometry(), maskParent.geometry());
        QCOMPARE(maskedBox.windowMask()->size(), maskedBox.size());

        QSignalSpy maskRejectedSpy(&maskedBox, &QDialog::rejected);
        QTest::mouseClick(maskedBox.windowMask(), Qt::LeftButton, Qt::NoModifier, QPoint(2, 2));
        QTRY_COMPARE(maskRejectedSpy.count(), 1);

        FluentQt::MessageDialog messageDialog(QStringLiteral("Title"), QStringLiteral("Message"));
        QCOMPARE(messageDialog.property("fqw").toString(), QStringLiteral("MessageDialog"));
        auto *messageDialogView = messageDialog.findChild<QFrame *>(QString(), Qt::FindDirectChildrenOnly);
        QVERIFY(messageDialogView != nullptr);
        QCOMPARE(messageDialogView->property("fqw").toString(), QStringLiteral("MessageDialog"));
    }

    void dialogFadeInDropsOpacityEffectBeforeButtonHover()
    {
        auto *manager = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = manager->theme();
        manager->setTheme(FluentQt::Theme::Light);

        FluentQt::Dialog dialog(QStringLiteral("Title"), QStringLiteral("Message"));
        dialog.show();
        QVERIFY(QTest::qWaitForWindowExposed(&dialog));
        QTRY_VERIFY(dialog.graphicsEffect() == nullptr);

        auto hasPaintedPixel = [](QWidget *widget) {
            const QImage image = widget->grab().toImage();
            for (int y = 0; y < image.height(); ++y) {
                for (int x = 0; x < image.width(); ++x) {
                    const QColor color = image.pixelColor(x, y);
                    if (color.alpha() > 0 &&
                        (color.red() < 245 || color.green() < 245 || color.blue() < 245)) {
                        return true;
                    }
                }
            }
            return false;
        };

        auto hoverKeepsButtonPainted = [&](QPushButton *button) {
            QTest::mouseMove(button, button->rect().center());
            QCoreApplication::processEvents();
            QTest::qWait(20);
            return button->isVisible() && hasPaintedPixel(button);
        };

        QVERIFY2(hoverKeepsButtonPainted(dialog.acceptButton()),
                 "Dialog accept button should stay painted after hover");
        QVERIFY2(hoverKeepsButtonPainted(dialog.cancelButton()),
                 "Dialog cancel button should stay painted after hover");
        QVERIFY(qobject_cast<QGraphicsOpacityEffect *>(dialog.graphicsEffect()) == nullptr);

        manager->setTheme(previousTheme);
    }

    void maskedDialogFadeKeepsNativeWindowOpaque()
    {
        FluentQt::MessageBox dialog(QStringLiteral("Title"), QStringLiteral("Message"));
        dialog.show();
        QCoreApplication::processEvents();

        QCOMPARE(dialog.windowOpacity(), 1.0);
        QVERIFY(qobject_cast<QGraphicsOpacityEffect *>(dialog.graphicsEffect()) != nullptr);
        QTRY_VERIFY(dialog.graphicsEffect() == nullptr);
        QCOMPARE(dialog.windowOpacity(), 1.0);
    }

    void folderListSettingCardTracksFoldersAndDialogDirectory()
    {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());

        FluentQt::FolderListSettingCard card(QStringList(), FluentQt::FluentIcon::Folder, QStringLiteral("Folders"),
                                             QString(), directory.path());
        QSignalSpy folderAddedSpy(&card, &FluentQt::FolderListSettingCard::folderAdded);
        QSignalSpy foldersChangedSpy(&card, &FluentQt::FolderListSettingCard::foldersChanged);

        QCOMPARE(card.dialogDirectory(), directory.path());
        QVERIFY(card.addFolder(directory.path()));
        QCOMPARE(folderAddedSpy.count(), 1);
        QCOMPARE(card.folders(), QStringList({QDir::cleanPath(directory.path())}));
        QCOMPARE(foldersChangedSpy.count(), 1);
        QVERIFY(!card.addFolder(directory.path()));
        QCOMPARE(folderAddedSpy.count(), 1);

        const QString nextDirectory = QDir::homePath();
        card.setDialogDirectory(nextDirectory);
        QCOMPARE(card.dialogDirectory(), nextDirectory);

        FluentQt::FolderListDialog listDialog(QStringList{QStringLiteral("/tmp/music")},
                                              QStringLiteral("Build your collection"),
                                              QStringLiteral("Right now, we're watching these folders:"));
        QCOMPARE(listDialog.folders(), QStringList{QStringLiteral("/tmp/music")});
        QVERIFY(listDialog.scrollArea() != nullptr);
        QVERIFY(listDialog.scrollWidget() != nullptr);
        QVERIFY(listDialog.addFolderButton() != nullptr);
        QVERIFY(listDialog.addFolder(QStringLiteral("/tmp/podcasts")));
        QVERIFY(!listDialog.addFolder(QStringLiteral("/tmp/podcasts")));
        QVERIFY(listDialog.removeFolder(QStringLiteral("/tmp/music")));
        QCOMPARE(listDialog.folders(), QStringList{QStringLiteral("/tmp/podcasts")});
        QSignalSpy changedSpy(&listDialog, &FluentQt::FolderListDialog::folderChanged);
        listDialog.yesButton()->click();
        QCOMPARE(changedSpy.count(), 1);
        QCOMPARE(changedSpy.takeFirst().at(0).toStringList(), QStringList{QStringLiteral("/tmp/podcasts")});
    }

    void itemViewsExposeFluentState()
    {
        FluentQt::ListWidget list;
        list.addItems({QStringLiteral("First"), QStringLiteral("Second")});
        list.setCurrentRow(1);
        list.setSelectRightClickedRow(true);
        list.setCheckedColor(QColor(QStringLiteral("#010203")), QColor(QStringLiteral("#aabbcc")));
        QCOMPARE(list.property("fqw").toString(), QStringLiteral("ListWidget"));
        QCOMPARE(list.currentRow(), 1);
        QVERIFY(list.isSelectRightClickedRow());
        QCOMPARE(list.delegate(), list.fluentItemDelegate());
        QCOMPARE(list.property("delegate").value<FluentQt::ListItemDelegate *>(), list.delegate());
        QCOMPARE(list.property("scrollDelegate").value<FluentQt::SmoothScrollDelegate *>(), list.scrollDelegate());
        QCOMPARE(list.fluentItemDelegate()->checkedColor(FluentQt::Theme::Light), QColor(QStringLiteral("#010203")));
        QCOMPARE(list.fluentItemDelegate()->checkedColor(FluentQt::Theme::Dark), QColor(QStringLiteral("#aabbcc")));

        FluentQt::ListView listView;
        QStringListModel listModel({QStringLiteral("First"), QStringLiteral("Second")});
        listView.setModel(&listModel);
        QCOMPARE(listView.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QCOMPARE(listView.scrollDelegate()->verticalScrollBar()->parentWidget(), &listView);

        FluentQt::TableWidget table;
        table.setColumnCount(2);
        table.setRowCount(1);
        table.setItem(0, 0, new QTableWidgetItem(QStringLiteral("Component")));
        table.setItem(0, 1, new QTableWidgetItem(QStringLiteral("Ready")));
        table.setBorderVisible(true);
        table.setCurrentCell(0, 0);
        QCOMPARE(table.property("fqw").toString(), QStringLiteral("TableWidget"));
        QVERIFY(table.isBorderVisible());
        QVERIFY(table.alternatingRowColors());
        QCOMPARE(table.selectionBehavior(), QAbstractItemView::SelectRows);
        QCOMPARE(table.horizontalHeader()->sectionResizeMode(0), QHeaderView::Interactive);
        QCOMPARE(table.item(0, 1)->text(), QStringLiteral("Ready"));
        QCOMPARE(table.delegate(), table.fluentItemDelegate());
        QCOMPARE(table.scrollDelagate(), table.scrollDelegate());
        QCOMPARE(table.property("delegate").value<FluentQt::TableItemDelegate *>(), table.delegate());
        QCOMPARE(table.property("scrollDelagate").value<FluentQt::SmoothScrollDelegate *>(), table.scrollDelegate());
        table.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        table.setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        QCOMPARE(table.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QCOMPARE(table.horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QVERIFY(!table.scrollDelegate()->verticalScrollBar()->isForceHidden());
        QVERIFY(!table.scrollDelegate()->horizontalScrollBar()->isForceHidden());
        table.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        table.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        QVERIFY(table.scrollDelegate()->verticalScrollBar()->isForceHidden());
        QVERIFY(table.scrollDelegate()->horizontalScrollBar()->isForceHidden());
        table.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        table.setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        QStyleOptionViewItem editorOption;
        editorOption.text = QStringLiteral("Component");
        QWidget *editor = table.delegate()->createEditor(&table, editorOption, table.model()->index(0, 0));
        auto *lineEditor = qobject_cast<FluentQt::LineEdit *>(editor);
        QVERIFY(lineEditor != nullptr);
        QCOMPARE(lineEditor->text(), QStringLiteral("Component"));
        QCOMPARE(lineEditor->property("transparent").toBool(), false);
        QVERIFY(lineEditor->isClearButtonEnabled());
        delete editor;

        FluentQt::TreeWidget tree;
        tree.setHeaderLabels({QStringLiteral("Area"), QStringLiteral("Status")});
        auto *root = new QTreeWidgetItem(&tree, QStringList{QStringLiteral("Views"), QStringLiteral("MVP")});
        root->addChild(new QTreeWidgetItem(QStringList{QStringLiteral("ListWidget"), QStringLiteral("Ready")}));
        tree.setBorderVisible(true);
        QCOMPARE(tree.property("fqw").toString(), QStringLiteral("TreeWidget"));
        QVERIFY(tree.isBorderVisible());
        QCOMPARE(tree.iconSize(), QSize(16, 16));
        QCOMPARE(tree.topLevelItemCount(), 1);
        QCOMPARE(tree.topLevelItem(0)->child(0)->text(0), QStringLiteral("ListWidget"));
        QCOMPARE(tree.delegate(), tree.fluentItemDelegate());
        QCOMPARE(tree.scrollDelagate(), tree.scrollDelegate());
        QCOMPARE(tree.property("delegate").value<FluentQt::TreeItemDelegate *>(), tree.delegate());
        QCOMPARE(tree.property("scrollDelagate").value<FluentQt::SmoothScrollDelegate *>(), tree.scrollDelegate());
        tree.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        QCOMPARE(tree.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QVERIFY(!tree.scrollDelegate()->verticalScrollBar()->isForceHidden());
    }

    void treeHoverDoesNotBleedAcrossParents()
    {
        const FluentQt::Theme previousTheme = FluentQt::ThemeManager::instance()->theme();
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        FluentQt::TreeWidget tree;
        tree.setColumnCount(1);

        auto *firstParent = new QTreeWidgetItem(&tree, QStringList{QStringLiteral("First parent")});
        auto *firstChild = new QTreeWidgetItem(firstParent, QStringList{QStringLiteral("Same row child A")});
        auto *secondParent = new QTreeWidgetItem(&tree, QStringList{QStringLiteral("Second parent")});
        auto *secondChild = new QTreeWidgetItem(secondParent, QStringList{QStringLiteral("Same row child B")});

        const QModelIndex firstChildIndex = tree.indexFromItem(firstChild);
        const QModelIndex secondChildIndex = tree.indexFromItem(secondChild);
        QCOMPARE(firstChildIndex.row(), secondChildIndex.row());
        QVERIFY(firstChildIndex.parent() != secondChildIndex.parent());

        tree.delegate()->setHoverIndex(secondChildIndex);

        auto renderIndexSample = [&tree](const QModelIndex &index) {
            QImage image(QSize(180, 30), QImage::Format_ARGB32_Premultiplied);
            image.fill(Qt::white);

            QStyleOptionViewItem option;
            option.rect = image.rect();
            option.state = QStyle::State_Enabled;
            option.widget = &tree;

            QPainter painter(&image);
            tree.delegate()->paint(&painter, option, index);
            painter.end();
            return image.pixelColor(165, image.height() / 2);
        };

        const QColor firstColor = renderIndexSample(firstChildIndex);
        const QColor secondColor = renderIndexSample(secondChildIndex);

        QVERIFY2(secondColor.red() + 4 < firstColor.red(),
                 qPrintable(QStringLiteral("first=%1 second=%2").arg(firstColor.name(QColor::HexArgb),
                                                                     secondColor.name(QColor::HexArgb))));

        FluentQt::ThemeManager::instance()->setTheme(previousTheme);
    }

    void treeCheckBoxPaintsInsideStyleIndicatorRect()
    {
        const FluentQt::Theme previousTheme = FluentQt::ThemeManager::instance()->theme();
        FluentQt::ThemeManager::instance()->setTheme(FluentQt::Theme::Light);

        FluentQt::TreeWidget tree;
        tree.setColumnCount(1);
        auto *item = new QTreeWidgetItem(&tree, QStringList{QStringLiteral("Tree text")});
        item->setCheckState(0, Qt::Unchecked);
        const QModelIndex index = tree.indexFromItem(item);

        QStyleOptionViewItem option;
        option.rect = QRect(40, 0, 140, 32);
        option.state = QStyle::State_Enabled;
        option.widget = &tree;
        tree.delegate()->initStyleOption(&option, index);

        const QRect indicatorRect =
            tree.style()->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &option, &tree);
        const QRect textRect = tree.style()->subElementRect(QStyle::SE_ItemViewItemText, &option, &tree);
        QVERIFY(indicatorRect.isValid());
        QVERIFY(textRect.isValid());
        QVERIFY2(indicatorRect.right() < textRect.left(),
                 qPrintable(QStringLiteral("indicator=%1 text=%2").arg(QString::number(indicatorRect.right()),
                                                                        QString::number(textRect.left()))));

        QImage image(QSize(220, 36), QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::white);

        QPainter painter(&image);
        tree.delegate()->paint(&painter, option, index);
        painter.end();

        QVERIFY(image.rect().contains(indicatorRect.center()));
        QVERIFY(image.pixelColor(indicatorRect.center()) != QColor(Qt::white));

        FluentQt::ThemeManager::instance()->setTheme(previousTheme);
    }

    void listViewsUseFluentOverlayScrollBars()
    {
        auto populate = [](auto *view) {
            for (int i = 0; i < 80; ++i) {
                view->addItem(QStringLiteral("Item %1").arg(i));
            }
            view->resize(260, 180);
            view->show();
            QVERIFY(QTest::qWaitForWindowExposed(view));
            QCoreApplication::processEvents();
        };

        FluentQt::ListWidget listWidget;
        populate(&listWidget);
        QCOMPARE(listWidget.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QCOMPARE(listWidget.scrollDelegate()->verticalScrollBar()->parentWidget(), &listWidget);
        QVERIFY(listWidget.scrollDelegate()->verticalScrollBar()->maximum() > 0);
        QVERIFY(!listWidget.verticalScrollBar()->isVisible());
        QVERIFY(listWidget.scrollDelegate()->verticalScrollBar()->isVisible());

        listWidget.scrollDelegate()->verticalScrollBar()->setValue(24);
        QCOMPARE(listWidget.verticalScrollBar()->value(), 24);

        QStringListModel model;
        QStringList items;
        for (int i = 0; i < 80; ++i) {
            items.append(QStringLiteral("Item %1").arg(i));
        }
        model.setStringList(items);

        FluentQt::ListView listView;
        listView.setModel(&model);
        listView.resize(260, 180);
        listView.show();
        QVERIFY(QTest::qWaitForWindowExposed(&listView));
        QCoreApplication::processEvents();

        QCOMPARE(listView.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        QCOMPARE(listView.scrollDelegate()->verticalScrollBar()->parentWidget(), &listView);
        QVERIFY(listView.scrollDelegate()->verticalScrollBar()->maximum() > 0);
        QVERIFY(!listView.verticalScrollBar()->isVisible());
        QVERIFY(listView.scrollDelegate()->verticalScrollBar()->isVisible());

        listView.scrollDelegate()->verticalScrollBar()->setValue(24);
        QCOMPARE(listView.verticalScrollBar()->value(), 24);
    }

    void scrollAreasExposeFluentPolicies()
    {
        FluentQt::ScrollBar bar(Qt::Vertical);
        QCOMPARE(bar.property("fqw").toString(), QStringLiteral("ScrollBar"));
        QVERIFY(bar.groove() != nullptr);
        QVERIFY(bar.handle() != nullptr);
        QVERIFY(bar.upButton() != nullptr);
        QVERIFY(bar.downButton() != nullptr);
        QCOMPARE(bar.groove()->property("fqw").toString(), QStringLiteral("ScrollBarGroove"));
        QCOMPARE(bar.handle()->property("fqw").toString(), QStringLiteral("ScrollBarHandle"));
        QCOMPARE(bar.upButton()->property("fqw").toString(), QStringLiteral("ArrowButton"));
        QCOMPARE(bar.groove()->orientation(), Qt::Vertical);
        QCOMPARE(bar.handle()->orientation(), Qt::Vertical);
        QCOMPARE(bar.upButton()->iconType(), FluentQt::FluentIcon::Up);
        QCOMPARE(bar.downButton()->iconType(), FluentQt::FluentIcon::ArrowDown);
        QCOMPARE(bar.groove()->opacity(), 0.0);
        QCOMPARE(bar.handle()->opacity(), 1.0);
        bar.setHandleDisplayMode(FluentQt::ScrollBarHandleDisplayMode::OnHover);
        QCOMPARE(bar.handleDisplayMode(), FluentQt::ScrollBarHandleDisplayMode::OnHover);
        QCOMPARE(bar.property("handleDisplayMode").toString(), QStringLiteral("OnHover"));
        QCOMPARE(bar.property("expanded").toBool(), false);
        bar.setHandleColor(QColor(QStringLiteral("#010203")), QColor(QStringLiteral("#aabbcc")));
        bar.setArrowColor(QColor(QStringLiteral("#111213")), QColor(QStringLiteral("#ddeeff")));
        bar.setGrooveColor(QColor(252, 252, 252, 217), QColor(44, 44, 44, 245));
        QCOMPARE(bar.handle()->lightColor(), QColor(QStringLiteral("#010203")));
        QCOMPARE(bar.handle()->darkColor(), QColor(QStringLiteral("#aabbcc")));
        QCOMPARE(bar.upButton()->lightColor(), QColor(QStringLiteral("#111213")));
        QCOMPARE(bar.downButton()->darkColor(), QColor(QStringLiteral("#ddeeff")));
        QCOMPARE(bar.groove()->lightBackgroundColor(), QColor(252, 252, 252, 217));
        QCOMPARE(bar.handleColor(FluentQt::Theme::Light), QColor(QStringLiteral("#010203")));
        QCOMPARE(bar.handleColor(FluentQt::Theme::Dark), QColor(QStringLiteral("#aabbcc")));
        QCOMPARE(bar.arrowColor(FluentQt::Theme::Light), QColor(QStringLiteral("#111213")));
        QCOMPARE(bar.arrowColor(FluentQt::Theme::Dark), QColor(QStringLiteral("#ddeeff")));
        bar.resize(12, 120);
        bar.setRange(0, 100);
        bar.setPageStep(20);
        bar.expand();
        QCOMPARE(bar.property("expanded").toBool(), true);
        QTRY_VERIFY(bar.groove()->opacity() > 0.0);
        bar.setValue(50);
        QVERIFY(bar.handle()->geometry().height() >= 30);
        QVERIFY(bar.handle()->geometry().y() > 0);
        QTest::mouseClick(bar.downButton(), Qt::LeftButton);
        QCOMPARE(bar.value(), 70);
        QTest::mouseClick(bar.upButton(), Qt::LeftButton);
        QCOMPARE(bar.value(), 50);
        bar.collapse();
        QCOMPARE(bar.property("expanded").toBool(), false);
        QVERIFY(FluentQt::StyleSheetManager::instance()
                    ->customStyleSheet(&bar, FluentQt::Theme::Light)
                    .contains(QStringLiteral("rgba(1, 2, 3")));
        bar.setForceHidden(true);
        QVERIFY(bar.isForceHidden());

        FluentQt::ScrollArea area;
        QVERIFY(area.verticalFluentScrollBar() != nullptr);
        QVERIFY(area.horizontalFluentScrollBar() != nullptr);
        QCOMPARE(area.vScrollBar(), area.verticalFluentScrollBar());
        QCOMPARE(area.hScrollBar(), area.horizontalFluentScrollBar());
        QCOMPARE(area.scrollDelagate(), area.scrollDelegate());
        QCOMPARE(area.property("vScrollBar").value<FluentQt::ScrollBar *>(), area.vScrollBar());
        QCOMPARE(area.property("hScrollBar").value<FluentQt::ScrollBar *>(), area.hScrollBar());
        QCOMPARE(area.property("scrollDelagate").value<FluentQt::SmoothScrollDelegate *>(), area.scrollDelegate());
        QCOMPARE(area.scrollDelegate()->vScrollBar(), area.scrollDelegate()->verticalScrollBar());
        QCOMPARE(area.scrollDelegate()->hScrollBar(), area.scrollDelegate()->horizontalScrollBar());
        QCOMPARE(area.property("fqw").toString(), QStringLiteral("ScrollArea"));
        area.setViewportMargins(1, 2, 3, 4);
        QCOMPARE(area.viewportMargins(), QMargins(1, 2, 3, 4));
        area.setViewportMargins(QMargins(5, 6, 7, 8));
        QCOMPARE(area.viewportMargins(), QMargins(5, 6, 7, 8));
        area.enableTransparentBackground();
        QCOMPARE(area.property("transparent").toBool(), true);
        QCOMPARE(area.viewport()->property("transparent").toBool(), true);
        auto *transparentContent = new QWidget;
        area.setWidget(transparentContent);
        QCOMPARE(transparentContent->property("transparent").toBool(), true);
        QVERIFY(!transparentContent->autoFillBackground());
        area.enableTransparentBackground(false);
        auto *opaqueContent = new QWidget;
        area.setWidget(opaqueContent);
        QCOMPARE(opaqueContent->property("transparent").toBool(), false);
        QVERIFY(opaqueContent->autoFillBackground());
        area.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        QVERIFY(area.verticalFluentScrollBar()->isForceHidden());
        area.setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        QVERIFY(!area.verticalFluentScrollBar()->isForceHidden());

        FluentQt::SingleDirectionScrollArea single(Qt::Vertical);
        QCOMPARE(single.orientation(), Qt::Vertical);
        QCOMPARE(single.horizontalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);
        single.setOrientation(Qt::Horizontal);
        QCOMPARE(single.orientation(), Qt::Horizontal);
        QCOMPARE(single.verticalScrollBarPolicy(), Qt::ScrollBarAlwaysOff);

        FluentQt::SmoothScrollArea smooth;
        QCOMPARE(smooth.delegate(), smooth.scrollDelegate());
        QCOMPARE(smooth.property("delegate").value<FluentQt::SmoothScrollDelegate *>(), smooth.delegate());
        smooth.setScrollAnimation(Qt::Horizontal, 240, QEasingCurve::OutQuad);
        QCOMPARE(smooth.scrollAnimationDuration(Qt::Horizontal), 240);
        QCOMPARE(smooth.scrollAnimationEasing(Qt::Horizontal), QEasingCurve::OutQuad);

        const QStringList scrollQssPaths = {
            QStringLiteral(":/qfluentwidgets/qss/light/scroll_bar.qss"),
            QStringLiteral(":/qfluentwidgets/qss/dark/scroll_bar.qss"),
        };
        for (const QString &path : scrollQssPaths) {
            QFile file(path);
            QVERIFY2(file.open(QIODevice::ReadOnly | QIODevice::Text), qPrintable(path));
            const QString qss = QString::fromUtf8(file.readAll());
            QVERIFY2(qss.contains(QStringLiteral("QWidget[fqw=\"ScrollBar\"]")), qPrintable(path));
            QVERIFY2(!qss.contains(QStringLiteral("QScrollBar[fqw=\"ScrollBar\"]")), qPrintable(path));
            QVERIFY2(!qss.contains(QStringLiteral("::handle")), qPrintable(path));
        }
    }

    void roundMenuAcceptsExternalActions()
    {
        FluentQt::RoundMenu menu;
        QAction external(FluentQt::icon(FluentQt::FluentIcon::Setting), QStringLiteral("Settings"), &menu);
        external.setShortcut(QKeySequence(QStringLiteral("Ctrl+,")));
        external.setEnabled(false);

        menu.addAction(&external);
        QCOMPARE(menu.menuActions(), QList<QAction *>({&external}));
        QCOMPARE(menu.view()->count(), 1);
        auto *item = menu.view()->item(0);
        QVERIFY(item != nullptr);
        QCOMPARE(external.property("item").value<QListWidgetItem *>(), item);
        QCOMPARE(item->flags(), Qt::NoItemFlags);

        external.setText(QStringLiteral("Preferences"));
        external.setEnabled(true);
        QVERIFY(item->text().contains(QStringLiteral("Preferences")));
        QVERIFY(item->flags() & Qt::ItemIsEnabled);
        QVERIFY(item->flags() & Qt::ItemIsSelectable);

        menu.removeAction(&external);
        QCOMPARE(menu.menuActions().size(), 0);
        QCOMPARE(menu.view()->count(), 0);
        QVERIFY(!external.property("item").isValid());

        {
            FluentQt::RoundMenu scopedMenu;
            scopedMenu.addAction(&external);
            QVERIFY(scopedMenu.view()->item(0) != nullptr);
            QCOMPARE(external.property("item").value<QListWidgetItem *>(), scopedMenu.view()->item(0));
        }
        QVERIFY(!external.property("item").isValid());

        auto *menuWithExternalLifetime = new FluentQt::RoundMenu;
        auto *shortLivedAction = new QAction(QStringLiteral("Short lived"));
        menuWithExternalLifetime->addAction(shortLivedAction);
        QCOMPARE(menuWithExternalLifetime->menuActions().size(), 1);
        delete shortLivedAction;
        QCOMPARE(menuWithExternalLifetime->menuActions().size(), 0);
        QCOMPARE(menuWithExternalLifetime->view()->count(), 0);
        delete menuWithExternalLifetime;
    }

    void tabWidgetsTrackPagesAndRoutes()
    {
        FluentQt::TabBar bar;
        QCOMPARE(bar.addTab(QStringLiteral("home"), QStringLiteral("Home")), 0);
        QCOMPARE(bar.addTab(QStringLiteral("settings"), QStringLiteral("Settings")), 1);
        QCOMPARE(bar.addTab(QStringLiteral("home"), QStringLiteral("Duplicate")), -1);
        QCOMPARE(bar.count(), 2);
        QCOMPARE(bar.currentRouteKey(), QStringLiteral("home"));
        QCOMPARE(bar.routeKeys(), QList<QString>({QStringLiteral("home"), QStringLiteral("settings")}));
        QCOMPARE(bar.tabMaximumWidth(), 240);
        bar.setTabMaximumWidth(180);
        QCOMPARE(bar.tabMaximumWidth(), 180);
        QCOMPARE(bar.tabItem(0)->maximumWidth(), 180);
        QVERIFY(bar.tabItem(0)->minimumWidth() < bar.tabMaximumWidth());
        bar.setScrollable(true);
        QVERIFY(bar.isScrollable());
        QCOMPARE(bar.tabItem(0)->minimumWidth(), 180);
        QCOMPARE(bar.tabItem(1)->minimumWidth(), 180);
        bar.setTabMaximumWidth(200);
        QCOMPARE(bar.tabItem(0)->maximumWidth(), 200);
        QCOMPARE(bar.tabItem(0)->minimumWidth(), 200);

        QSignalSpy currentSpy(&bar, &FluentQt::TabBar::currentChanged);
        bar.setCurrentTab(QStringLiteral("settings"));
        QCOMPARE(bar.currentIndex(), 1);
        QCOMPARE(bar.currentTab(), bar.tabItem(1));
        QCOMPARE(currentSpy.count(), 1);

        bar.setTabData(1, QVariant(QStringLiteral("payload")));
        QCOMPARE(bar.tabData(1).toString(), QStringLiteral("payload"));

        QSignalSpy closeSpy(&bar, &FluentQt::TabBar::tabCloseRequested);
        QVERIFY(bar.tabItem(1)->closeButton() != nullptr);
        bar.tabItem(1)->closeButton()->click();
        QCOMPARE(closeSpy.count(), 1);
        QCOMPARE(closeSpy.takeFirst().at(0).toInt(), 1);

        bar.removeTab(0);
        QCOMPARE(bar.count(), 1);
        QCOMPARE(bar.currentIndex(), 0);
        QCOMPARE(bar.currentRouteKey(), QStringLiteral("settings"));

        FluentQt::TabWidget tabs;
        QVERIFY(!tabs.isMovable());
        tabs.setMovable(true);
        QVERIFY(tabs.isMovable());
        auto *first = new QWidget;
        auto *second = new QWidget;
        QPointer<QWidget> firstPointer(first);

        QCOMPARE(tabs.addTab(first, QStringLiteral("First"), QIcon(), QStringLiteral("first")), 0);
        QCOMPARE(tabs.addTab(second, QStringLiteral("Second"), QIcon(), QStringLiteral("second")), 1);
        auto *duplicate = new QWidget;
        QCOMPARE(tabs.addTab(duplicate, QStringLiteral("Duplicate"), QIcon(), QStringLiteral("first")), -1);
        delete duplicate;

        QCOMPARE(tabs.count(), 2);
        QCOMPARE(tabs.currentWidget(), first);
        tabs.setCurrentIndex(1);
        QCOMPARE(tabs.currentWidget(), second);
        QCOMPARE(tabs.routeKey(1), QStringLiteral("second"));

        QSignalSpy addSpy(tabs.tabBar(), &FluentQt::TabBar::tabAddRequested);
        tabs.tabBar()->addButton()->click();
        QCOMPARE(addSpy.count(), 1);

        QSignalSpy closeWidgetSpy(&tabs, &FluentQt::TabWidget::tabCloseRequested);
        tabs.tabBar()->tabItem(1)->closeButton()->click();
        QCOMPARE(closeWidgetSpy.count(), 1);
        QCOMPARE(closeWidgetSpy.takeFirst().at(0).toInt(), 1);

        tabs.removeTab(0);
        QCOMPARE(tabs.count(), 1);
        QVERIFY(!firstPointer.isNull());
        first->deleteLater();
    }

    void segmentedToggleToolWidgetTracksCurrentItem()
    {
        FluentQt::SegmentedToggleToolWidget tools;
        auto *first = tools.addItem(QStringLiteral("first"), QIcon());
        auto *second = tools.addItem(QStringLiteral("second"), QIcon());

        QVERIFY(first != nullptr);
        QVERIFY(second != nullptr);
        QVERIFY(tools.styleSheet().contains(QStringLiteral("SegmentedToolWidget")));
        QVERIFY(first->styleSheet().contains(QStringLiteral("SegmentedToolItem")));
        QCOMPARE(first->size(), QSize(50, 32));
        QCOMPARE(second->size(), QSize(50, 32));
        QCOMPARE(tools.routeKeys(), QList<QString>({QStringLiteral("first"), QStringLiteral("second")}));
        QCOMPARE(tools.currentItem(), QStringLiteral("first"));
        QVERIFY(first->isChecked());
        QTRY_COMPARE(tools.selectionGeometry(), first->geometry());

        QSignalSpy currentSpy(&tools, &FluentQt::SegmentedToggleToolWidget::currentItemChanged);
        tools.setCurrentItem(QStringLiteral("second"));
        QCOMPARE(tools.currentItem(), QStringLiteral("second"));
        QVERIFY(second->isChecked());
        QCOMPARE(currentSpy.count(), 1);
        QTRY_COMPARE(tools.selectionGeometry(), second->geometry());

        tools.removeItem(QStringLiteral("second"));
        QCOMPARE(tools.currentItem(), QStringLiteral("first"));
        QVERIFY(!tools.contains(QStringLiteral("second")));
    }

  private:
    static QWidget *findVisibleTopLevelByRole(const QString &role)
    {
        const auto widgets = QApplication::topLevelWidgets();
        for (QWidget *widget : widgets) {
            if (widget->isVisible() && widget->property("fqw").toString() == role) {
                return widget;
            }
        }
        return nullptr;
    }
};

QTEST_MAIN(ThemeTest)

#include "tst_theme.moc"
