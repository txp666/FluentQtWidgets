// Theme manager, colours, icons, style sheet manager and translations.

#include <algorithm>
#include <FluentQtWidgets/FluentQtWidgets.h>
#include <functional>
#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QEasingCurve>
#include <QtCore/QFile>
#include <QtCore/QPointer>
#include <QtCore/QScopedPointer>
#include <QtCore/QStringListModel>
#include <QtCore/QTemporaryDir>
#include <QtCore/QTimer>
#include <QtGui/QAction>
#include <QtGui/QClipboard>
#include <QtGui/QColor>
#include <QtGui/QEnterEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QImage>
#include <QtGui/QKeyEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPalette>
#include <QtGui/QPixmap>
#include <QtGui/QScreen>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>
#include <QtNetwork/QHostAddress>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QtTest/QtTest>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QAbstractScrollArea>
#include <QtWidgets/QApplication>
#include <QtWidgets/QColorDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QGraphicsOpacityEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QProxyStyle>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOptionSlider>
#include <QtWidgets/QTableWidgetItem>
#include <QtWidgets/QTreeWidgetItem>
#include <QtWidgets/QVBoxLayout>
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

    void repositoryLinksUseConfiguredProjectUrl()
    {
        QCOMPARE(FluentQt::repositoryUrl(), QStringLiteral(FQW_REPOSITORY_URL));
        QCOMPARE(FluentQt::repositoryIssuesUrl(), QStringLiteral(FQW_REPOSITORY_URL "/issues"));
        QCOMPARE(FluentQt::repositoryDocsUrl(), QStringLiteral(FQW_REPOSITORY_URL "/tree/main/docs"));
        QCOMPARE(FluentQt::repositoryExampleUrl(), QStringLiteral(FQW_REPOSITORY_URL "/tree/main/examples"));
        QVERIFY(!FluentQt::repositoryUrl().contains(QStringLiteral("github.com/FluentQtWidgets/FluentQtWidgets")));
    }

    void themedIconFollowsThemeChanges()
    {
        // One file per theme (the black and the white Add glyph); a QIcon of either file keeps
        // its colour, the themed icon has to switch when the theme does.
        auto *manager = FluentQt::ThemeManager::instance();
        const FluentQt::Theme previousTheme = manager->theme();
        const QString lightPath = FluentQt::iconPath(FluentQt::FluentIcon::Add, FluentQt::Theme::Light);
        const QString darkPath = FluentQt::iconPath(FluentQt::FluentIcon::Add, FluentQt::Theme::Dark);
        QVERIFY(lightPath != darkPath);

        const QIcon themed = FluentQt::themedIcon(lightPath, darkPath);
        const QImage lightImage = QIcon(lightPath).pixmap(QSize(24, 24), 1.0).toImage();
        const QImage darkImage = QIcon(darkPath).pixmap(QSize(24, 24), 1.0).toImage();
        QVERIFY(lightImage != darkImage);

        manager->setTheme(FluentQt::Theme::Light);
        QCOMPARE(themed.pixmap(QSize(24, 24), 1.0).toImage(), lightImage);
        manager->setTheme(FluentQt::Theme::Dark);
        QCOMPARE(themed.pixmap(QSize(24, 24), 1.0).toImage(), darkImage);
        QVERIFY(!themed.isNull());

        manager->setTheme(previousTheme);
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

    void iconWidgetTracksIconAndSize()
    {
        FluentQt::IconWidget widget;
        QCOMPARE(widget.property("fqw").toString(), QStringLiteral("IconWidget"));
        QCOMPARE(widget.iconSize(), QSize(20, 20));

        QSignalSpy iconSpy(&widget, &FluentQt::IconWidget::iconChanged);
        widget.setIcon(FluentQt::FluentIcon::Setting);
        QCOMPARE(iconSpy.count(), 1);

        widget.setIconSize(QSize(32, 24));
        QCOMPARE(widget.iconSize(), QSize(32, 24));
        QCOMPARE(widget.sizeHint(), QSize(32, 24));

        QCOMPARE(FluentQt::iconName(FluentQt::FluentIcon::Basketball), QStringLiteral("basketball"));
        QCOMPARE(FluentQt::iconResourceName(FluentQt::FluentIcon::Basketball), QStringLiteral("Basketball"));
        QVERIFY(QFile::exists(FluentQt::iconPath(FluentQt::FluentIcon::Basketball, FluentQt::Theme::Light)));
        QVERIFY(!FluentQt::icon(FluentQt::FluentIcon::Basketball).isNull());

    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(ThemeTest)
#include "tst_theme.moc"
