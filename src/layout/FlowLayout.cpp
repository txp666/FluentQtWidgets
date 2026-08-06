#include <FluentQtWidgets/Layout/FlowLayout.h>

#include <QtCore/QEvent>
#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QTimer>
#include <QtCore/QVariant>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QWidget>
#include <QtWidgets/QWidgetItem>

#include <utility>

namespace FluentQt {

FlowLayout::FlowLayout(QWidget *parent, bool animation, bool tight) : QLayout(parent)
{
    m_animationEnabled = animation;
    m_tight = tight;
    m_parentWidget = parent;
    m_animationGroup = new QParallelAnimationGroup(this);
    m_debounceTimer = new QTimer(this);
    m_debounceTimer->setSingleShot(true);
    connect(m_debounceTimer, &QTimer::timeout, this, [this]() { doLayout(geometry(), true); });
    if (m_parentWidget) {
        m_parentWidget->installEventFilter(this);
    }
}

FlowLayout::FlowLayout(bool animation, QWidget *parent) : FlowLayout(parent, animation, false) {}

FlowLayout::FlowLayout(int margin, int horizontalSpacing, int verticalSpacing, QWidget *parent, bool animation,
                       bool tight)
    : FlowLayout(parent, animation, tight)
{
    setContentsMargins(margin, margin, margin, margin);
    m_horizontalSpacing = horizontalSpacing;
    m_verticalSpacing = verticalSpacing;
}

FlowLayout::~FlowLayout()
{
    QLayoutItem *item = nullptr;
    while ((item = takeAt(0)) != nullptr) {
        delete item;
    }
}

void FlowLayout::addItem(QLayoutItem *item)
{
    if (!item) {
        return;
    }
    m_items.append(item);
    setupAnimationForItem(item);
}

void FlowLayout::insertItem(int index, QLayoutItem *item)
{
    if (!item) {
        return;
    }

    const int boundedIndex = qBound(0, index, m_items.size());
    m_items.insert(boundedIndex, item);
    setupAnimationForItem(item, boundedIndex);
    invalidate();
}

void FlowLayout::insertWidget(int index, QWidget *widget)
{
    if (!widget) {
        return;
    }

    addChildWidget(widget);
    insertItem(index, new QWidgetItem(widget));
}

QWidget *FlowLayout::removeWidget(QWidget *widget)
{
    if (!widget) {
        return nullptr;
    }

    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i)->widget() == widget) {
            return takeWidgetAt(i);
        }
    }
    return nullptr;
}

QWidget *FlowLayout::takeWidgetAt(int index)
{
    QLayoutItem *item = takeAt(index);
    if (!item) {
        return nullptr;
    }

    QWidget *widget = item->widget();
    delete item;
    invalidate();
    return widget;
}

void FlowLayout::removeAllWidgets()
{
    while (!m_items.isEmpty()) {
        takeWidgetAt(0);
    }
}

void FlowLayout::takeAllWidgets()
{
    while (!m_items.isEmpty()) {
        QWidget *widget = takeWidgetAt(0);
        if (widget) {
            widget->deleteLater();
        }
    }
}

int FlowLayout::horizontalSpacing() const { return m_horizontalSpacing; }

int FlowLayout::verticalSpacing() const { return m_verticalSpacing; }

Qt::Alignment FlowLayout::verticalAlignment() const { return m_verticalAlignment; }

bool FlowLayout::isAnimationEnabled() const { return m_animationEnabled; }

bool FlowLayout::isTight() const { return m_tight; }

int FlowLayout::animationDuration() const { return m_duration; }

QEasingCurve::Type FlowLayout::animationEasing() const { return m_easing; }

Qt::Orientations FlowLayout::expandingDirections() const { return {}; }

bool FlowLayout::hasHeightForWidth() const { return true; }

int FlowLayout::heightForWidth(int width) const { return const_cast<FlowLayout *>(this)->doLayout(QRect(0, 0, width, 0), false); }

QSize FlowLayout::sizeHint() const { return minimumSize(); }

QSize FlowLayout::minimumSize() const
{
    QSize size;
    for (QLayoutItem *item : m_items) {
        if (shouldSkipItem(item)) {
            continue;
        }
        size = size.expandedTo(item->minimumSize());
    }

    const QMargins margins = contentsMargins();
    size.rwidth() += margins.left() + margins.right();
    size.rheight() += margins.top() + margins.bottom();
    return size;
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    if (m_animationEnabled) {
        m_debounceTimer->start(80);
    } else {
        doLayout(rect, true);
    }
}

int FlowLayout::count() const { return m_items.size(); }

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return index >= 0 && index < m_items.size() ? m_items.at(index) : nullptr;
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if (index < 0 || index >= m_items.size()) {
        return nullptr;
    }

    QLayoutItem *item = m_items.takeAt(index);
    removeAnimationForWidget(item ? item->widget() : nullptr);
    invalidate();
    return item;
}

void FlowLayout::setHorizontalSpacing(int spacing)
{
    if (m_horizontalSpacing == spacing) {
        return;
    }
    m_horizontalSpacing = spacing;
    invalidate();
}

void FlowLayout::setVerticalSpacing(int spacing)
{
    if (m_verticalSpacing == spacing) {
        return;
    }
    m_verticalSpacing = spacing;
    invalidate();
}

void FlowLayout::setVerticalAlignment(Qt::Alignment alignment)
{
    const Qt::Alignment vertical = alignment & Qt::AlignVertical_Mask;
    Qt::Alignment normalized = Qt::AlignTop;
    if (vertical.testFlag(Qt::AlignBottom)) {
        normalized = Qt::AlignBottom;
    } else if (vertical.testFlag(Qt::AlignVCenter)) {
        normalized = Qt::AlignVCenter;
    }

    if (m_verticalAlignment == normalized) {
        return;
    }
    m_verticalAlignment = normalized;
    invalidate();
}

void FlowLayout::setAnimationEnabled(bool enabled)
{
    if (m_animationEnabled == enabled) {
        return;
    }

    m_animationEnabled = enabled;
    if (m_animationEnabled) {
        for (int i = 0; i < m_items.size(); ++i) {
            setupAnimationForItem(m_items.at(i), i);
        }
    } else {
        m_animationGroup->stop();
        for (QPropertyAnimation *animation : std::as_const(m_animations)) {
            if (animation) {
                animation->stop();
            }
        }
    }
    invalidate();
}

void FlowLayout::setTight(bool tight)
{
    if (m_tight == tight) {
        return;
    }
    m_tight = tight;
    invalidate();
}

void FlowLayout::setAnimation(int durationMs, QEasingCurve::Type easing)
{
    m_duration = qMax(0, durationMs);
    m_easing = easing;
    for (QPropertyAnimation *animation : std::as_const(m_animations)) {
        if (!animation) {
            continue;
        }
        animation->setDuration(m_duration);
        animation->setEasingCurve(m_easing);
    }
}

bool FlowLayout::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_parentWidget && event->type() == QEvent::Show) {
        doLayout(geometry(), true);
    }
    return QLayout::eventFilter(watched, event);
}

int FlowLayout::doLayout(const QRect &rect, bool move)
{
    struct RowItem
    {
        QLayoutItem *item = nullptr;
        int index = -1;
        QSize size;
        int x = 0;
    };

    bool animationRestart = false;
    const QMargins margins = contentsMargins();
    int x = rect.x() + margins.left();
    int y = rect.y() + margins.top();
    int rowHeight = 0;
    const int spaceX = horizontalSpacing();
    const int spaceY = verticalSpacing();
    QList<RowItem> rowItems;

    const auto placeRow = [&](int rowY, int height) {
        if (!move) {
            return;
        }
        for (const RowItem &rowItem : std::as_const(rowItems)) {
            const int itemY = alignedItemY(rowItem.item, rowY, height, rowItem.size.height());
            const QRect target(QPoint(rowItem.x, itemY), rowItem.size);
            if (!m_animationEnabled || rowItem.index >= m_animations.size() || !m_animations.at(rowItem.index)) {
                rowItem.item->setGeometry(target);
            } else {
                QPropertyAnimation *animation = m_animations.at(rowItem.index);
                if (animation->endValue().toRect() != target) {
                    animation->stop();
                    animation->setEndValue(target);
                    animationRestart = true;
                }
            }
        }
    };

    for (int i = 0; i < m_items.size(); ++i) {
        QLayoutItem *item = m_items.at(i);
        if (shouldSkipItem(item)) {
            continue;
        }

        const QSize itemSize = itemLayoutSize(item);
        int nextX = x + itemSize.width() + spaceX;
        if (nextX - spaceX > rect.right() - margins.right() && !rowItems.isEmpty()) {
            placeRow(y, rowHeight);
            x = rect.x() + margins.left();
            y += rowHeight + spaceY;
            nextX = x + itemSize.width() + spaceX;
            rowHeight = 0;
            rowItems.clear();
        }

        rowItems.append({item, i, itemSize, x});
        x = nextX;
        rowHeight = qMax(rowHeight, itemSize.height());
    }
    placeRow(y, rowHeight);

    if (m_animationEnabled && animationRestart) {
        m_animationGroup->stop();
        m_animationGroup->start();
    }

    return y + rowHeight + margins.bottom() - rect.y();
}

void FlowLayout::setupAnimationForItem(QLayoutItem *item, int index)
{
    QWidget *widget = item ? item->widget() : nullptr;
    if (!widget || !m_animationEnabled) {
        return;
    }

    if (!m_parentWidget && widget->parentWidget()) {
        m_parentWidget = widget->parentWidget();
        m_parentWidget->installEventFilter(this);
    }

    auto *animation = new QPropertyAnimation(widget, "geometry", this);
    animation->setDuration(m_duration);
    animation->setEasingCurve(m_easing);
    animation->setEndValue(QRect(QPoint(0, 0), widget->size()));
    widget->setProperty("flowAni", QVariant::fromValue<QObject *>(animation));
    m_animationGroup->addAnimation(animation);

    if (index >= 0 && index <= m_animations.size()) {
        m_animations.insert(index, animation);
    } else {
        m_animations.append(animation);
    }
}

void FlowLayout::removeAnimationForWidget(QWidget *widget)
{
    if (!widget) {
        return;
    }

    QPropertyAnimation *animation = nullptr;
    for (int i = 0; i < m_animations.size(); ++i) {
        if (m_animations.at(i) && m_animations.at(i)->targetObject() == widget) {
            animation = m_animations.takeAt(i);
            break;
        }
    }

    if (!animation) {
        return;
    }

    m_animationGroup->removeAnimation(animation);
    widget->setProperty("flowAni", QVariant());
    animation->deleteLater();
}

bool FlowLayout::shouldSkipItem(QLayoutItem *item) const
{
    const QWidget *widget = item ? item->widget() : nullptr;
    return m_tight && widget && !widget->isVisible();
}

QSize FlowLayout::itemLayoutSize(QLayoutItem *item) const { return item->sizeHint().expandedTo(item->minimumSize()); }

int FlowLayout::alignedItemY(QLayoutItem *item, int rowY, int rowHeight, int itemHeight) const
{
    Qt::Alignment alignment = item ? item->alignment() & Qt::AlignVertical_Mask : Qt::Alignment{};
    if (!alignment) {
        alignment = m_verticalAlignment;
    }

    if (alignment.testFlag(Qt::AlignBottom)) {
        return rowY + rowHeight - itemHeight;
    }
    if (alignment.testFlag(Qt::AlignVCenter)) {
        return rowY + (rowHeight - itemHeight) / 2;
    }
    return rowY;
}

// ==========================================
// AdaptiveFlowLayout
// ==========================================

AdaptiveFlowLayout::AdaptiveFlowLayout(QWidget *parent, bool animation, bool tight)
    : FlowLayout(parent, animation, tight)
{
}

AdaptiveFlowLayout::AdaptiveFlowLayout(bool animation, QWidget *parent) : AdaptiveFlowLayout(parent, animation, false) {}

AdaptiveFlowLayout::AdaptiveFlowLayout(int margin, int horizontalSpacing, int verticalSpacing, QWidget *parent,
                                       bool animation, bool tight)
    : AdaptiveFlowLayout(parent, animation, tight)
{
    setContentsMargins(margin, margin, margin, margin);
    setHorizontalSpacing(horizontalSpacing);
    setVerticalSpacing(verticalSpacing);
}

int AdaptiveFlowLayout::widgetMinimumWidth() const { return m_widgetMinimumWidth; }

int AdaptiveFlowLayout::widgetMaximumWidth() const { return m_widgetMaximumWidth; }

void AdaptiveFlowLayout::setWidgetMinimumWidth(int width)
{
    const int boundedWidth = qMax(1, width);
    if (m_widgetMinimumWidth == boundedWidth) {
        return;
    }
    m_widgetMinimumWidth = boundedWidth;
    invalidate();
}

void AdaptiveFlowLayout::setWidgetMaximumWidth(int width)
{
    const int boundedWidth = width > 0 ? width : -1;
    if (m_widgetMaximumWidth == boundedWidth) {
        return;
    }
    m_widgetMaximumWidth = boundedWidth;
    invalidate();
}

void AdaptiveFlowLayout::clearWidgetMaximumWidth()
{
    setWidgetMaximumWidth(-1);
}

int AdaptiveFlowLayout::doLayout(const QRect &rect, bool move)
{
    struct RowItem
    {
        QLayoutItem *item = nullptr;
        int index = -1;
        QSize size;
        int x = 0;
    };

    bool animationRestart = false;
    const QMargins margins = contentsMargins();
    const int spaceX = horizontalSpacing();
    const int spaceY = verticalSpacing();
    const int availableWidth = qMax(0, rect.width() - margins.left() - margins.right());

    int cardsPerRow = 1;
    if (m_widgetMinimumWidth + spaceX > 0) {
        cardsPerRow = qMax(1, (availableWidth + spaceX) / (m_widgetMinimumWidth + spaceX));
    }

    int cardWidth = availableWidth;
    if (cardsPerRow > 1) {
        cardWidth = (availableWidth - (cardsPerRow - 1) * spaceX) / cardsPerRow;
    }

    if (m_widgetMaximumWidth > 0 && cardWidth > m_widgetMaximumWidth) {
        cardWidth = m_widgetMaximumWidth;
    }

    int x = rect.x() + margins.left();
    int y = rect.y() + margins.top();
    int rowHeight = 0;
    int columnIndex = 0;
    QList<RowItem> rowItems;

    const auto placeRow = [&](int rowY, int height) {
        if (!move) {
            return;
        }
        for (const RowItem &rowItem : std::as_const(rowItems)) {
            const int itemY = alignedItemY(rowItem.item, rowY, height, rowItem.size.height());
            const QRect target(QPoint(rowItem.x, itemY), rowItem.size);
            if (!m_animationEnabled || rowItem.index >= m_animations.size() || !m_animations.at(rowItem.index)) {
                rowItem.item->setGeometry(target);
            } else {
                QPropertyAnimation *animation = m_animations.at(rowItem.index);
                if (animation->endValue().toRect() != target) {
                    animation->stop();
                    animation->setEndValue(target);
                    animationRestart = true;
                }
            }
        }
    };

    for (int i = 0; i < m_items.size(); ++i) {
        QLayoutItem *item = m_items.at(i);
        if (shouldSkipItem(item)) {
            continue;
        }

        if (columnIndex >= cardsPerRow && cardsPerRow > 0) {
            placeRow(y, rowHeight);
            x = rect.x() + margins.left();
            y += rowHeight + spaceY;
            rowHeight = 0;
            columnIndex = 0;
            rowItems.clear();
        }

        const QSize itemSize(cardWidth, itemLayoutSize(item).height());
        rowItems.append({item, i, itemSize, x});
        x += cardWidth + spaceX;
        rowHeight = qMax(rowHeight, itemSize.height());
        ++columnIndex;
    }
    placeRow(y, rowHeight);

    if (m_animationEnabled && animationRestart) {
        m_animationGroup->stop();
        m_animationGroup->start();
    }

    return y + rowHeight + margins.bottom() - rect.y();
}

// ==========================================
// ExpandLayout
// ==========================================

ExpandLayout::ExpandLayout(QWidget *parent) : QLayout(parent) {}

ExpandLayout::~ExpandLayout()
{
    QLayoutItem *item = nullptr;
    while ((item = takeAt(0)) != nullptr) {
        delete item;
    }
}

void ExpandLayout::addWidget(QWidget *widget)
{
    if (!widget) {
        return;
    }

    for (QLayoutItem *item : std::as_const(m_items)) {
        if (item && item->widget() == widget) {
            return;
        }
    }

    addChildWidget(widget);
    addItem(new QWidgetItem(widget));
}

void ExpandLayout::addItem(QLayoutItem *item)
{
    if (!item) {
        return;
    }
    m_items.append(item);
    if (QWidget *widget = item->widget()) {
        widget->installEventFilter(this);
    }
    invalidate();
}

Qt::Orientations ExpandLayout::expandingDirections() const { return Qt::Vertical; }

bool ExpandLayout::hasHeightForWidth() const { return true; }

int ExpandLayout::heightForWidth(int width) const { return doLayout(QRect(0, 0, width, 0), false); }

void ExpandLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, true);
}

QSize ExpandLayout::sizeHint() const { return minimumSize(); }

QSize ExpandLayout::minimumSize() const
{
    QSize size;
    for (QLayoutItem *item : m_items) {
        if (!item) {
            continue;
        }
        size = size.expandedTo(item->minimumSize());
    }

    const QMargins margins = contentsMargins();
    size.rwidth() += margins.left() + margins.right();
    size.rheight() += margins.top() + margins.bottom();
    return size;
}

int ExpandLayout::count() const { return m_items.size(); }

QLayoutItem *ExpandLayout::itemAt(int index) const
{
    return index >= 0 && index < m_items.size() ? m_items.at(index) : nullptr;
}

QLayoutItem *ExpandLayout::takeAt(int index)
{
    if (index < 0 || index >= m_items.size()) {
        return nullptr;
    }

    QLayoutItem *item = m_items.takeAt(index);
    if (QWidget *widget = item ? item->widget() : nullptr) {
        widget->removeEventFilter(this);
    }
    invalidate();
    return item;
}

bool ExpandLayout::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        for (QLayoutItem *item : std::as_const(m_items)) {
            QWidget *widget = item ? item->widget() : nullptr;
            if (widget != watched) {
                continue;
            }

            auto *resizeEvent = static_cast<QResizeEvent *>(event);
            const QSize delta = resizeEvent->size() - resizeEvent->oldSize();
            QWidget *parent = parentWidget();
            if (parent && delta.height() != 0 && delta.width() == 0) {
                parent->resize(parent->width(), parent->height() + delta.height());
            }
            break;
        }
    }

    return QLayout::eventFilter(watched, event);
}

int ExpandLayout::doLayout(const QRect &rect, bool move) const
{
    const QMargins margins = contentsMargins();
    const int x = rect.x() + margins.left();
    int y = rect.y() + margins.top();
    const int width = qMax(0, rect.width() - margins.left() - margins.right());
    bool hasVisibleItem = false;

    for (QLayoutItem *item : m_items) {
        QWidget *widget = item ? item->widget() : nullptr;
        if (!widget || widget->isHidden()) {
            continue;
        }

        if (hasVisibleItem) {
            y += spacing();
        }

        if (move) {
            widget->setGeometry(QRect(QPoint(x, y), QSize(width, widget->height())));
        }

        y += widget->height();
        hasVisibleItem = true;
    }

    return y + margins.bottom() - rect.y();
}

} // namespace FluentQt
