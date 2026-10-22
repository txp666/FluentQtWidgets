#include <FluentQtWidgets/ThemeListener.h>

#include <QtCore/QEvent>
#include <QtGui/QPalette>
#include <QtWidgets/QApplication>

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
#include <QtGui/QStyleHints>
#endif

namespace FluentQt {

SystemThemeListener::SystemThemeListener(QObject *parent) : QObject(parent) { m_systemTheme = detectSystemTheme(); }

bool SystemThemeListener::isRunning() const { return m_running; }

Theme SystemThemeListener::systemTheme() const { return m_systemTheme; }

void SystemThemeListener::start()
{
    if (m_running) {
        return;
    }

    refresh();

    // Change notifications instead of polling: Qt 6.5 reports color scheme changes, and the
    // system palette fallback below changes together with an ApplicationPaletteChange.
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (qApp && qApp->styleHints()) {
        connect(qApp->styleHints(),
                &QStyleHints::colorSchemeChanged,
                this,
                &SystemThemeListener::refresh,
                Qt::UniqueConnection);
    }
#endif
    if (qApp) {
        qApp->installEventFilter(this);
    }

    setRunning(true);
}

void SystemThemeListener::stop()
{
    if (!m_running) {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (qApp && qApp->styleHints()) {
        disconnect(qApp->styleHints(), nullptr, this, nullptr);
    }
#endif

    if (qApp) {
        qApp->removeEventFilter(this);
    }

    setRunning(false);
}

void SystemThemeListener::refresh() { setSystemTheme(detectSystemTheme()); }

Theme SystemThemeListener::detectSystemTheme() const
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (qApp && qApp->styleHints()) {
        const Qt::ColorScheme colorScheme = qApp->styleHints()->colorScheme();
        if (colorScheme == Qt::ColorScheme::Dark) {
            return Theme::Dark;
        }
        if (colorScheme == Qt::ColorScheme::Light) {
            return Theme::Light;
        }
    }
#endif

    if (!qApp) {
        return m_systemTheme;
    }

    // The application palette only tells the system theme while nobody has set it. Once
    // ThemeManager::applyTo() (or the application) installs its own, it would just reflect the
    // theme back, and Auto could never leave dark again; keep the last known value instead.
    const QPalette palette = qApp->palette();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const bool explicitPalette = palette.resolveMask() != 0;
#else
    const bool explicitPalette = palette.resolve() != 0;
#endif
    if (explicitPalette) {
        return m_systemTheme;
    }
    return palette.color(QPalette::Window).lightness() < 128 ? Theme::Dark : Theme::Light;
}

void SystemThemeListener::setRunning(bool running)
{
    if (m_running == running) {
        return;
    }

    m_running = running;
    emit runningChanged(m_running);
}

void SystemThemeListener::setSystemTheme(Theme theme)
{
    if (m_systemTheme == theme) {
        return;
    }

    m_systemTheme = theme;
    emit systemThemeChanged(m_systemTheme);
}

bool SystemThemeListener::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == qApp && event->type() == QEvent::ApplicationPaletteChange) {
        refresh();
    }
    return QObject::eventFilter(watched, event);
}

} // namespace FluentQt
