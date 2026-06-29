#include <FluentQtWidgets/Widgets/IconWidget.h>

#include <FluentQtWidgets/StyleSheet.h>

#include <QtGui/QPaintEvent>
#include <QtGui/QPainter>

namespace FluentQt {

IconWidget::IconWidget(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    FluentStyleSheet::setRole(this, QStringLiteral("IconWidget"));
}

IconWidget::IconWidget(const QIcon &icon, QWidget *parent) : IconWidget(parent) { setIcon(icon); }

IconWidget::IconWidget(FluentIcon icon, QWidget *parent) : IconWidget(FluentQt::icon(icon), parent) {}

QIcon IconWidget::icon() const { return m_icon; }

QSize IconWidget::iconSize() const { return m_iconSize; }

QSize IconWidget::sizeHint() const { return m_iconSize.expandedTo(QSize(20, 20)); }

void IconWidget::setIcon(const QIcon &icon)
{
    if (m_icon.cacheKey() == icon.cacheKey()) {
        return;
    }

    m_icon = icon;
    update();
    emit iconChanged(m_icon);
}

void IconWidget::setIcon(FluentIcon icon) { setIcon(FluentQt::icon(icon)); }

void IconWidget::setIconSize(const QSize &size)
{
    const QSize boundedSize(qMax(1, size.width()), qMax(1, size.height()));
    if (m_iconSize == boundedSize) {
        return;
    }

    m_iconSize = boundedSize;
    updateGeometry();
    update();
}

void IconWidget::paintEvent(QPaintEvent *)
{
    if (m_icon.isNull()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    const QSize boundedSize(m_iconSize.width() > 0 ? qMin(width(), m_iconSize.width()) : width(),
                            m_iconSize.height() > 0 ? qMin(height(), m_iconSize.height()) : height());
    const QRect target((width() - boundedSize.width()) / 2, (height() - boundedSize.height()) / 2, boundedSize.width(),
                       boundedSize.height());
    m_icon.paint(&painter, target, Qt::AlignCenter);
}

} // namespace FluentQt
