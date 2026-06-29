#include <FluentQtWidgets/Theme.h>

#include <FluentQtWidgets/Config.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/ThemeListener.h>

#include "FontUtils_p.h"

#include <QtCore/QCoreApplication>
#include <QtGui/QFont>
#include <QtGui/QPalette>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtGui/QStyleHints>
#endif

namespace FluentQt {

QString themeName(Theme theme)
{
    switch (theme) {
    case Theme::Light:
        return QStringLiteral("light");
    case Theme::Dark:
        return QStringLiteral("dark");
    case Theme::Auto:
        return QStringLiteral("auto");
    }
    return QStringLiteral("light");
}

ThemeManager *ThemeManager::instance()
{
    static ThemeManager manager;
    return &manager;
}

ThemeManager::ThemeManager(QObject *parent) : QObject(parent)
{
    m_systemThemeListener = new SystemThemeListener(this);
    connect(m_systemThemeListener, &SystemThemeListener::systemThemeChanged, this, [this](Theme) {
        if (m_theme == Theme::Auto) {
            if (qApp) {
                applyTo(qApp);
            } else {
                FluentStyleSheet::updateAll(effectiveTheme(), true);
            }
            emit effectiveThemeChanged(effectiveTheme());
        }
    });
    m_systemThemeListener->start();
}

Theme ThemeManager::theme() const { return m_theme; }

Theme ThemeManager::effectiveTheme() const
{
    if (m_theme != Theme::Auto) {
        return m_theme;
    }

    if (m_systemThemeListener) {
        return m_systemThemeListener->systemTheme();
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (qApp && qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
        return Theme::Dark;
    }
#endif

    return Theme::Light;
}

QColor ThemeManager::accentColor() const { return m_accentColor; }

SystemThemeListener *ThemeManager::systemThemeListener() const { return m_systemThemeListener; }

void ThemeManager::setTheme(Theme theme)
{
    if (m_theme == theme) {
        if (qApp) {
            applyTo(qApp);
        } else {
            FluentStyleSheet::updateAll(effectiveTheme(), true);
        }
        return;
    }

    const Theme previousEffectiveTheme = effectiveTheme();
    m_theme = theme;

    if (qApp) {
        applyTo(qApp);
    } else {
        FluentStyleSheet::updateAll(effectiveTheme(), true);
    }

    emit themeChanged(m_theme);
    if (previousEffectiveTheme != effectiveTheme()) {
        emit effectiveThemeChanged(effectiveTheme());
    }
}

void ThemeManager::setAccentColor(const QColor &color)
{
    if (!color.isValid() || m_accentColor == color) {
        return;
    }

    m_accentColor = color;
    if (qApp) {
        applyTo(qApp);
    } else {
        FluentStyleSheet::updateAll(effectiveTheme(), true);
    }
    emit accentColorChanged(m_accentColor);
}

void ThemeManager::applyTo(QApplication *application) const
{
    if (!application) {
        return;
    }

    QPalette palette;
    if (effectiveTheme() == Theme::Dark) {
        palette.setColor(QPalette::Window, QColor(32, 32, 32));
        palette.setColor(QPalette::WindowText, QColor(243, 243, 243));
        palette.setColor(QPalette::Base, QColor(28, 28, 28));
        palette.setColor(QPalette::AlternateBase, QColor(42, 42, 42));
        palette.setColor(QPalette::Text, QColor(243, 243, 243));
        palette.setColor(QPalette::Button, QColor(45, 45, 45));
        palette.setColor(QPalette::ButtonText, QColor(243, 243, 243));
        palette.setColor(QPalette::Highlight, m_accentColor);
        palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    } else {
        palette.setColor(QPalette::Window, QColor(249, 249, 249));
        palette.setColor(QPalette::WindowText, QColor(32, 32, 32));
        palette.setColor(QPalette::Base, QColor(255, 255, 255));
        palette.setColor(QPalette::AlternateBase, QColor(243, 243, 243));
        palette.setColor(QPalette::Text, QColor(32, 32, 32));
        palette.setColor(QPalette::Button, QColor(251, 251, 251));
        palette.setColor(QPalette::ButtonText, QColor(32, 32, 32));
        palette.setColor(QPalette::Highlight, m_accentColor);
        palette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    }

    application->setPalette(palette);

    // Set application font from FluentConfig font families
    const QStringList families = Private::resolvedFontFamilies(FluentConfig::instance()->fontFamilies());
    if (!families.isEmpty()) {
        QFont appFont = application->font();
        appFont.setFamilies(families);
        application->setFont(appFont);
    }

    FluentStyleSheet::updateAll(effectiveTheme(), false);
}

void ThemeManager::applyTo(QWidget *widget) const { FluentStyleSheet::apply(widget, effectiveTheme()); }

} // namespace FluentQt
