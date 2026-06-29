#pragma once

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Global.h>

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QMargins>
#include <QtCore/QPoint>
#include <QtWidgets/QMenu>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QStyledItemDelegate>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class QEnterEvent;
#endif
class QCloseEvent;
class QShowEvent;
class QActionEvent;
class QWidget;
class QTimer;
class QHBoxLayout;
class QLabel;
class QPaintEvent;

namespace FluentQt {

class AcrylicBrush;

enum class MenuAnimationType
{
    None,
    DropDown,
    PullUp,
    FadeInDropDown,
    FadeInPullUp
};

enum class MenuIndicatorType
{
    Check = 0,
    Radio = 1
};

class RoundMenu;

// --- SubMenuItemWidget ---
class FQW_API SubMenuItemWidget : public QWidget
{
    Q_OBJECT

public:
    SubMenuItemWidget(RoundMenu *menu, QListWidgetItem *item, QWidget *parent = nullptr);

    RoundMenu *menu() const;
    QListWidgetItem *item() const;

signals:
    void showMenuSignal(QListWidgetItem *item);

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void paintEvent(QPaintEvent *event) override;

private:
    RoundMenu *m_menu = nullptr;
    QListWidgetItem *m_item = nullptr;
};

// --- MenuActionListWidget ---
class FQW_API MenuActionListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit MenuActionListWidget(QWidget *parent = nullptr);

    void setItemHeight(int height);
    int itemHeight() const;
    void setMaxVisibleItems(int num);
    int maxVisibleItems() const;
    void setViewportPadding(int left, int top, int right, int bottom);
    QMargins viewportPadding() const;

    int itemsHeight() const;
    int heightForAnimation(const QPoint &pos, MenuAnimationType aniType) const;
    QSize availableViewSize(const QPoint &pos, MenuAnimationType aniType) const;

    void adjustItemsSize(const QPoint &pos = QPoint(), MenuAnimationType aniType = MenuAnimationType::None);

protected:
    void rowsInserted(const QModelIndex &parent, int start, int end) override;
#if QT_VERSION < QT_VERSION_CHECK(6, 8, 0)
    void rowsRemoved(const QModelIndex &parent, int start, int end) override;
#else
    void rowsAboutToBeRemoved(const QModelIndex &parent, int first, int last) override;
#endif

private:
    int m_itemHeight = 28;
    int m_maxVisibleItems = -1;
};

// --- Menu item delegates ---
class FQW_API MenuItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit MenuItemDelegate(QObject *parent = nullptr);

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool isSeparator(const QModelIndex &index) const;
};

class FQW_API ShortcutMenuItemDelegate : public MenuItemDelegate
{
    Q_OBJECT

public:
    using MenuItemDelegate::MenuItemDelegate;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

// --- Indicator delegates ---
class FQW_API RadioIndicatorMenuItemDelegate : public ShortcutMenuItemDelegate
{
    Q_OBJECT
public:
    using ShortcutMenuItemDelegate::ShortcutMenuItemDelegate;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void drawIndicator(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class FQW_API CheckIndicatorMenuItemDelegate : public ShortcutMenuItemDelegate
{
    Q_OBJECT
public:
    using ShortcutMenuItemDelegate::ShortcutMenuItemDelegate;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void drawIndicator(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class FQW_API IndicatorMenuItemDelegate : public MenuItemDelegate
{
    Q_OBJECT
public:
    using MenuItemDelegate::MenuItemDelegate;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

// --- RoundMenu ---
class FQW_API RoundMenu : public QMenu
{
    Q_OBJECT
    Q_PROPERTY(FluentQt::MenuAnimationType animationType READ animationType WRITE setAnimationType)

public:
    explicit RoundMenu(QWidget *parent = nullptr);
    explicit RoundMenu(const QString &title, QWidget *parent = nullptr);
    ~RoundMenu() override;

    // Action management
    void addAction(QAction *action);
    QAction *addAction(const QString &text);
    QAction *addAction(const QIcon &icon, const QString &text);
    QAction *addAction(FluentIcon iconType, const QString &text);
    void addActions(const QList<QAction *> &actions);
    void insertAction(QAction *before, QAction *action);
    void insertActions(QAction *before, const QList<QAction *> &actions);
    QAction *addCheckableAction(const QString &text, bool checked = false);
    QAction *addWidget(QWidget *widget, bool selectable = true);
    void removeAction(QAction *action);
    QList<QAction *> menuActions() const;

    // Sub-menu management
    void addMenu(RoundMenu *menu);
    void insertMenu(QAction *before, RoundMenu *menu);
    void removeMenu(RoundMenu *menu);

    // Separator
    void addSeparator();
    void clear();

    // View accessor
    MenuActionListWidget *view() const;

    // Parent menu accessor (for sub-menu navigation)
    RoundMenu *parentMenu() const;

    // Properties
    void setIcon(const QIcon &icon);
    QIcon menuIcon() const;

    using QMenu::exec;
    QAction *exec(const QPoint &pos, bool animated = true,
                  MenuAnimationType animationType = MenuAnimationType::DropDown);

    MenuAnimationType animationType() const;
    void setAnimationType(MenuAnimationType type);
    int maxVisibleItems() const;
    void setMaxVisibleItems(int count);
    int itemHeight() const;
    void setItemHeight(int height);
    void adjustSize();
    void adjustContentSize(const QPoint &pos = QPoint(),
                           MenuAnimationType animationType = MenuAnimationType::None);
    void setShadowEffect(int blurRadius = 30, int offsetX = 0, int offsetY = 8);

    void setDefaultAction(QAction *action);

signals:
    void closedSignal();

protected:
    void closeEvent(QCloseEvent *event) override;
    void actionEvent(QActionEvent *event) override;
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void setMenuView(MenuActionListWidget *view);

private:
    void init();
    void trackAction(QAction *action);
    void untrackAction(QAction *action);
    QListWidgetItem *createActionItem(QAction *action, QAction *before = nullptr);
    void createSubMenuItem(RoundMenu *menu, QAction *before = nullptr);
    void adjustItemText(QListWidgetItem *item, QAction *action);
    QIcon createItemIcon(QAction *action) const;
    bool hasItemIcon() const;
    int longestShortcutWidth() const;
    void removeItem(QListWidgetItem *item);
    QRect visualItemRect(QListWidgetItem *item) const;
    void showSubMenu(QListWidgetItem *item);
    void onShowMenuTimeout();
    void onItemClicked(QListWidgetItem *item);
    void onItemEntered(QListWidgetItem *item);
    void hideMenu(bool isHideBySystem);
    void closeParentMenu();
    void onActionChanged();

protected:
    MenuActionListWidget *m_view = nullptr;

private:
    QHBoxLayout *m_layout = nullptr;
    QTimer *m_timer = nullptr;
    int m_shadowBlurRadius = 30;
    QPoint m_shadowOffset = QPoint(0, 8);

    MenuAnimationType m_animationType = MenuAnimationType::DropDown;
    int m_maxVisibleItems = -1;
    int m_itemHeight = 28;
    bool m_runAnimation = false;
    bool m_isSubMenu = false;
    bool m_isHideBySystem = true;
    QPoint m_execPos;

    QIcon m_icon;
    QList<QAction *> m_actionList;
    QHash<QAction *, QListWidgetItem *> m_actionItems;
    QList<RoundMenu *> m_subMenus;
    QListWidgetItem *m_lastHoverItem = nullptr;
    QListWidgetItem *m_lastHoverSubMenuItem = nullptr;

    // Sub-menu parent info
    RoundMenu *m_parentMenu = nullptr;
    QListWidgetItem *m_parentMenuItem = nullptr;

    friend class SubMenuItemWidget;
};

// --- CheckableMenu ---
class FQW_API CheckableMenu : public RoundMenu
{
    Q_OBJECT

public:
    explicit CheckableMenu(QWidget *parent = nullptr);
    explicit CheckableMenu(const QString &title, QWidget *parent = nullptr);
    explicit CheckableMenu(const QString &title, QWidget *parent, MenuIndicatorType indicatorType);
};

// --- SystemTrayMenu ---
class FQW_API SystemTrayMenu : public RoundMenu
{
    Q_OBJECT

public:
    explicit SystemTrayMenu(QWidget *parent = nullptr);

    QSize sizeHint() const override;
};

// --- Acrylic menus ---
class FQW_API AcrylicMenuActionListWidget : public MenuActionListWidget
{
    Q_OBJECT

public:
    explicit AcrylicMenuActionListWidget(QWidget *parent = nullptr);
    ~AcrylicMenuActionListWidget() override;

    AcrylicBrush *acrylicBrush() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    AcrylicBrush *m_acrylicBrush = nullptr;
};

class FQW_API AcrylicMenu : public RoundMenu
{
    Q_OBJECT

public:
    explicit AcrylicMenu(QWidget *parent = nullptr);
    explicit AcrylicMenu(const QString &title, QWidget *parent = nullptr);
};

class FQW_API AcrylicCheckableMenu : public CheckableMenu
{
    Q_OBJECT

public:
    explicit AcrylicCheckableMenu(QWidget *parent = nullptr);
    explicit AcrylicCheckableMenu(const QString &title, QWidget *parent = nullptr);
    explicit AcrylicCheckableMenu(const QString &title, QWidget *parent, MenuIndicatorType indicatorType);
};

class FQW_API AcrylicSystemTrayMenu : public AcrylicMenu
{
    Q_OBJECT

public:
    explicit AcrylicSystemTrayMenu(QWidget *parent = nullptr);

    QSize sizeHint() const override;
};

class FQW_API AcrylicCheckableSystemTrayMenu : public AcrylicCheckableMenu
{
    Q_OBJECT

public:
    explicit AcrylicCheckableSystemTrayMenu(QWidget *parent = nullptr);
};

// --- LabelContextMenu ---
class FQW_API LabelContextMenu : public RoundMenu
{
    Q_OBJECT

public:
    explicit LabelContextMenu(QLabel *parent);

    QLabel *label() const;
    QString selectedText() const;
    QAction *copyAction() const;
    QAction *selectAllAction() const;
    void createActions();

    using RoundMenu::exec;
    QAction *exec(const QPoint &pos, bool animated = true,
                  MenuAnimationType animationType = MenuAnimationType::DropDown);

private:
    void copySelection();
    void selectAllText();

    QLabel *m_label = nullptr;
    QString m_selectedText;
    QAction *m_copyAction = nullptr;
    QAction *m_selectAllAction = nullptr;
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::MenuAnimationType)
Q_DECLARE_METATYPE(FluentQt::MenuIndicatorType)
