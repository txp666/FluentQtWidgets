// Fluent icons, themed icons and IconWidget.

#include <FluentQtWidgets/FluentQtWidgets.h>
#include <QtCore/QFile>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QtGui/QPixmap>
#include <QtTest/QtTest>

#include "TestSupport.h"

#include <optional>

using namespace FluentQt;

class IconTest : public QObject
{
    Q_OBJECT

  private slots:
    void init() { m_globalState.emplace(); }

    void cleanup()
    {
        FqwTest::drainDeferredDeletes();
        m_globalState.reset();
    }

    void themedIconFollowsThemeChanges()
    {
        // One file per theme (the black and the white Add glyph); a QIcon of either file keeps
        // its colour, the themed icon has to switch when the theme does.
        auto *manager = FluentQt::ThemeManager::instance();
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
        // Python's IconWidget has no icon size of its own and draws its icon over the whole widget.
        QVERIFY(!widget.iconSize().isValid());
        {
            QPixmap red(64, 64);
            red.fill(Qt::red);
            FluentQt::IconWidget filled{QIcon(red)};
            filled.resize(40, 40);
            const QImage image = filled.grab().toImage();
            QCOMPARE(image.pixelColor(1, 1), QColor(Qt::red));
            QCOMPARE(image.pixelColor(38, 38), QColor(Qt::red));
        }

        QSignalSpy iconSpy(&widget, &FluentQt::IconWidget::iconChanged);
        widget.setIcon(FluentQt::FluentIcon::Setting);
        QCOMPARE(iconSpy.count(), 1);

        widget.setIconSize(QSize(32, 24));
        QCOMPARE(widget.iconSize(), QSize(32, 24));
        QCOMPARE(widget.sizeHint(), QSize(32, 24));

        QCOMPARE(FluentQt::iconName(FluentQt::FluentIcon::Basketball), QStringLiteral("basketball"));
    }

  private:
    std::optional<FqwTest::GlobalState> m_globalState;
};

QTEST_MAIN(IconTest)
#include "tst_icon.moc"
