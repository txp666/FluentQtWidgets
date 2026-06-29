#pragma once

#include <QtCore/QLocale>
#include <QtCore/QString>

class QCoreApplication;

namespace GalleryTranslation {

QString languageNameForIndex(int index);
int languageIndexForName(const QString &localeName);
QLocale localeForName(const QString &localeName);
bool installTranslators(QCoreApplication *application, const QString &localeName);

} // namespace GalleryTranslation
