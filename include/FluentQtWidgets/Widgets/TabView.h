#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QtGlobal>
#include <QtGui/QIcon>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

class QDragEnterEvent;
class QDragMoveEvent;
class QDropEvent;
class QEvent;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class QEnterEvent;
#endif
class QGraphicsDropShadowEffect;
class QHBoxLayout;
class QMouseEvent;
class QPropertyAnimation;
class QResizeEvent;
class QScrollArea;
class QStackedWidget;
class QToolButton;
class QVBoxLayout;
class QWheelEvent;

namespace FluentQt {

enum class TabCloseButtonDisplayMode
{
    Always,
    OnHover,
    Never
};

class FQW_API TabItem : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(QString routeKey READ routeKey WRITE setRouteKey)
    Q_PROPERTY(bool selected READ isSelected WRITE setSelected)
    Q_PROPERTY(FluentQt::TabCloseButtonDisplayMode closeButtonDisplayMode READ closeButtonDisplayMode WRITE
                   setCloseButtonDisplayMode)

  public:
    explicit TabItem(QWidget *parent = nullptr);
    TabItem(const QString &routeKey, const QString &text, const QIcon &icon = QIcon(), QWidget *parent = nullptr);

    QString routeKey() const;
    void setRouteKey(const QString &routeKey);

    bool isSelected() const;
    TabCloseButtonDisplayMode closeButtonDisplayMode() const;
    QToolButton *closeButton() const;
    bool isShadowEnabled() const;

  public slots:
    void setSelected(bool selected);
    void setCloseButtonDisplayMode(FluentQt::TabCloseButtonDisplayMode mode);
    void setShadowEnabled(bool enabled);
    void slideTo(int x, int durationMs = 250);

  signals:
    void closed();
    void doubleClicked();
    void dragStarted();

  protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private:
    void init();
    void updateCloseButtonVisibility();
    void updateShadowVisibility();

    QString m_routeKey;
    bool m_selected = false;
    bool m_hovered = false;
    bool m_shadowEnabled = true;
    bool m_dragStarted = false;
    QPoint m_dragStartPos;
    TabCloseButtonDisplayMode m_closeButtonDisplayMode = TabCloseButtonDisplayMode::Always;
    QToolButton *m_closeButton = nullptr;
    QGraphicsDropShadowEffect *m_shadowEffect = nullptr;
    QPropertyAnimation *m_slideAnimation = nullptr;
};

class FQW_API TabBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentChanged)
    Q_PROPERTY(bool tabsClosable READ tabsClosable WRITE setTabsClosable)
    Q_PROPERTY(bool addButtonVisible READ isAddButtonVisible WRITE setAddButtonVisible)
    Q_PROPERTY(bool movable READ isMovable WRITE setMovable)
    Q_PROPERTY(bool scrollable READ isScrollable WRITE setScrollable)
    Q_PROPERTY(bool tabShadowEnabled READ isTabShadowEnabled WRITE setTabShadowEnabled)

  public:
    explicit TabBar(QWidget *parent = nullptr);

    int addTab(const QString &routeKey, const QString &text, const QIcon &icon = QIcon());
    int insertTab(int index, const QString &routeKey, const QString &text, const QIcon &icon = QIcon());
    void removeTab(int index);
    void removeTabByKey(const QString &routeKey);
    void clear();

    int count() const;
    int currentIndex() const;
    QString currentRouteKey() const;
    TabItem *currentTab() const;
    int indexOf(const QString &routeKey) const;
    bool contains(const QString &routeKey) const;
    TabItem *tabItem(int index) const;
    TabItem *tab(const QString &routeKey) const;
    QList<QString> routeKeys() const;

    QString tabText(int index) const;
    void setTabText(int index, const QString &text);
    QIcon tabIcon(int index) const;
    void setTabIcon(int index, const QIcon &icon);
    QVariant tabData(int index) const;
    void setTabData(int index, const QVariant &data);
    bool isTabEnabled(int index) const;
    void setTabEnabled(int index, bool enabled);

    bool tabsClosable() const;
    void setTabsClosable(bool closable);
    TabCloseButtonDisplayMode closeButtonDisplayMode() const;
    void setCloseButtonDisplayMode(FluentQt::TabCloseButtonDisplayMode mode);
    int tabMaximumWidth() const;
    void setTabMaximumWidth(int width);
    bool isAddButtonVisible() const;
    QToolButton *addButton() const;
    bool isMovable() const;
    bool isScrollable() const;
    bool isTabShadowEnabled() const;

  public slots:
    void setCurrentIndex(int index);
    void setCurrentTab(const QString &routeKey);
    void setAddButtonVisible(bool visible);
    void setMovable(bool movable);
    void setScrollable(bool scrollable);
    void setTabShadowEnabled(bool enabled);

  signals:
    void currentChanged(int index);
    void tabBarClicked(int index);
    void tabBarDoubleClicked(int index);
    void tabCloseRequested(int index);
    void tabAddRequested();
    void tabMoved(int from, int to);

  protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

  private:
    void init();
    QString normalizeRouteKey(const QString &routeKey) const;
    void rebuildRouteMap();
    void updateSelectedItems();
    void connectItem(TabItem *item);
    void moveTab(int from, int to);
    void updateOverflowButtons();
    QWidget *tabStripWidget() const;

    QList<TabItem *> m_items;
    QHash<QString, TabItem *> m_itemMap;
    int m_currentIndex = -1;
    bool m_tabsClosable = true;
    bool m_movable = false;
    bool m_scrollable = false;
    bool m_tabShadowEnabled = true;
    int m_tabMaximumWidth = 240;
    TabCloseButtonDisplayMode m_closeButtonDisplayMode = TabCloseButtonDisplayMode::Always;
    QHBoxLayout *m_layout = nullptr;
    QHBoxLayout *m_itemLayout = nullptr;
    QScrollArea *m_scrollArea = nullptr;
    QToolButton *m_addButton = nullptr;
    QWidget *m_tabStrip = nullptr;
};

class FQW_API TabWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentChanged)
    Q_PROPERTY(bool movable READ isMovable WRITE setMovable)

  public:
    explicit TabWidget(QWidget *parent = nullptr);

    int addTab(QWidget *page, const QString &label, const QIcon &icon = QIcon(), const QString &routeKey = QString());
    int addPage(QWidget *page, const QString &label, const QIcon &icon = QIcon(), const QString &routeKey = QString());
    int insertTab(int index, QWidget *page, const QString &label, const QIcon &icon = QIcon(),
                  const QString &routeKey = QString());
    void removeTab(int index);
    void clear();

    int count() const;
    int currentIndex() const;
    QWidget *currentWidget() const;
    QWidget *widget(int index) const;
    int indexOf(QWidget *widget) const;
    QString tabText(int index) const;
    void setTabText(int index, const QString &text);
    QIcon tabIcon(int index) const;
    void setTabIcon(int index, const QIcon &icon);
    QString routeKey(int index) const;

    TabBar *tabBar() const;
    QStackedWidget *stackedWidget() const;
    bool tabsClosable() const;
    void setTabsClosable(bool closable);
    void setAddButtonVisible(bool visible);
    bool isMovable() const;
    void setMovable(bool movable);

  public slots:
    void setCurrentIndex(int index);

  signals:
    void currentChanged(int index);
    void tabBarClicked(int index);
    void tabBarDoubleClicked(int index);
    void tabCloseRequested(int index);
    void tabAddRequested();

  private:
    void init();
    QString ensureRouteKey(QWidget *page, const QString &routeKey) const;

    TabBar *m_tabBar = nullptr;
    QStackedWidget *m_stackedWidget = nullptr;
    QVBoxLayout *m_layout = nullptr;
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::TabCloseButtonDisplayMode)
