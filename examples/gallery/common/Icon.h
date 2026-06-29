#pragma once

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QString>

enum class GalleryIcon
{
    Grid,
    Menu,
    Text,
    EmojiTabSymbols
};

inline QString galleryIconName(GalleryIcon icon)
{
    switch (icon) {
    case GalleryIcon::Grid:
        return QStringLiteral("Grid");
    case GalleryIcon::Menu:
        return QStringLiteral("Menu");
    case GalleryIcon::Text:
        return QStringLiteral("Text");
    case GalleryIcon::EmojiTabSymbols:
        return QStringLiteral("EmojiTabSymbols");
    }
    return QString();
}

inline QString galleryIconPath(GalleryIcon icon, FluentQt::Theme theme = FluentQt::Theme::Auto)
{
    const QString colorSuffix =
        (theme == FluentQt::Theme::Dark)
            ? QStringLiteral("_white")
            : QStringLiteral("_black");
    return QStringLiteral(":/gallery/images/icons/%1%2.svg").arg(galleryIconName(icon), colorSuffix);
}
