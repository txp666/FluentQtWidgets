#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QStringList>

class QStackedWidget;

namespace FluentQt {

class FQW_API NavigationHistory : public QObject
{
    Q_OBJECT

  public:
    explicit NavigationHistory(QObject *parent = nullptr);

    void setDefaultRouteKey(QStackedWidget *stackedWidget, const QString &routeKey);
    void push(QStackedWidget *stackedWidget, const QString &routeKey);
    void pop(QStackedWidget *stackedWidget);
    void remove(QStackedWidget *stackedWidget, const QString &routeKey);
    bool canGoBack(QStackedWidget *stackedWidget) const;
    QString topRouteKey(QStackedWidget *stackedWidget) const;
    void clear(QStackedWidget *stackedWidget);

  signals:
    void emptyChanged(bool isEmpty);

  private:
    struct StackHistory
    {
        QString defaultRouteKey;
        QStringList history;
    };

    StackHistory &historyFor(QStackedWidget *stackedWidget);
    const StackHistory &historyFor(QStackedWidget *stackedWidget) const;
    void navigateToTop(QStackedWidget *stackedWidget, StackHistory &history);
    void emitEmptyChanged();

    QHash<QStackedWidget *, StackHistory> m_histories;
};

} // namespace FluentQt
