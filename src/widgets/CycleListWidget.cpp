#include <FluentQtWidgets/Widgets/CycleListWidget.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>

#include <QtCore/QEvent>
#include <QtCore/QEasingCurve>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/QEnterEvent>
#endif
#include <QtGui/QKeyEvent>
#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QScrollBar>

namespace FluentQt {

namespace {

QColor scrollIconColor()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(255, 255, 255) : QColor(94, 94, 94);
}

} // namespace

ScrollButton::ScrollButton(FluentIcon icon, QWidget *parent) : QToolButton(parent), m_icon(icon)
{
    setIcon(QIcon());
    setProperty("pickerOperation", true);
    FluentStyleSheet::setRole(this, QStringLiteral("ScrollButton"));
    installEventFilter(this);
}

FluentIcon ScrollButton::fluentIcon() const
{
    return m_icon;
}

bool ScrollButton::isPressed() const
{
    return m_isPressed;
}

void ScrollButton::setFluentIcon(FluentIcon icon)
{
    if (m_icon == icon) {
        return;
    }

    m_icon = icon;
    update();
}

bool ScrollButton::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this) {
        if (event->type() == QEvent::MouseButtonPress) {
            m_isPressed = true;
            update();
        } else if (event->type() == QEvent::MouseButtonRelease) {
            m_isPressed = false;
            update();
        }
    }

    return QToolButton::eventFilter(watched, event);
}

void ScrollButton::paintEvent(QPaintEvent *event)
{
    QToolButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const qreal extent = m_isPressed ? 8.0 : 10.0;
    const QRectF iconRect((width() - extent) / 2.0, (height() - extent) / 2.0, extent, extent);
    FluentQt::icon(m_icon, scrollIconColor()).paint(&painter, iconRect.toRect());
}

CycleListWidget::CycleListWidget(const QStringList &items, const QSize &itemSize, Qt::Alignment alignment,
                                 QWidget *parent)
    : QListWidget(parent), m_itemSize(itemSize), m_alignment(alignment)
{
    m_upButton = new ScrollButton(FluentIcon::Up, this);
    m_downButton = new ScrollButton(FluentIcon::ArrowDown, this);
    m_verticalSmoothScrollBar = new SmoothScrollBar(Qt::Vertical, this);
    m_lastScrollTime.start();

    setProperty("cycleColumn", true);
    FluentStyleSheet::setRole(this, QStringLiteral("CycleListWidget"));
    FluentStyleSheet::apply(this, FluentStyleSheetSource::TimePicker);

    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setViewportMargins(0, 0, 0, 0);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    if (horizontalScrollBar()) {
        horizontalScrollBar()->setFixedHeight(0);
    }

    m_verticalSmoothScrollBar->attachToScrollArea(this, Qt::Vertical);
    m_verticalSmoothScrollBar->setScrollAnimation(m_scrollDuration);
    m_verticalSmoothScrollBar->setForceHidden(true);

    connect(this, &QListWidget::itemClicked, this, [this](QListWidgetItem *item) {
        if (!item || !(item->flags() & Qt::ItemIsEnabled)) {
            return;
        }
        setCurrentIndex(row(item));
        scrollToItem(this->currentItem());
    });
    connect(m_upButton, &QToolButton::clicked, this, &CycleListWidget::scrollUp);
    connect(m_downButton, &QToolButton::clicked, this, &CycleListWidget::scrollDown);
    m_upButton->setAutoRepeatDelay(500);
    m_upButton->setAutoRepeatInterval(50);
    m_downButton->setAutoRepeatDelay(500);
    m_downButton->setAutoRepeatInterval(50);

    installEventFilter(this);
    setItems(items);
    setScrollButtonRepeatEnabled(true);
    setButtonsVisible(false);
}

QSize CycleListWidget::itemSize() const
{
    return m_itemSize;
}

Qt::Alignment CycleListWidget::alignment() const
{
    return m_alignment;
}

int CycleListWidget::visibleNumber() const
{
    return m_visibleNumber;
}

int CycleListWidget::scrollDuration() const
{
    return m_scrollDuration;
}

bool CycleListWidget::isCycle() const
{
    return m_isCycle;
}

bool CycleListWidget::isScrollButtonRepeatEnabled() const
{
    return m_scrollButtonRepeatEnabled;
}

int CycleListWidget::currentCycleIndex() const
{
    return m_currentIndex;
}

QStringList CycleListWidget::originItems() const
{
    return m_originItems;
}

ScrollButton *CycleListWidget::upButton() const
{
    return m_upButton;
}

ScrollButton *CycleListWidget::downButton() const
{
    return m_downButton;
}

SmoothScrollBar *CycleListWidget::verticalSmoothScrollBar() const
{
    return m_verticalSmoothScrollBar;
}

void CycleListWidget::setItems(const QStringList &items)
{
    m_originItems = items;
    clear();
    createItems(m_originItems);
    updateGeometryForVisibleItems();
}

void CycleListWidget::setItemSize(const QSize &size)
{
    if (m_itemSize == size || !size.isValid()) {
        return;
    }

    m_itemSize = size;
    setItems(m_originItems);
}

void CycleListWidget::setAlignment(Qt::Alignment alignment)
{
    if (m_alignment == alignment) {
        return;
    }

    m_alignment = alignment;
    for (int i = 0; i < count(); ++i) {
        item(i)->setTextAlignment(m_alignment | Qt::AlignVCenter);
    }
}

void CycleListWidget::setVisibleNumber(int visibleNumber)
{
    const int bounded = qMax(1, visibleNumber);
    if (m_visibleNumber == bounded) {
        return;
    }

    m_visibleNumber = bounded;
    setItems(m_originItems);
}

void CycleListWidget::setScrollDuration(int durationMs)
{
    m_scrollDuration = qMax(0, durationMs);
    if (m_verticalSmoothScrollBar) {
        m_verticalSmoothScrollBar->setScrollAnimation(m_scrollDuration);
    }
}

void CycleListWidget::setSelectedItem(const QString &text)
{
    if (text.isNull()) {
        return;
    }

    const QList<QListWidgetItem *> matches = findItems(text, Qt::MatchExactly);
    if (matches.isEmpty()) {
        return;
    }

    QListWidgetItem *target = matches.size() >= 2 ? matches.at(1) : matches.first();
    setCurrentIndex(row(target));
    QListWidget::scrollToItem(currentItem(), QAbstractItemView::PositionAtCenter);
}

void CycleListWidget::setScrollButtonRepeatEnabled(bool enabled)
{
    if (m_scrollButtonRepeatEnabled == enabled) {
        return;
    }

    m_scrollButtonRepeatEnabled = enabled;
    m_upButton->setAutoRepeat(enabled);
    m_downButton->setAutoRepeat(enabled);
}

void CycleListWidget::setCurrentIndex(int index)
{
    if (count() <= 0) {
        m_currentIndex = 0;
        return;
    }

    if (!m_isCycle) {
        const int padding = m_visibleNumber / 2;
        const int maxIndex = padding + m_originItems.size() - 1;
        m_currentIndex = qBound(padding, index, maxIndex);
        setCurrentRow(m_currentIndex, QItemSelectionModel::NoUpdate);
        return;
    }

    const int itemCount = count() / 2;
    const int margin = (m_visibleNumber + 1) / 2;
    m_currentIndex = index;

    if (index >= count() - margin) {
        m_currentIndex = itemCount + index - count();
        QListWidget::scrollToItem(item(qMax(0, m_currentIndex - 1)), QAbstractItemView::PositionAtCenter);
    } else if (index <= margin - 1) {
        m_currentIndex = itemCount + index;
        QListWidget::scrollToItem(item(qMin(count() - 1, itemCount + index + 1)), QAbstractItemView::PositionAtCenter);
    }

    setCurrentRow(m_currentIndex, QItemSelectionModel::NoUpdate);
}

void CycleListWidget::scrollDown()
{
    scrollWithAnimation(m_currentIndex + 1);
}

void CycleListWidget::scrollUp()
{
    scrollWithAnimation(m_currentIndex - 1);
}

void CycleListWidget::scrollToItem(QListWidgetItem *target, QAbstractItemView::ScrollHint /*hint*/)
{
    if (!target) {
        return;
    }

    const int index = row(target);
    const int y = target->sizeHint().height() * (index - m_visibleNumber / 2);
    m_verticalSmoothScrollBar->scrollTo(y);

    clearSelection();
    target->setSelected(false);
    emit cycleCurrentItemChanged(target);
    emit currentItemChanged(target);
}

bool CycleListWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Down) {
            scrollDown();
            return true;
        }
        if (keyEvent->key() == Qt::Key_Up) {
            scrollUp();
            return true;
        }
    }

    return QListWidget::eventFilter(watched, event);
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void CycleListWidget::enterEvent(QEnterEvent *event)
#else
void CycleListWidget::enterEvent(QEvent *event)
#endif
{
    setButtonsVisible(true);
    QListWidget::enterEvent(event);
}

void CycleListWidget::leaveEvent(QEvent *event)
{
    setButtonsVisible(false);
    QListWidget::leaveEvent(event);
}

void CycleListWidget::resizeEvent(QResizeEvent *event)
{
    QListWidget::resizeEvent(event);
    const int buttonHeight = 34;
    m_upButton->resize(width(), buttonHeight);
    m_downButton->resize(width(), buttonHeight);
    m_downButton->move(0, height() - buttonHeight);
}

void CycleListWidget::wheelEvent(QWheelEvent *event)
{
    if (event->angleDelta().y() < 0) {
        scrollDown();
    } else {
        scrollUp();
    }
    event->accept();
}

void CycleListWidget::createItems(const QStringList &items)
{
    const int itemCount = items.size();
    m_isCycle = itemCount > m_visibleNumber;

    if (m_isCycle) {
        addColumnItems(items);
        addColumnItems(items);
        m_currentIndex = itemCount;
        QListWidget::scrollToItem(item(qMax(0, m_currentIndex - m_visibleNumber / 2)), QAbstractItemView::PositionAtTop);
    } else {
        const int padding = m_visibleNumber / 2;
        addColumnItems(QStringList(padding, QString()), true);
        addColumnItems(items);
        addColumnItems(QStringList(padding, QString()), true);
        m_currentIndex = padding;
        setCurrentRow(m_currentIndex, QItemSelectionModel::NoUpdate);
    }
}

void CycleListWidget::addColumnItems(const QStringList &items, bool disabled)
{
    for (const QString &text : items) {
        auto *listItem = new QListWidgetItem(text, this);
        listItem->setSizeHint(m_itemSize);
        listItem->setTextAlignment(m_alignment | Qt::AlignVCenter);
        if (disabled) {
            listItem->setFlags(Qt::NoItemFlags);
        }
        addItem(listItem);
    }
}

void CycleListWidget::setButtonsVisible(bool visible)
{
    m_upButton->setVisible(visible);
    m_downButton->setVisible(visible);
}

void CycleListWidget::scrollWithAnimation(int index)
{
    const qint64 elapsed = m_lastScrollTime.restart();
    const bool rapidButtonRepeat = (m_upButton->isDown() || m_downButton->isDown()) && elapsed < 200;
    const int duration = rapidButtonRepeat ? 100 : m_scrollDuration;
    const QEasingCurve::Type easing = rapidButtonRepeat ? QEasingCurve::Linear : QEasingCurve::OutQuad;

    m_verticalSmoothScrollBar->setScrollAnimation(duration, easing);
    setCurrentIndex(index);
    scrollToItem(currentItem());
}

void CycleListWidget::updateGeometryForVisibleItems()
{
    setFixedSize(m_itemSize.width() + 8, m_itemSize.height() * m_visibleNumber);
}

} // namespace FluentQt
