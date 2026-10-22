#include <FluentQtWidgets/Widgets/InfoBarManager.h>
#include <FluentQtWidgets/Widgets/InfoBar.h>

#include <QtCore/QEvent>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QEasingCurve>
#include <QtCore/QVariant>

namespace FluentQt {

InfoBarManager *InfoBarManager::s_managers[7] = {};

InfoBarManager::InfoBarManager(QObject *parent)
    : QObject(parent)
{
}

InfoBarManager *InfoBarManager::make(InfoBarPosition position)
{
    const int idx = static_cast<int>(position);
    if (idx < 0 || idx >= 7) {
        return nullptr;
    }
    if (!s_managers[idx]) {
        switch (position) {
        case InfoBarPosition::Top:
            s_managers[idx] = new TopInfoBarManager;
            break;
        case InfoBarPosition::TopRight:
            s_managers[idx] = new TopRightInfoBarManager;
            break;
        case InfoBarPosition::TopLeft:
            s_managers[idx] = new TopLeftInfoBarManager;
            break;
        case InfoBarPosition::Bottom:
            s_managers[idx] = new BottomInfoBarManager;
            break;
        case InfoBarPosition::BottomRight:
            s_managers[idx] = new BottomRightInfoBarManager;
            break;
        case InfoBarPosition::BottomLeft:
            s_managers[idx] = new BottomLeftInfoBarManager;
            break;
        default:
            return nullptr;
        }
    }
    return s_managers[idx];
}

void InfoBarManager::add(InfoBar *infoBar)
{
    QWidget *p = infoBar->parentWidget();
    if (!p) {
        return;
    }

    if (!m_infoBars.contains(p)) {
        p->installEventFilter(this);
        m_infoBars[p] = QList<InfoBar *>();
        m_aniGroups[p] = new QParallelAnimationGroup(this);
        // The parent (and with it its bars) can go away without any bar being closed,
        // e.g. a WA_DeleteOnClose window; a later widget may then reuse the address.
        connect(p, &QObject::destroyed, this, [this, p]() { forgetParent(p); });
    }

    if (m_infoBars[p].contains(infoBar)) {
        return;
    }

    // Add drop animation for existing bars
    if (!m_infoBars[p].isEmpty()) {
        auto *dropAni = new QPropertyAnimation(infoBar, "pos");
        dropAni->setDuration(200);
        m_aniGroups[p]->addAnimation(dropAni);
        m_dropAnis.insert(infoBar, dropAni);
        infoBar->setProperty("dropAni", QVariant::fromValue(dropAni));
    }

    m_infoBars[p].append(infoBar);

    // Create and start slide animation
    auto *slideAni = createSlideAni(infoBar);
    m_slideAnis.insert(infoBar, slideAni);
    infoBar->setProperty("slideAni", QVariant::fromValue(slideAni));

    connect(infoBar, &InfoBar::closedSignal, this, [this, infoBar]() {
        remove(infoBar);
    });
    // A bar deleted without close() must not stay in the stacking list.
    connect(infoBar, &QObject::destroyed, this, [this, p, infoBar]() {
        if (forget(p, infoBar)) {
            QMetaObject::invokeMethod(this, [this, p]() { reposition(p); }, Qt::QueuedConnection);
        }
    });

    slideAni->start();
}

void InfoBarManager::remove(InfoBar *infoBar)
{
    QWidget *p = infoBar->parentWidget();
    if (forget(p, infoBar)) {
        reposition(p);
    }
}

bool InfoBarManager::forget(QWidget *parent, InfoBar *infoBar)
{
    const auto bars = m_infoBars.find(parent);
    if (bars == m_infoBars.end() || !bars->removeOne(infoBar)) {
        return false;
    }

    if (QPropertyAnimation *dropAni = m_dropAnis.take(infoBar)) {
        if (QParallelAnimationGroup *group = m_aniGroups.value(parent)) {
            group->removeAnimation(dropAni);
        }
        delete dropAni;
    }
    // The slide animation is a child of the bar and goes away with it.
    m_slideAnis.remove(infoBar);
    return true;
}

void InfoBarManager::forgetParent(QWidget *parent)
{
    const QList<InfoBar *> bars = m_infoBars.take(parent);
    for (InfoBar *bar : bars) {
        m_dropAnis.remove(bar);
        m_slideAnis.remove(bar);
    }
    // Owns the drop animations of the bars that belonged to this parent.
    delete m_aniGroups.take(parent);
}

void InfoBarManager::reposition(QWidget *parent)
{
    // The parent may have been destroyed (and forgotten) before a queued call runs.
    if (!m_infoBars.contains(parent)) {
        return;
    }

    updateDropAni(parent);
    m_aniGroups[parent]->start();
}

QPropertyAnimation *InfoBarManager::createSlideAni(InfoBar *infoBar)
{
    auto *ani = new QPropertyAnimation(infoBar, "pos", infoBar);
    ani->setEasingCurve(QEasingCurve::OutQuad);
    ani->setDuration(200);
    ani->setStartValue(slideStartPos(infoBar));
    ani->setEndValue(pos(infoBar));
    return ani;
}

void InfoBarManager::updateDropAni(QWidget *parent)
{
    if (!m_infoBars.contains(parent)) {
        return;
    }

    for (InfoBar *bar : m_infoBars[parent]) {
        QPropertyAnimation *ani = m_dropAnis.value(bar);
        if (!ani) {
            continue;
        }
        ani->setStartValue(bar->pos());
        ani->setEndValue(pos(bar));
    }
}

bool InfoBarManager::eventFilter(QObject *watched, QEvent *event)
{
    auto *widget = qobject_cast<QWidget *>(watched);
    if (!widget || !m_infoBars.contains(widget)) {
        return QObject::eventFilter(watched, event);
    }

    if (event->type() == QEvent::Resize || event->type() == QEvent::WindowStateChange) {
        for (InfoBar *bar : m_infoBars[widget]) {
            bar->move(pos(bar, widget->size()));
        }
    }

    return QObject::eventFilter(watched, event);
}

// --- TopInfoBarManager ---

QPoint TopInfoBarManager::pos(InfoBar *infoBar, const QSize &parentSize) const
{
    QWidget *p = infoBar->parentWidget();
    const QSize size = parentSize.isValid() ? parentSize : p->size();

    int x = (size.width() - infoBar->width()) / 2;
    int y = m_margin;

    int index = m_infoBars[p].indexOf(infoBar);
    for (int i = 0; i < index; ++i) {
        y += m_infoBars[p][i]->height() + m_spacing;
    }

    return QPoint(x, y);
}

QPoint TopInfoBarManager::slideStartPos(InfoBar *infoBar) const
{
    QPoint p = pos(infoBar);
    return QPoint(p.x(), p.y() - 16);
}

// --- TopRightInfoBarManager ---

QPoint TopRightInfoBarManager::pos(InfoBar *infoBar, const QSize &parentSize) const
{
    QWidget *p = infoBar->parentWidget();
    const QSize size = parentSize.isValid() ? parentSize : p->size();

    int x = size.width() - infoBar->width() - m_margin;
    int y = m_margin;

    int index = m_infoBars[p].indexOf(infoBar);
    for (int i = 0; i < index; ++i) {
        y += m_infoBars[p][i]->height() + m_spacing;
    }

    return QPoint(x, y);
}

QPoint TopRightInfoBarManager::slideStartPos(InfoBar *infoBar) const
{
    return QPoint(infoBar->parentWidget()->width(), pos(infoBar).y());
}

// --- TopLeftInfoBarManager ---

QPoint TopLeftInfoBarManager::pos(InfoBar *infoBar, const QSize &parentSize) const
{
    QWidget *p = infoBar->parentWidget();
    Q_UNUSED(parentSize);

    int y = m_margin;
    int index = m_infoBars[p].indexOf(infoBar);
    for (int i = 0; i < index; ++i) {
        y += m_infoBars[p][i]->height() + m_spacing;
    }

    return QPoint(m_margin, y);
}

QPoint TopLeftInfoBarManager::slideStartPos(InfoBar *infoBar) const
{
    return QPoint(-infoBar->width(), pos(infoBar).y());
}

// --- BottomInfoBarManager ---

QPoint BottomInfoBarManager::pos(InfoBar *infoBar, const QSize &parentSize) const
{
    QWidget *p = infoBar->parentWidget();
    const QSize size = parentSize.isValid() ? parentSize : p->size();

    int x = (size.width() - infoBar->width()) / 2;
    int y = size.height() - infoBar->height() - m_margin;

    int index = m_infoBars[p].indexOf(infoBar);
    for (int i = 0; i < index; ++i) {
        y -= (m_infoBars[p][i]->height() + m_spacing);
    }

    return QPoint(x, y);
}

QPoint BottomInfoBarManager::slideStartPos(InfoBar *infoBar) const
{
    QPoint p = pos(infoBar);
    return QPoint(p.x(), p.y() + 16);
}

// --- BottomRightInfoBarManager ---

QPoint BottomRightInfoBarManager::pos(InfoBar *infoBar, const QSize &parentSize) const
{
    QWidget *p = infoBar->parentWidget();
    const QSize size = parentSize.isValid() ? parentSize : p->size();

    int x = size.width() - infoBar->width() - m_margin;
    int y = size.height() - infoBar->height() - m_margin;

    int index = m_infoBars[p].indexOf(infoBar);
    for (int i = 0; i < index; ++i) {
        y -= (m_infoBars[p][i]->height() + m_spacing);
    }

    return QPoint(x, y);
}

QPoint BottomRightInfoBarManager::slideStartPos(InfoBar *infoBar) const
{
    return QPoint(infoBar->parentWidget()->width(), pos(infoBar).y());
}

// --- BottomLeftInfoBarManager ---

QPoint BottomLeftInfoBarManager::pos(InfoBar *infoBar, const QSize &parentSize) const
{
    QWidget *p = infoBar->parentWidget();
    const QSize size = parentSize.isValid() ? parentSize : p->size();

    int y = size.height() - infoBar->height() - m_margin;
    int index = m_infoBars[p].indexOf(infoBar);
    for (int i = 0; i < index; ++i) {
        y -= (m_infoBars[p][i]->height() + m_spacing);
    }

    return QPoint(m_margin, y);
}

QPoint BottomLeftInfoBarManager::slideStartPos(InfoBar *infoBar) const
{
    return QPoint(-infoBar->width(), pos(infoBar).y());
}

} // namespace FluentQt
