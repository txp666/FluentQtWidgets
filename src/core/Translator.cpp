#include <FluentQtWidgets/Translator.h>

#include <QtCore/QCoreApplication>

namespace FluentQt {

namespace {

bool isTraditionalChinese(const QLocale &locale)
{
    if (locale.language() != QLocale::Chinese) {
        return false;
    }

    if (locale.script() == QLocale::TraditionalChineseScript) {
        return true;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QLocale::Territory territory = locale.territory();
#else
    const QLocale::Country territory = locale.country();
#endif
    return territory == QLocale::HongKong || territory == QLocale::Macao || territory == QLocale::Taiwan;
}

void appendCandidate(QStringList &candidates, const QString &path)
{
    if (!path.isEmpty() && !candidates.contains(path)) {
        candidates.append(path);
    }
}

} // namespace

FluentTranslator::FluentTranslator(QObject *parent) : QTranslator(parent) { load(QLocale()); }

FluentTranslator::FluentTranslator(const QLocale &locale, QObject *parent) : QTranslator(parent) { load(locale); }

bool FluentTranslator::load(const QLocale &locale)
{
    const bool isChinese = locale.language() == QLocale::Chinese;
    QStringList candidates;
    appendCandidate(candidates, QStringLiteral(":/qfluentwidgets/i18n/qfluentwidgets.%1.qm").arg(locale.name()));
    if (isTraditionalChinese(locale)) {
        appendCandidate(candidates, QStringLiteral(":/qfluentwidgets/i18n/qfluentwidgets.zh_HK.qm"));
    }
    appendCandidate(candidates,
                    QStringLiteral(":/qfluentwidgets/i18n/qfluentwidgets.%1.qm").arg(locale.name().section('_', 0, 0)));
    if (isChinese) {
        appendCandidate(candidates, QStringLiteral(":/qfluentwidgets/i18n/qfluentwidgets.zh_CN.qm"));
    }

    for (const QString &path : candidates) {
        if (!path.isEmpty() && QTranslator::load(path)) {
            return true;
        }
    }

    return false;
}

FluentTranslator *installFluentTranslator(QCoreApplication *application, const QLocale &locale)
{
    if (!application) {
        return nullptr;
    }

    auto *translator = new FluentTranslator(locale, application);
    if (translator->isEmpty() || !application->installTranslator(translator)) {
        translator->deleteLater();
        return nullptr;
    }

    return translator;
}

} // namespace FluentQt
