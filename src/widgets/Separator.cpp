#include <FluentQtWidgets/Widgets/Separator.h>

#include <FluentQtWidgets/Theme.h>

#include <QtGui/QPainter>

namespace FluentQt {

namespace {
QColor separatorColor()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(255, 255, 255, 51)
                                                                     : QColor(0, 0, 0, 22);
}
} // namespace

HorizontalSeparator::HorizontalSeparator(QWidget *parent) : QWidget(parent)
{
    setFixedHeight(3);
    setProperty("fqw", QStringLiteral("HorizontalSeparator"));
}

void HorizontalSeparator::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(separatorColor());
    painter.drawLine(0, 1, width(), 1);
}

VerticalSeparator::VerticalSeparator(QWidget *parent) : QWidget(parent)
{
    setFixedWidth(3);
    setProperty("fqw", QStringLiteral("VerticalSeparator"));
}

void VerticalSeparator::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);
    painter.setPen(separatorColor());
    painter.drawLine(1, 0, 1, height());
}

} // namespace FluentQt
