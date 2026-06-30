#include <FluentQtWidgets/Widgets/TabView.h>

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Widgets/Button.h>
#include <FluentQtWidgets/Widgets/ToolTip.h>

#include <QtCore/QMimeData>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QUuid>
#include <QtCore/QVariant>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/QEnterEvent>
#endif
#include <QtGui/QDrag>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>

#include <utility>

namespace FluentQt {

namespace {
QString closeButtonModeName(TabCloseButtonDisplayMode mode)
{
    switch (mode) {
    case TabCloseButtonDisplayMode::Always:
        return QStringLiteral("Always");
    case TabCloseButtonDisplayMode::OnHover:
        return QStringLiteral("OnHover");
    case TabCloseButtonDisplayMode::Never:
        return QStringLiteral("Never");
    }
    return QStringLiteral("Always");
}

QString generatedRouteKey() { return QUuid::createUuid().toString(QUuid::WithoutBraces); }

const char *kTabMimeType = "application/x-fluent-tab-index";
} // namespace

TabItem::TabItem(QWidget *parent) : QPushButton(parent) { init(); }

TabItem::TabItem(const QString &routeKey, const QString &text, const QIcon &icon, QWidget *parent)
    : QPushButton(icon, text, parent)
{
    init();
    setRouteKey(routeKey);
}

QString TabItem::routeKey() const { return m_routeKey; }

void TabItem::setRouteKey(const QString &routeKey) { m_routeKey = routeKey; }

bool TabItem::isSelected() const { return m_selected; }

TabCloseButtonDisplayMode TabItem::closeButtonDisplayMode() const { return m_closeButtonDisplayMode; }

QToolButton *TabItem::closeButton() const { return m_closeButton; }

bool TabItem::isShadowEnabled() const { return m_shadowEnabled; }

void TabItem::setSelected(bool selected)
{
    if (m_selected == selected) {
        return;
    }

    m_selected = selected;
    setChecked(m_selected);
    setProperty("selected", m_selected);
    updateCloseButtonVisibility();
    updateShadowVisibility();
    if (m_selected) {
        raise();
    }
    FluentStyleSheet::polish(this);
}

void TabItem::setCloseButtonDisplayMode(TabCloseButtonDisplayMode mode)
{
    if (m_closeButtonDisplayMode == mode) {
        return;
    }

    m_closeButtonDisplayMode = mode;
    setProperty("closeButtonDisplayMode", closeButtonModeName(m_closeButtonDisplayMode));
    updateCloseButtonVisibility();
}

void TabItem::setShadowEnabled(bool enabled)
{
    if (m_shadowEnabled == enabled) {
        return;
    }

    m_shadowEnabled = enabled;
    updateShadowVisibility();
}

void TabItem::slideTo(int x, int durationMs)
{
    if (!m_slideAnimation) {
        return;
    }

    m_slideAnimation->stop();
    m_slideAnimation->setStartValue(pos());
    m_slideAnimation->setEndValue(QPoint(x, y()));
    m_slideAnimation->setDuration(qMax(0, durationMs));
    m_slideAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_slideAnimation->start();
}

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void TabItem::enterEvent(QEnterEvent *event)
#else
void TabItem::enterEvent(QEvent *event)
#endif
{
    m_hovered = true;
    updateCloseButtonVisibility();
    QPushButton::enterEvent(event);
}

void TabItem::leaveEvent(QEvent *event)
{
    m_hovered = false;
    updateCloseButtonVisibility();
    QPushButton::leaveEvent(event);
}

void TabItem::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPos = event->pos();
        m_dragStarted = false;
    }
    QPushButton::mousePressEvent(event);
}

void TabItem::mouseMoveEvent(QMouseEvent *event)
{
    auto *tabBar = qobject_cast<TabBar *>(parentWidget());
    for (QWidget *parent = parentWidget(); parent && !tabBar; parent = parent->parentWidget()) {
        tabBar = qobject_cast<TabBar *>(parent);
    }
    if (!tabBar || !tabBar->isMovable() || !(event->buttons() & Qt::LeftButton)) {
        QPushButton::mouseMoveEvent(event);
        return;
    }

    if ((event->pos() - m_dragStartPos).manhattanLength() < QApplication::startDragDistance()) {
        QPushButton::mouseMoveEvent(event);
        return;
    }

    const int index = tabBar->indexOf(routeKey());
    if (index < 0) {
        return;
    }

    auto *mimeData = new QMimeData();
    mimeData->setData(kTabMimeType, QByteArray::number(index));
    auto *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
    m_dragStarted = true;
    emit dragStarted();
}

void TabItem::mouseReleaseEvent(QMouseEvent *event)
{
    QPushButton::mouseReleaseEvent(event);
    m_dragStarted = false;
}

void TabItem::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit doubleClicked();
    }
    QPushButton::mouseDoubleClickEvent(event);
}

void TabItem::resizeEvent(QResizeEvent *event)
{
    QPushButton::resizeEvent(event);
    if (m_closeButton) {
        m_closeButton->move(width() - m_closeButton->width() - 6, (height() - m_closeButton->height()) / 2);
    }
}

void TabItem::init()
{
    setCheckable(true);
    setCursor(Qt::PointingHandCursor);
    setMinimumSize(QSize(64, 36));
    setMaximumWidth(240);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setProperty("selected", false);
    setProperty("closeButtonDisplayMode", closeButtonModeName(m_closeButtonDisplayMode));

    m_shadowEffect = new QGraphicsDropShadowEffect(this);
    m_shadowEffect->setBlurRadius(5);
    m_shadowEffect->setOffset(0, 1);
    setGraphicsEffect(m_shadowEffect);

    m_slideAnimation = new QPropertyAnimation(this, "pos", this);

    m_closeButton = new TransparentToolButton(FluentQt::icon(FluentIcon::Close), this);
    m_closeButton->setFixedSize(24, 24);
    m_closeButton->setIconSize(QSize(10, 10));
    m_closeButton->setToolTip(tr("Close"));
    connect(m_closeButton, &QToolButton::clicked, this, &TabItem::closed);

    installEventFilter(new ToolTipFilter(this, 1000));

    FluentStyleSheet::setRole(this, QStringLiteral("TabItem"));
    FluentStyleSheet::setRole(m_closeButton, QStringLiteral("TabCloseButton"));
    updateCloseButtonVisibility();
    updateShadowVisibility();
}

void TabItem::updateCloseButtonVisibility()
{
    if (!m_closeButton) {
        return;
    }

    const bool visible = m_closeButtonDisplayMode == TabCloseButtonDisplayMode::Always ||
                         (m_closeButtonDisplayMode == TabCloseButtonDisplayMode::OnHover && (m_hovered || m_selected));
    m_closeButton->setVisible(visible && m_closeButtonDisplayMode != TabCloseButtonDisplayMode::Never);
}

void TabItem::updateShadowVisibility()
{
    if (!m_shadowEffect) {
        return;
    }

    const int alpha = m_shadowEnabled && m_selected ? 50 : 0;
    m_shadowEffect->setColor(QColor(0, 0, 0, alpha));
}

TabBar::TabBar(QWidget *parent) : QWidget(parent) { init(); }

int TabBar::addTab(const QString &routeKey, const QString &text, const QIcon &icon)
{
    return insertTab(count(), routeKey, text, icon);
}

int TabBar::insertTab(int index, const QString &routeKey, const QString &text, const QIcon &icon)
{
    const QString key = normalizeRouteKey(routeKey);
    if (key.isEmpty() || m_itemMap.contains(key)) {
        return -1;
    }

    const int boundedIndex = qBound(0, index, count());
    auto *item = new TabItem(key, text, icon, this);
    item->setMaximumWidth(m_tabMaximumWidth);
    item->setCloseButtonDisplayMode(m_closeButtonDisplayMode);
    item->setShadowEnabled(m_tabShadowEnabled);
    item->setProperty("data", QVariant());
    connectItem(item);

    m_items.insert(boundedIndex, item);
    m_itemMap.insert(key, item);
    m_itemLayout->insertWidget(boundedIndex, item, 0, Qt::AlignVCenter);

    if (boundedIndex <= m_currentIndex) {
        ++m_currentIndex;
    }

    if (m_currentIndex < 0) {
        setCurrentIndex(0);
    } else {
        updateSelectedItems();
    }

    return boundedIndex;
}

void TabBar::removeTab(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }

    const int previousCurrentIndex = m_currentIndex;
    const bool currentAffected = index <= previousCurrentIndex;
    TabItem *item = m_items.takeAt(index);
    m_itemMap.remove(item->routeKey());
    m_itemLayout->removeWidget(item);
    item->deleteLater();

    if (m_items.isEmpty()) {
        m_currentIndex = -1;
        if (previousCurrentIndex != -1) {
            emit currentChanged(-1);
        }
        return;
    }

    if (index < m_currentIndex) {
        --m_currentIndex;
        updateSelectedItems();
    } else if (index == m_currentIndex) {
        m_currentIndex = qMin(index, count() - 1);
        updateSelectedItems();
    } else {
        updateSelectedItems();
    }

    if (currentAffected) {
        emit currentChanged(m_currentIndex);
    }
}

void TabBar::removeTabByKey(const QString &routeKey) { removeTab(indexOf(routeKey)); }

void TabBar::clear()
{
    while (!m_items.isEmpty()) {
        removeTab(0);
    }
}

int TabBar::count() const { return m_items.size(); }

int TabBar::currentIndex() const { return m_currentIndex; }

QString TabBar::currentRouteKey() const
{
    TabItem *item = tabItem(m_currentIndex);
    return item ? item->routeKey() : QString();
}

TabItem *TabBar::currentTab() const { return tabItem(m_currentIndex); }

int TabBar::indexOf(const QString &routeKey) const
{
    for (int i = 0; i < m_items.size(); ++i) {
        if (m_items.at(i)->routeKey() == routeKey) {
            return i;
        }
    }
    return -1;
}

bool TabBar::contains(const QString &routeKey) const { return m_itemMap.contains(routeKey); }

TabItem *TabBar::tabItem(int index) const { return index >= 0 && index < count() ? m_items.at(index) : nullptr; }

TabItem *TabBar::tab(const QString &routeKey) const { return m_itemMap.value(routeKey, nullptr); }

QList<QString> TabBar::routeKeys() const
{
    QList<QString> keys;
    keys.reserve(m_items.size());
    for (TabItem *item : m_items) {
        keys.append(item->routeKey());
    }
    return keys;
}

QString TabBar::tabText(int index) const
{
    TabItem *item = tabItem(index);
    return item ? item->text() : QString();
}

void TabBar::setTabText(int index, const QString &text)
{
    if (TabItem *item = tabItem(index)) {
        item->setText(text);
    }
}

QIcon TabBar::tabIcon(int index) const
{
    TabItem *item = tabItem(index);
    return item ? item->icon() : QIcon();
}

void TabBar::setTabIcon(int index, const QIcon &icon)
{
    if (TabItem *item = tabItem(index)) {
        item->setIcon(icon);
    }
}

QVariant TabBar::tabData(int index) const
{
    TabItem *item = tabItem(index);
    return item ? item->property("data") : QVariant();
}

void TabBar::setTabData(int index, const QVariant &data)
{
    if (TabItem *item = tabItem(index)) {
        item->setProperty("data", data);
    }
}

bool TabBar::isTabEnabled(int index) const
{
    TabItem *item = tabItem(index);
    return item && item->isEnabled();
}

void TabBar::setTabEnabled(int index, bool enabled)
{
    if (TabItem *item = tabItem(index)) {
        item->setEnabled(enabled);
    }
}

bool TabBar::tabsClosable() const { return m_tabsClosable; }

void TabBar::setTabsClosable(bool closable)
{
    m_tabsClosable = closable;
    setCloseButtonDisplayMode(closable ? TabCloseButtonDisplayMode::Always : TabCloseButtonDisplayMode::Never);
}

TabCloseButtonDisplayMode TabBar::closeButtonDisplayMode() const { return m_closeButtonDisplayMode; }

void TabBar::setCloseButtonDisplayMode(TabCloseButtonDisplayMode mode)
{
    if (m_closeButtonDisplayMode == mode) {
        return;
    }

    m_closeButtonDisplayMode = mode;
    m_tabsClosable = mode != TabCloseButtonDisplayMode::Never;
    for (TabItem *item : m_items) {
        item->setCloseButtonDisplayMode(mode);
    }
}

int TabBar::tabMaximumWidth() const { return m_tabMaximumWidth; }

void TabBar::setTabMaximumWidth(int width)
{
    const int boundedWidth = qMax(48, width);
    if (m_tabMaximumWidth == boundedWidth) {
        return;
    }

    m_tabMaximumWidth = boundedWidth;
    for (TabItem *item : m_items) {
        item->setMaximumWidth(m_tabMaximumWidth);
    }
    updateOverflowButtons();
}

bool TabBar::isAddButtonVisible() const { return m_addButton && m_addButton->isVisible(); }

QToolButton *TabBar::addButton() const { return m_addButton; }

void TabBar::setCurrentIndex(int index)
{
    if (index < 0 || index >= count() || m_currentIndex == index) {
        return;
    }

    m_currentIndex = index;
    updateSelectedItems();
    emit currentChanged(index);
}

void TabBar::setCurrentTab(const QString &routeKey) { setCurrentIndex(indexOf(routeKey)); }

void TabBar::setAddButtonVisible(bool visible)
{
    if (m_addButton) {
        m_addButton->setVisible(visible);
    }
}

bool TabBar::isMovable() const { return m_movable; }

bool TabBar::isScrollable() const { return m_scrollable; }

bool TabBar::isTabShadowEnabled() const { return m_tabShadowEnabled; }

void TabBar::setMovable(bool movable) { m_movable = movable; }

void TabBar::setScrollable(bool scrollable)
{
    if (m_scrollable == scrollable) {
        return;
    }

    m_scrollable = scrollable;
    const int minWidth = scrollable ? 240 : 64;
    for (TabItem *item : m_items) {
        item->setMinimumWidth(minWidth);
    }
    updateOverflowButtons();
}

void TabBar::setTabShadowEnabled(bool enabled)
{
    if (m_tabShadowEnabled == enabled) {
        return;
    }

    m_tabShadowEnabled = enabled;
    for (TabItem *item : m_items) {
        item->setShadowEnabled(enabled);
    }
}

void TabBar::scrollPrevious()
{
    if (m_scrollArea && m_scrollArea->horizontalScrollBar()) {
        auto *bar = m_scrollArea->horizontalScrollBar();
        bar->setValue(bar->value() - 120);
    }
}

void TabBar::scrollNext()
{
    if (m_scrollArea && m_scrollArea->horizontalScrollBar()) {
        auto *bar = m_scrollArea->horizontalScrollBar();
        bar->setValue(bar->value() + 120);
    }
}

void TabBar::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateOverflowButtons();
}

void TabBar::dragEnterEvent(QDragEnterEvent *event)
{
    if (m_movable && event->mimeData()->hasFormat(kTabMimeType)) {
        event->acceptProposedAction();
    }
}

void TabBar::dragMoveEvent(QDragMoveEvent *event)
{
    if (m_movable && event->mimeData()->hasFormat(kTabMimeType)) {
        event->acceptProposedAction();
    }
}

void TabBar::dropEvent(QDropEvent *event)
{
    if (!m_movable || !event->mimeData()->hasFormat(kTabMimeType)) {
        return;
    }

    const int from = event->mimeData()->data(kTabMimeType).toInt();
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPoint pos = m_tabStrip->mapFromGlobal(event->position().toPoint());
#else
    const QPoint pos = m_tabStrip->mapFromGlobal(event->pos());
#endif
    int to = 0;
    int offset = 0;
    for (int i = 0; i < m_items.size(); ++i) {
        const int width = m_items.at(i)->width();
        if (pos.x() < offset + width / 2) {
            to = i;
            break;
        }
        offset += width;
        to = i + 1;
    }

    to = qBound(0, to, count() - 1);
    if (from >= 0 && from < count() && from != to) {
        moveTab(from, to);
        emit tabMoved(from, to);
    }

    event->acceptProposedAction();
}

void TabBar::init()
{
    setAcceptDrops(true);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(5, 5, 5, 5);
    m_layout->setSpacing(2);
    m_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_previousButton = new TransparentToolButton(FluentQt::icon(FluentIcon::LeftArrow), this);
    m_previousButton->setFixedSize(24, 28);
    m_previousButton->setIconSize(QSize(10, 10));
    m_previousButton->setToolTip(tr("Scroll tabs left"));
    connect(m_previousButton, &QToolButton::clicked, this, &TabBar::scrollPrevious);

    m_nextButton = new TransparentToolButton(FluentQt::icon(FluentIcon::RightArrow), this);
    m_nextButton->setFixedSize(24, 28);
    m_nextButton->setIconSize(QSize(10, 10));
    m_nextButton->setToolTip(tr("Scroll tabs right"));
    connect(m_nextButton, &QToolButton::clicked, this, &TabBar::scrollNext);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(m_scrollArea->horizontalScrollBar(), &QScrollBar::valueChanged, this,
            [this]() { updateOverflowButtons(); });

    m_tabStrip = new QWidget(m_scrollArea);
    m_itemLayout = new QHBoxLayout(m_tabStrip);
    m_itemLayout->setContentsMargins(0, 0, 0, 0);
    m_itemLayout->setSpacing(2);
    m_itemLayout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_scrollArea->setWidget(m_tabStrip);

    m_addButton = new TransparentToolButton(FluentQt::icon(FluentIcon::Add), this);
    m_addButton->setFixedSize(32, 28);
    m_addButton->setIconSize(QSize(12, 12));
    m_addButton->setToolTip(tr("New tab"));
    connect(m_addButton, &QToolButton::clicked, this, &TabBar::tabAddRequested);

    m_layout->addWidget(m_previousButton, 0, Qt::AlignVCenter);
    m_layout->addWidget(m_scrollArea, 1);
    m_layout->addWidget(m_nextButton, 0, Qt::AlignVCenter);
    m_layout->addWidget(m_addButton, 0, Qt::AlignVCenter);

    FluentStyleSheet::setRole(this, QStringLiteral("TabBar"));
    FluentStyleSheet::setRole(m_addButton, QStringLiteral("TabAddButton"));
    FluentStyleSheet::setRole(m_previousButton, QStringLiteral("TabScrollButton"));
    FluentStyleSheet::setRole(m_nextButton, QStringLiteral("TabScrollButton"));
    updateOverflowButtons();
}

QWidget *TabBar::tabStripWidget() const { return m_tabStrip; }

void TabBar::moveTab(int from, int to)
{
    if (from < 0 || from >= count() || to < 0 || to >= count() || from == to) {
        return;
    }

    TabItem *item = m_items.takeAt(from);
    m_items.insert(to, item);

    while (QLayoutItem *child = m_itemLayout->takeAt(0)) {
        delete child;
    }
    for (TabItem *tabItem : std::as_const(m_items)) {
        m_itemLayout->addWidget(tabItem, 0, Qt::AlignVCenter);
    }

    if (m_currentIndex == from) {
        m_currentIndex = to;
    } else if (from < m_currentIndex && to >= m_currentIndex) {
        --m_currentIndex;
    } else if (from > m_currentIndex && to <= m_currentIndex) {
        ++m_currentIndex;
    }

    updateSelectedItems();
    emit currentChanged(m_currentIndex);
    updateOverflowButtons();
}

void TabBar::updateOverflowButtons()
{
    if (!m_scrollArea || !m_tabStrip || !m_previousButton || !m_nextButton) {
        return;
    }

    m_tabStrip->adjustSize();
    int totalWidth = 0;
    for (TabItem *item : std::as_const(m_items)) {
        totalWidth += item->sizeHint().width() + m_itemLayout->spacing();
    }
    if (!m_items.isEmpty()) {
        totalWidth -= m_itemLayout->spacing();
    }
    m_tabStrip->setMinimumWidth(totalWidth);

    const bool overflow = m_scrollArea->viewport()->width() < m_tabStrip->minimumWidth();
    const bool showButtons = m_scrollable && overflow;
    m_previousButton->setVisible(showButtons);
    m_nextButton->setVisible(showButtons);
}

QString TabBar::normalizeRouteKey(const QString &routeKey) const
{
    return routeKey.isEmpty() ? generatedRouteKey() : routeKey;
}

void TabBar::rebuildRouteMap()
{
    m_itemMap.clear();
    for (TabItem *item : m_items) {
        m_itemMap.insert(item->routeKey(), item);
    }
}

void TabBar::updateSelectedItems()
{
    for (int i = 0; i < m_items.size(); ++i) {
        m_items.at(i)->setSelected(i == m_currentIndex);
    }
}

void TabBar::connectItem(TabItem *item)
{
    connect(item, &QPushButton::clicked, this, [this, item]() {
        const int index = m_items.indexOf(item);
        if (index < 0) {
            return;
        }
        emit tabBarClicked(index);
        setCurrentIndex(index);
    });
    connect(item, &TabItem::doubleClicked, this, [this, item]() {
        const int index = m_items.indexOf(item);
        if (index >= 0) {
            emit tabBarDoubleClicked(index);
        }
    });
    connect(item, &TabItem::closed, this, [this, item]() {
        const int index = m_items.indexOf(item);
        if (index >= 0) {
            emit tabCloseRequested(index);
        }
    });
}

TabWidget::TabWidget(QWidget *parent) : QWidget(parent) { init(); }

int TabWidget::addTab(QWidget *page, const QString &label, const QIcon &icon, const QString &routeKey)
{
    return insertTab(count(), page, label, icon, routeKey);
}

int TabWidget::addPage(QWidget *page, const QString &label, const QIcon &icon, const QString &routeKey)
{
    return addTab(page, label, icon, routeKey);
}

int TabWidget::insertTab(int index, QWidget *page, const QString &label, const QIcon &icon, const QString &routeKey)
{
    if (!page || m_stackedWidget->indexOf(page) >= 0) {
        return -1;
    }

    const QString key = ensureRouteKey(page, routeKey);
    if (m_tabBar->contains(key)) {
        return -1;
    }

    const int boundedIndex = qBound(0, index, count());
    m_stackedWidget->insertWidget(boundedIndex, page);

    const int tabIndex = m_tabBar->insertTab(boundedIndex, key, label, icon);
    if (tabIndex < 0) {
        m_stackedWidget->removeWidget(page);
        return -1;
    }

    if (m_tabBar->currentIndex() == tabIndex || m_stackedWidget->currentIndex() < 0) {
        setCurrentIndex(tabIndex);
    }
    return tabIndex;
}

void TabWidget::removeTab(int index)
{
    QWidget *page = widget(index);
    if (!page) {
        return;
    }

    m_stackedWidget->removeWidget(page);
    m_tabBar->removeTab(index);
    if (m_tabBar->currentIndex() >= 0) {
        m_stackedWidget->setCurrentIndex(m_tabBar->currentIndex());
    }
}

void TabWidget::clear()
{
    while (m_stackedWidget->count() > 0) {
        m_stackedWidget->removeWidget(m_stackedWidget->widget(0));
    }
    m_tabBar->clear();
}

int TabWidget::count() const { return m_stackedWidget->count(); }

int TabWidget::currentIndex() const { return m_stackedWidget->currentIndex(); }

QWidget *TabWidget::currentWidget() const { return m_stackedWidget->currentWidget(); }

QWidget *TabWidget::widget(int index) const { return m_stackedWidget->widget(index); }

int TabWidget::indexOf(QWidget *widget) const { return m_stackedWidget->indexOf(widget); }

QString TabWidget::tabText(int index) const { return m_tabBar->tabText(index); }

void TabWidget::setTabText(int index, const QString &text) { m_tabBar->setTabText(index, text); }

QIcon TabWidget::tabIcon(int index) const { return m_tabBar->tabIcon(index); }

void TabWidget::setTabIcon(int index, const QIcon &icon) { m_tabBar->setTabIcon(index, icon); }

QString TabWidget::routeKey(int index) const
{
    TabItem *item = m_tabBar->tabItem(index);
    return item ? item->routeKey() : QString();
}

TabBar *TabWidget::tabBar() const { return m_tabBar; }

QStackedWidget *TabWidget::stackedWidget() const { return m_stackedWidget; }

bool TabWidget::tabsClosable() const { return m_tabBar->tabsClosable(); }

void TabWidget::setTabsClosable(bool closable) { m_tabBar->setTabsClosable(closable); }

void TabWidget::setAddButtonVisible(bool visible) { m_tabBar->setAddButtonVisible(visible); }

bool TabWidget::isMovable() const { return m_tabBar->isMovable(); }

void TabWidget::setMovable(bool movable) { m_tabBar->setMovable(movable); }

void TabWidget::setCurrentIndex(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }

    m_tabBar->setCurrentIndex(index);
    m_stackedWidget->setCurrentIndex(index);
}

void TabWidget::init()
{
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(0, 0, 0, 0);
    m_layout->setSpacing(1);

    m_tabBar = new TabBar(this);
    m_stackedWidget = new QStackedWidget(this);
    m_layout->addWidget(m_tabBar, 0);
    m_layout->addWidget(m_stackedWidget, 1);

    connect(m_tabBar, &TabBar::currentChanged, this, [this](int index) {
        if (index >= 0 && index < m_stackedWidget->count()) {
            m_stackedWidget->setCurrentIndex(index);
        }
        emit currentChanged(index);
    });
    connect(m_tabBar, &TabBar::tabBarClicked, this, &TabWidget::tabBarClicked);
    connect(m_tabBar, &TabBar::tabBarDoubleClicked, this, &TabWidget::tabBarDoubleClicked);
    connect(m_tabBar, &TabBar::tabCloseRequested, this, &TabWidget::tabCloseRequested);
    connect(m_tabBar, &TabBar::tabAddRequested, this, &TabWidget::tabAddRequested);
    connect(m_tabBar, &TabBar::tabMoved, this, [this](int from, int to) {
        if (from < 0 || to < 0 || from >= count() || to >= count()) {
            return;
        }

        QWidget *page = m_stackedWidget->widget(from);
        if (!page) {
            return;
        }

        m_stackedWidget->removeWidget(page);
        m_stackedWidget->insertWidget(to, page);
        setCurrentIndex(to);
    });

    FluentStyleSheet::setRole(this, QStringLiteral("TabWidget"));
}

QString TabWidget::ensureRouteKey(QWidget *page, const QString &routeKey) const
{
    const QString key = routeKey.isEmpty() ? generatedRouteKey() : routeKey;
    page->setProperty("routeKey", key);
    return key;
}

} // namespace FluentQt
