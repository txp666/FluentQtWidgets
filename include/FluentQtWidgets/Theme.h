#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtGui/QColor>

class QApplication;
class QWidget;

namespace FluentQt {
Q_NAMESPACE_EXPORT(FQW_API)

class SystemThemeListener;

enum class Theme
{
    Light,
    Dark,
    Auto
};
Q_ENUM_NS(Theme)

FQW_API QString themeName(Theme theme);

class FQW_API ThemeManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(FluentQt::Theme theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(FluentQt::Theme effectiveTheme READ effectiveTheme NOTIFY effectiveThemeChanged)
    Q_PROPERTY(QColor accentColor READ accentColor WRITE setAccentColor NOTIFY accentColorChanged)

  public:
    static ThemeManager *instance();

    Theme theme() const;
    Theme effectiveTheme() const;
    QColor accentColor() const;
    SystemThemeListener *systemThemeListener() const;

  public slots:
    void setTheme(FluentQt::Theme theme);
    void setAccentColor(const QColor &color);
    void applyTo(QApplication *application) const;
    void applyTo(QWidget *widget) const;

  signals:
    void themeChanged(FluentQt::Theme theme);
    void effectiveThemeChanged(FluentQt::Theme theme);
    void themeChangedFinished();
    void accentColorChanged(const QColor &color);

  private:
    explicit ThemeManager(QObject *parent = nullptr);

    Theme m_theme = Theme::Light;
    QColor m_accentColor = QColor(QStringLiteral("#009faa"));
    SystemThemeListener *m_systemThemeListener = nullptr;
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::Theme)
