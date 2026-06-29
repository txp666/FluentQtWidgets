#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Navigation/NavigationPanel.h>
#include <FluentQtWidgets/Navigation/NavigationWidget.h>

#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtWidgets/QWidget>

class QPaintEvent;
class QMouseEvent;
class QVBoxLayout;

namespace FluentQt {

class ScrollArea;

class FQW_API NavigationBarPushButton : public NavigationWidget
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(bool selectedTextVisible READ isSelectedTextVisible WRITE setSelectedTextVisible)

  public:
    explicit NavigationBarPushButton(const QIcon &icon, const QString &text, bool selectable = true,
                                     const QIcon &selectedIcon = QIcon(), QWidget *parent = nullptr);

    QString text() const;
    QIcon icon() const;
    QIcon selectedIcon() const;
    bool isSelectedTextVisible() const;
    QSize sizeHint() const override;

  public slots:
    void setText(const QString &text);
    void setIcon(const QIcon &icon);
    void setSelectedIcon(const QIcon &icon);
    void setSelectedTextVisible(bool visible);
    void setSelectedColor(const QColor &light, const QColor &dark);

  protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

  private:
    QColor selectedColor() const;

    QIcon m_icon;
    QIcon m_selectedIcon;
    QString m_text;
    QColor m_lightSelectedColor;
    QColor m_darkSelectedColor;
    bool m_selectedTextVisible = true;
};

class FQW_API NavigationBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentRouteKey READ currentRouteKey WRITE setCurrentItem NOTIFY currentItemChanged)
    Q_PROPERTY(bool selectedTextVisible READ isSelectedTextVisible WRITE setSelectedTextVisible)

  public:
    static constexpr int kBarWidth = 72;

    explicit NavigationBar(QWidget *parent = nullptr);

    NavigationBarPushButton *addItem(const QString &routeKey, const QIcon &icon, const QString &text,
                                     bool selectable = true, const QIcon &selectedIcon = QIcon(),
                                     NavigationItemPosition position = NavigationItemPosition::Top);
    NavigationBarPushButton *insertItem(int index, const QString &routeKey, const QIcon &icon, const QString &text,
                                        bool selectable = true, const QIcon &selectedIcon = QIcon(),
                                        NavigationItemPosition position = NavigationItemPosition::Top);
    NavigationWidget *addWidget(const QString &routeKey, NavigationWidget *widget,
                                NavigationItemPosition position = NavigationItemPosition::Top);
    NavigationWidget *insertWidget(int index, const QString &routeKey, NavigationWidget *widget,
                                   NavigationItemPosition position = NavigationItemPosition::Top);
    void removeWidget(const QString &routeKey);

    NavigationWidget *widget(const QString &routeKey) const;
    bool contains(const QString &routeKey) const;
    QString currentRouteKey() const;
    bool isSelectedTextVisible() const;
    bool isIndicatorAnimationEnabled() const;

  public slots:
    void setCurrentItem(const QString &routeKey);
    void setSelectedTextVisible(bool visible);
    void setIndicatorAnimationEnabled(bool enabled);
    void setSelectedColor(const QColor &light, const QColor &dark);

  signals:
    void itemClicked(const QString &routeKey);
    void currentItemChanged(const QString &routeKey);

  private:
    void initLayout();
    void insertWidgetToLayout(int index, NavigationWidget *widget, NavigationItemPosition position);
    void onWidgetClicked(NavigationWidget *widget, bool triggeredByUser);

    QVBoxLayout *m_mainLayout = nullptr;
    QVBoxLayout *m_topLayout = nullptr;
    QVBoxLayout *m_bottomLayout = nullptr;
    QVBoxLayout *m_scrollLayout = nullptr;
    QWidget *m_scrollWidget = nullptr;
    ScrollArea *m_scrollArea = nullptr;
    QHash<QString, NavigationWidget *> m_items;
    QString m_currentRouteKey;
    QColor m_lightSelectedColor;
    QColor m_darkSelectedColor;
    bool m_selectedTextVisible = true;
    bool m_indicatorAnimationEnabled = true;
};

} // namespace FluentQt
