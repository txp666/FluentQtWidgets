#pragma once

#include <FluentQtWidgets/Config.h>

#include <QtCore/QStringList>
#include <QtCore/QtGlobal>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>
#include <QtWidgets/QApplication>

namespace FluentQt::Private {

inline QStringList installedFontFamilies()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return QFontDatabase::families();
#else
    return QFontDatabase().families();
#endif
}

inline bool hasInstalledFontFamily(const QStringList &installedFamilies, const QString &family)
{
    for (const QString &installed : installedFamilies) {
        if (QString::compare(installed, family, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

inline QStringList platformFontFallbacks()
{
#if defined(Q_OS_MACOS)
    return {QStringLiteral(".AppleSystemUIFont"), QStringLiteral("PingFang SC"),
            QStringLiteral("Helvetica Neue"), QStringLiteral("Arial")};
#elif defined(Q_OS_WIN)
    return {QStringLiteral("Segoe UI"), QStringLiteral("Microsoft YaHei UI"),
            QStringLiteral("Microsoft YaHei")};
#else
    return {QStringLiteral("Noto Sans"), QStringLiteral("Ubuntu"), QStringLiteral("Arial"),
            QStringLiteral("DejaVu Sans")};
#endif
}

inline QStringList resolvedFontFamilies(const QStringList &families)
{
    static const QStringList installedFamilies = installedFontFamilies();
    QStringList resolved;

    for (const QString &family : families) {
        const QString trimmed = family.trimmed();
        if (trimmed.isEmpty() || resolved.contains(trimmed)) {
            continue;
        }
        if (hasInstalledFontFamily(installedFamilies, trimmed)) {
            resolved.append(trimmed);
        }
    }

    if (resolved.isEmpty()) {
        for (const QString &family : platformFontFallbacks()) {
            if (hasInstalledFontFamily(installedFamilies, family)) {
                resolved.append(family);
                break;
            }
        }
    }

    if (resolved.isEmpty() && qApp) {
        const QString appFamily = qApp->font().family().trimmed();
        if (!appFamily.isEmpty()) {
            resolved.append(appFamily);
        }
    }

    if (resolved.isEmpty()) {
        resolved.append(QStringLiteral("sans-serif"));
    }

    return resolved;
}

// Python's getFont(): the configured Fluent families at a pixel size, set in code so that a later
// setFont() still works, which a style sheet font would override.
inline QFont fluentFont(int pixelSize, QFont::Weight weight = QFont::Normal)
{
    QFont font;
    font.setFamilies(resolvedFontFamilies(FluentConfig::instance()->fontFamilies()));
    font.setPixelSize(pixelSize);
    font.setWeight(weight);
    return font;
}

} // namespace FluentQt::Private
