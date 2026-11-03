#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QObject>

class QEvent;

namespace FluentQt {

class FQW_API SystemThemeListener : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool running READ isRunning NOTIFY runningChanged)
    Q_PROPERTY(FluentQt::Theme systemTheme READ systemTheme NOTIFY systemThemeChanged)

  public:
    explicit SystemThemeListener(QObject *parent = nullptr);

    bool isRunning() const;
    Theme systemTheme() const;

    // While the application requests a color scheme of its own (ThemeManager does for Theme::Light and Theme::Dark),
    // Qt reports that scheme instead of the system's, so the listener keeps the system theme it last saw.
    void setColorSchemeRequested(bool requested);

  public slots:
    void start();
    void stop();
    void refresh();

  signals:
    void runningChanged(bool running);
    void systemThemeChanged(FluentQt::Theme theme);

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

  private:
    Theme detectSystemTheme() const;
    void setRunning(bool running);
    void setSystemTheme(Theme theme);

    bool m_running = false;
    bool m_colorSchemeRequested = false;
    Theme m_systemTheme = Theme::Light;
};

} // namespace FluentQt
