#include <FluentQtWidgets/Widgets/InfoBadgeManager.h>

#include <FluentQtWidgets/Widgets/InfoBadge.h>

#include <QtCore/QEvent>
#include <QtWidgets/QWidget>

namespace FluentQt {

namespace {

InfoBadgeManager *makeTopRight(QWidget *target, InfoBadge *badge)
{
    return new TopRightInfoBadgeManager(target, badge);
}

InfoBadgeManager *makeRight(QWidget *target, InfoBadge *badge) { return new RightInfoBadgeManager(target, badge); }

InfoBadgeManager *makeBottomRight(QWidget *target, InfoBadge *badge)
{
    return new BottomRightInfoBadgeManager(target, badge);
}

InfoBadgeManager *makeTopLeft(QWidget *target, InfoBadge *badge) { return new TopLeftInfoBadgeManager(target, badge); }

InfoBadgeManager *makeLeft(QWidget *target, InfoBadge *badge) { return new LeftInfoBadgeManager(target, badge); }

InfoBadgeManager *makeBottomLeft(QWidget *target, InfoBadge *badge)
{
    return new BottomLeftInfoBadgeManager(target, badge);
}

InfoBadgeManager *makeNavigationItem(QWidget *target, InfoBadge *badge)
{
    return new NavigationItemInfoBadgeManager(target, badge);
}

using InfoBadgeManagerFactory = InfoBadgeManager *(*)(QWidget *, InfoBadge *);

struct InfoBadgeManagerRegistry
{
    QHash<InfoBadgePosition, InfoBadgeManagerFactory> positions;
    QHash<QString, InfoBadgeManagerFactory> names;

    InfoBadgeManagerRegistry()
    {
        positions.insert(InfoBadgePosition::TopRight, makeTopRight);
        positions.insert(InfoBadgePosition::Right, makeRight);
        positions.insert(InfoBadgePosition::BottomRight, makeBottomRight);
        positions.insert(InfoBadgePosition::TopLeft, makeTopLeft);
        positions.insert(InfoBadgePosition::Left, makeLeft);
        positions.insert(InfoBadgePosition::BottomLeft, makeBottomLeft);
        positions.insert(InfoBadgePosition::NavigationItem, makeNavigationItem);
    }
};

InfoBadgeManagerRegistry &registry()
{
    static InfoBadgeManagerRegistry instance;
    return instance;
}

InfoBadgeManager *makeManager(InfoBadgePosition position, QWidget *target, InfoBadge *badge)
{
    const auto factory = registry().positions.value(position);
    if (!factory) {
        return nullptr;
    }
    return factory(target, badge);
}

} // namespace

InfoBadgeManager::InfoBadgeManager(QWidget *target, InfoBadge *badge, QObject *parent)
    : QObject(parent), m_target(target), m_badge(badge)
{
    if (m_target) {
        m_target->installEventFilter(this);
    }
    if (m_badge && m_target) {
        if (m_target->parentWidget()) {
            m_badge->setParent(m_target->parentWidget());
        } else {
            m_badge->setParent(m_target);
        }
        m_badge->setManager(this);
        m_badge->raise();
        m_badge->move(position());
        m_badge->show();
    }
}

QWidget *InfoBadgeManager::target() const { return m_target; }

InfoBadge *InfoBadgeManager::badge() const { return m_badge; }

QPoint InfoBadgeManager::position() const { return QPoint(); }

bool InfoBadgeManager::eventFilter(QObject *watched, QEvent *event)
{
    if (m_target && watched == m_target && m_badge) {
        if (event->type() == QEvent::Resize || event->type() == QEvent::Move || event->type() == QEvent::Show) {
            m_badge->move(position());
        }
    }
    return QObject::eventFilter(watched, event);
}

InfoBadgeManager *InfoBadgeManager::make(InfoBadgePosition position, QWidget *target, InfoBadge *badge)
{
    return makeManager(position, target, badge);
}

InfoBadgeManager *InfoBadgeManager::make(const QString &name, QWidget *target, InfoBadge *badge)
{
    const auto factory = registry().names.value(name);
    if (!factory) {
        return nullptr;
    }
    return factory(target, badge);
}

void InfoBadgeManager::registerManager(InfoBadgePosition position, const QString &name)
{
    const auto factory = registry().positions.value(position);
    if (factory && !name.isEmpty()) {
        registry().names.insert(name, factory);
    }
}

QStringList InfoBadgeManager::registeredNames() { return registry().names.keys(); }

InfoBadge *InfoBadgeManager::attach(InfoBadge *badge, QWidget *target, InfoBadgePosition position)
{
    if (!badge || !target) {
        return badge;
    }

    badge->adjustSize();
    InfoBadgeManager *manager = make(position, target, badge);
    if (manager) {
        badge->move(manager->position());
    }
    return badge;
}

QPoint TopRightInfoBadgeManager::position() const
{
    if (!m_target || !m_badge) {
        return QPoint();
    }

    const QPoint topRight = m_target->geometry().topRight();
    return QPoint(topRight.x() - m_badge->width() / 2, topRight.y() - m_badge->height() / 2);
}

QPoint RightInfoBadgeManager::position() const
{
    if (!m_target || !m_badge) {
        return QPoint();
    }

    const QRect geometry = m_target->geometry();
    return QPoint(geometry.right() - m_badge->width() / 2, geometry.center().y() - m_badge->height() / 2);
}

QPoint BottomRightInfoBadgeManager::position() const
{
    if (!m_target || !m_badge) {
        return QPoint();
    }

    const QPoint bottomRight = m_target->geometry().bottomRight();
    return QPoint(bottomRight.x() - m_badge->width() / 2, bottomRight.y() - m_badge->height() / 2);
}

QPoint TopLeftInfoBadgeManager::position() const
{
    if (!m_target || !m_badge) {
        return QPoint();
    }

    return QPoint(m_target->x() - m_badge->width() / 2, m_target->y() - m_badge->height() / 2);
}

QPoint LeftInfoBadgeManager::position() const
{
    if (!m_target || !m_badge) {
        return QPoint();
    }

    const QRect geometry = m_target->geometry();
    return QPoint(geometry.x() - m_badge->width() / 2, geometry.center().y() - m_badge->height() / 2);
}

QPoint BottomLeftInfoBadgeManager::position() const
{
    if (!m_target || !m_badge) {
        return QPoint();
    }

    const QPoint bottomLeft = m_target->geometry().bottomLeft();
    return QPoint(bottomLeft.x() - m_badge->width() / 2, bottomLeft.y() - m_badge->height() / 2);
}

QPoint NavigationItemInfoBadgeManager::position() const
{
    if (!m_target || !m_badge) {
        return QPoint();
    }

    m_badge->setVisible(m_target->isVisible());
    const QRect geometry = m_target->geometry();
    const int x = geometry.right() - m_badge->width() - 10;
    const int y = geometry.center().y() - m_badge->height() / 2;
    return QPoint(x, y);
}

bool NavigationItemInfoBadgeManager::eventFilter(QObject *watched, QEvent *event)
{
    if (m_target && watched == m_target && m_badge && event->type() == QEvent::Show) {
        m_badge->show();
    }
    return InfoBadgeManager::eventFilter(watched, event);
}

} // namespace FluentQt
