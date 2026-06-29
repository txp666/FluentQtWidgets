#include <FluentQtWidgets/Layout/VBoxLayout.h>

#include <QtWidgets/QWidget>

#include <utility>

namespace FluentQt {

VBoxLayout::VBoxLayout(QWidget *parent) : QVBoxLayout(parent) {}

const QList<QWidget *> &VBoxLayout::widgets() const { return m_widgets; }

void VBoxLayout::addWidgets(const QList<QWidget *> &widgets, int stretch, Qt::Alignment alignment)
{
    for (QWidget *widget : widgets) {
        addWidget(widget, stretch, alignment);
    }
}

void VBoxLayout::addWidget(QWidget *widget, int stretch, Qt::Alignment alignment)
{
    if (!widget) {
        return;
    }

    QVBoxLayout::addWidget(widget, stretch, alignment);
    m_widgets.append(widget);
    widget->show();
}

void VBoxLayout::removeWidget(QWidget *widget)
{
    if (!widget) {
        return;
    }

    QVBoxLayout::removeWidget(widget);
    m_widgets.removeAll(widget);
}

void VBoxLayout::deleteWidget(QWidget *widget)
{
    if (!widget) {
        return;
    }

    removeWidget(widget);
    widget->hide();
    widget->deleteLater();
}

void VBoxLayout::removeAllWidget()
{
    for (QWidget *widget : std::as_const(m_widgets)) {
        QVBoxLayout::removeWidget(widget);
    }
    m_widgets.clear();
}

} // namespace FluentQt
