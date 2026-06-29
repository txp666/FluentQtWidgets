#include <FluentQtWidgets/ThemeListener.h>

#include <QtCore/QTimerEvent>
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

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    if (qApp && qApp->styleHints()) {
        connect(qApp->styleHints(),
                &QStyleHints::colorSchemeChanged,
                this,
                &SystemThemeListener::refresh,
                Qt::UniqueConnection);
    }
#endif

    if (m_timerId == 0) {
        m_timerId = startTimer(2000);
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

    if (m_timerId != 0) {
        killTimer(m_timerId);
        m_timerId = 0;
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

    if (qApp && qApp->palette().color(QPalette::Window).lightness() < 128) {
        return Theme::Dark;
    }

    return Theme::Light;
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

void SystemThemeListener::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timerId) {
        refresh();
        return;
    }

    QObject::timerEvent(event);
}

} // namespace FluentQt
