#include <FluentQtWidgets/Navigation/NavigationInterface.h>

#include <FluentQtWidgets/Navigation/NavigationPanel.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Widgets/StackedWidget.h>

#include <QtCore/QEvent>
#include <QtCore/QSignalBlocker>
#include <QtCore/QUuid>
#include <QtWidgets/QAbstractScrollArea>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

namespace FluentQt {

NavigationInterface::NavigationInterface(QWidget *parent, bool showReturnButton)
    : QFrame(parent)
{
    setAttribute(Qt::WA_TranslucentBackground, true);
    FluentStyleSheet::setRole(this, QStringLiteral("NavigationInterface"));

    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_navigationSpacer = new QWidget(this);
    m_navigationSpacer->setAttribute(Qt::WA_StyledBackground, true);
    FluentStyleSheet::setRole(m_navigationSpacer, QStringLiteral("NavigationSpacer"));
    m_navigationSpacer->setFixedWidth(NavigationPanel::kCompactWidth);
    layout->addWidget(m_navigationSpacer);

    m_contentWidget = new QWidget(this);
    m_contentWidget->setAttribute(Qt::WA_TranslucentBackground, true);
    m_contentLayout = new QVBoxLayout(m_contentWidget);
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);

    m_stackedWidget = new PopUpAniStackedWidget(m_contentWidget);
    m_stackedWidget->setObjectName(QStringLiteral("NavigationStack"));
    m_stackedWidget->setAttribute(Qt::WA_StyledBackground, true);
    FluentStyleSheet::setRole(m_stackedWidget, QStringLiteral("NavigationStack"));
    m_contentLayout->addWidget(m_stackedWidget);
    layout->addWidget(m_contentWidget, 1);

    m_panel = new NavigationPanel(this);
    m_panel->setReturnButtonVisible(showReturnButton);
    m_panel->installEventFilter(this);
    updatePanelGeometry();
    ensureNavigationPanelOnTop();

    connect(m_panel, &NavigationPanel::itemClicked, this, &NavigationInterface::onPanelItemClicked);
    connect(m_panel, &NavigationPanel::displayModeChanged, this, [this](NavigationDisplayMode) {
        updateNavigationSpacerWidth();
        ensureNavigationPanelOnTop();
    });
    connect(m_stackedWidget, &PopUpAniStackedWidget::aniStart, this,
            &NavigationInterface::ensureNavigationPanelOnTop);
    connect(m_stackedWidget, &QStackedWidget::currentChanged, this,
            &NavigationInterface::ensureNavigationPanelOnTop);
}

int NavigationInterface::addPage(QWidget *page, const QString &title, const QIcon &icon)
{
    return addPage(page, title, icon, QString());
}

int NavigationInterface::addPage(QWidget *page, const QString &title, const QIcon &icon, const QString &routeKey)
{
    return addPage(page, title, icon, routeKey, NavigationItemPosition::Scroll);
}

int NavigationInterface::addPage(QWidget *page, const QString &title, const QIcon &icon, const QString &routeKey,
                                 NavigationItemPosition position)
{
    return addPage(page, title, icon, routeKey, position, QString());
}

int NavigationInterface::addPage(QWidget *page, const QString &title, const QIcon &icon, const QString &routeKey,
                                 NavigationItemPosition position, const QString &parentRouteKey)
{
    if (!page) {
        return -1;
    }

    const QString key = ensureRouteKey(page, routeKey);
    if (key.isEmpty() || contains(key)) {
        return -1;
    }

    const int index = m_stackedWidget->addWidget(page);
    page->setProperty("routeKey", key);
    page->setObjectName(key);

    m_panel->addItem(key, icon, title, position, parentRouteKey);
    m_routeOrder.append(key);

    if (index == 0) {
        setCurrentIndex(0);
    }

    ensureNavigationPanelOnTop();
    return index;
}

NavigationTreeWidget *NavigationInterface::addItem(const QString &routeKey, const QIcon &icon, const QString &text,
                                                   NavigationItemPosition position, const QString &parentRouteKey,
                                                   bool selectable)
{
    return m_panel->addItem(routeKey, icon, text, position, parentRouteKey, selectable);
}

NavigationWidget *NavigationInterface::addWidget(const QString &routeKey, NavigationWidget *widget,
                                                 NavigationItemPosition position, const QString &parentRouteKey)
{
    return m_panel->addWidget(routeKey, widget, position, parentRouteKey);
}

NavigationItemHeader *NavigationInterface::addItemHeader(const QString &text, NavigationItemPosition position)
{
    return m_panel->addItemHeader(text, position);
}

NavigationUserCard *NavigationInterface::addUserCard(const QString &routeKey, const QIcon &avatar,
                                                     const QString &title, const QString &subtitle,
                                                     NavigationItemPosition position)
{
    return m_panel->addUserCard(routeKey, avatar, title, subtitle, position);
}

NavigationUserCard *NavigationInterface::addUserCard(const QString &routeKey, const QString &avatarPath,
                                                     const QString &title, const QString &subtitle,
                                                     NavigationItemPosition position)
{
    return m_panel->addUserCard(routeKey, avatarPath, title, subtitle, position);
}

QFrame *NavigationInterface::addSeparator(NavigationItemPosition position) { return m_panel->addSeparator(position); }

bool NavigationInterface::removePage(int index, bool deleteWidget)
{
    if (index < 0 || index >= count()) {
        return false;
    }

    const int previousIndex = currentIndex();
    const QString previousRouteKey = currentRouteKey();
    QWidget *page = widget(index);
    if (!page) {
        return false;
    }

    const QString key = routeKey(index);
    m_panel->removeWidget(key);
    m_routeOrder.removeAt(index);

    m_stackedWidget->removeWidget(page);
    if (deleteWidget) {
        page->deleteLater();
    } else {
        page->setParent(nullptr);
    }

    if (count() == 0) {
        if (previousIndex != -1) {
            emit currentIndexChanged(-1);
        }
        if (!previousRouteKey.isEmpty()) {
            emit currentRouteKeyChanged(QString());
        }
        return true;
    }

    int nextIndex = previousIndex;
    if (index < previousIndex) {
        nextIndex = previousIndex - 1;
    } else if (index == previousIndex) {
        nextIndex = qMin(index, count() - 1);
    }

    m_stackedWidget->setCurrentIndex(nextIndex);
    m_panel->setCurrentItem(routeKey(nextIndex));

    const QString nextRouteKey = routeKey(nextIndex);
    if (previousIndex != nextIndex) {
        emit currentIndexChanged(nextIndex);
    }
    if (previousRouteKey != nextRouteKey) {
        emit currentRouteKeyChanged(nextRouteKey);
    }
    return true;
}

bool NavigationInterface::removePage(const QString &routeKey, bool deleteWidget)
{
    return removePage(indexOf(routeKey), deleteWidget);
}

int NavigationInterface::count() const { return m_stackedWidget->count(); }

int NavigationInterface::currentIndex() const { return m_stackedWidget->currentIndex(); }

QString NavigationInterface::currentRouteKey() const { return routeKey(currentIndex()); }

QWidget *NavigationInterface::currentWidget() const { return m_stackedWidget->currentWidget(); }

QWidget *NavigationInterface::widget(int index) const
{
    return index >= 0 && index < count() ? m_stackedWidget->widget(index) : nullptr;
}

QWidget *NavigationInterface::widget(const QString &routeKey) const { return widget(indexOf(routeKey)); }

int NavigationInterface::indexOf(QWidget *page) const { return m_stackedWidget->indexOf(page); }

int NavigationInterface::indexOf(const QString &routeKey) const { return m_routeOrder.indexOf(routeKey); }

QString NavigationInterface::routeKey(int index) const
{
    return index >= 0 && index < m_routeOrder.size() ? m_routeOrder.at(index) : QString();
}

QList<QString> NavigationInterface::routeKeys() const { return m_routeOrder; }

bool NavigationInterface::contains(const QString &routeKey) const { return indexOf(routeKey) >= 0; }

QStackedWidget *NavigationInterface::stackedWidget() const { return m_stackedWidget; }

NavigationPanel *NavigationInterface::navigationPanel() const { return m_panel; }

int NavigationInterface::contentTopMargin() const { return m_contentTopMargin; }

bool NavigationInterface::isAcrylicEnabled() const { return m_panel && m_panel->isAcrylicEnabled(); }

void NavigationInterface::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_stackedWidget->count()) {
        return;
    }

    const bool changed = index != m_stackedWidget->currentIndex();
    if (changed) {
        if (auto *scrollArea = qobject_cast<QAbstractScrollArea *>(m_stackedWidget->widget(index))) {
            scrollArea->verticalScrollBar()->setValue(0);
        }
        m_stackedWidget->setCurrentIndex(index, false, true, 300, QEasingCurve::OutQuad);
    }

    const QString key = routeKey(index);
    if (m_panel->currentRouteKey() != key) {
        m_panel->setCurrentItem(key);
    }

    if (changed) {
        emit currentIndexChanged(index);
        emit currentRouteKeyChanged(key);
    }
    ensureNavigationPanelOnTop();
}

void NavigationInterface::setCurrentRouteKey(const QString &routeKey) { setCurrentIndex(indexOf(routeKey)); }

void NavigationInterface::setContentTopMargin(int margin)
{
    m_contentTopMargin = qMax(0, margin);
    if (m_contentLayout) {
        m_contentLayout->setContentsMargins(0, m_contentTopMargin, 0, 0);
    }
}

void NavigationInterface::setAcrylicEnabled(bool enabled)
{
    if (m_panel) {
        m_panel->setAcrylicEnabled(enabled);
    }
}

void NavigationInterface::setMinimumExpandWidth(int width)
{
    if (m_panel) {
        m_panel->setMinimumExpandWidth(width);
    }
}

bool NavigationInterface::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_panel && event->type() == QEvent::Resize) {
        updateNavigationSpacerWidth();
    }
    return QFrame::eventFilter(watched, event);
}

void NavigationInterface::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    updatePanelGeometry();
    updateNavigationSpacerWidth();
    ensureNavigationPanelOnTop();
}

QString NavigationInterface::ensureRouteKey(QWidget *page, const QString &routeKey) const
{
    if (!routeKey.isEmpty()) {
        return routeKey;
    }

    if (page && !page->objectName().isEmpty()) {
        return page->objectName();
    }

    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void NavigationInterface::onPanelItemClicked(const QString &routeKey)
{
    const int index = indexOf(routeKey);
    if (index < 0) {
        emit navigationItemClicked(-1, routeKey);
        return;
    }

    setCurrentIndex(index);
    emit navigationItemClicked(index, routeKey);
}

void NavigationInterface::ensureNavigationPanelOnTop()
{
    if (m_panel) {
        m_panel->raise();
    }
}

void NavigationInterface::updatePanelGeometry()
{
    if (!m_panel) {
        return;
    }

    m_panel->setFixedHeight(height());
    m_panel->move(0, 0);
    if (m_panel->width() <= 0) {
        m_panel->resize(NavigationPanel::kCompactWidth, height());
    }
}

void NavigationInterface::updateNavigationSpacerWidth()
{
    if (!m_panel || !m_navigationSpacer) {
        return;
    }

    const int width = m_panel->displayMode() == NavigationDisplayMode::Expand
                          ? qMax(m_panel->width(), NavigationPanel::kCompactWidth)
                          : NavigationPanel::kCompactWidth;
    if (m_navigationSpacer->width() != width) {
        m_navigationSpacer->setFixedWidth(width);
        m_navigationSpacer->updateGeometry();
        if (layout()) {
            layout()->activate();
        }
    }
}

} // namespace FluentQt
