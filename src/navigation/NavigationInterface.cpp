#include <FluentQtWidgets/Navigation/NavigationInterface.h>

#include <FluentQtWidgets/StyleSheet.h>

#include <QtCore/QEvent>
#include <QtGui/QResizeEvent>

namespace FluentQt {

NavigationInterface::NavigationInterface(QWidget *parent, bool showMenuButton, bool showReturnButton, bool collapsible)
    : QWidget(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("NavigationInterface"));

    m_panel = new NavigationPanel(this);
    m_panel->setMenuButtonVisible(showMenuButton && collapsible);
    m_panel->setReturnButtonVisible(showReturnButton);
    m_panel->setCollapsible(collapsible);
    m_panel->installEventFilter(this);
    connect(m_panel, &NavigationPanel::displayModeChanged, this, &NavigationInterface::displayModeChanged);

    resize(NavigationPanel::kCompactWidth, height());
    setMinimumWidth(NavigationPanel::kCompactWidth);
    setAttribute(Qt::WA_TranslucentBackground);
}

NavigationPanel *NavigationInterface::panel() const { return m_panel; }

NavigationTreeWidget *NavigationInterface::addItem(const QString &routeKey, const QIcon &icon, const QString &text,
                                                   NavigationItemPosition position, const QString &parentRouteKey,
                                                   bool selectable)
{
    return insertItem(-1, routeKey, icon, text, position, parentRouteKey, selectable);
}

NavigationTreeWidget *NavigationInterface::insertItem(int index, const QString &routeKey, const QIcon &icon,
                                                      const QString &text, NavigationItemPosition position,
                                                      const QString &parentRouteKey, bool selectable)
{
    NavigationTreeWidget *item = m_panel->insertItem(index, routeKey, icon, text, position, parentRouteKey, selectable);
    updateMinimumHeight();
    return item;
}

NavigationWidget *NavigationInterface::addWidget(const QString &routeKey, NavigationWidget *widget,
                                                 NavigationItemPosition position, const QString &parentRouteKey)
{
    return insertWidget(-1, routeKey, widget, position, parentRouteKey);
}

NavigationWidget *NavigationInterface::insertWidget(int index, const QString &routeKey, NavigationWidget *widget,
                                                    NavigationItemPosition position, const QString &parentRouteKey)
{
    NavigationWidget *added = m_panel->insertWidget(index, routeKey, widget, position, parentRouteKey);
    updateMinimumHeight();
    return added;
}

QFrame *NavigationInterface::addSeparator(NavigationItemPosition position) { return insertSeparator(-1, position); }

QFrame *NavigationInterface::insertSeparator(int index, NavigationItemPosition position)
{
    QFrame *separator = m_panel->insertSeparator(index, position);
    updateMinimumHeight();
    return separator;
}

NavigationItemHeader *NavigationInterface::addItemHeader(const QString &text, NavigationItemPosition position)
{
    return m_panel->addItemHeader(text, position);
}

NavigationItemHeader *NavigationInterface::insertItemHeader(int index, const QString &text,
                                                            NavigationItemPosition position)
{
    return m_panel->insertItemHeader(index, text, position);
}

NavigationUserCard *NavigationInterface::addUserCard(const QString &routeKey, const QIcon &avatar,
                                                     const QString &title, const QString &subtitle,
                                                     NavigationItemPosition position)
{
    return m_panel->addUserCard(routeKey, avatar, title, subtitle, position);
}

NavigationUserCard *NavigationInterface::addUserCard(const QString &routeKey, const QString &avatarPath,
                                                     const QString &title, const QString &subtitle,
                                                     NavigationItemPosition position)
{
    return m_panel->addUserCard(routeKey, avatarPath, title, subtitle, position);
}

NavigationWidget *NavigationInterface::widget(const QString &routeKey) const
{
    return m_panel->navigationWidget(routeKey);
}

bool NavigationInterface::isAcrylicEnabled() const { return m_panel->isAcrylicEnabled(); }

bool NavigationInterface::isIndicatorAnimationEnabled() const { return m_panel->isIndicatorAnimationEnabled(); }

void NavigationInterface::removeWidget(const QString &routeKey) { m_panel->removeWidget(routeKey); }

void NavigationInterface::setCurrentItem(const QString &routeKey) { m_panel->setCurrentItem(routeKey); }

void NavigationInterface::expand(bool animated) { m_panel->expand(animated); }

void NavigationInterface::toggle() { m_panel->toggle(); }

void NavigationInterface::setExpandWidth(int width) { m_panel->setExpandWidth(width); }

void NavigationInterface::setMinimumExpandWidth(int width) { m_panel->setMinimumExpandWidth(width); }

void NavigationInterface::setMenuButtonVisible(bool visible) { m_panel->setMenuButtonVisible(visible); }

void NavigationInterface::setReturnButtonVisible(bool visible) { m_panel->setReturnButtonVisible(visible); }

void NavigationInterface::setCollapsible(bool collapsible) { m_panel->setCollapsible(collapsible); }

void NavigationInterface::setAcrylicEnabled(bool enabled) { m_panel->setAcrylicEnabled(enabled); }

void NavigationInterface::setIndicatorAnimationEnabled(bool enabled) { m_panel->setIndicatorAnimationEnabled(enabled); }

// The interface follows the panel's width while it is compact or expanded in place; as a menu the
// panel sits on the window and the interface keeps its compact width.
bool NavigationInterface::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_panel && event->type() == QEvent::Resize &&
        m_panel->displayMode() != NavigationDisplayMode::Menu) {
        auto *resizeEvent = static_cast<QResizeEvent *>(event);
        if (resizeEvent->oldSize().width() != resizeEvent->size().width()) {
            setFixedWidth(resizeEvent->size().width());
        }
    }
    return QWidget::eventFilter(watched, event);
}

void NavigationInterface::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (event->oldSize().height() != height()) {
        m_panel->setFixedHeight(height());
    }
}

void NavigationInterface::updateMinimumHeight() { setMinimumHeight(m_panel->layoutMinHeight()); }

} // namespace FluentQt
