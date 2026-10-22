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
    Theme m_systemTheme = Theme::Light;
};

} // namespace FluentQt
