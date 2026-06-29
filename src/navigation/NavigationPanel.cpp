#include <FluentQtWidgets/Navigation/NavigationPanel.h>

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>

#include <QtCore/QPropertyAnimation>
#include <QtCore/QSet>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

NavigationPanel::NavigationPanel(QWidget *parent) : QFrame(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    FluentStyleSheet::setRole(this, QStringLiteral("NavigationPanel"));
    setProperty("menu", false);
    setFixedWidth(kCompactWidth);

    initLayout();

    connect(m_menuButton, &TransparentToolButton::clicked, this, &NavigationPanel::toggle);
    connect(m_returnButton, &TransparentToolButton::clicked, this, &NavigationPanel::onReturnClicked);

    m_returnButton->hide();
    m_returnButton->setDisabled(true);
    m_returnButton->setToolTip(QStringLiteral("Back"));
    m_menuButton->setToolTip(QStringLiteral("Open Navigation"));

    m_expandAnimation = new QPropertyAnimation(this, "geometry", this);
    m_expandAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_expandAnimation->setDuration(150);
    connect(m_expandAnimation, &QPropertyAnimation::finished, this, &NavigationPanel::onExpandAnimationFinished);
}

NavigationDisplayMode NavigationPanel::displayMode() const { return m_displayMode; }

bool NavigationPanel::isMenu() const { return m_menu; }

int NavigationPanel::expandWidth() const { return m_expandWidth; }

NavigationTreeWidget *NavigationPanel::addItem(const QString &routeKey, const QIcon &icon, const QString &text,
                                               NavigationItemPosition position, const QString &parentRouteKey,
                                               bool selectable)
{
    return insertItem(-1, routeKey, icon, text, position, parentRouteKey, selectable);
}

NavigationTreeWidget *NavigationPanel::insertItem(int index, const QString &routeKey, const QIcon &icon,
                                                  const QString &text, NavigationItemPosition position,
                                                  const QString &parentRouteKey, bool selectable)
{
    if (routeKey.isEmpty() || m_items.contains(routeKey)) {
        return nullptr;
    }

    auto *item = new NavigationTreeWidget(icon, text, selectable, this);
    insertWidget(index, routeKey, item, position, parentRouteKey);

    return item;
}

NavigationWidget *NavigationPanel::addWidget(const QString &routeKey, NavigationWidget *widget,
                                             NavigationItemPosition position, const QString &parentRouteKey)
{
    return insertWidget(-1, routeKey, widget, position, parentRouteKey);
}

NavigationWidget *NavigationPanel::insertWidget(int index, const QString &routeKey, NavigationWidget *widget,
                                                NavigationItemPosition position, const QString &parentRouteKey)
{
    if (routeKey.isEmpty() || !widget || m_items.contains(routeKey)) {
        return nullptr;
    }

    widget->setRouteKey(routeKey);
    widget->setCompacted(m_displayMode == NavigationDisplayMode::Compact);

    NavigationItemEntry entry;
    entry.routeKey = routeKey;
    entry.parentRouteKey = parentRouteKey;
    entry.widget = widget;
    m_items.insert(routeKey, entry);

    auto *treeWidget = qobject_cast<NavigationTreeWidget *>(widget);
    auto *parentTreeWidget =
        !parentRouteKey.isEmpty() ? qobject_cast<NavigationTreeWidget *>(navigationWidget(parentRouteKey)) : nullptr;
    if (treeWidget && parentTreeWidget) {
        parentTreeWidget->insertChild(index, treeWidget);
    } else {
        insertWidgetToLayout(index, widget, position);
    }

    connect(widget, &NavigationWidget::clicked, this, [this, widget](bool triggeredByUser) {
        onItemClicked(widget, triggeredByUser);
    });

    return widget;
}

NavigationItemHeader *NavigationPanel::addItemHeader(const QString &text, NavigationItemPosition position)
{
    return insertItemHeader(-1, text, position);
}

NavigationItemHeader *NavigationPanel::insertItemHeader(int index, const QString &text,
                                                        NavigationItemPosition position)
{
    auto *header = new NavigationItemHeader(text, this);
    header->setCompacted(m_displayMode == NavigationDisplayMode::Compact);
    insertWidgetToLayout(index, header, position);
    return header;
}

NavigationUserCard *NavigationPanel::addUserCard(const QString &routeKey, const QIcon &avatar, const QString &title,
                                                 const QString &subtitle, NavigationItemPosition position)
{
    auto *card = new NavigationUserCard(this);
    card->setTitle(title);
    card->setSubtitle(subtitle);
    card->setAvatarIcon(avatar);
    return qobject_cast<NavigationUserCard *>(addWidget(routeKey, card, position));
}

NavigationUserCard *NavigationPanel::addUserCard(const QString &routeKey, const QString &avatarPath,
                                                 const QString &title, const QString &subtitle,
                                                 NavigationItemPosition position)
{
    auto *card = new NavigationUserCard(this);
    card->setTitle(title);
    card->setSubtitle(subtitle);
    card->setAvatar(avatarPath);
    return qobject_cast<NavigationUserCard *>(addWidget(routeKey, card, position));
}

QFrame *NavigationPanel::addSeparator(NavigationItemPosition position)
{
    auto *separator = new QFrame(this);
    separator->setFrameShape(QFrame::HLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setFixedHeight(9);
    separator->setAttribute(Qt::WA_StyledBackground, true);
    FluentStyleSheet::setRole(separator, QStringLiteral("NavigationSeparator"));

    QVBoxLayout *layout = m_scrollLayout;
    if (position == NavigationItemPosition::Top) {
        layout = m_topLayout;
    } else if (position == NavigationItemPosition::Bottom) {
        layout = m_bottomLayout;
    }

    layout->addWidget(separator, 0, Qt::AlignTop);
    return separator;
}

NavigationTreeWidget *NavigationPanel::widget(const QString &routeKey) const
{
    return qobject_cast<NavigationTreeWidget *>(navigationWidget(routeKey));
}

NavigationWidget *NavigationPanel::navigationWidget(const QString &routeKey) const
{
    return m_items.contains(routeKey) ? m_items.value(routeKey).widget : nullptr;
}

bool NavigationPanel::contains(const QString &routeKey) const { return m_items.contains(routeKey); }

QString NavigationPanel::currentRouteKey() const { return m_currentRouteKey; }

bool NavigationPanel::isMenuButtonVisible() const { return m_menuButtonVisible; }

bool NavigationPanel::isReturnButtonVisible() const { return m_returnButtonVisible; }

TransparentToolButton *NavigationPanel::returnButton() const { return m_returnButton; }

void NavigationPanel::setMenu(bool menu)
{
    if (m_menu == menu) {
        return;
    }

    m_menu = menu;
    setProperty("menu", menu);
    if (style()) {
        style()->unpolish(this);
        style()->polish(this);
    }
    update();
    emit menuChanged(menu);
}

void NavigationPanel::setExpandWidth(int width)
{
    if (width > kCompactWidth) {
        m_expandWidth = width;
    }
}

void NavigationPanel::setMenuButtonVisible(bool visible)
{
    m_menuButtonVisible = visible;
    m_menuButton->setVisible(visible);
}

void NavigationPanel::setReturnButtonVisible(bool visible)
{
    m_returnButtonVisible = visible;
    m_returnButton->setVisible(visible);
}

void NavigationPanel::setCurrentItem(const QString &routeKey)
{
    if (routeKey.isEmpty() || !m_items.contains(routeKey) || m_currentRouteKey == routeKey) {
        return;
    }

    if (!m_currentRouteKey.isEmpty() && m_items.contains(m_currentRouteKey)) {
        m_items.value(m_currentRouteKey).widget->setSelected(false);
    }

    m_currentRouteKey = routeKey;
    m_items.value(routeKey).widget->setSelected(true);
}

void NavigationPanel::removeWidget(const QString &routeKey)
{
    if (!m_items.contains(routeKey)) {
        return;
    }

    NavigationItemEntry entry = m_items.take(routeKey);
    if (m_currentRouteKey == routeKey) {
        m_currentRouteKey.clear();
    }

    auto *treeWidget = qobject_cast<NavigationTreeWidget *>(entry.widget);
    auto *parentTreeWidget =
        !entry.parentRouteKey.isEmpty() ? qobject_cast<NavigationTreeWidget *>(navigationWidget(entry.parentRouteKey))
                                        : nullptr;
    if (treeWidget && parentTreeWidget) {
        parentTreeWidget->removeChild(treeWidget);
    } else {
        m_scrollLayout->removeWidget(entry.widget);
        m_topLayout->removeWidget(entry.widget);
        m_bottomLayout->removeWidget(entry.widget);
    }

    entry.widget->deleteLater();
}

void NavigationPanel::expand(bool animated)
{
    QWidget *topLevel = QWidget::window();
    const bool useMenuMode = topLevel && topLevel->width() < m_expandWidth + 200;
    if (useMenuMode) {
        setDisplayMode(NavigationDisplayMode::Menu);
        setMenu(true);
    } else {
        setDisplayMode(NavigationDisplayMode::Expand);
        setMenu(false);
    }

    setItemsCompacted(false);

    if (animated) {
        m_expandAnimation->stop();
        m_expandAnimation->setStartValue(geometry());
        m_expandAnimation->setEndValue(QRect(x(), y(), m_expandWidth, height()));
        m_expandAnimation->start();
    } else {
        updatePanelWidth(m_expandWidth);
        emit displayModeChanged(m_displayMode);
    }

    m_menuButton->setToolTip(QStringLiteral("Close Navigation"));
}

void NavigationPanel::collapse(bool animated)
{
    setDisplayMode(NavigationDisplayMode::Compact);
    setMenu(false);

    if (animated) {
        m_expandAnimation->stop();
        m_expandAnimation->setStartValue(geometry());
        m_expandAnimation->setEndValue(QRect(x(), y(), kCompactWidth, height()));
        m_expandAnimation->start();
    } else {
        setItemsCompacted(true);
        updatePanelWidth(kCompactWidth);
        emit displayModeChanged(m_displayMode);
    }

    m_menuButton->setToolTip(QStringLiteral("Open Navigation"));
}

void NavigationPanel::toggle()
{
    if (m_displayMode == NavigationDisplayMode::Compact) {
        expand();
    } else {
        collapse();
    }
}

void NavigationPanel::onExpandAnimationFinished()
{
    if (m_displayMode == NavigationDisplayMode::Compact) {
        setItemsCompacted(true);
        updatePanelWidth(kCompactWidth);
    } else {
        updatePanelWidth(m_expandWidth);
    }
    emit displayModeChanged(m_displayMode);
}

bool NavigationPanel::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    Q_UNUSED(event)
    return QFrame::eventFilter(watched, event);
}

void NavigationPanel::initLayout()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 5, 0, 5);
    m_mainLayout->setSpacing(4);

    m_topLayout = new QVBoxLayout;
    m_topLayout->setContentsMargins(4, 0, 4, 0);
    m_topLayout->setSpacing(4);

    m_bottomLayout = new QVBoxLayout;
    m_bottomLayout->setContentsMargins(4, 0, 4, 0);
    m_bottomLayout->setSpacing(4);

    m_returnButton = new TransparentToolButton(icon(FluentIcon::Back), this);
    m_returnButton->setFixedSize(40, 36);
    m_menuButton = new TransparentToolButton(this);
    m_menuButton->setFixedSize(40, 36);
    m_menuButton->setText(QStringLiteral("\u2630"));
    m_menuButton->setToolButtonStyle(Qt::ToolButtonTextOnly);

    m_topLayout->addWidget(m_returnButton, 0, Qt::AlignTop);
    m_topLayout->addWidget(m_menuButton, 0, Qt::AlignTop);

    m_scrollArea = new ScrollArea(this);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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

void NavigationPanel::insertWidgetToLayout(int index, NavigationWidget *widget, NavigationItemPosition position)
{
    QVBoxLayout *layout = m_scrollLayout;
    if (position == NavigationItemPosition::Top) {
        layout = m_topLayout;
    } else if (position == NavigationItemPosition::Bottom) {
        layout = m_bottomLayout;
    }

    const int insertIndex = index < 0 ? layout->count() : qBound(0, index, layout->count());
    layout->insertWidget(insertIndex, widget, 0, Qt::AlignTop);
    widget->show();
}

void NavigationPanel::setDisplayMode(NavigationDisplayMode mode)
{
    m_displayMode = mode;
}

void NavigationPanel::setItemsCompacted(bool compacted)
{
    QSet<NavigationWidget *> updated;
    for (auto it = m_items.constBegin(); it != m_items.constEnd(); ++it) {
        it.value().widget->setCompacted(compacted);
        updated.insert(it.value().widget);
    }

    const auto updateLayout = [&updated, compacted](QLayout *layout) {
        if (!layout) {
            return;
        }

        for (int i = 0; i < layout->count(); ++i) {
            auto *widget = qobject_cast<NavigationWidget *>(layout->itemAt(i)->widget());
            if (widget && !updated.contains(widget)) {
                widget->setCompacted(compacted);
                updated.insert(widget);
            }
        }
    };

    updateLayout(m_topLayout);
    updateLayout(m_scrollLayout);
    updateLayout(m_bottomLayout);
}

void NavigationPanel::updatePanelWidth(int width)
{
    setFixedWidth(width);
}

void NavigationPanel::onItemClicked(NavigationWidget *item, bool triggeredByUser)
{
    Q_UNUSED(triggeredByUser)

    if (!item || item->routeKey().isEmpty()) {
        return;
    }

    if (item->isSelectable()) {
        setCurrentItem(item->routeKey());
    }
    emit itemClicked(item->routeKey());
}

void NavigationPanel::onReturnClicked() { emit returnRequested(); }

} // namespace FluentQt
