#include <FluentQtWidgets/Widgets/Menu.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/AcrylicLabel.h>
#include <FluentQtWidgets/Widgets/SmoothScrollDelegate.h>

#include <QtCore/QEasingCurve>
#include <QtCore/QParallelAnimationGroup>
#include <QtCore/QPointer>
#include <QtCore/QPropertyAnimation>
#include <QtCore/QTimer>
#include <QtGui/QActionEvent>
#include <QtGui/QClipboard>
#include <QtGui/QCloseEvent>
#include <QtGui/QGuiApplication>
#include <QtGui/QKeySequence>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QScreen>
#include <QtGui/QShowEvent>
#include <QtGui/QHideEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QRegion>
#include <QtGui/QTextLayout>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QtGui/QEnterEvent>
#endif
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLabel>
#include <QtWidgets/QWidgetAction>

namespace FluentQt {

namespace {

bool isDarkTheme()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark;
}

QColor themeColor()
{
    return ThemeManager::instance()->accentColor();
}

QColor acrylicTintColor()
{
    return isDarkTheme() ? QColor(32, 32, 32, 200) : QColor(255, 255, 255, 160);
}

QColor acrylicLuminosityColor()
{
    return isDarkTheme() ? QColor(0, 0, 0, 0) : QColor(255, 255, 255, 50);
}

QRect screenGeometryAt(const QPoint &pos)
{
    QScreen *screen = QGuiApplication::screenAt(pos);
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (!screen) {
        return QRect();
    }
    return screen->availableGeometry();
}

// ==========================================
// MenuAnimationManager hierarchy
// ==========================================

class MenuAnimationManager : public QObject
{
public:
    explicit MenuAnimationManager(RoundMenu *menu, QObject *parent = nullptr)
        : QObject(parent), m_menu(menu)
    {
        m_positionAni = new QPropertyAnimation(menu, "pos", this);
        m_positionAni->setDuration(250);
        m_positionAni->setEasingCurve(QEasingCurve::OutQuad);

        m_opacityAni = new QPropertyAnimation(menu, "windowOpacity", this);
        m_opacityAni->setDuration(150);
        m_opacityAni->setEasingCurve(QEasingCurve::OutQuad);

        m_group = new QParallelAnimationGroup(this);
        m_group->addAnimation(m_positionAni);
    }

    virtual QSize availableViewSize(const QPoint &pos) const
    {
        const QRect ss = screenGeometryAt(pos);
        return QSize(ss.width() - 100, ss.height() - 100);
    }

    virtual QPoint endPosition(const QPoint &pos) const
    {
        if (!isMenuUsable()) {
            return pos;
        }
        const QRect rect = screenGeometryAt(pos);
        const int w = m_menu->width() + 5;
        const int h = m_menu->height();
        const int x = qMin(pos.x() - m_menu->layout()->contentsMargins().left(), rect.right() - w);
        const int y = qMin(pos.y() - 4, rect.bottom() - h + 10);
        return QPoint(x, y);
    }

    virtual QSize menuSize() const
    {
        if (!isMenuUsable()) {
            return QSize();
        }
        const QMargins m = m_menu->layout()->contentsMargins();
        const int w = m_menu->view()->width() + m.left() + m.right() + 120;
        const int h = m_menu->view()->height() + m.top() + m.bottom() + 20;
        return QSize(qMax(1, w), qMax(1, h));
    }

    virtual void exec(const QPoint &pos) = 0;

protected:
    bool isMenuUsable() const
    {
        return m_menu && m_menu->layout() && m_menu->view();
    }

    void setSafeMask(int x, int y, int width, int height)
    {
        if (!m_menu || width <= 0 || height <= 0) {
            return;
        }
        m_menu->setMask(QRegion(x, qMax(0, y), qMax(1, width), qMax(1, height)));
    }

    void clearSafeMask()
    {
        if (m_menu) {
            m_menu->clearMask();
        }
    }

    QPointer<RoundMenu> m_menu;
    QPropertyAnimation *m_positionAni = nullptr;
    QPropertyAnimation *m_opacityAni = nullptr;
    QParallelAnimationGroup *m_group = nullptr;
};

class DropDownMenuAnimationManager : public MenuAnimationManager
{
public:
    using MenuAnimationManager::MenuAnimationManager;

    QSize availableViewSize(const QPoint &pos) const override
    {
        const QRect ss = screenGeometryAt(pos);
        return QSize(ss.width() - 100, qMax(ss.bottom() - pos.y() - 10, 1));
    }

    void exec(const QPoint &pos) override
    {
        if (!isMenuUsable()) {
            return;
        }
        const QPoint end = endPosition(pos);
        const int h = m_menu->height() + 5;
        m_positionAni->setStartValue(end - QPoint(0, h / 2));
        m_positionAni->setEndValue(end);
        m_menu->setWindowOpacity(1.0);

        connect(m_positionAni, &QPropertyAnimation::valueChanged, this, [this]() {
            const QSize s = menuSize();
            const int y = m_positionAni->endValue().toPoint().y() - m_positionAni->currentValue().toPoint().y();
            setSafeMask(0, y, s.width(), s.height());
        });
        connect(m_positionAni, &QPropertyAnimation::finished, this, [this]() {
            clearSafeMask();
            deleteLater();
        });
        m_positionAni->start();
    }
};

class PullUpMenuAnimationManager : public MenuAnimationManager
{
public:
    using MenuAnimationManager::MenuAnimationManager;

    QPoint endPosition(const QPoint &pos) const override
    {
        if (!isMenuUsable()) {
            return pos;
        }
        const QRect rect = screenGeometryAt(pos);
        const int w = m_menu->width() + 5;
        const int h = m_menu->height();
        const int x = qMin(pos.x() - m_menu->layout()->contentsMargins().left(), rect.right() - w);
        const int y = qMax(pos.y() - h + 13, rect.top() + 4);
        return QPoint(x, y);
    }

    QSize availableViewSize(const QPoint &pos) const override
    {
        const QRect ss = screenGeometryAt(pos);
        return QSize(ss.width() - 100, qMax(pos.y() - ss.top() - 28, 1));
    }

    void exec(const QPoint &pos) override
    {
        if (!isMenuUsable()) {
            return;
        }
        const QPoint end = endPosition(pos);
        const int h = m_menu->height() + 5;
        m_positionAni->setStartValue(end + QPoint(0, h / 2));
        m_positionAni->setEndValue(end);
        m_menu->setWindowOpacity(1.0);

        connect(m_positionAni, &QPropertyAnimation::valueChanged, this, [this]() {
            const QSize s = menuSize();
            const int y = m_positionAni->endValue().toPoint().y() - m_positionAni->currentValue().toPoint().y();
            setSafeMask(0, y, s.width(), s.height() - 28);
        });
        connect(m_positionAni, &QPropertyAnimation::finished, this, [this]() {
            clearSafeMask();
            deleteLater();
        });
        m_positionAni->start();
    }
};

class FadeInDropDownMenuAnimationManager : public DropDownMenuAnimationManager
{
public:
    FadeInDropDownMenuAnimationManager(RoundMenu *menu, QObject *parent = nullptr)
        : DropDownMenuAnimationManager(menu, parent)
    {
        m_group->addAnimation(m_opacityAni);
    }

    void exec(const QPoint &pos) override
    {
        if (!isMenuUsable()) {
            return;
        }
        const QPoint end = endPosition(pos);
        m_positionAni->setStartValue(end - QPoint(0, 8));
        m_positionAni->setEndValue(end);
        m_positionAni->setDuration(150);
        m_menu->setWindowOpacity(0.0);
        m_opacityAni->setStartValue(0.0);
        m_opacityAni->setEndValue(1.0);

        connect(m_positionAni, &QPropertyAnimation::valueChanged, this, [this]() {
            const QSize s = menuSize();
            setSafeMask(0, 0, s.width(), s.height());
        });
        connect(m_positionAni, &QPropertyAnimation::finished, this, [this]() {
            clearSafeMask();
        });
        connect(m_group, &QParallelAnimationGroup::finished, this, [this]() {
            clearSafeMask();
            deleteLater();
        });
        m_group->start();
    }
};

class FadeInPullUpMenuAnimationManager : public PullUpMenuAnimationManager
{
public:
    FadeInPullUpMenuAnimationManager(RoundMenu *menu, QObject *parent = nullptr)
        : PullUpMenuAnimationManager(menu, parent)
    {
        m_group->addAnimation(m_opacityAni);
    }

    void exec(const QPoint &pos) override
    {
        if (!isMenuUsable()) {
            return;
        }
        const QPoint end = endPosition(pos);
        m_positionAni->setStartValue(end + QPoint(0, 8));
        m_positionAni->setEndValue(end);
        m_positionAni->setDuration(200);
        m_menu->setWindowOpacity(0.0);
        m_opacityAni->setStartValue(0.0);
        m_opacityAni->setEndValue(1.0);

        connect(m_positionAni, &QPropertyAnimation::valueChanged, this, [this]() {
            const QSize s = menuSize();
            setSafeMask(0, 0, s.width(), s.height());
        });
        connect(m_positionAni, &QPropertyAnimation::finished, this, [this]() {
            clearSafeMask();
        });
        connect(m_group, &QParallelAnimationGroup::finished, this, [this]() {
            clearSafeMask();
            deleteLater();
        });
        m_group->start();
    }
};

MenuAnimationManager *makeMenuAnimationManager(RoundMenu *menu, MenuAnimationType type)
{
    switch (type) {
    case MenuAnimationType::DropDown:
        return new DropDownMenuAnimationManager(menu, menu);
    case MenuAnimationType::PullUp:
        return new PullUpMenuAnimationManager(menu, menu);
    case MenuAnimationType::FadeInDropDown:
        return new FadeInDropDownMenuAnimationManager(menu, menu);
    case MenuAnimationType::FadeInPullUp:
        return new FadeInPullUpMenuAnimationManager(menu, menu);
    case MenuAnimationType::None:
    default:
        return nullptr;
    }
}

} // namespace

// ==========================================
// SubMenuItemWidget
// ==========================================

SubMenuItemWidget::SubMenuItemWidget(RoundMenu *menu, QListWidgetItem *item, QWidget *parent)
    : QWidget(parent), m_menu(menu), m_item(item)
{
}

RoundMenu *SubMenuItemWidget::menu() const { return m_menu; }

QListWidgetItem *SubMenuItemWidget::item() const { return m_item; }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
void SubMenuItemWidget::enterEvent(QEnterEvent *event)
#else
void SubMenuItemWidget::enterEvent(QEvent *event)
#endif
{
    QWidget::enterEvent(event);
    emit showMenuSignal(m_item);
}

void SubMenuItemWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing);

    // Draw right arrow using FluentIcon
    const int arrowSize = 9;
    const qreal x = width() - 14;
    const qreal y = height() / 2.0 - arrowSize / 2.0;

    const Theme theme = isDarkTheme() ? Theme::Dark : Theme::Light;
    QColor arrowColor = isDarkTheme() ? QColor(255, 255, 255, 150) : QColor(0, 0, 0, 130);
    painter.setOpacity(arrowColor.alphaF());

    FluentQt::icon(FluentIcon::RightArrow, theme, arrowColor)
        .paint(&painter, QRect(static_cast<int>(x), static_cast<int>(y), arrowSize, arrowSize));
}

// ==========================================
// MenuActionListWidget
// ==========================================

MenuActionListWidget::MenuActionListWidget(QWidget *parent) : QListWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);
    setViewportMargins(0, 6, 0, 6);
    setTextElideMode(Qt::ElideNone);
    setDragEnabled(false);
    setMouseTracking(true);
    setIconSize(QSize(14, 14));
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setItemDelegate(new ShortcutMenuItemDelegate(this));
    new SmoothScrollDelegate(this);
    FluentStyleSheet::setRole(this, QStringLiteral("MenuActionListWidget"));
}

void MenuActionListWidget::setItemHeight(int height)
{
    if (height == m_itemHeight) {
        return;
    }
    m_itemHeight = height;
    for (int i = 0; i < count(); ++i) {
        QListWidgetItem *item = this->item(i);
        if (!this->itemWidget(item)) {
            item->setSizeHint(QSize(item->sizeHint().width(), height));
        }
    }
    adjustItemsSize();
}

int MenuActionListWidget::itemHeight() const { return m_itemHeight; }

void MenuActionListWidget::setMaxVisibleItems(int num)
{
    m_maxVisibleItems = num;
    adjustItemsSize();
}

int MenuActionListWidget::maxVisibleItems() const { return m_maxVisibleItems; }

void MenuActionListWidget::setViewportPadding(int left, int top, int right, int bottom)
{
    setViewportMargins(left, top, right, bottom);
    adjustItemsSize();
}

QMargins MenuActionListWidget::viewportPadding() const { return viewportMargins(); }

int MenuActionListWidget::itemsHeight() const
{
    const int N = (m_maxVisibleItems < 0) ? count() : qMin(m_maxVisibleItems, count());
    int h = 0;
    for (int i = 0; i < N; ++i) {
        h += item(i)->sizeHint().height();
    }
    const QMargins m = viewportMargins();
    return h + m.top() + m.bottom();
}

int MenuActionListWidget::heightForAnimation(const QPoint &pos, MenuAnimationType aniType) const
{
    const int ih = itemsHeight();
    return qMin(ih, availableViewSize(pos, aniType).height());
}

QSize MenuActionListWidget::availableViewSize(const QPoint &pos, MenuAnimationType aniType) const
{
    // Create temporary manager just for size calculation
    const QRect ss = screenGeometryAt(pos);
    int w = ss.width() - 100;
    int h = ss.height() - 100;

    switch (aniType) {
    case MenuAnimationType::DropDown:
    case MenuAnimationType::FadeInDropDown:
        h = qMax(ss.bottom() - pos.y() - 10, 1);
        break;
    case MenuAnimationType::PullUp:
    case MenuAnimationType::FadeInPullUp:
        h = qMax(pos.y() - ss.top() - 28, 1);
        break;
    default:
        break;
    }

    return QSize(w, h);
}

void MenuActionListWidget::adjustItemsSize(const QPoint &pos, MenuAnimationType aniType)
{
    QSize size;
    for (int i = 0; i < count(); ++i) {
        const QSize s = item(i)->sizeHint();
        size.setWidth(qMax(s.width(), size.width()));
        size.setHeight(qMax(1, size.height() + s.height()));
    }

    const QMargins m = viewportMargins();
    const QSize avail = availableViewSize(pos, aniType);
    const int w = m.left() + m.right() + 2;
    const int h = m.top() + m.bottom();
    size += QSize(w, h);
    size.setHeight(qMin(avail.height(), size.height() + 3));
    size.setWidth(qMax(qMin(avail.width(), size.width()), minimumWidth()));

    if (m_maxVisibleItems > 0) {
        size.setHeight(qMin(size.height(), m_maxVisibleItems * m_itemHeight + h + 3));
    }

    setFixedSize(size);
}

void MenuActionListWidget::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QListWidget::rowsInserted(parent, start, end);
    adjustItemsSize();
}

#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
void MenuActionListWidget::rowsRemoved(const QModelIndex &parent, int start, int end)
{
    QListWidget::rowsRemoved(parent, start, end);
    adjustItemsSize();
}
#else
void MenuActionListWidget::rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    QListWidget::rowsAboutToBeRemoved(parent, first, last);
    adjustItemsSize();
}
#endif

// ==========================================
// MenuItemDelegate
// ==========================================

MenuItemDelegate::MenuItemDelegate(QObject *parent) : QStyledItemDelegate(parent) {}

bool MenuItemDelegate::isSeparator(const QModelIndex &index) const
{
    return index.data(Qt::DecorationRole).toString() == QStringLiteral("seperator");
}

void MenuItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    if (isSeparator(index)) {
        painter->save();
        const int c = isDarkTheme() ? 255 : 0;
        QPen pen(QColor(c, c, c, 25), 1);
        pen.setCosmetic(true);
        painter->setPen(pen);
        const QRect rect = option.rect;
        painter->drawLine(0, rect.y() + 4, rect.width() + 12, rect.y() + 4);
        painter->restore();
        return;
    }

    QStyledItemDelegate::paint(painter, option, index);
}

// ==========================================
// ShortcutMenuItemDelegate
// ==========================================

void ShortcutMenuItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
    MenuItemDelegate::paint(painter, option, index);
    if (isSeparator(index)) {
        return;
    }

    // Draw shortcut key
    QAction *action = index.data(Qt::UserRole).value<QAction *>();
    if (!action || action->shortcut().isEmpty()) {
        return;
    }

    painter->save();

    if (!(option.state & QStyle::State_Enabled)) {
        painter->setOpacity(isDarkTheme() ? 0.5 : 0.6);
    }

    QFont font = painter->font();
    font.setPixelSize(12);
    painter->setFont(font);
    painter->setPen(isDarkTheme() ? QColor(255, 255, 255, 200) : QColor(0, 0, 0, 153));

    const QString shortcut = action->shortcut().toString(QKeySequence::NativeText);

    // Use QTextLayout for accurate measurement
    QTextLayout layout(shortcut, font);
    layout.beginLayout();
    QTextLine line = layout.createLine();
    line.setNumColumns(1);
    layout.endLayout();

    const QRectF bounds = layout.boundingRect();
    const qreal y = option.rect.y() + (option.rect.height() - bounds.height()) / 2.0;
    const qreal x = option.rect.right() - bounds.width() - 20.0;
    layout.draw(painter, QPointF(x, y));

    painter->restore();
}

// ==========================================
// RadioIndicatorMenuItemDelegate
// ==========================================

void RadioIndicatorMenuItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    if (isSeparator(index)) {
        ShortcutMenuItemDelegate::paint(painter, option, index);
        return;
    }

    ShortcutMenuItemDelegate::paint(painter, option, index);

    QAction *action = index.data(Qt::UserRole).value<QAction *>();
    if (!action || !action->isChecked()) {
        return;
    }

    painter->save();
    drawIndicator(painter, option, index);
    painter->restore();
}

void RadioIndicatorMenuItemDelegate::drawIndicator(QPainter *painter, const QStyleOptionViewItem &option,
                                                    const QModelIndex &index) const
{
    Q_UNUSED(index)
    const QRect rect = option.rect;
    const qreal r = 5.0;
    const qreal x = rect.x() + 22.0;
    const qreal y = rect.center().y() - r / 2.0;

    painter->setRenderHints(QPainter::Antialiasing);
    if (!(option.state & QStyle::State_MouseOver)) {
        painter->setOpacity(isDarkTheme() ? 0.75 : 0.65);
    }

    painter->setPen(Qt::NoPen);
    painter->setBrush(isDarkTheme() ? Qt::white : Qt::black);
    painter->drawEllipse(QRectF(x, y, r, r));
}

// ==========================================
// CheckIndicatorMenuItemDelegate
// ==========================================

void CheckIndicatorMenuItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                            const QModelIndex &index) const
{
    if (isSeparator(index)) {
        ShortcutMenuItemDelegate::paint(painter, option, index);
        return;
    }

    ShortcutMenuItemDelegate::paint(painter, option, index);

    QAction *action = index.data(Qt::UserRole).value<QAction *>();
    if (!action || !action->isChecked()) {
        return;
    }

    painter->save();
    drawIndicator(painter, option, index);
    painter->restore();
}

void CheckIndicatorMenuItemDelegate::drawIndicator(QPainter *painter, const QStyleOptionViewItem &option,
                                                    const QModelIndex &index) const
{
    Q_UNUSED(index)
    const QRect rect = option.rect;
    const int s = 11;
    const int x = rect.x() + 19;
    const int y = rect.center().y() - s / 2;

    painter->setRenderHints(QPainter::Antialiasing);
    if (!(option.state & QStyle::State_MouseOver)) {
        painter->setOpacity(0.75);
    }

    FluentQt::icon(FluentIcon::Accept, Theme::Auto)
        .paint(painter, QRect(x, y, s, s));
}

// ==========================================
// IndicatorMenuItemDelegate
// ==========================================

void IndicatorMenuItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    MenuItemDelegate::paint(painter, option, index);
    if (!(option.state & QStyle::State_Selected)) {
        return;
    }

    painter->save();
    painter->setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform |
                            QPainter::TextAntialiasing);
    painter->setPen(Qt::NoPen);
    painter->setBrush(themeColor());
    painter->drawRoundedRect(QRectF(6, option.rect.y() + 11, 3, 15), 1.5, 1.5);
    painter->restore();
}

// ==========================================
// RoundMenu
// ==========================================

RoundMenu::RoundMenu(QWidget *parent) : QMenu(parent) { init(); }

RoundMenu::RoundMenu(const QString &title, QWidget *parent) : QMenu(title, parent)
{
    init();
    setTitle(title);
}

RoundMenu::~RoundMenu()
{
    const QList<QAction *> actions = m_actionList;
    for (QAction *action : actions) {
        untrackAction(action);
    }
    m_actionItems.clear();
}

void RoundMenu::init()
{
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(true);

    m_timer = new QTimer(this);
    m_timer->setSingleShot(true);
    m_timer->setInterval(400);
    connect(m_timer, &QTimer::timeout, this, &RoundMenu::onShowMenuTimeout);

    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(12, 8, 12, 20);

    m_view = new MenuActionListWidget(this);
    m_layout->addWidget(m_view, 1, Qt::AlignCenter);

    setShadowEffect();
    FluentStyleSheet::setRole(this, QStringLiteral("RoundMenu"));

    connect(m_view, &QListWidget::itemClicked, this, &RoundMenu::onItemClicked);
    connect(m_view, &QListWidget::itemEntered, this, &RoundMenu::onItemEntered);
}

void RoundMenu::setMenuView(MenuActionListWidget *view)
{
    if (!view || view == m_view) {
        return;
    }

    if (m_view) {
        disconnect(m_view, nullptr, this, nullptr);
        m_layout->removeWidget(m_view);
        m_view->deleteLater();
    }

    m_view = view;
    if (!m_view->parent()) {
        m_view->setParent(this);
    }
    m_layout->addWidget(m_view, 1, Qt::AlignCenter);
    setShadowEffect();

    connect(m_view, &QListWidget::itemClicked, this, &RoundMenu::onItemClicked);
    connect(m_view, &QListWidget::itemEntered, this, &RoundMenu::onItemEntered);
    adjustSize();
}

void RoundMenu::setShadowEffect(int blurRadius, int offsetX, int offsetY)
{
    m_shadowBlurRadius = qMax(0, blurRadius);
    m_shadowOffset = QPoint(offsetX, offsetY);
    if (m_view) {
        m_view->setGraphicsEffect(nullptr);
    }
    update();
}

MenuActionListWidget *RoundMenu::view() const { return m_view; }

void RoundMenu::setMaxVisibleItems(int count) { m_view->setMaxVisibleItems(count); }

int RoundMenu::maxVisibleItems() const { return m_view->maxVisibleItems(); }

void RoundMenu::setItemHeight(int height)
{
    if (height == m_itemHeight) {
        return;
    }
    m_itemHeight = height;
    m_view->setItemHeight(height);
    adjustSize();
}

int RoundMenu::itemHeight() const { return m_itemHeight; }

void RoundMenu::adjustContentSize(const QPoint &pos, MenuAnimationType animationType)
{
    m_view->adjustItemsSize(pos, animationType);
    adjustSize();
}

void RoundMenu::setIcon(const QIcon &icon) { m_icon = icon; }

QIcon RoundMenu::menuIcon() const { return m_icon; }

QAction *RoundMenu::addAction(const QString &text)
{
    auto *action = new QAction(text, this);
    createActionItem(action);
    return action;
}

void RoundMenu::addAction(QAction *action)
{
    if (!action || m_actionList.contains(action)) {
        return;
    }
    createActionItem(action);
}

QAction *RoundMenu::addAction(const QIcon &icon, const QString &text)
{
    auto *action = new QAction(icon, text, this);
    createActionItem(action);
    return action;
}

QAction *RoundMenu::addAction(FluentIcon iconType, const QString &text)
{
    return addAction(FluentQt::icon(iconType), text);
}

void RoundMenu::addActions(const QList<QAction *> &actions)
{
    for (QAction *action : actions) {
        addAction(action);
    }
}

void RoundMenu::insertAction(QAction *before, QAction *action)
{
    if (!before || !m_actionList.contains(before)) {
        return;
    }
    createActionItem(action, before);
}

void RoundMenu::insertActions(QAction *before, const QList<QAction *> &actions)
{
    if (!before || !m_actionList.contains(before)) {
        return;
    }
    for (QAction *action : actions) {
        if (action) {
            insertAction(before, action);
        }
    }
}

QAction *RoundMenu::addCheckableAction(const QString &text, bool checked)
{
    QAction *action = addAction(text);
    action->setCheckable(true);
    action->setChecked(checked);
    return action;
}

QAction *RoundMenu::addWidget(QWidget *widget, bool selectable)
{
    auto *action = new QWidgetAction(this);
    action->setDefaultWidget(widget);
    action->setProperty("selectable", selectable);
    QListWidgetItem *item = createActionItem(action);
    QSize widgetSize = widget->size();
    if (!widgetSize.isValid() || widgetSize.isEmpty()) {
        widgetSize = widget->sizeHint();
    }
    item->setSizeHint(widgetSize);
    m_view->setItemWidget(item, widget);

    if (!selectable) {
        item->setFlags(Qt::NoItemFlags);
    }

    return action;
}

void RoundMenu::removeAction(QAction *action)
{
    if (!action || !m_actionList.contains(action)) {
        return;
    }

    untrackAction(action);

    QListWidgetItem *item = m_actionItems.take(action);
    if (item) {
        removeItem(item);
    }

    m_actionList.removeAll(action);
    QMenu::removeAction(action);
}

QList<QAction *> RoundMenu::menuActions() const { return m_actionList; }

void RoundMenu::addMenu(RoundMenu *menu)
{
    if (!menu) {
        return;
    }
    createSubMenuItem(menu);
}

void RoundMenu::insertMenu(QAction *before, RoundMenu *menu)
{
    if (!before || !m_actionList.contains(before) || !menu) {
        return;
    }
    createSubMenuItem(menu, before);
}

void RoundMenu::removeMenu(RoundMenu *menu)
{
    if (!menu || !m_subMenus.contains(menu)) {
        return;
    }

    QListWidgetItem *item = menu->m_parentMenuItem;
    if (item) {
        removeItem(item);
    }
    m_subMenus.removeAll(menu);
}

void RoundMenu::addSeparator()
{
    const QMargins m = m_view->contentsMargins();
    const int w = m_view->width() - m.left() - m.right();

    auto *item = new QListWidgetItem();
    item->setFlags(Qt::NoItemFlags);
    item->setSizeHint(QSize(w, 9));
    item->setData(Qt::DecorationRole, QStringLiteral("seperator"));
    m_view->addItem(item);
    adjustSize();
}

void RoundMenu::clear()
{
    while (!m_actionList.isEmpty()) {
        removeAction(m_actionList.last());
    }
    while (!m_subMenus.isEmpty()) {
        removeMenu(m_subMenus.last());
    }
}

void RoundMenu::setDefaultAction(QAction *action)
{
    if (!action || !m_actionList.contains(action)) {
        return;
    }
    QListWidgetItem *item = m_actionItems.value(action, nullptr);
    if (item) {
        m_view->setCurrentItem(item);
    }
}

QListWidgetItem *RoundMenu::createActionItem(QAction *action, QAction *before)
{
    if (!before) {
        m_actionList.append(action);
        QMenu::addAction(action);
    } else if (m_actionList.contains(before)) {
        const int index = m_actionList.indexOf(before);
        m_actionList.insert(index, action);
        QMenu::insertAction(before, action);
    }

    QListWidgetItem *item = new QListWidgetItem(createItemIcon(action), action->text());
    adjustItemText(item, action);

    if (!action->isEnabled()) {
        item->setFlags(Qt::NoItemFlags);
    }
    if (action->text() != action->toolTip()) {
        item->setToolTip(action->toolTip());
    }

    item->setData(Qt::UserRole, QVariant::fromValue(action));
    action->setProperty("item", QVariant::fromValue(item));
    m_actionItems.insert(action, item);

    trackAction(action);

    if (before) {
        QListWidgetItem *beforeItem = m_actionItems.value(before, nullptr);
        if (beforeItem) {
            m_view->insertItem(m_view->row(beforeItem), item);
        } else {
            m_view->addItem(item);
        }
    } else {
        m_view->addItem(item);
    }

    adjustSize();
    return item;
}

void RoundMenu::trackAction(QAction *action)
{
    if (!action) {
        return;
    }
    connect(action, &QAction::changed, this, &RoundMenu::onActionChanged, Qt::UniqueConnection);
}

void RoundMenu::untrackAction(QAction *action)
{
    if (!action) {
        return;
    }

    disconnect(action, nullptr, this, nullptr);

    QListWidgetItem *item = m_actionItems.value(action, nullptr);
    if (item && action->property("item").value<QListWidgetItem *>() == item) {
        action->setProperty("item", QVariant());
    }
}

void RoundMenu::createSubMenuItem(RoundMenu *menu, QAction *before)
{
    m_subMenus.append(menu);
    menu->m_parentMenu = this;

    auto *item = new QListWidgetItem(createItemIcon(menu->menuAction()), menu->title());
    if (!hasItemIcon()) {
        const int w = 60 + m_view->fontMetrics().horizontalAdvance(menu->title());
        item->setSizeHint(QSize(w, m_itemHeight));
    } else {
        item->setText(QStringLiteral(" ") + menu->title());
        const int w = 72 + m_view->fontMetrics().horizontalAdvance(item->text());
        item->setSizeHint(QSize(w, m_itemHeight));
    }

    menu->m_isSubMenu = true;
    menu->m_parentMenuItem = item;

    item->setData(Qt::UserRole, QVariant::fromValue(menu));

    auto *widget = new SubMenuItemWidget(menu, item, m_view);
    connect(widget, &SubMenuItemWidget::showMenuSignal, this, &RoundMenu::showSubMenu);
    widget->resize(item->sizeHint());

    if (before) {
        QListWidgetItem *beforeItem = m_actionItems.value(before, nullptr);
        if (beforeItem) {
            m_view->insertItem(m_view->row(beforeItem), item);
        } else {
            m_view->addItem(item);
        }
    } else {
        m_view->addItem(item);
    }

    m_view->setItemWidget(item, widget);
    adjustSize();
}

void RoundMenu::adjustItemText(QListWidgetItem *item, QAction *action)
{
    int sw = 0;
    if (qobject_cast<ShortcutMenuItemDelegate *>(m_view->itemDelegate())) {
        sw = longestShortcutWidth();
        if (sw > 0) {
            sw += 22;
        }
    }

    int w = 0;
    if (!hasItemIcon()) {
        item->setText(action->text());
        w = 40 + m_view->fontMetrics().horizontalAdvance(action->text()) + sw;
    } else {
        item->setText(QStringLiteral(" ") + action->text());
        const int space = 4 - m_view->fontMetrics().horizontalAdvance(QStringLiteral(" "));
        w = 60 + m_view->fontMetrics().horizontalAdvance(item->text()) + sw + space;
    }

    if (m_view && m_view->objectName() == QStringLiteral("checkableListWidget")) {
        w += 26;
    }

    item->setSizeHint(QSize(w, m_itemHeight));
}

QIcon RoundMenu::createItemIcon(QAction *action) const
{
    const bool hasIcon = hasItemIcon();
    QIcon result = action->icon();

    if (hasIcon && result.isNull()) {
        QPixmap pixmap(m_view->iconSize());
        pixmap.fill(Qt::transparent);
        result = QIcon(pixmap);
    } else if (!hasIcon) {
        result = QIcon();
    }

    return result;
}

bool RoundMenu::hasItemIcon() const
{
    for (QAction *a : m_actionList) {
        if (!a->icon().isNull()) {
            return true;
        }
    }
    for (RoundMenu *m : m_subMenus) {
        if (!m->menuIcon().isNull()) {
            return true;
        }
    }
    return false;
}

int RoundMenu::longestShortcutWidth() const
{
    QFont font;
    font.setPixelSize(12);
    int maxWidth = 0;

    for (QAction *action : m_actionList) {
        const QString shortcut = action->shortcut().toString(QKeySequence::NativeText);

        QTextLayout layout(shortcut, font);
        layout.beginLayout();
        QTextLine line = layout.createLine();
        line.setNumColumns(1);
        layout.endLayout();

        const int w = qCeil(layout.boundingRect().width());
        if (w > maxWidth) {
            maxWidth = w;
        }
    }

    return maxWidth;
}

void RoundMenu::removeItem(QListWidgetItem *item)
{
    if (!item) {
        return;
    }

    const int row = m_view->row(item);
    if (row < 0) {
        return;
    }

    QWidget *widget = m_view->itemWidget(item);
    if (widget) {
        m_view->removeItemWidget(item);
        widget->deleteLater();
    }

    QListWidgetItem *taken = m_view->takeItem(row);
    if (taken) {
        taken->setData(Qt::UserRole, QVariant());
        delete taken;
    }
}

QRect RoundMenu::visualItemRect(QListWidgetItem *item) const
{
    return m_view->visualItemRect(item);
}

void RoundMenu::showSubMenu(QListWidgetItem *item)
{
    m_lastHoverItem = item;
    m_lastHoverSubMenuItem = item;
    m_timer->stop();
    m_timer->start();
}

void RoundMenu::onShowMenuTimeout()
{
    if (!m_lastHoverSubMenuItem || m_lastHoverItem != m_lastHoverSubMenuItem) {
        return;
    }

    QWidget *widget = m_view->itemWidget(m_lastHoverSubMenuItem);
    auto *subWidget = qobject_cast<SubMenuItemWidget *>(widget);
    if (!subWidget || !subWidget->menu()) {
        return;
    }

    RoundMenu *subMenu = subWidget->menu();
    if (subMenu->parentMenu() && subMenu->parentMenu()->isHidden()) {
        return;
    }

    const QPoint topLeft = widget->mapToGlobal(QPoint(0, 0));
    const QSize widgetSize = widget->size();
    QPoint pos(topLeft.x() + widgetSize.width() + 5, topLeft.y() - 5);

    const QRect screenRect = screenGeometryAt(pos);
    const QSize subSize = subMenu->sizeHint();

    if (pos.x() + subSize.width() > screenRect.right()) {
        pos.setX(qMax(topLeft.x() - subSize.width() - 5, screenRect.left()));
    }
    if (pos.y() + subSize.height() > screenRect.bottom()) {
        pos.setY(screenRect.bottom() - subSize.height());
    }
    pos.setY(qMax(pos.y(), screenRect.top()));

    subMenu->exec(pos, true, MenuAnimationType::DropDown);
}

void RoundMenu::onItemClicked(QListWidgetItem *item)
{
    QPointer<RoundMenu> self(this);
    if (!item || !m_view) {
        return;
    }

    QVariant data = item->data(Qt::UserRole);
    QAction *action = data.value<QAction *>();
    if (!action || !m_actionList.contains(action) || !action->isEnabled()) {
        return;
    }

    if (m_view->itemWidget(item) && !action->property("selectable").toBool()) {
        return;
    }

    const bool wasSubMenu = m_isSubMenu;

    if (wasSubMenu) {
        m_isHideBySystem = false;
        m_view->clearSelection();
        hide();
    } else {
        m_isHideBySystem = false;
        m_view->clearSelection();
        hide();
    }

    action->trigger();

    if (!self) {
        return;
    }

    if (wasSubMenu) {
        closeParentMenu();
    } else {
        close();
    }
}

void RoundMenu::onItemEntered(QListWidgetItem *item)
{
    m_lastHoverItem = item;

    QVariant data = item->data(Qt::UserRole);
    auto *subMenu = data.value<RoundMenu *>();
    if (subMenu) {
        showSubMenu(item);
    }
}

void RoundMenu::hideMenu(bool isHideBySystem)
{
    m_isHideBySystem = isHideBySystem;
    if (m_view) {
        m_view->clearSelection();
    }
    if (m_isSubMenu) {
        hide();
    } else {
        close();
    }
}

void RoundMenu::closeParentMenu()
{
    RoundMenu *menu = this;
    while (menu) {
        RoundMenu *parent = menu->m_parentMenu;
        menu->close();
        menu = parent;
    }
}

void RoundMenu::onActionChanged()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (!action) {
        return;
    }

    QListWidgetItem *item = m_actionItems.value(action, nullptr);
    if (!item) {
        return;
    }

    item->setIcon(createItemIcon(action));

    if (action->text() != action->toolTip()) {
        item->setToolTip(action->toolTip());
    }

    adjustItemText(item, action);

    if (action->isEnabled()) {
        item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    } else {
        item->setFlags(Qt::NoItemFlags);
    }

    m_view->adjustItemsSize();
    adjustSize();
}

QAction *RoundMenu::exec(const QPoint &pos, bool animated, MenuAnimationType animationType)
{
    setAnimationType(animationType);
    m_runAnimation = animated && animationType != MenuAnimationType::None;
    m_execPos = pos;

    const MenuAnimationType effectiveAnimation = m_runAnimation ? animationType : MenuAnimationType::None;
    MenuAnimationManager *manager = makeMenuAnimationManager(this, effectiveAnimation);
    const QPoint endPosition = manager ? manager->endPosition(pos) : pos;
    if (auto *acrylicView = qobject_cast<AcrylicMenuActionListWidget *>(m_view)) {
        acrylicView->acrylicBrush()->grabImage(QRect(endPosition, layout()->sizeHint()));
    }

    if (manager) {
        manager->exec(pos);
    } else {
        move(pos);
    }

    show();

    if (m_isSubMenu && m_parentMenuItem) {
        m_parentMenuItem->setSelected(true);
    }

    return nullptr;
}

MenuAnimationType RoundMenu::animationType() const { return m_animationType; }

void RoundMenu::setAnimationType(MenuAnimationType type) { m_animationType = type; }

RoundMenu *RoundMenu::parentMenu() const { return m_parentMenu; }

void RoundMenu::adjustSize()
{
    const QMargins m = layout()->contentsMargins();
    const int w = m_view->width() + m.left() + m.right();
    const int h = m_view->height() + m.top() + m.bottom();
    setFixedSize(w, h);
}

void RoundMenu::closeEvent(QCloseEvent *event)
{
    event->accept();
    emit closedSignal();
    clearMask();
    if (m_view) {
        m_view->clearSelection();
    }
}

void RoundMenu::actionEvent(QActionEvent *event)
{
    QMenu::actionEvent(event);
    if (!event || event->type() != QEvent::ActionRemoved) {
        return;
    }

    QAction *action = event->action();
    if (!action || !m_actionList.contains(action)) {
        return;
    }

    untrackAction(action);
    QListWidgetItem *item = m_actionItems.take(action);
    if (item) {
        removeItem(item);
    }
    m_actionList.removeAll(action);
}

void RoundMenu::showEvent(QShowEvent *event)
{
    QMenu::showEvent(event);
}

void RoundMenu::hideEvent(QHideEvent *event)
{
    if (m_isHideBySystem && m_isSubMenu) {
        closeParentMenu();
    }
    m_isHideBySystem = true;
    clearMask();
    QMenu::hideEvent(event);
}

void RoundMenu::paintEvent(QPaintEvent *)
{
    if (!m_view || m_shadowBlurRadius <= 0) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);

    const QRectF baseRect = QRectF(m_view->geometry()).translated(m_shadowOffset);
    const int layers = qBound(4, m_shadowBlurRadius / 4, 12);
    const qreal maxSpread = m_shadowBlurRadius / 2.0;

    for (int i = layers; i >= 1; --i) {
        const qreal progress = static_cast<qreal>(i) / layers;
        const qreal spread = progress * maxSpread;
        const int alpha = qRound(30 * (1.0 - progress) * (1.0 - progress) + 2);
        QColor color(0, 0, 0, qBound(0, alpha, 30));
        painter.setBrush(color);
        painter.drawRoundedRect(baseRect.adjusted(-spread, -spread, spread, spread), 9 + spread, 9 + spread);
    }
}

void RoundMenu::mousePressEvent(QMouseEvent *event)
{
    QWidget *w = childAt(event->pos());
    if (w != m_view && !m_view->isAncestorOf(w)) {
        hideMenu(true);
        return;
    }
    QMenu::mousePressEvent(event);
}

void RoundMenu::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_isSubMenu) {
        return;
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QPoint pos = event->globalPosition().toPoint();
#else
    const QPoint pos = event->globalPos();
#endif
    // Hide submenu when mouse moves out of parent menu item
    const QMargins margin = m_parentMenu->m_view->contentsMargins();
    QRect rect = m_parentMenu->m_view->visualItemRect(m_parentMenuItem);
    const QPoint globalView = m_parentMenu->m_view->mapToGlobal(QPoint(0, 0));
    rect.translate(globalView);
    rect.translate(margin.left(), margin.top() + 2);

    if (m_parentMenu->geometry().contains(pos) && !rect.contains(pos) && !geometry().contains(pos)) {
        m_parentMenu->m_view->clearSelection();
        hideMenu(false);
    }

    QMenu::mouseMoveEvent(event);
}

// ==========================================
// CheckableMenu
// ==========================================

CheckableMenu::CheckableMenu(QWidget *parent) : RoundMenu(parent)
{
    m_view->setItemDelegate(new CheckIndicatorMenuItemDelegate(m_view));
    m_view->setObjectName(QStringLiteral("checkableListWidget"));
    FluentStyleSheet::setRole(this, QStringLiteral("CheckableMenu"));
}

CheckableMenu::CheckableMenu(const QString &title, QWidget *parent) : RoundMenu(title, parent)
{
    m_view->setItemDelegate(new CheckIndicatorMenuItemDelegate(m_view));
    m_view->setObjectName(QStringLiteral("checkableListWidget"));
    FluentStyleSheet::setRole(this, QStringLiteral("CheckableMenu"));
}

CheckableMenu::CheckableMenu(const QString &title, QWidget *parent, MenuIndicatorType indicatorType)
    : RoundMenu(title, parent)
{
    if (indicatorType == MenuIndicatorType::Radio) {
        m_view->setItemDelegate(new RadioIndicatorMenuItemDelegate(m_view));
    } else {
        m_view->setItemDelegate(new CheckIndicatorMenuItemDelegate(m_view));
    }
    m_view->setObjectName(QStringLiteral("checkableListWidget"));
    FluentStyleSheet::setRole(this, QStringLiteral("CheckableMenu"));
}

// ==========================================
// SystemTrayMenu
// ==========================================

SystemTrayMenu::SystemTrayMenu(QWidget *parent) : RoundMenu(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("SystemTrayMenu"));
}

QSize SystemTrayMenu::sizeHint() const
{
    const QMargins m = layout()->contentsMargins();
    const QSize s = layout()->sizeHint();
    return QSize(s.width() - m.right() + 5, s.height() - m.bottom());
}

// ==========================================
// Acrylic menus
// ==========================================

AcrylicMenuActionListWidget::AcrylicMenuActionListWidget(QWidget *parent)
    : MenuActionListWidget(parent)
    , m_acrylicBrush(new AcrylicBrush(viewport(), 35))
{
    setViewportMargins(0, 0, 0, 0);
    setProperty("transparent", true);
    FluentStyleSheet::polish(this);
}

AcrylicMenuActionListWidget::~AcrylicMenuActionListWidget()
{
    delete m_acrylicBrush;
    m_acrylicBrush = nullptr;
}

AcrylicBrush *AcrylicMenuActionListWidget::acrylicBrush() const { return m_acrylicBrush; }

void AcrylicMenuActionListWidget::paintEvent(QPaintEvent *event)
{
    {
        QPainter painter(viewport());
        painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

        QPainterPath path;
        path.addRoundedRect(QRectF(rect()).adjusted(1, 1, -2.5, -2.5), 8, 8);
        m_acrylicBrush->setClipPath(path);
        m_acrylicBrush->setTintColor(acrylicTintColor());
        m_acrylicBrush->setLuminosityColor(acrylicLuminosityColor());
        m_acrylicBrush->paint(&painter);
    }

    MenuActionListWidget::paintEvent(event);
}

AcrylicMenu::AcrylicMenu(QWidget *parent) : RoundMenu(parent)
{
    setMenuView(new AcrylicMenuActionListWidget(this));
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicMenu"));
}

AcrylicMenu::AcrylicMenu(const QString &title, QWidget *parent) : RoundMenu(title, parent)
{
    setMenuView(new AcrylicMenuActionListWidget(this));
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicMenu"));
}

AcrylicCheckableMenu::AcrylicCheckableMenu(QWidget *parent) : CheckableMenu(parent)
{
    setMenuView(new AcrylicMenuActionListWidget(this));
    view()->setItemDelegate(new CheckIndicatorMenuItemDelegate(view()));
    view()->setObjectName(QStringLiteral("checkableListWidget"));
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicCheckableMenu"));
}

AcrylicCheckableMenu::AcrylicCheckableMenu(const QString &title, QWidget *parent) : CheckableMenu(title, parent)
{
    setMenuView(new AcrylicMenuActionListWidget(this));
    view()->setItemDelegate(new CheckIndicatorMenuItemDelegate(view()));
    view()->setObjectName(QStringLiteral("checkableListWidget"));
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicCheckableMenu"));
}

AcrylicCheckableMenu::AcrylicCheckableMenu(const QString &title, QWidget *parent,
                                           MenuIndicatorType indicatorType)
    : CheckableMenu(title, parent, indicatorType)
{
    setMenuView(new AcrylicMenuActionListWidget(this));
    if (indicatorType == MenuIndicatorType::Radio) {
        view()->setItemDelegate(new RadioIndicatorMenuItemDelegate(view()));
    } else {
        view()->setItemDelegate(new CheckIndicatorMenuItemDelegate(view()));
    }
    view()->setObjectName(QStringLiteral("checkableListWidget"));
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicCheckableMenu"));
}

AcrylicSystemTrayMenu::AcrylicSystemTrayMenu(QWidget *parent) : AcrylicMenu(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicSystemTrayMenu"));
}

QSize AcrylicSystemTrayMenu::sizeHint() const
{
    const QMargins m = layout()->contentsMargins();
    const QSize s = layout()->sizeHint();
    return QSize(s.width() - m.right() + 5, s.height() - m.bottom());
}

AcrylicCheckableSystemTrayMenu::AcrylicCheckableSystemTrayMenu(QWidget *parent)
    : AcrylicCheckableMenu(parent)
{
    FluentStyleSheet::setRole(this, QStringLiteral("AcrylicCheckableSystemTrayMenu"));
}

// ==========================================
// LabelContextMenu
// ==========================================

LabelContextMenu::LabelContextMenu(QLabel *parent)
    : RoundMenu(QString(), parent)
    , m_label(parent)
    , m_selectedText(parent ? parent->selectedText() : QString())
    , m_copyAction(new QAction(tr("Copy"), this))
    , m_selectAllAction(new QAction(tr("Select all"), this))
{
    m_copyAction->setShortcut(QKeySequence::Copy);
    m_selectAllAction->setShortcut(QKeySequence::SelectAll);

    connect(m_copyAction, &QAction::triggered, this, &LabelContextMenu::copySelection);
    connect(m_selectAllAction, &QAction::triggered, this, &LabelContextMenu::selectAllText);
}

QLabel *LabelContextMenu::label() const { return m_label; }

QString LabelContextMenu::selectedText() const { return m_selectedText; }

QAction *LabelContextMenu::copyAction() const { return m_copyAction; }

QAction *LabelContextMenu::selectAllAction() const { return m_selectAllAction; }

void LabelContextMenu::createActions()
{
    clear();
    if (!m_label) {
        return;
    }

    m_selectedText = m_label->selectedText();
    if (m_label->hasSelectedText()) {
        addActions({m_copyAction, m_selectAllAction});
    } else {
        addAction(m_selectAllAction);
    }
}

QAction *LabelContextMenu::exec(const QPoint &pos, bool animated, MenuAnimationType animationType)
{
    createActions();
    if (menuActions().isEmpty()) {
        close();
        return nullptr;
    }
    return RoundMenu::exec(pos, animated, animationType);
}

void LabelContextMenu::copySelection()
{
    if (QApplication::clipboard()) {
        QApplication::clipboard()->setText(m_selectedText);
    }
}

void LabelContextMenu::selectAllText()
{
    if (!m_label) {
        return;
    }
    m_label->setSelection(0, m_label->text().size());
}

} // namespace FluentQt
