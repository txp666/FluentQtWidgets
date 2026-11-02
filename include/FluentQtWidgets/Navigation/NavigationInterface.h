#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Navigation/NavigationPanel.h>

#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtWidgets/QWidget>

class QEvent;
class QResizeEvent;

namespace FluentQt {

// Python's NavigationInterface: a NavigationPanel in a widget that is as wide as the panel when
// it is compact (48 px) or expanded in place, and stays 48 px while the panel opens as a menu over
// the content. It holds no pages: FluentWindow, or the caller, keeps the stack and switches it on
// the panel's itemClicked().
class FQW_API NavigationInterface : public QWidget
{
    Q_OBJECT

  public:
    explicit NavigationInterface(QWidget *parent = nullptr, bool showMenuButton = true, bool showReturnButton = false,
                                 bool collapsible = true);

    NavigationPanel *panel() const;

    NavigationTreeWidget *addItem(const QString &routeKey, const QIcon &icon, const QString &text,
                                  NavigationItemPosition position = NavigationItemPosition::Top,
                                  const QString &parentRouteKey = QString(), bool selectable = true);
    NavigationTreeWidget *insertItem(int index, const QString &routeKey, const QIcon &icon, const QString &text,
                                     NavigationItemPosition position = NavigationItemPosition::Top,
                                     const QString &parentRouteKey = QString(), bool selectable = true);
    NavigationWidget *addWidget(const QString &routeKey, NavigationWidget *widget,
                                NavigationItemPosition position = NavigationItemPosition::Top,
                                const QString &parentRouteKey = QString());
    NavigationWidget *insertWidget(int index, const QString &routeKey, NavigationWidget *widget,
                                   NavigationItemPosition position = NavigationItemPosition::Top,
                                   const QString &parentRouteKey = QString());
    QFrame *addSeparator(NavigationItemPosition position = NavigationItemPosition::Top);
    QFrame *insertSeparator(int index, NavigationItemPosition position = NavigationItemPosition::Top);
    NavigationItemHeader *addItemHeader(const QString &text,
                                        NavigationItemPosition position = NavigationItemPosition::Top);
    NavigationItemHeader *insertItemHeader(int index, const QString &text,
                                           NavigationItemPosition position = NavigationItemPosition::Top);
    NavigationUserCard *addUserCard(const QString &routeKey, const QIcon &avatar, const QString &title,
                                    const QString &subtitle = QString(),
                                    NavigationItemPosition position = NavigationItemPosition::Top);
    NavigationUserCard *addUserCard(const QString &routeKey, const QString &avatarPath, const QString &title,
                                    const QString &subtitle = QString(),
                                    NavigationItemPosition position = NavigationItemPosition::Top);
    NavigationWidget *widget(const QString &routeKey) const;

    bool isAcrylicEnabled() const;
    bool isIndicatorAnimationEnabled() const;

  public slots:
    void removeWidget(const QString &routeKey);
    void setCurrentItem(const QString &routeKey);
    void expand(bool animated = true);
    void toggle();
    void setExpandWidth(int width);
    void setMinimumExpandWidth(int width);
    void setMenuButtonVisible(bool visible);
    void setReturnButtonVisible(bool visible);
    void setCollapsible(bool collapsible);
    void setAcrylicEnabled(bool enabled);
    void setIndicatorAnimationEnabled(bool enabled);

  signals:
    void displayModeChanged(FluentQt::NavigationDisplayMode mode);

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private:
    void updateMinimumHeight();

    NavigationPanel *m_panel = nullptr;
};

} // namespace FluentQt
