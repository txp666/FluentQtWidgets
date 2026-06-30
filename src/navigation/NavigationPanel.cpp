#include <FluentQtWidgets/Navigation/NavigationPanel.h>

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Navigation/NavigationTreeWidget.h>
#include <FluentQtWidgets/Widgets/AcrylicLabel.h>
#include <FluentQtWidgets/Widgets/ScrollArea.h>

#include <QtCore/QEvent>
#include <QtCore/QAbstractAnimation>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QSet>
#include <QtCore/QVariantAnimation>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QStyle>
#include <QtWidgets/QVBoxLayout>

namespace FluentQt {

class NavigationIndicator : public QWidget
{
  public:
    explicit NavigationIndicator(QWidget *parent = nullptr) : QWidget(parent)
    {
        resize(3, 16);
        setAttribute(Qt::WA_TransparentForMouseEvents, true);
        setAttribute(Qt::WA_TranslucentBackground, true);
        hide();

        m_animation = new QVariantAnimation(this);
        m_animation->setStartValue(0.0);
        m_animation->setEndValue(1.0);
        m_animation->setDuration(600);
        m_animation->setEasingCurve(QEasingCurve::Linear);
        connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            setGeometry(animatedGeometry(value.toReal()).toRect());
        });
    }

    QVariantAnimation *animation() const { return m_animation; }

    bool isRunning() const { return m_animation && m_animation->state() == QAbstractAnimation::Running; }

    void setIndicatorColor(const QColor &light, const QColor &dark)
    {
        m_lightColor = light;
        m_darkColor = dark;
        update();
    }

    void startAnimation(const QRectF &startRect, const QRectF &endRect)
    {
        if (!startRect.isValid() || !endRect.isValid()) {
            setGeometry(endRect.toRect());
            show();
            return;
        }

        m_startRect = startRect;
        m_endRect = endRect;
        setGeometry(startRect.toRect());
        show();
        raise();

        m_animation->stop();
        m_animation->setDuration(600);
        m_animation->setStartValue(0.0);
        m_animation->setEndValue(1.0);
        m_animation->start();
    }

    void stopAnimation()
    {
        if (m_animation) {
            m_animation->stop();
        }
        hide();
    }

  protected:
    void paintEvent(QPaintEvent *event) override
    {
        Q_UNUSED(event)

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(Qt::NoPen);

        const bool dark = ThemeManager::instance()->effectiveTheme() == Theme::Dark;
        QColor color = dark ? m_darkColor : m_lightColor;
        if (!color.isValid()) {
            color = ThemeManager::instance()->accentColor();
        }
        painter.setBrush(color);
        painter.drawRoundedRect(rect(), 1.5, 1.5);
    }

  private:
    QRectF animatedGeometry(qreal progress) const
    {
        progress = qBound(0.0, progress, 1.0);
        const bool sameLevel = qAbs(m_startRect.x() - m_endRect.x()) < 1.0;
        const qreal dimension = m_startRect.height();
        const qreal distance = qAbs(m_endRect.y() - m_startRect.y());

        if (dimension <= 0 || distance <= 0) {
            return m_endRect;
        }

        if (!sameLevel) {
            const bool nextBelow = m_endRect.y() > m_startRect.y();
            const qreal eased = QEasingCurve(QEasingCurve::OutQuint).valueForProgress(progress);
            const qreal height = dimension * eased;
            const qreal top = nextBelow ? m_endRect.y() : m_endRect.y() + dimension - height;
            return QRectF(m_endRect.x(), top, m_endRect.width(), height);
        }

        const qreal middleLength = distance + dimension;
        const bool forward = m_endRect.y() > m_startRect.y();
        if (progress <= 1.0 / 3.0) {
            const qreal local = progress * 3.0;
            const qreal height = dimension + distance * local;
            const qreal top = forward ? m_startRect.y() : m_startRect.y() + (m_endRect.y() - m_startRect.y()) * local;
            return QRectF(m_startRect.x(), top, m_startRect.width(), height);
        }

        const qreal local = (progress - 1.0 / 3.0) * 1.5;
        const qreal height = middleLength + (dimension - middleLength) * local;
        const qreal top = forward ? m_startRect.y() + (m_endRect.y() - m_startRect.y()) * local : m_endRect.y();
        return QRectF(m_startRect.x(), top, m_startRect.width(), height);
    }

    QVariantAnimation *m_animation = nullptr;
    QRectF m_startRect;
    QRectF m_endRect;
    QColor m_lightColor;
    QColor m_darkColor;
};

NavigationPanel::NavigationPanel(QWidget *parent) : QFrame(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    FluentStyleSheet::setRole(this, QStringLiteral("NavigationPanel"));
    setProperty("menu", false);
    setProperty("transparent", false);
    resize(kCompactWidth, height());

    m_acrylicBrush = new AcrylicBrush(this, 30);
    initLayout();
    m_indicator = new NavigationIndicator(this);
    connect(m_indicator->animation(), &QVariantAnimation::finished, this,
            &NavigationPanel::onIndicatorAnimationFinished);

    connect(m_menuButton, &NavigationToolButton::clicked, this, &NavigationPanel::toggle);
    connect(m_returnButton, &NavigationToolButton::clicked, this, &NavigationPanel::onReturnClicked);

    m_returnButton->hide();
    m_returnButton->setDisabled(true);
    m_returnButton->setToolTip(QStringLiteral("Back"));
    m_menuButton->setToolTip(QStringLiteral("Open Navigation"));

    m_expandAnimation = new QPropertyAnimation(this, "geometry", this);
    m_expandAnimation->setEasingCurve(QEasingCurve::OutQuad);
    m_expandAnimation->setDuration(150);
    connect(m_expandAnimation, &QPropertyAnimation::finished, this, &NavigationPanel::onExpandAnimationFinished);

    if (QWidget *topLevel = window()) {
        topLevel->installEventFilter(this);
    }
}

NavigationPanel::~NavigationPanel()
{
    delete m_acrylicBrush;
    m_acrylicBrush = nullptr;
}

NavigationDisplayMode NavigationPanel::displayMode() const { return m_displayMode; }

bool NavigationPanel::isMenu() const { return m_menu; }

bool NavigationPanel::isAcrylicEnabled() const { return m_acrylicEnabled; }

bool NavigationPanel::isIndicatorAnimationEnabled() const { return m_indicatorAnimationEnabled; }

int NavigationPanel::expandWidth() const { return m_expandWidth; }

int NavigationPanel::minimumExpandWidth() const { return m_minimumExpandWidth; }

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

NavigationToolButton *NavigationPanel::returnButton() const { return m_returnButton; }

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

void NavigationPanel::setAcrylicEnabled(bool enabled)
{
    if (m_acrylicEnabled == enabled) {
        return;
    }

    m_acrylicEnabled = enabled;
    updateTransparentProperty();
    update();
}

void NavigationPanel::setIndicatorAnimationEnabled(bool enabled)
{
    if (m_indicatorAnimationEnabled == enabled) {
        return;
    }

    if (!enabled && m_indicator && m_indicator->isRunning()) {
        m_indicator->stopAnimation();
        onIndicatorAnimationFinished();
    }
    m_indicatorAnimationEnabled = enabled;
}

void NavigationPanel::setExpandWidth(int width)
{
    if (width > kCompactWidth) {
        m_expandWidth = width;
    }
}

void NavigationPanel::setMinimumExpandWidth(int width)
{
    if (width > kCompactWidth) {
        m_minimumExpandWidth = width;
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

    stopIndicatorAnimation();

    NavigationWidget *prevItem = navigationWidget(m_currentRouteKey);
    m_currentRouteKey = routeKey;
    NavigationWidget *newItem = navigationWidget(routeKey);
    NavigationWidget *prevIndicatorItem = findIndicatorWidget(prevItem);
    NavigationWidget *newIndicatorItem = findIndicatorWidget(newItem);

    if (!m_indicatorAnimationEnabled || !prevItem || !prevIndicatorItem || !newIndicatorItem) {
        for (auto it = m_items.constBegin(); it != m_items.constEnd(); ++it) {
            it.value().widget->setAboutSelected(false);
            it.value().widget->setSelected(it.key() == routeKey);
        }
        if (NavigationWidget *indicatorItem = findIndicatorWidget(newItem)) {
            indicatorItem->update();
        }
        return;
    }

    const QRectF startRect = indicatorRectFor(prevIndicatorItem);
    const QRectF endRect = indicatorRectFor(newIndicatorItem);
    if (!startRect.isValid() || !endRect.isValid()) {
        for (auto it = m_items.constBegin(); it != m_items.constEnd(); ++it) {
            it.value().widget->setAboutSelected(false);
            it.value().widget->setSelected(it.key() == routeKey);
        }
        if (NavigationWidget *indicatorItem = findIndicatorWidget(newItem)) {
            indicatorItem->update();
        }
        return;
    }

    prevItem->setSelected(false);
    prevIndicatorItem->setSelected(false);
    newIndicatorItem->setAboutSelected(true);
    m_indicator->setIndicatorColor(ThemeManager::instance()->accentColor(), ThemeManager::instance()->accentColor());
    m_indicator->raise();
    m_indicator->startAnimation(startRect, endRect);
}

void NavigationPanel::removeWidget(const QString &routeKey)
{
    if (!m_items.contains(routeKey)) {
        return;
    }

    stopIndicatorAnimation();

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
    stopIndicatorAnimation();

    const int expandThreshold = m_minimumExpandWidth + m_expandWidth - kExpandWidth;
    const bool useExpandMode = window() && window()->width() >= expandThreshold;

    setDisplayMode(useExpandMode ? NavigationDisplayMode::Expand : NavigationDisplayMode::Menu);
    setMenu(!useExpandMode);
    setItemsCompacted(false);
    m_expandAnimationExpanding = true;

    if (animated) {
        emit displayModeChanged(m_displayMode);
        m_expandAnimation->stop();
        m_expandAnimation->setStartValue(QRect(pos(), QSize(kCompactWidth, height())));
        m_expandAnimation->setEndValue(QRect(x(), y(), m_expandWidth, height()));
        if (m_displayMode == NavigationDisplayMode::Menu && canDrawAcrylic()) {
            m_acrylicBrush->grabImage(QRect(mapToGlobal(QPoint(0, 0)), QSize(m_expandWidth, height())));
        }
        m_expandAnimation->start();
    } else {
        updatePanelWidth(m_expandWidth);
        emit displayModeChanged(m_displayMode);
    }

    m_menuButton->setToolTip(QStringLiteral("Close Navigation"));
}

void NavigationPanel::collapse(bool animated)
{
    if (m_expandAnimation->state() == QPropertyAnimation::Running) {
        return;
    }

    stopIndicatorAnimation();

    m_expandAnimationExpanding = false;

    if (animated) {
        m_expandAnimation->setStartValue(geometry());
        m_expandAnimation->setEndValue(QRect(x(), y(), kCompactWidth, height()));
        m_expandAnimation->start();
    } else {
        setDisplayMode(NavigationDisplayMode::Compact);
        setMenu(false);
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
    if (!m_expandAnimationExpanding) {
        setDisplayMode(NavigationDisplayMode::Compact);
        setMenu(false);
        setItemsCompacted(true);
        updatePanelWidth(kCompactWidth);
        emit displayModeChanged(m_displayMode);
    } else {
        updatePanelWidth(m_expandWidth);
    }
}

bool NavigationPanel::eventFilter(QObject *watched, QEvent *event)
{
    auto *topLevel = qobject_cast<QWidget *>(watched);
    if (!topLevel || topLevel != window()) {
        return QFrame::eventFilter(watched, event);
    }

    if (event->type() == QEvent::MouseButtonRelease && m_displayMode == NavigationDisplayMode::Menu) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        const QPoint pos = mouseEvent->position().toPoint();
#else
        const QPoint pos = mouseEvent->pos();
#endif
        const QRect panelRect(mapTo(topLevel, QPoint(0, 0)), size());
        if (!panelRect.contains(pos)) {
            collapse();
        }
    } else if (event->type() == QEvent::Resize) {
        auto *resizeEvent = static_cast<QResizeEvent *>(event);
        if (resizeEvent->size().width() < m_minimumExpandWidth && m_displayMode == NavigationDisplayMode::Expand) {
            collapse();
        } else if (resizeEvent->size().width() >= m_minimumExpandWidth
                   && m_displayMode == NavigationDisplayMode::Compact && !m_menuButtonVisible) {
            expand();
        }
    }

    return QFrame::eventFilter(watched, event);
}

void NavigationPanel::paintEvent(QPaintEvent *event)
{
    if (!canDrawAcrylic() || m_displayMode != NavigationDisplayMode::Menu) {
        QFrame::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(QRectF(0, 1, width() - 1, height() - 1), 7, 7);
    path.addRect(QRectF(0, 8, 8, qMax(0, height() - 16)));

    updateAcrylicColor();
    m_acrylicBrush->setClipPath(path);
    m_acrylicBrush->paint(&painter);

    QFrame::paintEvent(event);
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

    m_returnButton = new NavigationToolButton(icon(FluentIcon::Return), this);
    m_menuButton = new NavigationToolButton(icon(FluentIcon::Menu), this);

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
    if (m_displayMode == mode) {
        return;
    }

    m_displayMode = mode;
    updateTransparentProperty();
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
    resize(width, height());
}

void NavigationPanel::updateTransparentProperty()
{
    const bool transparent = canDrawAcrylic() && m_displayMode == NavigationDisplayMode::Menu;
    if (property("transparent").toBool() == transparent) {
        return;
    }

    setProperty("transparent", transparent);
    if (style()) {
        style()->unpolish(this);
        style()->polish(this);
    }
}

void NavigationPanel::updateAcrylicColor()
{
    if (!m_acrylicBrush) {
        return;
    }

    if (ThemeManager::instance()->effectiveTheme() == Theme::Dark) {
        m_acrylicBrush->setTintColor(QColor(32, 32, 32, 200));
        m_acrylicBrush->setLuminosityColor(QColor(0, 0, 0, 0));
    } else {
        m_acrylicBrush->setTintColor(QColor(255, 255, 255, 180));
        m_acrylicBrush->setLuminosityColor(QColor(255, 255, 255, 0));
    }
}

bool NavigationPanel::canDrawAcrylic() const
{
    return m_acrylicEnabled && m_acrylicBrush && m_acrylicBrush->isAvailable();
}

void NavigationPanel::stopIndicatorAnimation()
{
    if (!m_indicatorAnimationEnabled || !m_indicator || !m_indicator->isRunning()) {
        return;
    }

    m_indicator->stopAnimation();
    onIndicatorAnimationFinished();
}

NavigationWidget *NavigationPanel::findIndicatorWidget(NavigationWidget *item) const
{
    auto *candidate = item;
    while (candidate && !candidate->isVisible()) {
        candidate = qobject_cast<NavigationWidget *>(candidate->parentWidget());
    }

    return candidate;
}

QRectF NavigationPanel::indicatorRectFor(NavigationWidget *item) const
{
    if (!item || !item->isVisible()) {
        return {};
    }

    return item->indicatorRect().translated(item->mapTo(const_cast<NavigationPanel *>(this), QPoint(0, 0)));
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
    if (m_displayMode == NavigationDisplayMode::Menu && item->isSelectable()) {
        collapse();
    }
}

void NavigationPanel::onReturnClicked() { emit returnRequested(); }

void NavigationPanel::onIndicatorAnimationFinished()
{
    NavigationWidget *item = navigationWidget(m_currentRouteKey);
    if (!item) {
        if (m_indicator) {
            m_indicator->hide();
        }
        return;
    }

    item->setSelected(true);
    if (NavigationWidget *indicatorItem = findIndicatorWidget(item)) {
        indicatorItem->setAboutSelected(false);
    }
    if (m_indicator) {
        m_indicator->hide();
    }
}

} // namespace FluentQt
