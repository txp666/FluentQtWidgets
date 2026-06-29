#include <FluentQtWidgets/Widgets/CommandBar.h>

#include <FluentQtWidgets/StyleSheet.h>
#include <FluentQtWidgets/Theme.h>
#include <FluentQtWidgets/Widgets/Button.h>

#include <QtCore/QPointer>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QResizeEvent>
#include <QtGui/QShowEvent>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QSizePolicy>
#include <QtWidgets/QStyle>
#include <QtWidgets/QStyleOptionToolButton>
#include <QtWidgets/QToolButton>

namespace FluentQt {

namespace {

bool commandBarDarkTheme()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark;
}

} // namespace

CommandButton::CommandButton(QWidget *parent) : TransparentToggleToolButton(parent)
{
    setCheckable(false);
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setIconSize(QSize(16, 16));
    setProperty("fqw", QStringLiteral("CommandButton"));
    installEventFilter(new CommandToolTipFilter(this));
}

CommandButton::CommandButton(const QIcon &icon, QWidget *parent) : CommandButton(parent)
{
    setIcon(icon);
}

bool CommandButton::isTight() const { return m_tight; }

bool CommandButton::isIconOnly() const
{
    if (text().isEmpty()) {
        return true;
    }

    return toolButtonStyle() == Qt::ToolButtonIconOnly || toolButtonStyle() == Qt::ToolButtonFollowStyle;
}

QAction *CommandButton::action() const { return m_action; }

QSize CommandButton::sizeHint() const
{
    if (isIconOnly()) {
        return m_tight ? QSize(36, 34) : QSize(48, 34);
    }

    const int textWidth = fontMetrics().horizontalAdvance(text());
    switch (toolButtonStyle()) {
    case Qt::ToolButtonTextBesideIcon:
        return QSize(textWidth + 47, 34);
    case Qt::ToolButtonTextOnly:
        return QSize(textWidth + 32, 34);
    case Qt::ToolButtonTextUnderIcon:
        return QSize(textWidth + 32, 50);
    default:
        return QSize(textWidth + 32, 50);
    }
}

QSize CommandButton::minimumSizeHint() const { return sizeHint(); }

void CommandButton::setTight(bool tight)
{
    if (m_tight == tight) {
        return;
    }

    m_tight = tight;
    updateGeometry();
    update();
}

void CommandButton::setAction(QAction *action)
{
    if (m_action == action) {
        return;
    }

    if (m_action) {
        disconnect(m_action, nullptr, this, nullptr);
    }

    m_action = action;
    setDefaultAction(action);
    if (m_action) {
        connect(m_action, &QAction::changed, this, &CommandButton::syncActionState);
    }
    syncActionState();
}

void CommandButton::syncActionState()
{
    if (!m_action) {
        return;
    }

    setIcon(m_action->icon());
    setText(m_action->text());
    setToolTip(m_action->toolTip());
    setEnabled(m_action->isEnabled());
    setCheckable(m_action->isCheckable());
    setChecked(m_action->isChecked());
    updateGeometry();
    update();
}

void CommandButton::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

    QStyleOptionToolButton opt;
    initStyleOption(&opt);
    opt.icon = QIcon();
    opt.text.clear();
    style()->drawComplexControl(QStyle::CC_ToolButton, &opt, &painter, this);

    if (!isChecked()) {
        painter.setPen(commandBarDarkTheme() ? Qt::white : Qt::black);
    } else {
        painter.setPen(commandBarDarkTheme() ? Qt::black : Qt::white);
    }

    if (!isEnabled()) {
        painter.setOpacity(0.43);
    } else if (isDown()) {
        painter.setOpacity(0.63);
    }

    const Qt::ToolButtonStyle style = toolButtonStyle();
    const int iw = iconSize().width();
    const int ih = iconSize().height();

    if (isIconOnly()) {
        const int y = (height() - ih) / 2;
        const int x = (width() - iw) / 2;
        icon().paint(&painter, QRect(x, y, iw, ih));
    } else if (style == Qt::ToolButtonTextOnly) {
        painter.drawText(rect(), Qt::AlignCenter, text());
    } else if (style == Qt::ToolButtonTextBesideIcon) {
        const int y = (height() - ih) / 2;
        icon().paint(&painter, QRect(11, y, iw, ih));
        painter.drawText(QRect(26, 0, width() - 26, height()), Qt::AlignCenter, text());
    } else if (style == Qt::ToolButtonTextUnderIcon) {
        const int x = (width() - iw) / 2;
        icon().paint(&painter, QRect(x, 9, iw, ih));
        painter.drawText(QRect(0, ih + 13, width(), height() - ih - 13), Qt::AlignHCenter | Qt::AlignTop, text());
    }
}

CommandToolTipFilter::CommandToolTipFilter(CommandButton *parent, int showDelayMs, ToolTipPosition position)
    : ToolTipFilter(parent, showDelayMs, position)
{
}

bool CommandToolTipFilter::canShowToolTip(QWidget *widget) const
{
    auto *button = qobject_cast<CommandButton *>(widget);
    return ToolTipFilter::canShowToolTip(widget) && button && button->isIconOnly();
}

MoreActionsButton::MoreActionsButton(QWidget *parent) : CommandButton(parent)
{
    setProperty("fqw", QStringLiteral("MoreActionsButton"));
    setToolTip(tr("More"));
    setFixedSize(sizeHint());
}

QSize MoreActionsButton::sizeHint() const { return QSize(40, 34); }

QSize MoreActionsButton::minimumSizeHint() const { return sizeHint(); }

void MoreActionsButton::clearState()
{
    m_isHover = false;
    m_isPressed = false;
    setDown(false);
    setAttribute(Qt::WA_UnderMouse, false);
    update();
}

void MoreActionsButton::paintEvent(QPaintEvent *event)
{
    CommandButton::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    QColor color = commandBarDarkTheme() ? QColor(Qt::white) : QColor(Qt::black);
    if (!isEnabled()) {
        color.setAlphaF(0.43);
    } else if (isDown()) {
        color.setAlphaF(0.63);
    }

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    const int y = height() / 2;
    const int centerX = width() / 2;
    for (int x : {centerX - 7, centerX, centerX + 7}) {
        painter.drawEllipse(QPointF(x, y), 1.5, 1.5);
    }
}

CommandSeparator::CommandSeparator(QWidget *parent) : QWidget(parent)
{
    setFixedSize(9, 34);
    setProperty("fqw", QStringLiteral("CommandSeparator"));
}

void CommandSeparator::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(commandBarDarkTheme() ? QColor(255, 255, 255, 21) : QColor(0, 0, 0, 15));
    painter.drawLine(5, 2, 5, height() - 2);
}

CommandMenu::CommandMenu(QWidget *parent) : RoundMenu(QString(), parent)
{
    setItemHeight(32);
    view()->setIconSize(QSize(16, 16));
    setProperty("fqw", QStringLiteral("CommandMenu"));
}

CommandBar::CommandBar(QWidget *parent) : QFrame(parent) { init(); }

CommandBar::~CommandBar()
{
    m_destroying = true;
    const QList<QAction *> actions = m_actions + m_hiddenActions;
    for (QAction *action : actions) {
        if (action) {
            disconnect(action, nullptr, this, nullptr);
        }
    }
}

QAction *CommandBar::addAction(const QString &text)
{
    auto *action = new QAction(text, this);
    addAction(action);
    return action;
}

QAction *CommandBar::addAction(const QIcon &icon, const QString &text)
{
    auto *action = new QAction(icon, text, this);
    addAction(action);
    return action;
}

QAction *CommandBar::addAction(FluentIcon iconType, const QString &text)
{
    return addAction(icon(iconType), text);
}

void CommandBar::addAction(QAction *action)
{
    if (!action || m_actions.contains(action)) {
        return;
    }

    if (!action->parent()) {
        action->setParent(this);
    }
    m_actions.append(action);
    trackAction(action);
    rebuild();
}

void CommandBar::addActions(const QList<QAction *> &actions)
{
    for (QAction *action : actions) {
        if (action) {
            addAction(action);
        }
    }
}

QAction *CommandBar::addSeparator()
{
    auto *action = new QAction(this);
    action->setSeparator(true);
    addAction(action);
    return action;
}

QAction *CommandBar::insertSeparator(int index)
{
    auto *action = new QAction(this);
    action->setSeparator(true);
    index = qBound(0, index < 0 ? m_actions.size() : index, m_actions.size());
    m_actions.insert(index, action);
    trackAction(action);
    rebuild();
    return action;
}

void CommandBar::addWidget(QWidget *widget)
{
    if (!widget || m_customWidgets.contains(widget)) {
        return;
    }

    widget->setParent(this);
    m_customWidgets.append(widget);
    rebuild();
}

void CommandBar::removeAction(QAction *action)
{
    if (!action) {
        return;
    }

    const bool removed = m_actions.removeAll(action) > 0 || m_hiddenActions.removeAll(action) > 0;
    if (!removed) {
        return;
    }
    if (action->parent() == this) {
        action->deleteLater();
    }
    rebuild();
}

void CommandBar::removeWidget(QWidget *widget)
{
    if (!widget || !m_customWidgets.removeOne(widget)) {
        return;
    }
    widget->hide();
    widget->deleteLater();
    rebuild();
}

void CommandBar::addHiddenAction(QAction *action)
{
    if (!action || m_hiddenActions.contains(action)) {
        return;
    }

    if (!action->parent()) {
        action->setParent(this);
    }
    m_hiddenActions.append(action);
    trackAction(action);
    rebuild();
}

QAction *CommandBar::addHiddenAction(const QIcon &icon, const QString &text)
{
    auto *action = new QAction(icon, text, this);
    addHiddenAction(action);
    return action;
}

QAction *CommandBar::addHiddenAction(FluentIcon iconType, const QString &text)
{
    return addHiddenAction(icon(iconType), text);
}

void CommandBar::addHiddenActions(const QList<QAction *> &actions)
{
    for (QAction *action : actions) {
        if (action) {
            addHiddenAction(action);
        }
    }
}

void CommandBar::removeHiddenAction(QAction *action)
{
    if (!action || !m_hiddenActions.removeOne(action)) {
        return;
    }
    if (action->parent() == this) {
        action->deleteLater();
    }
    rebuild();
}

QList<QAction *> CommandBar::commandActions() const { return m_actions; }

QList<QAction *> CommandBar::hiddenActions() const { return m_hiddenActions; }

QList<CommandButton *> CommandBar::commandButtons() const
{
    QList<CommandButton *> buttons;
    if (!m_layout) {
        return buttons;
    }

    for (int i = 0; i < m_layout->count(); ++i) {
        auto *item = m_layout->itemAt(i);
        auto *button = item ? qobject_cast<CommandButton *>(item->widget()) : nullptr;
        if (button && button != moreButton()) {
            buttons.append(button);
        }
    }
    return buttons;
}

QList<QWidget *> CommandBar::hiddenWidgets() const { return m_hiddenWidgets; }

Qt::ToolButtonStyle CommandBar::toolButtonStyle() const { return m_toolButtonStyle; }

bool CommandBar::isButtonTight() const { return m_buttonTight; }

QSize CommandBar::iconSize() const { return m_iconSize; }

int CommandBar::spacing() const { return m_spacing; }

QToolButton *CommandBar::overflowButton() const { return m_overflowButton; }

MoreActionsButton *CommandBar::moreButton() const { return m_overflowButton; }

int CommandBar::suitableWidth() const
{
    int width = m_layout ? m_layout->contentsMargins().left() + m_layout->contentsMargins().right() : 0;
    int visibleItems = 0;
    for (QAction *action : m_actions) {
        if (!action) {
            continue;
        }
        ++visibleItems;
        if (action->isSeparator()) {
            width += 9;
        } else {
            CommandButton button;
            button.setAction(action);
            button.setToolButtonStyle(m_toolButtonStyle);
            button.setTight(m_buttonTight);
            button.setIconSize(m_iconSize);
            button.setFont(font());
            width += button.sizeHint().width();
        }
    }
    for (QWidget *widget : m_customWidgets) {
        if (widget) {
            ++visibleItems;
            width += widget->sizeHint().width();
        }
    }
    if (!m_hiddenActions.isEmpty()) {
        ++visibleItems;
        width += MoreActionsButton().sizeHint().width();
    }
    if (visibleItems > 1) {
        width += m_spacing * (visibleItems - 1);
    }
    return width;
}

MenuAnimationType CommandBar::menuAnimation() const { return m_menuAnimation; }

bool CommandBar::isMenuDropDown() const
{
    return m_menuAnimation == MenuAnimationType::DropDown || m_menuAnimation == MenuAnimationType::FadeInDropDown;
}

void CommandBar::setToolButtonStyle(Qt::ToolButtonStyle style)
{
    if (m_toolButtonStyle == style) {
        return;
    }

    m_toolButtonStyle = style;
    rebuild();
}

void CommandBar::setButtonTight(bool tight)
{
    if (m_buttonTight == tight) {
        return;
    }

    m_buttonTight = tight;
    rebuild();
}

void CommandBar::setIconSize(const QSize &size)
{
    if (m_iconSize == size || !size.isValid()) {
        return;
    }

    m_iconSize = size;
    rebuild();
}

void CommandBar::setSpacing(int spacing)
{
    const int boundedSpacing = qMax(0, spacing);
    if (m_spacing == boundedSpacing) {
        return;
    }

    m_spacing = boundedSpacing;
    if (m_layout) {
        m_layout->setSpacing(m_spacing);
    }
    updateGeometry();
}

void CommandBar::setSpaing(int spacing)
{
    setSpacing(spacing);
}

void CommandBar::setMenuDropDown(bool down)
{
    m_menuAnimation = down ? MenuAnimationType::DropDown : MenuAnimationType::PullUp;
}

void CommandBar::resizeToSuitableWidth()
{
    setFixedWidth(suitableWidth());
}

void CommandBar::clear()
{
    const QList<QAction *> allActions = m_actions + m_hiddenActions;
    m_actions.clear();
    m_hiddenActions.clear();
    m_hiddenWidgets.clear();
    qDeleteAll(m_customWidgets);
    m_customWidgets.clear();
    qDeleteAll(allActions);
    rebuild();
}

void CommandBar::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    updateOverflowState();
}

void CommandBar::showEvent(QShowEvent *event)
{
    QFrame::showEvent(event);
    updateOverflowState();
}

void CommandBar::init()
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(4, 4, 4, 4);
    m_layout->setSpacing(m_spacing);
    m_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setAttribute(Qt::WA_TranslucentBackground);
    FluentStyleSheet::setRole(this, QStringLiteral("CommandBar"));
}

void CommandBar::rebuild()
{
    if (m_destroying || !m_layout) {
        return;
    }

    while (QLayoutItem *item = m_layout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->hide();
            if (!m_customWidgets.contains(widget)) {
                widget->deleteLater();
            }
        }
        delete item;
    }

    for (QAction *action : std::as_const(m_actions)) {
        if (!action) {
            continue;
        }

        if (action->isSeparator()) {
            m_layout->addWidget(new CommandSeparator(this), 0, Qt::AlignVCenter);
            continue;
        }

        m_layout->addWidget(createButtonForAction(action), 0, Qt::AlignVCenter);
    }

    for (QWidget *widget : std::as_const(m_customWidgets)) {
        if (!widget) {
            continue;
        }
        widget->show();
        m_layout->addWidget(widget, 0, Qt::AlignVCenter);
    }

    m_overflowButton = new MoreActionsButton(this);
    m_overflowButton->setVisible(!m_hiddenActions.isEmpty());
    connect(m_overflowButton, &QToolButton::clicked, this, &CommandBar::showHiddenActionsMenu);
    m_layout->addWidget(m_overflowButton, 0, Qt::AlignVCenter);
    m_layout->addStretch();
    updateOverflowState();
}

CommandButton *CommandBar::createButtonForAction(QAction *action)
{
    auto *button = new CommandButton(this);
    button->setAction(action);
    button->setToolButtonStyle(m_toolButtonStyle);
    button->setTight(m_buttonTight);
    button->setIconSize(m_iconSize);
    button->setFont(font());
    button->setFixedSize(button->sizeHint());
    return button;
}

CommandMenu *CommandBar::createMoreActionsMenu()
{
    auto *menu = new CommandMenu(m_overflowButton);
    for (QWidget *widget : std::as_const(m_hiddenWidgets)) {
        auto *button = qobject_cast<CommandButton *>(widget);
        if (button && button->action()) {
            menu->addAction(button->action());
        }
    }
    for (QAction *action : std::as_const(m_hiddenActions)) {
        if (!action) {
            continue;
        }
        if (action->isSeparator()) {
            menu->addSeparator();
        } else {
            menu->addAction(action);
        }
    }
    return menu;
}

void CommandBar::showHiddenActionsMenu()
{
    if ((!m_hiddenActions.size() && m_hiddenWidgets.isEmpty()) || !m_overflowButton) {
        return;
    }

    m_overflowButton->clearState();
    auto *menu = createMoreActionsMenu();
    menu->setAttribute(Qt::WA_DeleteOnClose);
    const int x = -menu->width() + menu->layout()->contentsMargins().right() + m_overflowButton->width() + 18;
    const int y = isMenuDropDown() ? m_overflowButton->height() : -5;
    menu->exec(m_overflowButton->mapToGlobal(QPoint(x, y)), true, m_menuAnimation);
}

void CommandBar::updateOverflowState()
{
    m_hiddenWidgets.clear();
    if (!m_layout || !m_overflowButton) {
        return;
    }

    QList<QWidget *> widgets;
    for (int i = 0; i < m_layout->count(); ++i) {
        auto *item = m_layout->itemAt(i);
        auto *widget = item ? item->widget() : nullptr;
        if (!widget || widget == m_overflowButton) {
            continue;
        }
        widgets.append(widget);
    }

    const bool hasConstrainedWidth = width() > 0 && suitableWidth() > width();
    const QMargins margins = m_layout->contentsMargins();
    int usedWidth = margins.left() + margins.right();
    int visibleCount = 0;
    const int reservedMoreWidth = hasConstrainedWidth || !m_hiddenActions.isEmpty() ? m_overflowButton->sizeHint().width() : 0;

    for (QWidget *widget : std::as_const(widgets)) {
        const QSize hint = widget->sizeHint().isValid() ? widget->sizeHint() : widget->minimumSizeHint();
        const int itemWidth = qMax(widget->width(), hint.width());
        const int nextWidth = usedWidth + (visibleCount > 0 ? m_spacing : 0) + itemWidth + reservedMoreWidth;
        if (hasConstrainedWidth && nextWidth > width()) {
            widget->hide();
            m_hiddenWidgets.append(widget);
            continue;
        }

        widget->show();
        usedWidth += (visibleCount > 0 ? m_spacing : 0) + itemWidth;
        ++visibleCount;
    }

    m_overflowButton->setVisible(!m_hiddenActions.isEmpty() || !m_hiddenWidgets.isEmpty());
}

void CommandBar::trackAction(QAction *action)
{
    connect(action, &QAction::changed, this, &CommandBar::rebuild, Qt::UniqueConnection);
    connect(action, &QObject::destroyed, this, [this, action]() {
        m_actions.removeAll(action);
        m_hiddenActions.removeAll(action);
        if (!m_destroying) {
            rebuild();
        }
    });
}

CommandViewMenu::CommandViewMenu(QWidget *parent) : CommandMenu(parent)
{
    view()->setObjectName(QStringLiteral("commandListWidget"));
    setProperty("fqw", QStringLiteral("CommandViewMenu"));
}

void CommandViewMenu::setDropDown(bool down, bool isLong)
{
    view()->setProperty("dropDown", down);
    view()->setProperty("long", isLong);
    FluentStyleSheet::polish(view());
    view()->update();
}

CommandViewBar::CommandViewBar(QWidget *parent) : CommandBar(parent)
{
    setMenuDropDown(true);
    setProperty("fqw", QStringLiteral("CommandViewBar"));
}

void CommandViewBar::setMenuDropDown(bool down)
{
    CommandBar::setMenuDropDown(down);
    m_menuAnimation = down ? MenuAnimationType::FadeInDropDown : MenuAnimationType::FadeInPullUp;
}

CommandMenu *CommandViewBar::createMoreActionsMenu()
{
    auto *menu = new CommandViewMenu(moreButton());
    for (QWidget *widget : std::as_const(m_hiddenWidgets)) {
        auto *button = qobject_cast<CommandButton *>(widget);
        if (button && button->action()) {
            menu->addAction(button->action());
        }
    }
    for (QAction *action : std::as_const(m_hiddenActions)) {
        if (!action) {
            continue;
        }
        if (action->isSeparator()) {
            menu->addSeparator();
        } else {
            menu->addAction(action);
        }
    }
    auto *view = qobject_cast<CommandBarView *>(parentWidget());
    menu->setDropDown(isMenuDropDown(), view && menu->view()->width() > view->width() + 5);
    if (view && menu->view()->width() < view->width()) {
        menu->view()->setFixedWidth(view->width());
        menu->adjustContentSize();
    }
    if (view) {
        QPointer<CommandBarView> guardedView(view);
        connect(menu, &RoundMenu::closedSignal, view, [guardedView]() {
            if (guardedView) {
                guardedView->setMenuVisible(false);
            }
        });
    }
    return menu;
}

void CommandViewBar::showHiddenActionsMenu()
{
    if ((m_hiddenActions.isEmpty() && m_hiddenWidgets.isEmpty()) || !moreButton()) {
        return;
    }

    auto *view = qobject_cast<CommandBarView *>(parentWidget());
    if (view) {
        view->setMenuVisible(true);
    }

    CommandBar::showHiddenActionsMenu();
}

CommandBarView::CommandBarView(QWidget *parent) : FlyoutViewBase(parent)
{
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(6, 6, 6, 6);
    m_layout->setSpacing(0);
    m_commandBar = new CommandViewBar(this);
    m_commandBar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    m_commandBar->setButtonTight(true);
    m_commandBar->setIconSize(QSize(14, 14));
    m_layout->addWidget(m_commandBar);
    m_layout->setSizeConstraint(QHBoxLayout::SetMinAndMaxSize);
    FluentStyleSheet::setRole(this, QStringLiteral("CommandBarView"));
}

QAction *CommandBarView::addAction(const QString &text) { return m_commandBar->addAction(text); }

QAction *CommandBarView::addAction(const QIcon &icon, const QString &text)
{
    return m_commandBar->addAction(icon, text);
}

QAction *CommandBarView::addAction(FluentIcon iconType, const QString &text)
{
    return m_commandBar->addAction(iconType, text);
}

void CommandBarView::addAction(QAction *action) { m_commandBar->addAction(action); }

void CommandBarView::addActions(const QList<QAction *> &actions) { m_commandBar->addActions(actions); }

QAction *CommandBarView::addSeparator() { return m_commandBar->addSeparator(); }

QAction *CommandBarView::insertSeparator(int index) { return m_commandBar->insertSeparator(index); }

void CommandBarView::addWidget(QWidget *widget) { m_commandBar->addWidget(widget); }

void CommandBarView::removeAction(QAction *action) { m_commandBar->removeAction(action); }

void CommandBarView::removeWidget(QWidget *widget) { m_commandBar->removeWidget(widget); }

QAction *CommandBarView::addHiddenAction(const QIcon &icon, const QString &text)
{
    return m_commandBar->addHiddenAction(icon, text);
}

QAction *CommandBarView::addHiddenAction(FluentIcon iconType, const QString &text)
{
    return m_commandBar->addHiddenAction(iconType, text);
}

void CommandBarView::addHiddenAction(QAction *action) { m_commandBar->addHiddenAction(action); }

void CommandBarView::addHiddenActions(const QList<QAction *> &actions) { m_commandBar->addHiddenActions(actions); }

void CommandBarView::removeHiddenAction(QAction *action) { m_commandBar->removeHiddenAction(action); }

CommandBar *CommandBarView::commandBar() const { return m_commandBar; }

void CommandBarView::setSpaing(int spacing) { m_commandBar->setSpaing(spacing); }

int CommandBarView::spacing() const { return m_commandBar->spacing(); }

void CommandBarView::setToolButtonStyle(Qt::ToolButtonStyle style) { m_commandBar->setToolButtonStyle(style); }

Qt::ToolButtonStyle CommandBarView::toolButtonStyle() const { return m_commandBar->toolButtonStyle(); }

void CommandBarView::setButtonTight(bool tight) { m_commandBar->setButtonTight(tight); }

bool CommandBarView::isButtonTight() const { return m_commandBar->isButtonTight(); }

void CommandBarView::setIconSize(const QSize &size) { m_commandBar->setIconSize(size); }

QSize CommandBarView::iconSize() const { return m_commandBar->iconSize(); }

void CommandBarView::setMenuDropDown(bool down) { m_commandBar->setMenuDropDown(down); }

bool CommandBarView::isMenuVisible() const { return m_menuVisible; }

void CommandBarView::setMenuVisible(bool visible)
{
    if (m_menuVisible == visible) {
        return;
    }
    m_menuVisible = visible;
    update();
}

int CommandBarView::suitableWidth() const
{
    const QMargins margins = contentsMargins();
    return margins.left() + margins.right() + (m_commandBar ? m_commandBar->suitableWidth() : 0);
}

void CommandBarView::resizeToSuitableWidth()
{
    if (m_commandBar) {
        m_commandBar->resizeToSuitableWidth();
    }
    setFixedWidth(suitableWidth());
}

void CommandBarView::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPainterPath path;
    path.setFillRule(Qt::WindingFill);
    path.addRoundedRect(QRectF(rect().adjusted(1, 1, -1, -1)), 8, 8);

    if (m_menuVisible && m_commandBar) {
        const int y = m_commandBar->isMenuDropDown() ? height() - 10 : 1;
        path.addRect(1, y, width() - 2, 9);
    }

    painter.setBrush(commandBarDarkTheme() ? QColor(40, 40, 40) : QColor(248, 248, 248));
    painter.setPen(commandBarDarkTheme() ? QColor(56, 56, 56) : QColor(233, 233, 233));
    painter.drawPath(path.simplified());
}

} // namespace FluentQt
