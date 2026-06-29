#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Navigation/NavigationWidget.h>
#include <FluentQtWidgets/Navigation/NavigationTreeWidget.h>

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtWidgets/QFrame>

class QPropertyAnimation;
class QFrame;
class QVBoxLayout;
class QWidget;

namespace FluentQt {

class ScrollArea;
class TransparentToolButton;

enum class NavigationDisplayMode
{
    Compact = 0,
    Expand = 1,
    Menu = 2
};

enum class NavigationItemPosition
{
    Top = 0,
    Scroll = 1,
    Bottom = 2
};

class FQW_API NavigationPanel : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(bool menu READ isMenu WRITE setMenu NOTIFY menuChanged)

  public:
    static constexpr int kCompactWidth = 48;
    static constexpr int kExpandWidth = 322;

    explicit NavigationPanel(QWidget *parent = nullptr);

    NavigationDisplayMode displayMode() const;
    bool isMenu() const;
    int expandWidth() const;

    NavigationTreeWidget *addItem(const QString &routeKey, const QIcon &icon, const QString &text,
                                  NavigationItemPosition position = NavigationItemPosition::Scroll,
                                  const QString &parentRouteKey = QString(), bool selectable = true);
    NavigationTreeWidget *insertItem(int index, const QString &routeKey, const QIcon &icon, const QString &text,
                                     NavigationItemPosition position = NavigationItemPosition::Scroll,
                                     const QString &parentRouteKey = QString(), bool selectable = true);
    NavigationWidget *addWidget(const QString &routeKey, NavigationWidget *widget,
                                NavigationItemPosition position = NavigationItemPosition::Scroll,
                                const QString &parentRouteKey = QString());
    NavigationWidget *insertWidget(int index, const QString &routeKey, NavigationWidget *widget,
                                   NavigationItemPosition position = NavigationItemPosition::Scroll,
                                   const QString &parentRouteKey = QString());
    NavigationItemHeader *addItemHeader(const QString &text,
                                        NavigationItemPosition position = NavigationItemPosition::Scroll);
    NavigationItemHeader *insertItemHeader(int index, const QString &text,
                                           NavigationItemPosition position = NavigationItemPosition::Scroll);
    NavigationUserCard *addUserCard(const QString &routeKey, const QIcon &avatar, const QString &title,
                                    const QString &subtitle = QString(),
                                    NavigationItemPosition position = NavigationItemPosition::Scroll);
    NavigationUserCard *addUserCard(const QString &routeKey, const QString &avatarPath, const QString &title,
                                    const QString &subtitle = QString(),
                                    NavigationItemPosition position = NavigationItemPosition::Scroll);
    QFrame *addSeparator(NavigationItemPosition position = NavigationItemPosition::Scroll);
    NavigationTreeWidget *widget(const QString &routeKey) const;
    NavigationWidget *navigationWidget(const QString &routeKey) const;
    bool contains(const QString &routeKey) const;
    QString currentRouteKey() const;

    bool isMenuButtonVisible() const;
    bool isReturnButtonVisible() const;
    TransparentToolButton *returnButton() const;

  public slots:
    void setMenu(bool menu);
    void setExpandWidth(int width);
    void setMenuButtonVisible(bool visible);
    void setReturnButtonVisible(bool visible);
    void setCurrentItem(const QString &routeKey);
    void removeWidget(const QString &routeKey);
    void expand(bool animated = true);
    void collapse(bool animated = true);
    void toggle();

  signals:
    void displayModeChanged(FluentQt::NavigationDisplayMode mode);
    void menuChanged(bool menu);
    void itemClicked(const QString &routeKey);
    void returnRequested();

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

  private:
    struct NavigationItemEntry
    {
        QString routeKey;
        QString parentRouteKey;
        NavigationWidget *widget = nullptr;
    };

    void initLayout();
    void insertWidgetToLayout(int index, NavigationWidget *widget, NavigationItemPosition position);
    void setDisplayMode(NavigationDisplayMode mode);
    void setItemsCompacted(bool compacted);
    void updatePanelWidth(int width);
    void onItemClicked(NavigationWidget *item, bool triggeredByUser);
    void onReturnClicked();
    void onExpandAnimationFinished();

    QPropertyAnimation *m_expandAnimation = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
    QVBoxLayout *m_topLayout = nullptr;
    QVBoxLayout *m_bottomLayout = nullptr;
    QVBoxLayout *m_scrollLayout = nullptr;
    QWidget *m_scrollWidget = nullptr;
    ScrollArea *m_scrollArea = nullptr;
    TransparentToolButton *m_menuButton = nullptr;
    TransparentToolButton *m_returnButton = nullptr;

    QHash<QString, NavigationItemEntry> m_items;
    QString m_currentRouteKey;
    NavigationDisplayMode m_displayMode = NavigationDisplayMode::Compact;
    bool m_menu = false;
    bool m_menuButtonVisible = true;
    bool m_returnButtonVisible = false;
    int m_expandWidth = kExpandWidth;
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::NavigationDisplayMode)
