#include <FluentQtWidgets/Navigation/SegmentedWidget.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QPropertyAnimation>
#include <QtGui/QFont>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QShowEvent>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLayout>
#include <QtWidgets/QSizePolicy>

namespace FluentQt {

SegmentedItem::SegmentedItem(QWidget *parent) : PivotItem(parent)
{
    QFont itemFont = font();
    itemFont.setPixelSize(14);
    setFont(itemFont);
    FluentStyleSheet::setRole(this, QStringLiteral("SegmentedItem"));
}

SegmentedItem::SegmentedItem(const QString &routeKey, const QString &text, QWidget *parent) : SegmentedItem(parent)
{
    setRouteKey(routeKey);
    setText(text);
}

SegmentedWidget::SegmentedWidget(QWidget *parent) : Pivot(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("SegmentedWidget"));
    setAttribute(Qt::WA_StyledBackground);
}

PivotItem *SegmentedWidget::createItem(const QString &routeKey, const QString &text)
{
    return new SegmentedItem(routeKey, text, this);
}

void SegmentedWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    auto *current = item(currentItem());
    if (!current) {
        return;
    }

    const QRect indicator = indicatorGeometry();
    const int animatedX = indicator.isNull() ? current->x() : indicator.x() - (current->width() - 16) / 2;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    if (dark) {
        painter.setPen(QColor(255, 255, 255, 14));
        painter.setBrush(QColor(255, 255, 255, 15));
    } else {
        painter.setPen(QColor(0, 0, 0, 19));
        painter.setBrush(QColor(255, 255, 255, 179));
    }

    painter.drawRoundedRect(QRectF(animatedX + 1, current->y() + 1, current->width() - 2, current->height() - 2), 5,
                            5);

    painter.setPen(Qt::NoPen);
    painter.setBrush(ThemeManager::instance()->accentColor());
    painter.drawRoundedRect(QRectF(animatedX + current->width() / 2.0 - 8.0, height() - 3.5, 16, 3), 1.5,
                            1.5);
}

SegmentedToggleToolWidget::SegmentedToggleToolWidget(QWidget *parent) : QWidget(parent)
{
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_layout->setSizeConstraint(QLayout::SetMinimumSize);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    FluentStyleSheet::setRole(this, QStringLiteral("SegmentedToolWidget"));
    setAttribute(Qt::WA_StyledBackground);

    m_selectionAnimation = new QPropertyAnimation(this, "selectionGeometry", this);
    m_selectionAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_selectionAnimation->setDuration(100);
    connect(m_selectionAnimation, &QPropertyAnimation::valueChanged, this, qOverload<>(&QWidget::update));
}

TransparentToggleToolButton *SegmentedToggleToolWidget::addItem(const QString &routeKey, const QIcon &icon)
{
    return insertItem(m_items.size(), routeKey, icon);
}

TransparentToggleToolButton *SegmentedToggleToolWidget::insertItem(int index, const QString &routeKey,
                                                                   const QIcon &icon)
{
    if (routeKey.isEmpty() || m_itemMap.contains(routeKey)) {
        return nullptr;
    }

    auto *button = new TransparentToggleToolButton(icon, this);
    FluentStyleSheet::setRole(button, QStringLiteral("SegmentedToolItem"));
    button->setFixedSize(50, 32);
    button->setIconSize(QSize(16, 16));
    button->setProperty("isSelected", false);
    button->setProperty("routeKey", routeKey);
    button->setToolTip(routeKey);

    const int boundedIndex = qBound(0, index, m_items.size());
    m_items.insert(boundedIndex, button);
    m_itemMap.insert(routeKey, button);
    m_buttonGroup->addButton(button);
    m_layout->insertWidget(boundedIndex, button);

    connect(button, &QToolButton::clicked, this, [this, routeKey]() {
        setCurrentItem(routeKey);
        emit itemClicked(routeKey);
    });

    if (m_currentItem.isEmpty()) {
        setCurrentItem(routeKey);
    } else {
        updateCheckedButton();
    }

    return button;
}

void SegmentedToggleToolWidget::removeItem(const QString &routeKey)
{
    auto *button = m_itemMap.take(routeKey);
    if (!button) {
        return;
    }

    m_items.removeOne(button);
    m_buttonGroup->removeButton(button);
    m_layout->removeWidget(button);
    button->deleteLater();

    if (m_currentItem == routeKey) {
        m_currentItem.clear();
        if (!m_items.isEmpty()) {
            setCurrentItem(m_items.first()->property("routeKey").toString());
        } else {
            setSelectionGeometry(QRect());
            update();
            emit currentItemChanged(QString());
        }
    }
}

bool SegmentedToggleToolWidget::contains(const QString &routeKey) const { return m_itemMap.contains(routeKey); }

QString SegmentedToggleToolWidget::currentItem() const { return m_currentItem; }

TransparentToggleToolButton *SegmentedToggleToolWidget::item(const QString &routeKey) const
{
    return m_itemMap.value(routeKey, nullptr);
}

QList<QString> SegmentedToggleToolWidget::routeKeys() const
{
    QList<QString> keys;
    keys.reserve(m_items.size());
    for (auto *button : m_items) {
        keys.append(button->property("routeKey").toString());
    }
    return keys;
}

void SegmentedToggleToolWidget::setCurrentItem(const QString &routeKey)
{
    if (!m_itemMap.contains(routeKey) || m_currentItem == routeKey) {
        return;
    }

    m_currentItem = routeKey;
    updateCheckedButton();
    const QRect targetGeometry = computeSelectionGeometry();
    if (targetGeometry.isNull()) {
        setSelectionGeometry(targetGeometry);
        update();
    } else if (m_selectionAnimation && !m_selectionGeometry.isNull()) {
        m_selectionAnimation->stop();
        m_selectionAnimation->setStartValue(m_selectionGeometry);
        m_selectionAnimation->setEndValue(targetGeometry);
        m_selectionAnimation->start();
    } else {
        setSelectionGeometry(targetGeometry);
        update();
    }
    emit currentItemChanged(routeKey);
}

QRect SegmentedToggleToolWidget::selectionGeometry() const { return m_selectionGeometry; }

void SegmentedToggleToolWidget::setSelectionGeometry(const QRect &geometry)
{
    if (m_selectionGeometry != geometry) {
        m_selectionGeometry = geometry;
    }
}

void SegmentedToggleToolWidget::updateCheckedButton()
{
    for (auto *button : m_items) {
        const bool selected = button->property("routeKey").toString() == m_currentItem;
        button->setChecked(selected);
        button->setProperty("isSelected", selected);
        FluentStyleSheet::polish(button);
    }
}

QRect SegmentedToggleToolWidget::computeSelectionGeometry() const
{
    auto *button = m_itemMap.value(m_currentItem, nullptr);
    return button ? button->geometry() : QRect();
}

void SegmentedToggleToolWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    setSelectionGeometry(computeSelectionGeometry());
}

void SegmentedToggleToolWidget::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    setSelectionGeometry(computeSelectionGeometry());
}

void SegmentedToggleToolWidget::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    const QRect selection = m_selectionGeometry.isNull() ? computeSelectionGeometry() : m_selectionGeometry;
    if (selection.isNull()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(ThemeManager::instance()->accentColor());
    painter.drawRoundedRect(QRectF(selection), 4, 4);
}

} // namespace FluentQt
