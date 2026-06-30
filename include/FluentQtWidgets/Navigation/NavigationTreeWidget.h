#pragma once

#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Navigation/NavigationWidget.h>

#include <QtCore/QList>
#include <QtCore/QString>
#include <QtGui/QIcon>
#include <QtWidgets/QWidget>

class QMouseEvent;
class QPropertyAnimation;

namespace FluentQt {

class FQW_API NavigationToolButton : public NavigationWidget
{
    Q_OBJECT

  public:
    explicit NavigationToolButton(const QIcon &icon, QWidget *parent = nullptr);

    QIcon icon() const;

  public slots:
    void setIcon(const QIcon &icon);
    void setCompacted(bool compacted) override;

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

  private:
    QIcon m_icon;
};

class FQW_API NavigationTreeWidget : public NavigationWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal arrowAngle READ arrowAngle WRITE setArrowAngle)

  public:
    explicit NavigationTreeWidget(const QIcon &icon, const QString &text, bool selectable = true,
                                  QWidget *parent = nullptr);

    QString text() const;
    QIcon icon() const;
    bool isExpanded() const;
    qreal arrowAngle() const;
    QList<NavigationTreeWidget *> childItems() const;

    void insertChild(int index, NavigationTreeWidget *child);
    void removeChild(NavigationTreeWidget *child);

  public slots:
    void setText(const QString &text);
    void setIcon(const QIcon &icon);
    void setCompacted(bool compacted) override;
    void setExpanded(bool expanded);
    void setArrowAngle(qreal angle);

  protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

  private slots:
    void onExpandAnimationFinished();

  private:
    void updateFixedSize();
    void relayoutChildren();
    bool canDrawIndicator() const;

    QIcon m_icon;
    QString m_text;
    bool m_expanded = false;
    qreal m_arrowAngle = 0.0;
    NavigationTreeWidget *m_treeParent = nullptr;
    QList<NavigationTreeWidget *> m_children;

    QPropertyAnimation *m_expandAnimation = nullptr;
    QPropertyAnimation *m_arrowAnimation = nullptr;
};

} // namespace FluentQt
