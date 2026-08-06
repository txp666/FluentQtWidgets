#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Navigation/NavigationPanel.h>

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtWidgets/QFrame>

class QStackedWidget;
class QVBoxLayout;
class QWidget;
class QEvent;
class QResizeEvent;

namespace FluentQt {

class NavigationTreeWidget;
class PopUpAniStackedWidget;

class FQW_API NavigationInterface : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
    Q_PROPERTY(QString currentRouteKey READ currentRouteKey WRITE setCurrentRouteKey NOTIFY currentRouteKeyChanged)

  public:
    explicit NavigationInterface(QWidget *parent = nullptr, bool showReturnButton = false);

    int addPage(QWidget *page, const QString &title, const QIcon &icon = QIcon());
    int addPage(QWidget *page, const QString &title, const QIcon &icon, const QString &routeKey);
    int addPage(QWidget *page, const QString &title, const QIcon &icon, const QString &routeKey,
                NavigationItemPosition position);
    int addPage(QWidget *page, const QString &title, const QIcon &icon, const QString &routeKey,
                NavigationItemPosition position, const QString &parentRouteKey);
    NavigationTreeWidget *addItem(const QString &routeKey, const QIcon &icon, const QString &text,
                                  NavigationItemPosition position = NavigationItemPosition::Scroll,
                                  const QString &parentRouteKey = QString(), bool selectable = true);
    NavigationWidget *addWidget(const QString &routeKey, NavigationWidget *widget,
                                NavigationItemPosition position = NavigationItemPosition::Scroll,
                                const QString &parentRouteKey = QString());
    NavigationItemHeader *addItemHeader(const QString &text,
                                        NavigationItemPosition position = NavigationItemPosition::Scroll);
    NavigationUserCard *addUserCard(const QString &routeKey, const QIcon &avatar, const QString &title,
                                    const QString &subtitle = QString(),
                                    NavigationItemPosition position = NavigationItemPosition::Scroll);
    NavigationUserCard *addUserCard(const QString &routeKey, const QString &avatarPath, const QString &title,
                                    const QString &subtitle = QString(),
                                    NavigationItemPosition position = NavigationItemPosition::Scroll);
    QFrame *addSeparator(NavigationItemPosition position = NavigationItemPosition::Scroll);
    bool removePage(int index, bool deleteWidget = false);
    bool removePage(const QString &routeKey, bool deleteWidget = false);
    int count() const;
    int currentIndex() const;
    QString currentRouteKey() const;
    QWidget *currentWidget() const;
    QWidget *widget(int index) const;
    QWidget *widget(const QString &routeKey) const;
    int indexOf(QWidget *page) const;
    int indexOf(const QString &routeKey) const;
    QString routeKey(int index) const;
    QList<QString> routeKeys() const;
    bool contains(const QString &routeKey) const;
    QStackedWidget *stackedWidget() const;
    NavigationPanel *navigationPanel() const;
    int contentTopMargin() const;
    bool isAcrylicEnabled() const;

  public slots:
    void setCurrentIndex(int index);
    void setCurrentRouteKey(const QString &routeKey);
    void setContentTopMargin(int margin);
    void setAcrylicEnabled(bool enabled);
    void setMinimumExpandWidth(int width);

  signals:
    void currentIndexChanged(int index);
    void currentRouteKeyChanged(const QString &routeKey);
    void navigationItemClicked(int index, const QString &routeKey);

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

  private:
    QString ensureRouteKey(QWidget *page, const QString &routeKey) const;
    void onPanelItemClicked(const QString &routeKey);
    void onCustomNavigationItemClicked(const QString &routeKey);
    void ensureNavigationPanelOnTop();
    void updatePanelGeometry();
    void updateNavigationSpacerWidth();

    NavigationPanel *m_panel = nullptr;
    QWidget *m_navigationSpacer = nullptr;
    QWidget *m_contentWidget = nullptr;
    QVBoxLayout *m_contentLayout = nullptr;
    PopUpAniStackedWidget *m_stackedWidget = nullptr;
    QList<QString> m_routeOrder;
    int m_contentTopMargin = 0;
};

} // namespace FluentQt
