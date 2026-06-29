#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QIcon>

namespace FluentQt {

enum class FluentIcon
{
    Add,
    Remove,
    Cut,
    Copy,
    Paste,
    Search,
    Settings,
    Home,
    Back,
    Forward,
    Check,
    Close,
    Cancel,
    Constract,
    Info,
    Warning,
    Error,
    Success,
    Play,
    Pause,
    Video,
    Album,
    Volume,
    Music,
    Folder,
    FolderAdd,
    Calendar,
    Camera,
    Ringer,
    StopWatch,
    PowerButton,
    Pin,
    More,
    Edit,
    Flag,
    Download,
    Upload,
    Delete,
    Heart,
    People,
    ShoppingCart,
    Basketball,
    Tag,
    BookShelf,
    Mail,
    Send,
    Share,
    Save,
    Star,
    ArrowDown,
    View,
    Rotate,
    ZoomIn,
    ZoomOut,
    Print,
    Sync,
    Completed,
    Minimize,
    Maximize,
    Restore,
    Brush,
    Transparent,
    Palette,
    Zoom,
    Language,
    Help,
    Update,
    Font,
    PencilInk,
    Highlight,
    Alignment,
    Link,
    Code,
    Feedback,
    GitHub,
    Scroll
};

FQW_API QString iconName(FluentIcon icon);
FQW_API QString iconResourceName(FluentIcon icon);
FQW_API QString iconPath(FluentIcon icon, Theme theme = Theme::Auto);
FQW_API QIcon icon(FluentIcon icon, Theme theme = Theme::Auto);
FQW_API QIcon icon(FluentIcon icon, Theme theme, const QColor &tint);
FQW_API QIcon icon(FluentIcon icon, const QColor &tint);

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::FluentIcon)
