#include <FluentQtWidgets/Navigation/BreadcrumbBar.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Widgets/Button.h>

#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>

#include <utility>

namespace FluentQt {

BreadcrumbItem::BreadcrumbItem(QString key, QString text) : key(std::move(key)), text(std::move(text)) {}

BreadcrumbBar::BreadcrumbBar(QWidget *parent) : QWidget(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(m_spacing);
    FluentStyleSheet::setRole(this, QStringLiteral("BreadcrumbBar"));
}

void BreadcrumbBar::addItem(const QString &key, const QString &text)
{
    if (key.isEmpty()) {
        return;
    }

    for (const BreadcrumbItem &item : std::as_const(m_items)) {
        if (item.key == key) {
            return;
        }
    }

    m_items.append(BreadcrumbItem(key, text));
    rebuild();
    setCurrentIndex(m_items.size() - 1);
}

void BreadcrumbBar::setItems(const QList<BreadcrumbItem> &items)
{
    const bool hadCurrent = m_currentIndex != -1;
    m_items = items;
    m_currentIndex = -1;
    rebuild();
    if (m_items.isEmpty()) {
        if (hadCurrent) {
            emit currentIndexChanged(-1);
            emit currentItemChanged(QString());
        }
    } else {
        setCurrentIndex(m_items.size() - 1);
    }
}

void BreadcrumbBar::clear()
{
    const bool hadCurrent = m_currentIndex != -1;
    m_items.clear();
    m_currentIndex = -1;
    rebuild();
    if (hadCurrent) {
        emit currentIndexChanged(-1);
        emit currentItemChanged(QString());
    }
}

void BreadcrumbBar::popItem()
{
    if (m_items.isEmpty()) {
        return;
    }

    if (m_items.size() >= 2) {
        setCurrentIndex(m_currentIndex - 1);
    } else {
        clear();
    }
}

QList<BreadcrumbItem> BreadcrumbBar::items() const { return m_items; }

BreadcrumbItem BreadcrumbBar::itemAt(int index) const
{
    if (index < 0 || index >= m_items.size()) {
        return BreadcrumbItem();
    }

    return m_items.at(index);
}

BreadcrumbItem BreadcrumbBar::item(const QString &key) const
{
    for (const BreadcrumbItem &breadcrumb : m_items) {
        if (breadcrumb.key == key) {
            return breadcrumb;
        }
    }

    return BreadcrumbItem();
}

void BreadcrumbBar::setItemText(const QString &key, const QString &text)
{
    for (BreadcrumbItem &breadcrumb : m_items) {
        if (breadcrumb.key == key) {
            if (breadcrumb.text == text) {
                return;
            }
            breadcrumb.text = text;
            rebuild();
            return;
        }
    }
}

int BreadcrumbBar::count() const { return m_items.size(); }

QString BreadcrumbBar::currentItem() const
{
    if (m_currentIndex < 0 || m_currentIndex >= m_items.size()) {
        return QString();
    }

    return m_items.at(m_currentIndex).key;
}

int BreadcrumbBar::currentIndex() const { return m_currentIndex; }

int BreadcrumbBar::spacing() const { return m_spacing; }

void BreadcrumbBar::setCurrentItem(const QString &key)
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i).key == key) {
            setCurrentIndex(i);
            return;
        }
    }
}

void BreadcrumbBar::setCurrentIndex(int index)
{
    if (index < -1 || index >= m_items.size() || index == m_currentIndex) {
        return;
    }

    if (index == -1) {
        clear();
        return;
    }

    if (index < m_items.size() - 1) {
        while (m_items.size() > index + 1) {
            m_items.removeLast();
        }
    }

    m_currentIndex = index;
    rebuild();

    emit currentIndexChanged(m_currentIndex);
    emit currentItemChanged(currentItem());
}

void BreadcrumbBar::setSpacing(int spacing)
{
    if (m_spacing == spacing) {
        return;
    }

    m_spacing = spacing;
    if (m_layout) {
        m_layout->setSpacing(m_spacing);
    }
}

void BreadcrumbBar::rebuild()
{
    while (QLayoutItem *item = m_layout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }

    for (int i = 0; i < m_items.size(); ++i) {
        const BreadcrumbItem breadcrumb = m_items.at(i);
        auto *button = new TransparentPushButton(breadcrumb.text, this);
        button->setProperty("breadcrumbKey", breadcrumb.key);
        FluentStyleSheet::setRole(button, QStringLiteral("BreadcrumbItem"));
        m_layout->addWidget(button);

        connect(button, &QPushButton::clicked, this, [this, breadcrumb, i]() {
            emit itemClicked(breadcrumb.key, i);
            setCurrentItem(breadcrumb.key);
        });

        if (i != m_items.size() - 1) {
            auto *separator = new QLabel(QStringLiteral(">"), this);
            separator->setAlignment(Qt::AlignCenter);
            FluentStyleSheet::setRole(separator, QStringLiteral("BreadcrumbSeparator"));
            m_layout->addWidget(separator);
        }
    }

    m_layout->addStretch();
}

} // namespace FluentQt
