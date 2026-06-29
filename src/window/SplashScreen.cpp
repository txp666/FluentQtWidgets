#include <FluentQtWidgets/Window/SplashScreen.h>

#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Window/FluentTitleBar.h>
#include <FluentQtWidgets/Widgets/IconWidget.h>

#include <QtGui/QPainter>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsDropShadowEffect>

namespace FluentQt {

SplashScreen::SplashScreen(const QIcon &icon, QWidget *parent, bool enableShadow)
    : QWidget(parent), m_icon(icon), m_enableShadow(enableShadow)
{
    setAttribute(Qt::WA_StyledBackground, true);
    if (parent) {
        parent->installEventFilter(this);
        resize(parent->size());
    }

    m_titleBar = new FluentTitleBar(this);
    m_titleBar->hide();
    m_iconWidget = new IconWidget(icon, this);
    m_iconWidget->setFixedSize(m_iconSize);
    m_iconWidget->setIconSize(m_iconSize);

    if (m_enableShadow) {
        auto *shadow = new QGraphicsDropShadowEffect(m_iconWidget);
        shadow->setColor(QColor(0, 0, 0, 50));
        shadow->setBlurRadius(15);
        shadow->setOffset(0, 4);
        m_iconWidget->setGraphicsEffect(shadow);
    }

    raise();
    repositionIcon();
}

QIcon SplashScreen::icon() const { return m_icon; }

QSize SplashScreen::iconSize() const { return m_iconSize; }

FluentTitleBar *SplashScreen::titleBar() const { return m_titleBar; }

IconWidget *SplashScreen::iconWidget() const { return m_iconWidget; }

void SplashScreen::setIcon(const QIcon &icon)
{
    m_icon = icon;
    m_iconWidget->setIcon(icon);
    update();
}

void SplashScreen::setIconSize(const QSize &size)
{
    m_iconSize = size;
    m_iconWidget->setFixedSize(size);
    m_iconWidget->setIconSize(size);
    repositionIcon();
}

void SplashScreen::setTitleBar(FluentTitleBar *titleBar)
{
    if (m_titleBar && m_titleBar != titleBar) {
        m_titleBar->deleteLater();
    }

    m_titleBar = titleBar;
    if (titleBar) {
        titleBar->setParent(this);
        titleBar->raise();
        titleBar->resize(width(), titleBar->height());
        titleBar->hide();
    }
}

void SplashScreen::finish() { close(); }

bool SplashScreen::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == parent()) {
        switch (event->type()) {
        case QEvent::Resize:
            resize(static_cast<QResizeEvent *>(event)->size());
            break;
        case QEvent::ChildAdded:
            raise();
            break;
        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void SplashScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    repositionIcon();
    if (m_titleBar) {
        m_titleBar->resize(width(), m_titleBar->height());
    }
}

void SplashScreen::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setPen(Qt::NoPen);

    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const int channel = dark ? 32 : 255;
    painter.setBrush(QColor(channel, channel, channel));
    painter.drawRect(rect());
}

void SplashScreen::repositionIcon()
{
    const int iw = m_iconSize.width();
    const int ih = m_iconSize.height();
    m_iconWidget->move(width() / 2 - iw / 2, height() / 2 - ih / 2);
}

} // namespace FluentQt
