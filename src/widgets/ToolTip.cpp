#include <FluentQtWidgets/Widgets/ToolTip.h>

#include <FluentQtWidgets/StyleSheet.h>

#include <QtCore/QEvent>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QTimer>
#include <QtGui/QHelpEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QColor>
#include <QtGui/QScreen>
#include <QtGui/QShowEvent>
#include <QtGui/QHideEvent>
#include <QtWidgets/QAbstractItemView>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTableView>

namespace FluentQt {

ToolTip::ToolTip(QWidget *parent) : QFrame(parent)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 12);
    layout->setSpacing(0);

    m_container = new QFrame(this);
    m_container->setObjectName(QStringLiteral("container"));
    layout->addWidget(m_container);

    m_containerLayout = new QHBoxLayout(m_container);
    m_containerLayout->setContentsMargins(8, 6, 8, 6);
    m_containerLayout->setSpacing(0);

    m_label = new QLabel(m_container);
    m_label->setObjectName(QStringLiteral("contentLabel"));
    m_label->setWordWrap(true);
    m_containerLayout->addWidget(m_label);

    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setBlurRadius(25);
    m_shadowEffect->setColor(QColor(0, 0, 0, 50));
    m_shadowEffect->setOffset(0, 5);
    m_container->setGraphicsEffect(m_shadowEffect);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    connect(m_timer, &QTimer::timeout, this, &ToolTip::hide);

    m_opacityAnimation = new QPropertyAnimation(this, "windowOpacity", this);
    m_opacityAnimation->setDuration(150);

    FluentStyleSheet::setRole(this, QStringLiteral("ToolTip"));
}

ToolTip::ToolTip(const QString &text, QWidget *parent) : ToolTip(parent) { setText(text); }

QString ToolTip::text() const { return m_label->text(); }

int ToolTip::duration() const { return m_duration; }

QFrame *ToolTip::container() const { return m_container; }

QHBoxLayout *ToolTip::containerLayout() const { return m_containerLayout; }

QLabel *ToolTip::label() const { return m_label; }

QPropertyAnimation *ToolTip::opacityAni() const { return m_opacityAnimation; }

QGraphicsDropShadowEffect *ToolTip::shadowEffect() const { return m_shadowEffect; }

void ToolTip::setText(const QString &text)
{
    m_label->setText(text);
    if (m_container) {
        m_container->adjustSize();
    }
    adjustSize();
}

void ToolTip::setDuration(int durationMs) { m_duration = durationMs; }

void ToolTip::adjustPos(QWidget *target, ToolTipPosition position)
{
    if (!target) {
        return;
    }

    adjustSize();
    move(calculatePosition(target, position));
}

void ToolTip::showFor(QWidget *target, ToolTipPosition position, int durationMs)
{
    if (!target) {
        return;
    }

    setDuration(durationMs);
    adjustPos(target, position);
    show();
}

ToolTip *ToolTip::showText(QWidget *target, const QString &text, ToolTipPosition position, int durationMs)
{
    auto *tip = new ToolTip(text);
    tip->showFor(target, position, durationMs);
    return tip;
}

QPoint ToolTip::calculatePosition(QWidget *target, ToolTipPosition position) const
{
    const QPoint globalTopLeft = target->mapToGlobal(QPoint(0, 0));
    const QSize targetSize = target->size();
    const QSize tipSize = sizeHint();
    const QMargins margins = layout() ? layout()->contentsMargins() : QMargins();
    QPoint pos;

    switch (position) {
    case ToolTipPosition::Top:
        pos = QPoint(globalTopLeft.x() + (targetSize.width() - tipSize.width()) / 2,
                     globalTopLeft.y() - tipSize.height());
        break;
    case ToolTipPosition::Bottom:
        pos = QPoint(globalTopLeft.x() + (targetSize.width() - tipSize.width()) / 2,
                     globalTopLeft.y() + targetSize.height());
        break;
    case ToolTipPosition::Left:
        pos = QPoint(globalTopLeft.x() - tipSize.width(),
                     globalTopLeft.y() + (targetSize.height() - tipSize.height()) / 2);
        break;
    case ToolTipPosition::Right:
        pos = QPoint(globalTopLeft.x() + targetSize.width(),
                     globalTopLeft.y() + (targetSize.height() - tipSize.height()) / 2);
        break;
    case ToolTipPosition::TopLeft:
        pos = QPoint(globalTopLeft.x() - margins.left(), globalTopLeft.y() - tipSize.height());
        break;
    case ToolTipPosition::TopRight:
        pos = QPoint(globalTopLeft.x() + targetSize.width() - tipSize.width() + margins.right(),
                     globalTopLeft.y() - tipSize.height());
        break;
    case ToolTipPosition::BottomLeft:
        pos = QPoint(globalTopLeft.x() - margins.left(), globalTopLeft.y() + targetSize.height());
        break;
    case ToolTipPosition::BottomRight:
        pos = QPoint(globalTopLeft.x() + targetSize.width() - tipSize.width() + margins.right(),
                     globalTopLeft.y() + targetSize.height());
        break;
    }

    QScreen *screen = QGuiApplication::screenAt(globalTopLeft);
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return pos;
    }

    const QRect rect = screen->availableGeometry();
    const int x = qBound(rect.left(), pos.x(), rect.right() - tipSize.width() - 4);
    const int y = qBound(rect.top(), pos.y(), rect.bottom() - tipSize.height() - 4);
    return QPoint(x, y);
}

void ToolTip::showEvent(QShowEvent *event)
{
    if (m_opacityAnimation) {
        m_opacityAnimation->stop();
        setWindowOpacity(0.0);
        m_opacityAnimation->setStartValue(0.0);
        m_opacityAnimation->setEndValue(1.0);
        m_opacityAnimation->start();
    }

    if (m_timer) {
        m_timer->stop();
        if (m_duration > 0) {
            m_timer->start(m_duration + (m_opacityAnimation ? m_opacityAnimation->duration() : 0));
        }
    }

    QFrame::showEvent(event);
}

void ToolTip::hideEvent(QHideEvent *event)
{
    if (m_timer) {
        m_timer->stop();
    }
    QFrame::hideEvent(event);
}

ToolTipFilter::ToolTipFilter(QWidget *parent, int showDelayMs, ToolTipPosition position)
    : QObject(parent), m_position(position), m_showDelayMs(qMax(0, showDelayMs))
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &ToolTipFilter::showToolTip);
}

ToolTipFilter::ToolTipFilter(const QString &text, ToolTipPosition position, QObject *parent)
    : ToolTipFilter(text, 300, position, parent)
{
}

ToolTipFilter::ToolTipFilter(const QString &text, int showDelayMs, ToolTipPosition position, QObject *parent)
    : QObject(parent), m_text(text), m_position(position), m_showDelayMs(qMax(0, showDelayMs))
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &ToolTipFilter::showToolTip);
}

bool ToolTipFilter::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ToolTip) {
        return true;
    }

    auto *widget = qobject_cast<QWidget *>(watched);
    if (!widget) {
        return QObject::eventFilter(watched, event);
    }

    if (event->type() == QEvent::Hide || event->type() == QEvent::Leave || event->type() == QEvent::MouseButtonPress) {
        hideToolTip();
    } else if (event->type() == QEvent::Enter) {
        m_pointerInside = true;
        m_targetWidget = widget;
        if (canShowToolTip(widget)) {
            if (!m_toolTip) {
                m_toolTip = createToolTip(m_text.isEmpty() ? widget->toolTip() : m_text, widget->window());
            }

            const int duration = widget->toolTipDuration() > 0 ? widget->toolTipDuration() : -1;
            m_toolTip->setDuration(duration);
            m_timer.start(m_showDelayMs);
        }
    }

    return QObject::eventFilter(watched, event);
}

int ToolTipFilter::toolTipDelay() const { return m_showDelayMs; }

void ToolTipFilter::setToolTipDelay(int delayMs) { m_showDelayMs = qMax(0, delayMs); }

void ToolTipFilter::hideToolTip()
{
    m_pointerInside = false;
    m_timer.stop();
    if (m_toolTip) {
        m_toolTip->hide();
    }
}

void ToolTipFilter::showToolTip()
{
    if (!m_pointerInside) {
        return;
    }

    auto *widget = m_targetWidget ? m_targetWidget : qobject_cast<QWidget *>(parent());
    if (!widget) {
        return;
    }

    const QString text = m_text.isEmpty() ? widget->toolTip() : m_text;
    if (text.isEmpty()) {
        return;
    }

    if (!m_toolTip) {
        m_toolTip = createToolTip(text, widget->window());
    }
    m_toolTip->setText(text);
    m_toolTip->adjustPos(widget, m_position);
    m_toolTip->show();
}

bool ToolTipFilter::canShowToolTip(QWidget *widget) const
{
    return widget && widget->isWidgetType() && widget->isEnabled() && (!m_text.isEmpty() || !widget->toolTip().isEmpty());
}

ToolTip *ToolTipFilter::createToolTip(const QString &text, QWidget *parent) const { return new ToolTip(text, parent); }

AcrylicToolTip::AcrylicToolTip(QWidget *parent) : ToolTip(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicToolTip"));
    if (container()) {
        container()->setProperty("transparent", true);
        FluentStyleSheet::polish(container());
    }
}

AcrylicToolTip::AcrylicToolTip(const QString &text, QWidget *parent) : AcrylicToolTip(parent) { setText(text); }

AcrylicToolTipFilter::AcrylicToolTipFilter(QWidget *parent, int showDelayMs, ToolTipPosition position)
    : ToolTipFilter(parent, showDelayMs, position)
{
}

AcrylicToolTipFilter::AcrylicToolTipFilter(const QString &text, ToolTipPosition position, QObject *parent)
    : ToolTipFilter(text, position, parent)
{
}

AcrylicToolTipFilter::AcrylicToolTipFilter(const QString &text, int showDelayMs, ToolTipPosition position,
                                           QObject *parent)
    : ToolTipFilter(text, showDelayMs, position, parent)
{
}

ToolTip *AcrylicToolTipFilter::createToolTip(const QString &text, QWidget *parent) const
{
    return new AcrylicToolTip(text, parent);
}

ItemViewToolTipDelegate::ItemViewToolTipDelegate(QAbstractItemView *parent, int showDelayMs, ItemViewToolTipType type)
    : QObject(parent), m_view(parent), m_type(type), m_showDelayMs(showDelayMs)
{
    m_timer.setSingleShot(true);
    connect(&m_timer, &QTimer::timeout, this, &ItemViewToolTipDelegate::showToolTip);

    if (m_view) {
        m_view->installEventFilter(this);
        if (m_view->viewport()) {
            m_view->viewport()->installEventFilter(this);
        }
        if (m_view->horizontalScrollBar()) {
            connect(m_view->horizontalScrollBar(), &QScrollBar::valueChanged, this, &ItemViewToolTipDelegate::hideToolTip);
        }
        if (m_view->verticalScrollBar()) {
            connect(m_view->verticalScrollBar(), &QScrollBar::valueChanged, this, &ItemViewToolTipDelegate::hideToolTip);
        }
    }
}

bool ItemViewToolTipDelegate::helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &,
                                        const QModelIndex &index)
{
    if (!event || !view || event->type() != QEvent::ToolTip) {
        return false;
    }

    const QString text = index.data(Qt::ToolTipRole).toString();
    if (text.isEmpty()) {
        hideToolTip();
        return false;
    }

    m_text = text;
    m_currentIndex = index;
    m_pointerInside = true;
    m_timer.start(m_showDelayMs);
    return true;
}

void ItemViewToolTipDelegate::setToolTipDelay(int delayMs) { m_showDelayMs = qMax(0, delayMs); }

void ItemViewToolTipDelegate::setToolTipDuration(int durationMs)
{
    m_durationMs = durationMs;
    if (m_toolTip) {
        m_toolTip->setDuration(durationMs);
    }
}

void ItemViewToolTipDelegate::hideToolTip()
{
    m_pointerInside = false;
    m_timer.stop();
    if (m_toolTip) {
        m_toolTip->hide();
    }
}

bool ItemViewToolTipDelegate::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_view) {
        return QObject::eventFilter(watched, event);
    }

    if (watched == m_view) {
        if (event->type() == QEvent::Hide || event->type() == QEvent::Leave) {
            hideToolTip();
        } else if (event->type() == QEvent::Enter) {
            m_pointerInside = true;
        }
    } else if (watched == m_view->viewport()) {
        if (event->type() == QEvent::MouseButtonPress) {
            hideToolTip();
        }
    }

    return QObject::eventFilter(watched, event);
}

void ItemViewToolTipDelegate::showToolTip()
{
    if (!m_pointerInside || !m_view || !m_currentIndex.isValid()) {
        return;
    }

    if (!m_toolTip) {
        m_toolTip = new ToolTip(m_text, m_view->window());
        m_toolTip->setDuration(m_durationMs);
    } else {
        m_toolTip->setText(m_text);
    }

    const QRect itemRect = m_view->visualRect(m_currentIndex);
    m_toolTip->move(calculatePosition(m_view, itemRect));
    m_toolTip->show();
}

QPoint ItemViewToolTipDelegate::calculatePosition(QAbstractItemView *view, const QRect &itemRect) const
{
    const QPoint topLeft = view->mapToGlobal(itemRect.topLeft());
    int x = topLeft.x();
    int y = topLeft.y() - (m_toolTip ? m_toolTip->height() : 0) + 10;

    if (m_type == ItemViewToolTipType::Table) {
        if (auto *tableView = qobject_cast<QTableView *>(view)) {
            if (tableView->verticalHeader() && tableView->verticalHeader()->isVisible()) {
                x += tableView->verticalHeader()->width();
            }
            if (tableView->horizontalHeader() && tableView->horizontalHeader()->isVisible()) {
                y += tableView->horizontalHeader()->height();
            }
        }
    }

    return QPoint(x, y);
}

} // namespace FluentQt
