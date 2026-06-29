#include <FluentQtWidgets/FluentIcon.h>

#include <QtCore/QFile>
#include <QtCore/QtMath>
#include <QtGui/QGuiApplication>
#include <QtCore/QSize>
#include <QtGui/QIconEngine>
#include <QtGui/QPainter>
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

        const QString resourcePath = iconPath(m_icon, m_theme);
        if (!QFile::exists(resourcePath)) {
            return;
        }

        if (!m_tint.isValid()) {
            painter->save();
            painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform, true);
            QSvgRenderer renderer(resourcePath);
            renderer.render(painter, QRectF(rect));
            painter->restore();
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
        const QSize deviceSize(qMax(1, qCeil(logicalSize.width() * dpr)),
                               qMax(1, qCeil(logicalSize.height() * dpr)));
        QPixmap result(deviceSize);
        result.setDevicePixelRatio(dpr);
        result.fill(Qt::transparent);

        QPainter painter(&result);
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
        QSvgRenderer renderer(resourcePath);
        renderer.render(&painter, QRectF(QPointF(0, 0), QSizeF(logicalSize)));

        if (m_tint.isValid()) {
            painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
            painter.fillRect(QRectF(QPointF(0, 0), QSizeF(logicalSize)), m_tint);
        }

        return result;
    }

    FluentIcon m_icon;
    Theme m_theme = Theme::Auto;
    QColor m_tint;
};

QString iconName(FluentIcon icon)
{
    switch (icon) {
    case FluentIcon::Add:
        return QStringLiteral("add");
    case FluentIcon::Remove:
        return QStringLiteral("remove");
    case FluentIcon::Cut:
        return QStringLiteral("cut");
    case FluentIcon::Copy:
        return QStringLiteral("copy");
    case FluentIcon::Paste:
        return QStringLiteral("paste");
    case FluentIcon::Search:
        return QStringLiteral("search");
    case FluentIcon::Settings:
        return QStringLiteral("settings");
    case FluentIcon::Home:
        return QStringLiteral("home");
    case FluentIcon::Back:
        return QStringLiteral("back");
    case FluentIcon::Forward:
        return QStringLiteral("forward");
    case FluentIcon::Check:
        return QStringLiteral("check");
    case FluentIcon::Close:
        return QStringLiteral("close");
    case FluentIcon::Cancel:
        return QStringLiteral("cancel");
    case FluentIcon::Constract:
        return QStringLiteral("constract");
    case FluentIcon::Info:
        return QStringLiteral("info");
    case FluentIcon::Link:
        return QStringLiteral("link");
    case FluentIcon::Code:
        return QStringLiteral("code");
    case FluentIcon::Feedback:
        return QStringLiteral("feedback");
    case FluentIcon::GitHub:
        return QStringLiteral("github");
    case FluentIcon::Scroll:
        return QStringLiteral("scroll");
    case FluentIcon::Warning:
        return QStringLiteral("warning");
    case FluentIcon::Error:
        return QStringLiteral("error");
    case FluentIcon::Success:
        return QStringLiteral("success");
    case FluentIcon::Play:
        return QStringLiteral("play");
    case FluentIcon::Pause:
        return QStringLiteral("pause");
    case FluentIcon::Video:
        return QStringLiteral("video");
    case FluentIcon::Album:
        return QStringLiteral("album");
    case FluentIcon::Volume:
        return QStringLiteral("volume");
    case FluentIcon::Music:
        return QStringLiteral("music");
    case FluentIcon::Folder:
        return QStringLiteral("folder");
    case FluentIcon::FolderAdd:
        return QStringLiteral("folder_add");
    case FluentIcon::Calendar:
        return QStringLiteral("calendar");
    case FluentIcon::Camera:
        return QStringLiteral("camera");
    case FluentIcon::Ringer:
        return QStringLiteral("ringer");
    case FluentIcon::StopWatch:
        return QStringLiteral("stop_watch");
    case FluentIcon::PowerButton:
        return QStringLiteral("power_button");
    case FluentIcon::Pin:
        return QStringLiteral("pin");
    case FluentIcon::More:
        return QStringLiteral("more");
    case FluentIcon::Edit:
        return QStringLiteral("edit");
    case FluentIcon::Flag:
        return QStringLiteral("flag");
    case FluentIcon::Download:
        return QStringLiteral("download");
    case FluentIcon::Upload:
        return QStringLiteral("upload");
    case FluentIcon::Delete:
        return QStringLiteral("delete");
    case FluentIcon::Heart:
        return QStringLiteral("heart");
    case FluentIcon::People:
        return QStringLiteral("people");
    case FluentIcon::ShoppingCart:
        return QStringLiteral("shopping_cart");
    case FluentIcon::Basketball:
        return QStringLiteral("basketball");
    case FluentIcon::Tag:
        return QStringLiteral("tag");
    case FluentIcon::BookShelf:
        return QStringLiteral("book_shelf");
    case FluentIcon::Mail:
        return QStringLiteral("mail");
    case FluentIcon::Send:
        return QStringLiteral("send");
    case FluentIcon::Share:
        return QStringLiteral("share");
    case FluentIcon::Save:
        return QStringLiteral("save");
    case FluentIcon::Star:
        return QStringLiteral("star");
    case FluentIcon::ArrowDown:
        return QStringLiteral("arrow_down");
    case FluentIcon::View:
        return QStringLiteral("view");
    case FluentIcon::Rotate:
        return QStringLiteral("rotate");
    case FluentIcon::ZoomIn:
        return QStringLiteral("zoom_in");
    case FluentIcon::ZoomOut:
        return QStringLiteral("zoom_out");
    case FluentIcon::Print:
        return QStringLiteral("print");
    case FluentIcon::Sync:
        return QStringLiteral("sync");
    case FluentIcon::Completed:
        return QStringLiteral("completed");
    case FluentIcon::Minimize:
        return QStringLiteral("minimize");
    case FluentIcon::Maximize:
        return QStringLiteral("maximize");
    case FluentIcon::Restore:
        return QStringLiteral("restore");
    case FluentIcon::Brush:
        return QStringLiteral("brush");
    case FluentIcon::Transparent:
        return QStringLiteral("transparent");
    case FluentIcon::Palette:
        return QStringLiteral("palette");
    case FluentIcon::Zoom:
        return QStringLiteral("zoom");
    case FluentIcon::Language:
        return QStringLiteral("language");
    case FluentIcon::Help:
        return QStringLiteral("help");
    case FluentIcon::Update:
        return QStringLiteral("update");
    case FluentIcon::Font:
        return QStringLiteral("font");
    case FluentIcon::PencilInk:
        return QStringLiteral("pencil_ink");
    case FluentIcon::Highlight:
        return QStringLiteral("highlight");
    case FluentIcon::Alignment:
        return QStringLiteral("alignment");
    }
    return QStringLiteral("icon");
}

QString iconResourceName(FluentIcon icon)
{
    switch (icon) {
    case FluentIcon::Add:
        return QStringLiteral("Add");
    case FluentIcon::Remove:
        return QStringLiteral("Remove");
    case FluentIcon::Cut:
        return QStringLiteral("Cut");
    case FluentIcon::Copy:
        return QStringLiteral("Copy");
    case FluentIcon::Paste:
        return QStringLiteral("Paste");
    case FluentIcon::Search:
        return QStringLiteral("Search");
    case FluentIcon::Settings:
        return QStringLiteral("Setting");
    case FluentIcon::Home:
        return QStringLiteral("Home");
    case FluentIcon::Back:
        return QStringLiteral("LeftArrow");
    case FluentIcon::Forward:
        return QStringLiteral("RightArrow");
    case FluentIcon::Check:
        return QStringLiteral("Accept");
    case FluentIcon::Close:
        return QStringLiteral("Close");
    case FluentIcon::Cancel:
        return QStringLiteral("Cancel");
    case FluentIcon::Constract:
        return QStringLiteral("Constract");
    case FluentIcon::Info:
        return QStringLiteral("Info");
    case FluentIcon::Link:
        return QStringLiteral("Link");
    case FluentIcon::Code:
        return QStringLiteral("Code");
    case FluentIcon::Feedback:
        return QStringLiteral("Feedback");
    case FluentIcon::GitHub:
        return QStringLiteral("GitHub");
    case FluentIcon::Scroll:
        return QStringLiteral("Scroll");
    case FluentIcon::Warning:
        return QStringLiteral("Warning");
    case FluentIcon::Error:
        return QStringLiteral("Cancel");
    case FluentIcon::Success:
        return QStringLiteral("Accept");
    case FluentIcon::Play:
        return QStringLiteral("Play");
    case FluentIcon::Pause:
        return QStringLiteral("Pause");
    case FluentIcon::Video:
        return QStringLiteral("Video");
    case FluentIcon::Album:
        return QStringLiteral("Album");
    case FluentIcon::Volume:
        return QStringLiteral("Volume");
    case FluentIcon::Music:
        return QStringLiteral("Music");
    case FluentIcon::Folder:
        return QStringLiteral("Folder");
    case FluentIcon::FolderAdd:
        return QStringLiteral("FolderAdd");
    case FluentIcon::Calendar:
        return QStringLiteral("Calendar");
    case FluentIcon::Camera:
        return QStringLiteral("Camera");
    case FluentIcon::Ringer:
        return QStringLiteral("Ringer");
    case FluentIcon::StopWatch:
        return QStringLiteral("Stopwatch");
    case FluentIcon::PowerButton:
        return QStringLiteral("PowerButton");
    case FluentIcon::Pin:
        return QStringLiteral("Pin");
    case FluentIcon::More:
        return QStringLiteral("More");
    case FluentIcon::Edit:
        return QStringLiteral("Edit");
    case FluentIcon::Flag:
        return QStringLiteral("Flag");
    case FluentIcon::Download:
        return QStringLiteral("Download");
    case FluentIcon::Upload:
        return QStringLiteral("Up");
    case FluentIcon::Delete:
        return QStringLiteral("Delete");
    case FluentIcon::Heart:
        return QStringLiteral("Heart");
    case FluentIcon::People:
        return QStringLiteral("People");
    case FluentIcon::ShoppingCart:
        return QStringLiteral("ShoppingCart");
    case FluentIcon::Basketball:
        return QStringLiteral("Basketball");
    case FluentIcon::Tag:
        return QStringLiteral("Tag");
    case FluentIcon::BookShelf:
        return QStringLiteral("BookShelf");
    case FluentIcon::Mail:
        return QStringLiteral("Mail");
    case FluentIcon::Send:
        return QStringLiteral("Send");
    case FluentIcon::Share:
        return QStringLiteral("Share");
    case FluentIcon::Save:
        return QStringLiteral("Save");
    case FluentIcon::Star:
        return QStringLiteral("Star");
    case FluentIcon::ArrowDown:
        return QStringLiteral("ChevronDown");
    case FluentIcon::View:
        return QStringLiteral("View");
    case FluentIcon::Rotate:
        return QStringLiteral("Rotate");
    case FluentIcon::ZoomIn:
        return QStringLiteral("ZoomIn");
    case FluentIcon::ZoomOut:
        return QStringLiteral("ZoomOut");
    case FluentIcon::Print:
        return QStringLiteral("Print");
    case FluentIcon::Sync:
        return QStringLiteral("Sync");
    case FluentIcon::Completed:
        return QStringLiteral("Completed");
    case FluentIcon::Minimize:
        return QStringLiteral("Minimize");
    case FluentIcon::Maximize:
        return QStringLiteral("Maximize");
    case FluentIcon::Restore:
        return QStringLiteral("Restore");
    case FluentIcon::Brush:
        return QStringLiteral("Brush");
    case FluentIcon::Transparent:
        return QStringLiteral("Transparent");
    case FluentIcon::Palette:
        return QStringLiteral("Palette");
    case FluentIcon::Zoom:
        return QStringLiteral("Zoom");
    case FluentIcon::Language:
        return QStringLiteral("Language");
    case FluentIcon::Help:
        return QStringLiteral("Help");
    case FluentIcon::Update:
        return QStringLiteral("Update");
    case FluentIcon::Font:
        return QStringLiteral("Font");
    case FluentIcon::PencilInk:
        return QStringLiteral("PencilInk");
    case FluentIcon::Highlight:
        return QStringLiteral("Highlight");
    case FluentIcon::Alignment:
        return QStringLiteral("Alignment");
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

QIcon icon(FluentIcon icon, Theme theme, const QColor &tint)
{
    return QIcon(new FluentIconEngine(icon, theme, tint));
}

QIcon icon(FluentIcon fluentIcon, const QColor &tint)
{
    return icon(fluentIcon, Theme::Auto, tint);
}

} // namespace FluentQt
