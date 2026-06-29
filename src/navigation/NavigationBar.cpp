#include <FluentQtWidgets/Navigation/NavigationBar.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>

#include <QtCore/QRectF>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>

#include <utility>

namespace FluentQt {

NavigationBarPushButton::NavigationBarPushButton(const QIcon &icon, const QString &text, bool selectable,
                                                 const QIcon &selectedIcon, QWidget *parent)
    : NavigationWidget(selectable, parent), m_icon(icon), m_selectedIcon(selectedIcon), m_text(text)
{
    FluentStyleSheet::setRole(this, QStringLiteral("NavigationBarPushButton"));
    QFont f = font();
    f.setPixelSize(11);
    setFont(f);
    setFixedSize(sizeHint());
}

QString NavigationBarPushButton::text() const { return m_text; }

QIcon NavigationBarPushButton::icon() const { return m_icon; }

QIcon NavigationBarPushButton::selectedIcon() const { return m_selectedIcon; }

bool NavigationBarPushButton::isSelectedTextVisible() const { return m_selectedTextVisible; }

QSize NavigationBarPushButton::sizeHint() const { return QSize(64, 58); }

void NavigationBarPushButton::setText(const QString &text)
{
    m_text = text;
    update();
}

void NavigationBarPushButton::setIcon(const QIcon &icon)
{
    m_icon = icon;
    update();
}

void NavigationBarPushButton::setSelectedIcon(const QIcon &icon)
{
    m_selectedIcon = icon;
    update();
}

void NavigationBarPushButton::setSelectedTextVisible(bool visible)
{
    m_selectedTextVisible = visible;
    update();
}

void NavigationBarPushButton::setSelectedColor(const QColor &light, const QColor &dark)
{
    m_lightSelectedColor = light;
    m_darkSelectedColor = dark;
    update();
}

void NavigationBarPushButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        NavigationWidget::mouseReleaseEvent(event);
        return;
    }

    m_pressed = false;
    emit clicked(true);
    update();
}

void NavigationBarPushButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
    painter.setPen(Qt::NoPen);

    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    if (m_pressed) {
        painter.setOpacity(0.7);
    }

    if (m_selected) {
        painter.setBrush(dark ? QColor(255, 255, 255, 42) : QColor(Qt::white));
        painter.drawRoundedRect(rect(), 5, 5);
        painter.setBrush(selectedColor());
        const QRectF indicatorRect(0, m_pressed ? 19 : 16, 4, m_pressed ? 18 : 24);
        painter.drawRoundedRect(indicatorRect, 2, 2);
    } else if (m_pressed || m_hovered) {
        const int base = dark ? 255 : 0;
        painter.setBrush(QColor(base, base, base, m_hovered ? 9 : 6));
        painter.drawRoundedRect(rect(), 5, 5);
    }

    QIcon paintIcon = (m_selected && !m_selectedIcon.isNull()) ? m_selectedIcon : m_icon;
    QRect iconRect(22, 13, 20, 20);
    if (m_selected && !m_selectedTextVisible) {
        iconRect.translate(0, 6);
    }
    if (!paintIcon.isNull()) {
        paintIcon.paint(&painter, iconRect);
    }

    if (m_selected && !m_selectedTextVisible) {
        return;
    }

    painter.setOpacity(isEnabled() ? 1.0 : 0.4);
    painter.setPen(m_selected ? selectedColor() : textColor());
    painter.setFont(font());
    painter.drawText(QRect(0, 32, width(), 26), Qt::AlignCenter, m_text);
}

QColor NavigationBarPushButton::selectedColor() const
{
    const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
    const QColor custom = dark ? m_darkSelectedColor : m_lightSelectedColor;
    return custom.isValid() ? custom : ThemeManager::instance()->accentColor();
}

NavigationBar::NavigationBar(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedWidth(kBarWidth);
    FluentStyleSheet::setRole(this, QStringLiteral("NavigationBar"));
    initLayout();
}

NavigationBarPushButton *NavigationBar::addItem(const QString &routeKey, const QIcon &icon, const QString &text,
                                                bool selectable, const QIcon &selectedIcon,
                                                NavigationItemPosition position)
{
    return insertItem(-1, routeKey, icon, text, selectable, selectedIcon, position);
}

NavigationBarPushButton *NavigationBar::insertItem(int index, const QString &routeKey, const QIcon &icon,
                                                   const QString &text, bool selectable, const QIcon &selectedIcon,
                                                   NavigationItemPosition position)
{
    if (routeKey.isEmpty() || m_items.contains(routeKey)) {
        return nullptr;
    }

    auto *button = new NavigationBarPushButton(icon, text, selectable, selectedIcon, this);
    button->setSelectedTextVisible(m_selectedTextVisible);
    button->setSelectedColor(m_lightSelectedColor, m_darkSelectedColor);
    insertWidget(index, routeKey, button, position);
    return button;
}

NavigationWidget *NavigationBar::addWidget(const QString &routeKey, NavigationWidget *widget,
                                           NavigationItemPosition position)
{
    return insertWidget(-1, routeKey, widget, position);
}

NavigationWidget *NavigationBar::insertWidget(int index, const QString &routeKey, NavigationWidget *widget,
                                              NavigationItemPosition position)
{
    if (routeKey.isEmpty() || !widget || m_items.contains(routeKey)) {
        return nullptr;
    }

    widget->setRouteKey(routeKey);
    widget->setCompacted(false);
    m_items.insert(routeKey, widget);
    insertWidgetToLayout(index, widget, position);
    connect(widget, &NavigationWidget::clicked, this, [this, widget](bool triggeredByUser) {
        onWidgetClicked(widget, triggeredByUser);
    });
    return widget;
}

void NavigationBar::removeWidget(const QString &routeKey)
{
    NavigationWidget *removed = m_items.take(routeKey);
    if (!removed) {
        return;
    }

    if (m_currentRouteKey == routeKey) {
        m_currentRouteKey.clear();
    }

    m_topLayout->removeWidget(removed);
    m_scrollLayout->removeWidget(removed);
    m_bottomLayout->removeWidget(removed);
    removed->deleteLater();
}

NavigationWidget *NavigationBar::widget(const QString &routeKey) const { return m_items.value(routeKey, nullptr); }

bool NavigationBar::contains(const QString &routeKey) const { return m_items.contains(routeKey); }

QString NavigationBar::currentRouteKey() const { return m_currentRouteKey; }

bool NavigationBar::isSelectedTextVisible() const { return m_selectedTextVisible; }

bool NavigationBar::isIndicatorAnimationEnabled() const { return m_indicatorAnimationEnabled; }

void NavigationBar::setCurrentItem(const QString &routeKey)
{
    if (routeKey.isEmpty() || !m_items.contains(routeKey) || m_currentRouteKey == routeKey) {
        return;
    }

    m_currentRouteKey = routeKey;
    for (auto it = m_items.constBegin(); it != m_items.constEnd(); ++it) {
        it.value()->setSelected(it.key() == routeKey);
    }
    emit currentItemChanged(routeKey);
}

void NavigationBar::setSelectedTextVisible(bool visible)
{
    if (m_selectedTextVisible == visible) {
        return;
    }

    m_selectedTextVisible = visible;
    for (NavigationWidget *widget : std::as_const(m_items)) {
        if (auto *button = qobject_cast<NavigationBarPushButton *>(widget)) {
            button->setSelectedTextVisible(visible);
        }
    }
}

void NavigationBar::setIndicatorAnimationEnabled(bool enabled) { m_indicatorAnimationEnabled = enabled; }

void NavigationBar::setSelectedColor(const QColor &light, const QColor &dark)
{
    m_lightSelectedColor = light;
    m_darkSelectedColor = dark;
    for (NavigationWidget *widget : std::as_const(m_items)) {
        if (auto *button = qobject_cast<NavigationBarPushButton *>(widget)) {
            button->setSelectedColor(light, dark);
        }
    }
}

void NavigationBar::initLayout()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 5, 0, 5);
    m_mainLayout->setSpacing(4);

    m_topLayout = new QVBoxLayout;
    m_topLayout->setContentsMargins(4, 0, 4, 0);
    m_topLayout->setSpacing(4);
    m_topLayout->setAlignment(Qt::AlignTop);

    m_bottomLayout = new QVBoxLayout;
    m_bottomLayout->setContentsMargins(4, 0, 4, 0);
    m_bottomLayout->setSpacing(4);
    m_bottomLayout->setAlignment(Qt::AlignBottom);

    m_scrollArea = new ScrollArea(this);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->enableTransparentBackground(true);
    m_scrollWidget = new QWidget(m_scrollArea);
    m_scrollWidget->setObjectName(QStringLiteral("NavigationScrollWidget"));
    FluentStyleSheet::setRole(m_scrollWidget, QStringLiteral("NavigationScrollWidget"));

    m_scrollLayout = new QVBoxLayout(m_scrollWidget);
    m_scrollLayout->setContentsMargins(4, 0, 4, 0);
    m_scrollLayout->setSpacing(4);
    m_scrollLayout->setAlignment(Qt::AlignTop);
    m_scrollArea->setWidget(m_scrollWidget);
    m_scrollArea->setWidgetResizable(true);

    m_mainLayout->addLayout(m_topLayout);
    m_mainLayout->addWidget(m_scrollArea, 1);
    m_mainLayout->addLayout(m_bottomLayout);
}

void NavigationBar::insertWidgetToLayout(int index, NavigationWidget *widget, NavigationItemPosition position)
{
    QVBoxLayout *layout = m_topLayout;
    Qt::Alignment alignment = Qt::AlignTop | Qt::AlignHCenter;
    if (position == NavigationItemPosition::Scroll) {
        layout = m_scrollLayout;
    } else if (position == NavigationItemPosition::Bottom) {
        layout = m_bottomLayout;
        alignment = Qt::AlignBottom | Qt::AlignHCenter;
    }

    const int insertIndex = index < 0 ? layout->count() : qBound(0, index, layout->count());
    layout->insertWidget(insertIndex, widget, 0, alignment);
    widget->show();
}

void NavigationBar::onWidgetClicked(NavigationWidget *widget, bool triggeredByUser)
{
    Q_UNUSED(triggeredByUser)

    if (!widget || widget->routeKey().isEmpty()) {
        return;
    }

    if (widget->isSelectable()) {
        setCurrentItem(widget->routeKey());
    }
    emit itemClicked(widget->routeKey());
}

} // namespace FluentQt
