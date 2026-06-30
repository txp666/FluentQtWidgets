#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Navigation/NavigationWidget.h>
#include <FluentQtWidgets/Navigation/NavigationTreeWidget.h>

#include <QtCore/QHash>
#include <QtCore/QRectF>
#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtWidgets/QFrame>

class QPropertyAnimation;
class QFrame;
class QPaintEvent;
class QVBoxLayout;
class QWidget;

namespace FluentQt {

class AcrylicBrush;
class NavigationIndicator;
class NavigationToolButton;
class ScrollArea;

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
    Q_PROPERTY(bool acrylicEnabled READ isAcrylicEnabled WRITE setAcrylicEnabled)

  public:
    static constexpr int kCompactWidth = 48;
    static constexpr int kExpandWidth = 322;

    explicit NavigationPanel(QWidget *parent = nullptr);
    ~NavigationPanel() override;

    NavigationDisplayMode displayMode() const;
    bool isMenu() const;
    bool isAcrylicEnabled() const;
    bool isIndicatorAnimationEnabled() const;
    int expandWidth() const;
    int minimumExpandWidth() const;

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
    NavigationToolButton *returnButton() const;

  public slots:
    void setMenu(bool menu);
    void setAcrylicEnabled(bool enabled);
    void setIndicatorAnimationEnabled(bool enabled);
    void setExpandWidth(int width);
    void setMinimumExpandWidth(int width);
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
    void paintEvent(QPaintEvent *event) override;

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
    void updateTransparentProperty();
    void updateAcrylicColor();
    bool canDrawAcrylic() const;
    void stopIndicatorAnimation();
    NavigationWidget *findIndicatorWidget(NavigationWidget *item) const;
    QRectF indicatorRectFor(NavigationWidget *item) const;
    void onItemClicked(NavigationWidget *item, bool triggeredByUser);
    void onReturnClicked();
    void onIndicatorAnimationFinished();
    void onExpandAnimationFinished();

    QPropertyAnimation *m_expandAnimation = nullptr;
    QVBoxLayout *m_mainLayout = nullptr;
    QVBoxLayout *m_topLayout = nullptr;
    QVBoxLayout *m_bottomLayout = nullptr;
    QVBoxLayout *m_scrollLayout = nullptr;
    QWidget *m_scrollWidget = nullptr;
    ScrollArea *m_scrollArea = nullptr;
    NavigationToolButton *m_menuButton = nullptr;
    NavigationToolButton *m_returnButton = nullptr;
    NavigationIndicator *m_indicator = nullptr;
    AcrylicBrush *m_acrylicBrush = nullptr;

    QHash<QString, NavigationItemEntry> m_items;
    QString m_currentRouteKey;
    NavigationDisplayMode m_displayMode = NavigationDisplayMode::Compact;
    bool m_menu = false;
    bool m_acrylicEnabled = false;
    bool m_indicatorAnimationEnabled = true;
    bool m_menuButtonVisible = true;
    bool m_returnButtonVisible = false;
    bool m_expandAnimationExpanding = false;
    int m_expandWidth = kExpandWidth;
    int m_minimumExpandWidth = 1008;
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::NavigationDisplayMode)
