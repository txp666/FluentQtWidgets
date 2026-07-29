#include <FluentQtWidgets/FluentIcon.h>

#include <QtCore/QFile>
#include <QtCore/QSize>
#include <QtCore/QtMath>
#include <QtGui/QGuiApplication>
#include <QtGui/QIconEngine>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QPixmapCache>
#include <QtGui/QScreen>
#include <QtSvg/QSvgRenderer>

namespace FluentQt {

class FluentIconEngine final : public QIconEngine
{
  public:
    FluentIconEngine(FluentIcon icon, Theme theme, QColor tint = {}) : m_icon(icon), m_theme(theme), m_tint(tint) {}

    QIconEngine *clone() const override { return new FluentIconEngine(m_icon, m_theme, m_tint); }

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override
    {
        Q_UNUSED(mode)
        Q_UNUSED(state)

        if (!painter || !rect.isValid()) {
            return;
        }

        const qreal dpr = painter->device() ? painter->device()->devicePixelRatioF() : screenDevicePixelRatio();
        painter->drawPixmap(rect, renderPixmap(rect.size(), dpr));
    }

    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override
    {
        Q_UNUSED(mode)
        Q_UNUSED(state)

        return renderPixmap(size, screenDevicePixelRatio());
    }

    QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale) override
    {
        Q_UNUSED(mode)
        Q_UNUSED(state)

        return renderPixmap(size, scale);
    }

  private:
    static qreal normalizedDevicePixelRatio(qreal devicePixelRatio)
    {
        return qIsFinite(devicePixelRatio) && devicePixelRatio > 0.0 ? devicePixelRatio : 1.0;
    }

    static qreal screenDevicePixelRatio()
    {
        if (!QGuiApplication::instance()) {
            return 1.0;
        }

        const auto *screen = QGuiApplication::primaryScreen();
        return normalizedDevicePixelRatio(screen ? screen->devicePixelRatio() : 1.0);
    }

    QPixmap renderPixmap(const QSize &logicalSize, qreal devicePixelRatio) const
    {
        if (!logicalSize.isValid() || logicalSize.isEmpty()) {
            return {};
        }

        const QString resourcePath = iconPath(m_icon, m_theme);
        if (!QFile::exists(resourcePath)) {
            return {};
        }

        const qreal dpr = normalizedDevicePixelRatio(devicePixelRatio);
        const QSize deviceSize(qMax(1, qCeil(logicalSize.width() * dpr)), qMax(1, qCeil(logicalSize.height() * dpr)));
        const QString cacheKey =
            QStringLiteral("FluentQt.FluentIcon/%1/%2x%3/%4/%5")
                .arg(resourcePath)
                .arg(deviceSize.width())
                .arg(deviceSize.height())
                .arg(qRound64(dpr * 1000.0))
                .arg(m_tint.isValid() ? QString::number(m_tint.rgba(), 16) : QStringLiteral("original"));
        QPixmap cached;
        if (QPixmapCache::find(cacheKey, &cached)) {
            return cached;
        }

        QImage source(deviceSize, QImage::Format_ARGB32_Premultiplied);
        source.fill(Qt::transparent);

        QPainter painter(&source);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        QSvgRenderer renderer(resourcePath);
        renderer.render(&painter, QRectF(QPointF(0, 0), QSizeF(deviceSize)));

        if (m_tint.isValid()) {
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(source.rect(), m_tint);
        }
        painter.end();

        QRect visibleBounds;
        for (int y = 0; y < source.height(); ++y) {
            const QRgb *line = reinterpret_cast<const QRgb *>(source.constScanLine(y));
            for (int x = 0; x < source.width(); ++x) {
                if (qAlpha(line[x]) > 8) {
                    visibleBounds |= QRect(x, y, 1, 1);
                }
            }
        }

        QImage aligned(deviceSize, QImage::Format_ARGB32_Premultiplied);
        aligned.fill(Qt::transparent);
        if (visibleBounds.isValid()) {
            const int desiredX = qRound((deviceSize.width() - 1 - visibleBounds.left() - visibleBounds.right()) / 2.0);
            const int desiredY = qRound((deviceSize.height() - 1 - visibleBounds.top() - visibleBounds.bottom()) / 2.0);
            const int offsetX = qBound(-visibleBounds.left(), desiredX, deviceSize.width() - 1 - visibleBounds.right());
            const int offsetY =
                qBound(-visibleBounds.top(), desiredY, deviceSize.height() - 1 - visibleBounds.bottom());
            QPainter alignedPainter(&aligned);
            alignedPainter.drawImage(QPoint(offsetX, offsetY), source);
        }

        QPixmap result = QPixmap::fromImage(aligned);
        result.setDevicePixelRatio(dpr);
        QPixmapCache::insert(cacheKey, result);

        return result;
    }

    FluentIcon m_icon;
    Theme m_theme = Theme::Auto;
    QColor m_tint;
};

QString iconName(FluentIcon icon)
{
    switch (icon) {
    case FluentIcon::Up:
        return QStringLiteral("up");
    case FluentIcon::Add:
        return QStringLiteral("add");
    case FluentIcon::Bus:
        return QStringLiteral("bus");
    case FluentIcon::Car:
        return QStringLiteral("car");
    case FluentIcon::Cut:
        return QStringLiteral("cut");
    case FluentIcon::IOT:
        return QStringLiteral("iot");
    case FluentIcon::Pin:
        return QStringLiteral("pin");
    case FluentIcon::Tag:
        return QStringLiteral("tag");
    case FluentIcon::VPN:
        return QStringLiteral("vpn");
    case FluentIcon::Cafe:
        return QStringLiteral("cafe");
    case FluentIcon::Chat:
        return QStringLiteral("chat");
    case FluentIcon::Copy:
        return QStringLiteral("copy");
    case FluentIcon::Code:
        return QStringLiteral("code");
    case FluentIcon::Down:
        return QStringLiteral("down");
    case FluentIcon::Edit:
        return QStringLiteral("edit");
    case FluentIcon::Flag:
        return QStringLiteral("flag");
    case FluentIcon::Font:
        return QStringLiteral("font");
    case FluentIcon::Game:
        return QStringLiteral("game");
    case FluentIcon::Help:
        return QStringLiteral("help");
    case FluentIcon::Hide:
        return QStringLiteral("hide");
    case FluentIcon::Home:
        return QStringLiteral("home");
    case FluentIcon::Info:
        return QStringLiteral("info");
    case FluentIcon::Leaf:
        return QStringLiteral("leaf");
    case FluentIcon::Link:
        return QStringLiteral("link");
    case FluentIcon::Mail:
        return QStringLiteral("mail");
    case FluentIcon::Menu:
        return QStringLiteral("menu");
    case FluentIcon::Mute:
        return QStringLiteral("mute");
    case FluentIcon::More:
        return QStringLiteral("more");
    case FluentIcon::Move:
        return QStringLiteral("move");
    case FluentIcon::Play:
        return QStringLiteral("play");
    case FluentIcon::Save:
        return QStringLiteral("save");
    case FluentIcon::Send:
        return QStringLiteral("send");
    case FluentIcon::Sync:
        return QStringLiteral("sync");
    case FluentIcon::Unit:
        return QStringLiteral("unit");
    case FluentIcon::View:
        return QStringLiteral("view");
    case FluentIcon::Wifi:
        return QStringLiteral("wifi");
    case FluentIcon::Zoom:
        return QStringLiteral("zoom");
    case FluentIcon::Album:
        return QStringLiteral("album");
    case FluentIcon::Brush:
        return QStringLiteral("brush");
    case FluentIcon::Broom:
        return QStringLiteral("broom");
    case FluentIcon::Close:
        return QStringLiteral("close");
    case FluentIcon::Cloud:
        return QStringLiteral("cloud");
    case FluentIcon::Embed:
        return QStringLiteral("embed");
    case FluentIcon::Globe:
        return QStringLiteral("globe");
    case FluentIcon::Heart:
        return QStringLiteral("heart");
    case FluentIcon::Label:
        return QStringLiteral("label");
    case FluentIcon::Media:
        return QStringLiteral("media");
    case FluentIcon::Movie:
        return QStringLiteral("movie");
    case FluentIcon::Music:
        return QStringLiteral("music");
    case FluentIcon::Robot:
        return QStringLiteral("robot");
    case FluentIcon::Pause:
        return QStringLiteral("pause");
    case FluentIcon::Paste:
        return QStringLiteral("paste");
    case FluentIcon::Photo:
        return QStringLiteral("photo");
    case FluentIcon::Phone:
        return QStringLiteral("phone");
    case FluentIcon::Print:
        return QStringLiteral("print");
    case FluentIcon::Share:
        return QStringLiteral("share");
    case FluentIcon::Tiles:
        return QStringLiteral("tiles");
    case FluentIcon::Unpin:
        return QStringLiteral("unpin");
    case FluentIcon::Video:
        return QStringLiteral("video");
    case FluentIcon::Train:
        return QStringLiteral("train");
    case FluentIcon::AddTo:
        return QStringLiteral("add_to");
    case FluentIcon::Accept:
        return QStringLiteral("accept");
    case FluentIcon::Camera:
        return QStringLiteral("camera");
    case FluentIcon::Cancel:
        return QStringLiteral("cancel");
    case FluentIcon::Delete:
        return QStringLiteral("delete");
    case FluentIcon::Folder:
        return QStringLiteral("folder");
    case FluentIcon::Filter:
        return QStringLiteral("filter");
    case FluentIcon::Market:
        return QStringLiteral("market");
    case FluentIcon::Scroll:
        return QStringLiteral("scroll");
    case FluentIcon::Layout:
        return QStringLiteral("layout");
    case FluentIcon::GitHub:
        return QStringLiteral("git_hub");
    case FluentIcon::Update:
        return QStringLiteral("update");
    case FluentIcon::Remove:
        return QStringLiteral("remove");
    case FluentIcon::Return:
        return QStringLiteral("return");
    case FluentIcon::People:
        return QStringLiteral("people");
    case FluentIcon::QRCode:
        return QStringLiteral("qr_code");
    case FluentIcon::Ringer:
        return QStringLiteral("ringer");
    case FluentIcon::Rotate:
        return QStringLiteral("rotate");
    case FluentIcon::Search:
        return QStringLiteral("search");
    case FluentIcon::Volume:
        return QStringLiteral("volume");
    case FluentIcon::Frigid:
        return QStringLiteral("frigid");
    case FluentIcon::SaveAs:
        return QStringLiteral("save_as");
    case FluentIcon::ZoomIn:
        return QStringLiteral("zoom_in");
    case FluentIcon::Connect:
        return QStringLiteral("connect");
    case FluentIcon::History:
        return QStringLiteral("history");
    case FluentIcon::Setting:
        return QStringLiteral("setting");
    case FluentIcon::Palette:
        return QStringLiteral("palette");
    case FluentIcon::Message:
        return QStringLiteral("message");
    case FluentIcon::FitPage:
        return QStringLiteral("fit_page");
    case FluentIcon::ZoomOut:
        return QStringLiteral("zoom_out");
    case FluentIcon::Airplane:
        return QStringLiteral("airplane");
    case FluentIcon::Asterisk:
        return QStringLiteral("asterisk");
    case FluentIcon::Calories:
        return QStringLiteral("calories");
    case FluentIcon::Calendar:
        return QStringLiteral("calendar");
    case FluentIcon::Feedback:
        return QStringLiteral("feedback");
    case FluentIcon::Library:
        return QStringLiteral("library");
    case FluentIcon::Minimize:
        return QStringLiteral("minimize");
    case FluentIcon::Checkbox:
        return QStringLiteral("checkbox");
    case FluentIcon::Document:
        return QStringLiteral("document");
    case FluentIcon::Language:
        return QStringLiteral("language");
    case FluentIcon::Download:
        return QStringLiteral("download");
    case FluentIcon::Question:
        return QStringLiteral("question");
    case FluentIcon::Speakers:
        return QStringLiteral("speakers");
    case FluentIcon::DateTime:
        return QStringLiteral("date_time");
    case FluentIcon::FontSize:
        return QStringLiteral("font_size");
    case FluentIcon::HomeFill:
        return QStringLiteral("home_fill");
    case FluentIcon::PageLeft:
        return QStringLiteral("page_left");
    case FluentIcon::SaveCopy:
        return QStringLiteral("save_copy");
    case FluentIcon::SendFill:
        return QStringLiteral("send_fill");
    case FluentIcon::SkipBack:
        return QStringLiteral("skip_back");
    case FluentIcon::SpeedOff:
        return QStringLiteral("speed_off");
    case FluentIcon::Alignment:
        return QStringLiteral("alignment");
    case FluentIcon::Bluetooth:
        return QStringLiteral("bluetooth");
    case FluentIcon::Completed:
        return QStringLiteral("completed");
    case FluentIcon::Constract:
        return QStringLiteral("constract");
    case FluentIcon::Headphone:
        return QStringLiteral("headphone");
    case FluentIcon::Megaphone:
        return QStringLiteral("megaphone");
    case FluentIcon::Projector:
        return QStringLiteral("projector");
    case FluentIcon::Education:
        return QStringLiteral("education");
    case FluentIcon::LeftArrow:
        return QStringLiteral("left_arrow");
    case FluentIcon::EraseTool:
        return QStringLiteral("erase_tool");
    case FluentIcon::PageRight:
        return QStringLiteral("page_right");
    case FluentIcon::PlaySolid:
        return QStringLiteral("play_solid");
    case FluentIcon::BookShelf:
        return QStringLiteral("book_shelf");
    case FluentIcon::Hightlight:
        return QStringLiteral("hightlight");
    case FluentIcon::FolderAdd:
        return QStringLiteral("folder_add");
    case FluentIcon::PauseBold:
        return QStringLiteral("pause_bold");
    case FluentIcon::PencilInk:
        return QStringLiteral("pencil_ink");
    case FluentIcon::PieSingle:
        return QStringLiteral("pie_single");
    case FluentIcon::QuickNote:
        return QStringLiteral("quick_note");
    case FluentIcon::SpeedHigh:
        return QStringLiteral("speed_high");
    case FluentIcon::StopWatch:
        return QStringLiteral("stop_watch");
    case FluentIcon::ZipFolder:
        return QStringLiteral("zip_folder");
    case FluentIcon::Basketball:
        return QStringLiteral("basketball");
    case FluentIcon::Brightness:
        return QStringLiteral("brightness");
    case FluentIcon::Dictionary:
        return QStringLiteral("dictionary");
    case FluentIcon::Microphone:
        return QStringLiteral("microphone");
    case FluentIcon::ArrowDown:
        return QStringLiteral("arrow_down");
    case FluentIcon::FullScreen:
        return QStringLiteral("full_screen");
    case FluentIcon::MixVolumes:
        return QStringLiteral("mix_volumes");
    case FluentIcon::RemoveFrom:
        return QStringLiteral("remove_from");
    case FluentIcon::RightArrow:
        return QStringLiteral("right_arrow");
    case FluentIcon::QuietHours:
        return QStringLiteral("quiet_hours");
    case FluentIcon::Fingerprint:
        return QStringLiteral("fingerprint");
    case FluentIcon::Application:
        return QStringLiteral("application");
    case FluentIcon::Certificate:
        return QStringLiteral("certificate");
    case FluentIcon::Transparent:
        return QStringLiteral("transparent");
    case FluentIcon::ImageExport:
        return QStringLiteral("image_export");
    case FluentIcon::SpeedMedium:
        return QStringLiteral("speed_medium");
    case FluentIcon::LibraryFill:
        return QStringLiteral("library_fill");
    case FluentIcon::MusicFolder:
        return QStringLiteral("music_folder");
    case FluentIcon::PowerButton:
        return QStringLiteral("power_button");
    case FluentIcon::SkipForward:
        return QStringLiteral("skip_forward");
    case FluentIcon::CareUpSolid:
        return QStringLiteral("care_up_solid");
    case FluentIcon::AcceptMedium:
        return QStringLiteral("accept_medium");
    case FluentIcon::CancelMedium:
        return QStringLiteral("cancel_medium");
    case FluentIcon::ChevronRight:
        return QStringLiteral("chevron_right");
    case FluentIcon::ClippingTool:
        return QStringLiteral("clipping_tool");
    case FluentIcon::SearchMirror:
        return QStringLiteral("search_mirror");
    case FluentIcon::ShoppingCart:
        return QStringLiteral("shopping_cart");
    case FluentIcon::FontIncrease:
        return QStringLiteral("font_increase");
    case FluentIcon::BackToWindow:
        return QStringLiteral("back_to_window");
    case FluentIcon::CommandPrompt:
        return QStringLiteral("command_prompt");
    case FluentIcon::CloudDownload:
        return QStringLiteral("cloud_download");
    case FluentIcon::DictionaryAdd:
        return QStringLiteral("dictionary_add");
    case FluentIcon::CareDownSolid:
        return QStringLiteral("care_down_solid");
    case FluentIcon::CareLeftSolid:
        return QStringLiteral("care_left_solid");
    case FluentIcon::ClearSelection:
        return QStringLiteral("clear_selection");
    case FluentIcon::DeveloperTools:
        return QStringLiteral("developer_tools");
    case FluentIcon::BackgroundFill:
        return QStringLiteral("background_fill");
    case FluentIcon::CareRightSolid:
        return QStringLiteral("care_right_solid");
    case FluentIcon::ChevronDownMed:
        return QStringLiteral("chevron_down_med");
    case FluentIcon::ChevronRightMed:
        return QStringLiteral("chevron_right_med");
    case FluentIcon::EmojiTabSymbols:
        return QStringLiteral("emoji_tab_symbols");
    case FluentIcon::ExpressiveInputEntry:
        return QStringLiteral("expressive_input_entry");
    }
    return QStringLiteral("icon");
}

QString iconResourceName(FluentIcon icon)
{
    switch (icon) {
    case FluentIcon::Up:
        return QStringLiteral("Up");
    case FluentIcon::Add:
        return QStringLiteral("Add");
    case FluentIcon::Bus:
        return QStringLiteral("Bus");
    case FluentIcon::Car:
        return QStringLiteral("Car");
    case FluentIcon::Cut:
        return QStringLiteral("Cut");
    case FluentIcon::IOT:
        return QStringLiteral("IOT");
    case FluentIcon::Pin:
        return QStringLiteral("Pin");
    case FluentIcon::Tag:
        return QStringLiteral("Tag");
    case FluentIcon::VPN:
        return QStringLiteral("VPN");
    case FluentIcon::Cafe:
        return QStringLiteral("Cafe");
    case FluentIcon::Chat:
        return QStringLiteral("Chat");
    case FluentIcon::Copy:
        return QStringLiteral("Copy");
    case FluentIcon::Code:
        return QStringLiteral("Code");
    case FluentIcon::Down:
        return QStringLiteral("Down");
    case FluentIcon::Edit:
        return QStringLiteral("Edit");
    case FluentIcon::Flag:
        return QStringLiteral("Flag");
    case FluentIcon::Font:
        return QStringLiteral("Font");
    case FluentIcon::Game:
        return QStringLiteral("Game");
    case FluentIcon::Help:
        return QStringLiteral("Help");
    case FluentIcon::Hide:
        return QStringLiteral("Hide");
    case FluentIcon::Home:
        return QStringLiteral("Home");
    case FluentIcon::Info:
        return QStringLiteral("Info");
    case FluentIcon::Leaf:
        return QStringLiteral("Leaf");
    case FluentIcon::Link:
        return QStringLiteral("Link");
    case FluentIcon::Mail:
        return QStringLiteral("Mail");
    case FluentIcon::Menu:
        return QStringLiteral("Menu");
    case FluentIcon::Mute:
        return QStringLiteral("Mute");
    case FluentIcon::More:
        return QStringLiteral("More");
    case FluentIcon::Move:
        return QStringLiteral("Move");
    case FluentIcon::Play:
        return QStringLiteral("Play");
    case FluentIcon::Save:
        return QStringLiteral("Save");
    case FluentIcon::Send:
        return QStringLiteral("Send");
    case FluentIcon::Sync:
        return QStringLiteral("Sync");
    case FluentIcon::Unit:
        return QStringLiteral("Unit");
    case FluentIcon::View:
        return QStringLiteral("View");
    case FluentIcon::Wifi:
        return QStringLiteral("Wifi");
    case FluentIcon::Zoom:
        return QStringLiteral("Zoom");
    case FluentIcon::Album:
        return QStringLiteral("Album");
    case FluentIcon::Brush:
        return QStringLiteral("Brush");
    case FluentIcon::Broom:
        return QStringLiteral("Broom");
    case FluentIcon::Close:
        return QStringLiteral("Close");
    case FluentIcon::Cloud:
        return QStringLiteral("Cloud");
    case FluentIcon::Embed:
        return QStringLiteral("Embed");
    case FluentIcon::Globe:
        return QStringLiteral("Globe");
    case FluentIcon::Heart:
        return QStringLiteral("Heart");
    case FluentIcon::Label:
        return QStringLiteral("Label");
    case FluentIcon::Media:
        return QStringLiteral("Media");
    case FluentIcon::Movie:
        return QStringLiteral("Movie");
    case FluentIcon::Music:
        return QStringLiteral("Music");
    case FluentIcon::Robot:
        return QStringLiteral("Robot");
    case FluentIcon::Pause:
        return QStringLiteral("Pause");
    case FluentIcon::Paste:
        return QStringLiteral("Paste");
    case FluentIcon::Photo:
        return QStringLiteral("Photo");
    case FluentIcon::Phone:
        return QStringLiteral("Phone");
    case FluentIcon::Print:
        return QStringLiteral("Print");
    case FluentIcon::Share:
        return QStringLiteral("Share");
    case FluentIcon::Tiles:
        return QStringLiteral("Tiles");
    case FluentIcon::Unpin:
        return QStringLiteral("Unpin");
    case FluentIcon::Video:
        return QStringLiteral("Video");
    case FluentIcon::Train:
        return QStringLiteral("Train");
    case FluentIcon::AddTo:
        return QStringLiteral("AddTo");
    case FluentIcon::Accept:
        return QStringLiteral("Accept");
    case FluentIcon::Camera:
        return QStringLiteral("Camera");
    case FluentIcon::Cancel:
        return QStringLiteral("Cancel");
    case FluentIcon::Delete:
        return QStringLiteral("Delete");
    case FluentIcon::Folder:
        return QStringLiteral("Folder");
    case FluentIcon::Filter:
        return QStringLiteral("Filter");
    case FluentIcon::Market:
        return QStringLiteral("Market");
    case FluentIcon::Scroll:
        return QStringLiteral("Scroll");
    case FluentIcon::Layout:
        return QStringLiteral("Layout");
    case FluentIcon::GitHub:
        return QStringLiteral("GitHub");
    case FluentIcon::Update:
        return QStringLiteral("Update");
    case FluentIcon::Remove:
        return QStringLiteral("Remove");
    case FluentIcon::Return:
        return QStringLiteral("Return");
    case FluentIcon::People:
        return QStringLiteral("People");
    case FluentIcon::QRCode:
        return QStringLiteral("QRCode");
    case FluentIcon::Ringer:
        return QStringLiteral("Ringer");
    case FluentIcon::Rotate:
        return QStringLiteral("Rotate");
    case FluentIcon::Search:
        return QStringLiteral("Search");
    case FluentIcon::Volume:
        return QStringLiteral("Volume");
    case FluentIcon::Frigid:
        return QStringLiteral("Frigid");
    case FluentIcon::SaveAs:
        return QStringLiteral("SaveAs");
    case FluentIcon::ZoomIn:
        return QStringLiteral("ZoomIn");
    case FluentIcon::Connect:
        return QStringLiteral("Connect");
    case FluentIcon::History:
        return QStringLiteral("History");
    case FluentIcon::Setting:
        return QStringLiteral("Setting");
    case FluentIcon::Palette:
        return QStringLiteral("Palette");
    case FluentIcon::Message:
        return QStringLiteral("Message");
    case FluentIcon::FitPage:
        return QStringLiteral("FitPage");
    case FluentIcon::ZoomOut:
        return QStringLiteral("ZoomOut");
    case FluentIcon::Airplane:
        return QStringLiteral("Airplane");
    case FluentIcon::Asterisk:
        return QStringLiteral("Asterisk");
    case FluentIcon::Calories:
        return QStringLiteral("Calories");
    case FluentIcon::Calendar:
        return QStringLiteral("Calendar");
    case FluentIcon::Feedback:
        return QStringLiteral("Feedback");
    case FluentIcon::Library:
        return QStringLiteral("BookShelf");
    case FluentIcon::Minimize:
        return QStringLiteral("Minimize");
    case FluentIcon::Checkbox:
        return QStringLiteral("CheckBox");
    case FluentIcon::Document:
        return QStringLiteral("Document");
    case FluentIcon::Language:
        return QStringLiteral("Language");
    case FluentIcon::Download:
        return QStringLiteral("Download");
    case FluentIcon::Question:
        return QStringLiteral("Question");
    case FluentIcon::Speakers:
        return QStringLiteral("Speakers");
    case FluentIcon::DateTime:
        return QStringLiteral("DateTime");
    case FluentIcon::FontSize:
        return QStringLiteral("FontSize");
    case FluentIcon::HomeFill:
        return QStringLiteral("HomeFill");
    case FluentIcon::PageLeft:
        return QStringLiteral("PageLeft");
    case FluentIcon::SaveCopy:
        return QStringLiteral("SaveCopy");
    case FluentIcon::SendFill:
        return QStringLiteral("SendFill");
    case FluentIcon::SkipBack:
        return QStringLiteral("SkipBack");
    case FluentIcon::SpeedOff:
        return QStringLiteral("SpeedOff");
    case FluentIcon::Alignment:
        return QStringLiteral("Alignment");
    case FluentIcon::Bluetooth:
        return QStringLiteral("Bluetooth");
    case FluentIcon::Completed:
        return QStringLiteral("Completed");
    case FluentIcon::Constract:
        return QStringLiteral("Constract");
    case FluentIcon::Headphone:
        return QStringLiteral("Headphone");
    case FluentIcon::Megaphone:
        return QStringLiteral("Megaphone");
    case FluentIcon::Projector:
        return QStringLiteral("Projector");
    case FluentIcon::Education:
        return QStringLiteral("Education");
    case FluentIcon::LeftArrow:
        return QStringLiteral("LeftArrow");
    case FluentIcon::EraseTool:
        return QStringLiteral("EraseTool");
    case FluentIcon::PageRight:
        return QStringLiteral("PageRight");
    case FluentIcon::PlaySolid:
        return QStringLiteral("PlaySolid");
    case FluentIcon::BookShelf:
        return QStringLiteral("BookShelf");
    case FluentIcon::Hightlight:
        return QStringLiteral("Highlight");
    case FluentIcon::FolderAdd:
        return QStringLiteral("FolderAdd");
    case FluentIcon::PauseBold:
        return QStringLiteral("PauseBold");
    case FluentIcon::PencilInk:
        return QStringLiteral("PencilInk");
    case FluentIcon::PieSingle:
        return QStringLiteral("PieSingle");
    case FluentIcon::QuickNote:
        return QStringLiteral("QuickNote");
    case FluentIcon::SpeedHigh:
        return QStringLiteral("SpeedHigh");
    case FluentIcon::StopWatch:
        return QStringLiteral("StopWatch");
    case FluentIcon::ZipFolder:
        return QStringLiteral("ZipFolder");
    case FluentIcon::Basketball:
        return QStringLiteral("Basketball");
    case FluentIcon::Brightness:
        return QStringLiteral("Brightness");
    case FluentIcon::Dictionary:
        return QStringLiteral("Dictionary");
    case FluentIcon::Microphone:
        return QStringLiteral("Microphone");
    case FluentIcon::ArrowDown:
        return QStringLiteral("ChevronDown");
    case FluentIcon::FullScreen:
        return QStringLiteral("FullScreen");
    case FluentIcon::MixVolumes:
        return QStringLiteral("MixVolumes");
    case FluentIcon::RemoveFrom:
        return QStringLiteral("RemoveFrom");
    case FluentIcon::RightArrow:
        return QStringLiteral("RightArrow");
    case FluentIcon::QuietHours:
        return QStringLiteral("QuietHours");
    case FluentIcon::Fingerprint:
        return QStringLiteral("Fingerprint");
    case FluentIcon::Application:
        return QStringLiteral("Application");
    case FluentIcon::Certificate:
        return QStringLiteral("Certificate");
    case FluentIcon::Transparent:
        return QStringLiteral("Transparent");
    case FluentIcon::ImageExport:
        return QStringLiteral("ImageExport");
    case FluentIcon::SpeedMedium:
        return QStringLiteral("SpeedMedium");
    case FluentIcon::LibraryFill:
        return QStringLiteral("LibraryFill");
    case FluentIcon::MusicFolder:
        return QStringLiteral("MusicFolder");
    case FluentIcon::PowerButton:
        return QStringLiteral("PowerButton");
    case FluentIcon::SkipForward:
        return QStringLiteral("SkipForward");
    case FluentIcon::CareUpSolid:
        return QStringLiteral("CareUpSolid");
    case FluentIcon::AcceptMedium:
        return QStringLiteral("AcceptMedium");
    case FluentIcon::CancelMedium:
        return QStringLiteral("CancelMedium");
    case FluentIcon::ChevronRight:
        return QStringLiteral("ChevronRight");
    case FluentIcon::ClippingTool:
        return QStringLiteral("ClippingTool");
    case FluentIcon::SearchMirror:
        return QStringLiteral("SearchMirror");
    case FluentIcon::ShoppingCart:
        return QStringLiteral("ShoppingCart");
    case FluentIcon::FontIncrease:
        return QStringLiteral("FontIncrease");
    case FluentIcon::BackToWindow:
        return QStringLiteral("BackToWindow");
    case FluentIcon::CommandPrompt:
        return QStringLiteral("CommandPrompt");
    case FluentIcon::CloudDownload:
        return QStringLiteral("CloudDownload");
    case FluentIcon::DictionaryAdd:
        return QStringLiteral("DictionaryAdd");
    case FluentIcon::CareDownSolid:
        return QStringLiteral("CareDownSolid");
    case FluentIcon::CareLeftSolid:
        return QStringLiteral("CareLeftSolid");
    case FluentIcon::ClearSelection:
        return QStringLiteral("ClearSelection");
    case FluentIcon::DeveloperTools:
        return QStringLiteral("DeveloperTools");
    case FluentIcon::BackgroundFill:
        return QStringLiteral("BackgroundColor");
    case FluentIcon::CareRightSolid:
        return QStringLiteral("CareRightSolid");
    case FluentIcon::ChevronDownMed:
        return QStringLiteral("ChevronDownMed");
    case FluentIcon::ChevronRightMed:
        return QStringLiteral("ChevronRightMed");
    case FluentIcon::EmojiTabSymbols:
        return QStringLiteral("EmojiTabSymbols");
    case FluentIcon::ExpressiveInputEntry:
        return QStringLiteral("ExpressiveInputEntry");
    }
    return QStringLiteral("Icon");
}

QString iconPath(FluentIcon icon, Theme theme)
{
    const Theme resolvedTheme = theme == Theme::Auto ? ThemeManager::instance()->effectiveTheme() : theme;
    const QString colorToken = resolvedTheme == Theme::Dark ? QStringLiteral("white") : QStringLiteral("black");
    return QStringLiteral(":/qfluentwidgets/images/icons/%1_%2.svg").arg(iconResourceName(icon), colorToken);
}

QIcon icon(FluentIcon icon, Theme theme) { return QIcon(new FluentIconEngine(icon, theme)); }

QIcon icon(FluentIcon icon, Theme theme, const QColor &tint) { return QIcon(new FluentIconEngine(icon, theme, tint)); }

QIcon icon(FluentIcon fluentIcon, const QColor &tint) { return icon(fluentIcon, Theme::Auto, tint); }

} // namespace FluentQt
