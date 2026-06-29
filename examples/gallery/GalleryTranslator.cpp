#include "GalleryTranslator.h"

#include <FluentQtWidgets/Translator.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QPointer>
#include <QtCore/QTranslator>
#include <QtCore/QtGlobal>

namespace {

QPointer<FluentQt::FluentTranslator> s_fluentTranslator;
QPointer<QTranslator> s_galleryTranslator;

bool isAutoLocale(const QString &localeName)
{
    return localeName.trimmed().isEmpty() || localeName.compare(QStringLiteral("Auto"), Qt::CaseInsensitive) == 0;
}

QString normalizedLocaleName(const QString &localeName)
{
    if (isAutoLocale(localeName)) {
        return QStringLiteral("Auto");
    }
    if (localeName.compare(QStringLiteral("zh_Hant"), Qt::CaseInsensitive) == 0 ||
        localeName.compare(QStringLiteral("zh_TW"), Qt::CaseInsensitive) == 0 ||
        localeName.compare(QStringLiteral("zh_MO"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("zh_HK");
    }
    if (localeName.compare(QStringLiteral("zh_Hans"), Qt::CaseInsensitive) == 0 ||
        localeName.compare(QStringLiteral("zh"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("zh_CN");
    }
    if (localeName.compare(QStringLiteral("en_US"), Qt::CaseInsensitive) == 0 ||
        localeName.compare(QStringLiteral("en_GB"), Qt::CaseInsensitive) == 0) {
        return QStringLiteral("en");
    }
    return localeName;
}

QString galleryTranslationName(const QLocale &locale)
{
    if (locale.language() != QLocale::Chinese) {
        return QString();
    }

    switch (locale.script()) {
    case QLocale::TraditionalChineseScript:
        return QStringLiteral("zh_HK");
    case QLocale::SimplifiedChineseScript:
        return QStringLiteral("zh_CN");
    default:
        break;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QLocale::Territory territory = locale.territory();
#else
    const QLocale::Country territory = locale.country();
#endif

    switch (territory) {
    case QLocale::HongKong:
    case QLocale::Macao:
    case QLocale::Taiwan:
        return QStringLiteral("zh_HK");
    default:
        return QStringLiteral("zh_CN");
    }
}

void removeTranslator(QCoreApplication *application, QTranslator *translator)
{
    if (!application || !translator) {
        return;
    }

    application->removeTranslator(translator);
    translator->deleteLater();
}

} // namespace

namespace GalleryTranslation {

QString languageNameForIndex(int index)
{
    switch (index) {
    case 0:
        return QStringLiteral("zh_CN");
    case 1:
        return QStringLiteral("zh_HK");
    case 2:
        return QStringLiteral("en");
    default:
        return QStringLiteral("Auto");
    }
}

int languageIndexForName(const QString &localeName)
{
    const QString normalized = normalizedLocaleName(localeName);
    if (normalized == QStringLiteral("zh_CN")) {
        return 0;
    }
    if (normalized == QStringLiteral("zh_HK")) {
        return 1;
    }
    if (normalized == QStringLiteral("en")) {
        return 2;
    }
    return 3;
}

QLocale localeForName(const QString &localeName)
{
    const QString normalized = normalizedLocaleName(localeName);
    if (normalized == QStringLiteral("Auto")) {
        return QLocale();
    }
    return QLocale(normalized);
}

bool installTranslators(QCoreApplication *application, const QString &localeName)
{
    if (!application) {
        return false;
    }

    removeTranslator(application, s_galleryTranslator);
    removeTranslator(application, s_fluentTranslator);

    const QLocale locale = localeForName(localeName);
    bool installed = false;

    auto *fluentTranslator = new FluentQt::FluentTranslator(locale, application);
    if (!fluentTranslator->isEmpty() && application->installTranslator(fluentTranslator)) {
        s_fluentTranslator = fluentTranslator;
        installed = true;
    } else {
        fluentTranslator->deleteLater();
    }

    const QString galleryName = galleryTranslationName(locale);
    if (!galleryName.isEmpty()) {
        auto *galleryTranslator = new QTranslator(application);
        if (galleryTranslator->load(QStringLiteral(":/gallery/i18n/gallery.%1.qm").arg(galleryName)) &&
            application->installTranslator(galleryTranslator)) {
            s_galleryTranslator = galleryTranslator;
            installed = true;
        } else {
            galleryTranslator->deleteLater();
        }
    }

    return installed;
}

} // namespace GalleryTranslation
