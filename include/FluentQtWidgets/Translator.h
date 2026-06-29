#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QLocale>
#include <QtCore/QTranslator>

class QCoreApplication;

namespace FluentQt {

class FQW_API FluentTranslator : public QTranslator
{
    Q_OBJECT

  public:
    explicit FluentTranslator(QObject *parent = nullptr);
    explicit FluentTranslator(const QLocale &locale, QObject *parent = nullptr);

    bool load(const QLocale &locale);
};

FQW_API FluentTranslator *installFluentTranslator(QCoreApplication *application, const QLocale &locale = QLocale());

} // namespace FluentQt
