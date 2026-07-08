#include <FluentQtWidgets/Navigation/Pivot.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QPropertyAnimation>
#include <QtGui/QFont>
#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>
#include <QtGui/QShowEvent>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLayout>
#include <QtWidgets/QSizePolicy>

namespace FluentQt {

PivotItem::PivotItem(QWidget *parent) : PushButton(parent)
{
    setCheckable(true);
    setFlat(true);
    setProperty("isSelected", false);
    QFont itemFont = font();
    itemFont.setPixelSize(18);
    setFont(itemFont);
    FluentStyleSheet::setRole(this, QStringLiteral("PivotItem"));
}

PivotItem::PivotItem(const QString &routeKey, const QString &text, QWidget *parent) : PivotItem(parent)
{
    setRouteKey(routeKey);
    setText(text);
}

QString PivotItem::routeKey() const { return m_routeKey; }

void PivotItem::setRouteKey(const QString &routeKey) { m_routeKey = routeKey; }

Pivot::Pivot(QWidget *parent) : QWidget(parent), m_indicatorGeometry(0, 0, 0, 0)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(0);
    m_layout->setAlignment(Qt::AlignLeft);
    m_layout->setSizeConstraint(QLayout::SetMinimumSize);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    m_slideAnimation = new QPropertyAnimation(this, "indicatorGeometry", this);
    m_slideAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_slideAnimation->setDuration(100);
    connect(m_slideAnimation, &QPropertyAnimation::valueChanged, this, qOverload<>(&QWidget::update));

    FluentStyleSheet::setRole(this, QStringLiteral("Pivot"));
}

PivotItem *Pivot::addItem(const QString &routeKey, const QString &text)
{
    return insertItem(m_layout->count(), routeKey, text);
}

PivotItem *Pivot::insertItem(int index, const QString &routeKey, const QString &text)
{
    if (routeKey.isEmpty() || m_items.contains(routeKey)) {
        return nullptr;
    }

    PivotItem *newItem = createItem(routeKey, text);
    m_items.insert(routeKey, newItem);
    m_buttonGroup->addButton(newItem);
    m_layout->insertWidget(qBound(0, index, m_layout->count()), newItem, 1);

    connect(newItem, &QPushButton::clicked, this, [this, routeKey]() {
        setCurrentItem(routeKey);
        emit itemClicked(routeKey);
    });

    if (m_currentItem.isEmpty()) {
        setCurrentItem(routeKey);
    }

    return newItem;
}

void Pivot::removeItem(const QString &routeKey)
{
    PivotItem *removedItem = m_items.take(routeKey);
    if (!removedItem) {
        return;
    }

    m_buttonGroup->removeButton(removedItem);
    m_layout->removeWidget(removedItem);
    removedItem->deleteLater();

    if (m_currentItem == routeKey) {
        m_currentItem.clear();
        if (!m_items.isEmpty()) {
            setCurrentItem(m_items.constBegin().key());
        } else {
            emit currentItemChanged(QString());
        }
    }
}

bool Pivot::contains(const QString &routeKey) const { return m_items.contains(routeKey); }

QString Pivot::currentItem() const { return m_currentItem; }

PivotItem *Pivot::item(const QString &routeKey) const { return m_items.value(routeKey, nullptr); }

QList<QString> Pivot::routeKeys() const { return m_items.keys(); }

void Pivot::setCurrentItem(const QString &routeKey)
{
    PivotItem *targetItem = item(routeKey);
    if (!targetItem || m_currentItem == routeKey) {
        return;
    }

    m_currentItem = routeKey;
    for (PivotItem *pivotItem : std::as_const(m_items)) {
        const bool selected = pivotItem == targetItem;
        pivotItem->setChecked(selected);
        pivotItem->setProperty("isSelected", selected);
        FluentStyleSheet::polish(pivotItem);
    }

    if (m_slideAnimation) {
        const QRect targetGeometry = computeIndicatorGeometry();
        m_slideAnimation->stop();
        m_slideAnimation->setStartValue(m_indicatorGeometry);
        m_slideAnimation->setEndValue(targetGeometry);
        m_slideAnimation->start();
    }

    emit currentItemChanged(m_currentItem);
}

QRect Pivot::indicatorGeometry() const { return m_indicatorGeometry; }

void Pivot::setIndicatorGeometry(const QRect &geometry)
{
    if (m_indicatorGeometry != geometry) {
        m_indicatorGeometry = geometry;
    }
}

PivotItem *Pivot::createItem(const QString &routeKey, const QString &text)
{
    return new PivotItem(routeKey, text, this);
}

QHBoxLayout *Pivot::layoutHandle() const { return m_layout; }

QRect Pivot::computeIndicatorGeometry() const
{
    PivotItem *cur = item(m_currentItem);
    if (!cur) {
        return QRect();
    }

    const int indicatorWidth = 16;
    const int indicatorHeight = 3;
    const int x = cur->x() + (cur->width() - indicatorWidth) / 2;
    const int y = height() - indicatorHeight;
    return QRect(x, y, indicatorWidth, indicatorHeight);
}

void Pivot::adjustIndicatorPosition()
{
    m_indicatorGeometry = computeIndicatorGeometry();
    if (m_slideAnimation && m_slideAnimation->state() != QAbstractAnimation::Running) {
        update();
    }
}

void Pivot::paintEvent(QPaintEvent *event)
{
    QWidget::paintEvent(event);

    if (!item(m_currentItem) || m_indicatorGeometry.isNull()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    painter.setBrush(ThemeManager::instance()->accentColor());
    painter.drawRoundedRect(m_indicatorGeometry, 1.5, 1.5);
}

void Pivot::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
    adjustIndicatorPosition();
}

void Pivot::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    adjustIndicatorPosition();
}

} // namespace FluentQt
