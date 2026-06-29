#include <FluentQtWidgets/Navigation/NavigationHistory.h>

#include <algorithm>

#include <QtCore/QHash>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QWidget>

namespace FluentQt {

NavigationHistory::NavigationHistory(QObject *parent) : QObject(parent) {}

void NavigationHistory::setDefaultRouteKey(QStackedWidget *stackedWidget, const QString &routeKey)
{
    if (!stackedWidget || routeKey.isEmpty()) {
        return;
    }

    StackHistory &history = historyFor(stackedWidget);
    history.defaultRouteKey = routeKey;
    if (history.history.isEmpty()) {
        history.history.append(routeKey);
    } else {
        history.history[0] = routeKey;
    }
    emitEmptyChanged();
}

void NavigationHistory::push(QStackedWidget *stackedWidget, const QString &routeKey)
{
    if (!stackedWidget || routeKey.isEmpty()) {
        return;
    }

    StackHistory &history = historyFor(stackedWidget);
    if (history.history.isEmpty()) {
        history.history.append(routeKey);
        emitEmptyChanged();
        return;
    }

    if (history.history.constLast() == routeKey) {
        return;
    }

    history.history.append(routeKey);
    emitEmptyChanged();
}

void NavigationHistory::pop(QStackedWidget *stackedWidget)
{
    if (!stackedWidget || !canGoBack(stackedWidget)) {
        return;
    }

    StackHistory &history = historyFor(stackedWidget);
    history.history.removeLast();
    navigateToTop(stackedWidget, history);
    emitEmptyChanged();
}

void NavigationHistory::remove(QStackedWidget *stackedWidget, const QString &routeKey)
{
    if (!stackedWidget || routeKey.isEmpty()) {
        return;
    }

    StackHistory &history = historyFor(stackedWidget);
    if (history.history.size() <= 1) {
        return;
    }

    QStringList filtered;
    filtered.reserve(history.history.size());
    filtered.append(history.history.constFirst());
    for (int i = 1; i < history.history.size(); ++i) {
        if (history.history.at(i) != routeKey) {
            filtered.append(history.history.at(i));
        }
    }

    filtered.erase(std::unique(filtered.begin() + 1, filtered.end()), filtered.end());
    history.history = filtered;
    navigateToTop(stackedWidget, history);
    emitEmptyChanged();
}

bool NavigationHistory::canGoBack(QStackedWidget *stackedWidget) const
{
    if (!stackedWidget || !m_histories.contains(stackedWidget)) {
        return false;
    }

    return m_histories.value(stackedWidget).history.size() > 1;
}

QString NavigationHistory::topRouteKey(QStackedWidget *stackedWidget) const
{
    if (!stackedWidget || !m_histories.contains(stackedWidget)) {
        return QString();
    }

    const StackHistory &history = m_histories.value(stackedWidget);
    return history.history.isEmpty() ? QString() : history.history.constLast();
}

void NavigationHistory::clear(QStackedWidget *stackedWidget)
{
    if (!stackedWidget) {
        return;
    }

    m_histories.remove(stackedWidget);
    emitEmptyChanged();
}

NavigationHistory::StackHistory &NavigationHistory::historyFor(QStackedWidget *stackedWidget)
{
    return m_histories[stackedWidget];
}

const NavigationHistory::StackHistory &NavigationHistory::historyFor(QStackedWidget *stackedWidget) const
{
    static const StackHistory empty;
    const auto it = m_histories.constFind(stackedWidget);
    return it != m_histories.constEnd() ? it.value() : empty;
}

void NavigationHistory::navigateToTop(QStackedWidget *stackedWidget, StackHistory &history)
{
    if (history.history.isEmpty()) {
        return;
    }

    const QString routeKey = history.history.constLast();
    QWidget *page = stackedWidget->findChild<QWidget *>(routeKey);
    if (page) {
        stackedWidget->setCurrentWidget(page);
    }
}

void NavigationHistory::emitEmptyChanged()
{
    bool hasHistory = false;
    for (auto it = m_histories.constBegin(); it != m_histories.constEnd(); ++it) {
        if (it.value().history.size() > 1) {
            hasHistory = true;
            break;
        }
    }
    emit emptyChanged(!hasHistory);
}

} // namespace FluentQt
