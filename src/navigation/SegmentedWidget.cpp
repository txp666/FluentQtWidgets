#include <FluentQtWidgets/Navigation/SegmentedWidget.h>

#include <FluentQtWidgets/StyleSheet.h>

#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QHBoxLayout>

namespace FluentQt {

SegmentedItem::SegmentedItem(QWidget *parent) : PivotItem(parent)
{
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
}

PivotItem *SegmentedWidget::createItem(const QString &routeKey, const QString &text)
{
    return new SegmentedItem(routeKey, text, this);
}

SegmentedToggleToolWidget::SegmentedToggleToolWidget(QWidget *parent) : QWidget(parent)
{
    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->setExclusive(true);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(2, 2, 2, 2);
    m_layout->setSpacing(0);
    m_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    FluentStyleSheet::setRole(this, QStringLiteral("SegmentedWidget"));
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
    button->setFixedSize(36, 32);
    button->setIconSize(QSize(16, 16));
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
    emit currentItemChanged(routeKey);
}

void SegmentedToggleToolWidget::updateCheckedButton()
{
    for (auto *button : m_items) {
        button->setChecked(button->property("routeKey").toString() == m_currentItem);
    }
}

} // namespace FluentQt
