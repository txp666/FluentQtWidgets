#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QModelIndex>
#include <QtCore/QPoint>
#include <QtCore/QString>
#include <QtCore/QTimer>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGraphicsDropShadowEffect>
#include <QtWidgets/QHBoxLayout>

class QEvent;
class QHelpEvent;
class QHideEvent;
class QLabel;
class QPropertyAnimation;
class QShowEvent;
class QStyleOptionViewItem;
class QAbstractItemView;
class QWidget;

namespace FluentQt {

enum class ToolTipPosition
{
    Top,
    Bottom,
    Left,
    Right,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight
};

class FQW_API ToolTip : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(QString text READ text WRITE setText)
    Q_PROPERTY(int duration READ duration WRITE setDuration)
    Q_PROPERTY(QFrame *container READ container)
    Q_PROPERTY(QHBoxLayout *containerLayout READ containerLayout)
    Q_PROPERTY(QLabel *label READ label)
    Q_PROPERTY(QPropertyAnimation *opacityAni READ opacityAni)
    Q_PROPERTY(QGraphicsDropShadowEffect *shadowEffect READ shadowEffect)

  public:
    explicit ToolTip(QWidget *parent = nullptr);
    explicit ToolTip(const QString &text, QWidget *parent = nullptr);

    QString text() const;
    int duration() const;
    QFrame *container() const;
    QHBoxLayout *containerLayout() const;
    QLabel *label() const;
    QPropertyAnimation *opacityAni() const;
    QGraphicsDropShadowEffect *shadowEffect() const;
    void setText(const QString &text);
    void setDuration(int durationMs);
    void adjustPos(QWidget *target, ToolTipPosition position = ToolTipPosition::Top);
    void showFor(QWidget *target, ToolTipPosition position = ToolTipPosition::Top, int durationMs = 1000);

    static ToolTip *showText(QWidget *target, const QString &text, ToolTipPosition position = ToolTipPosition::Top,
                             int durationMs = 1000);

  protected:
    void showEvent(QShowEvent *event) override;
    void hideEvent(QHideEvent *event) override;

  private:
    QPoint calculatePosition(QWidget *target, ToolTipPosition position) const;

    QFrame *m_container = nullptr;
    QHBoxLayout *m_containerLayout = nullptr;
    QLabel *m_label = nullptr;
    QTimer *m_timer = nullptr;
    QPropertyAnimation *m_opacityAnimation = nullptr;
    QGraphicsDropShadowEffect *m_shadowEffect = nullptr;
    int m_duration = 1000;
};

class FQW_API ToolTipFilter : public QObject
{
    Q_OBJECT

  public:
    explicit ToolTipFilter(QWidget *parent, int showDelayMs = 300,
                           ToolTipPosition position = ToolTipPosition::Top);
    explicit ToolTipFilter(const QString &text, ToolTipPosition position = ToolTipPosition::Top,
                           QObject *parent = nullptr);
    explicit ToolTipFilter(const QString &text, int showDelayMs, ToolTipPosition position,
                           QObject *parent = nullptr);

    int toolTipDelay() const;
    void setToolTipDelay(int delayMs);
    void hideToolTip();

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    virtual bool canShowToolTip(QWidget *widget) const;
    virtual ToolTip *createToolTip(const QString &text, QWidget *parent) const;

  private:
    void showToolTip();

    QString m_text;
    ToolTipPosition m_position = ToolTipPosition::Top;
    QTimer m_timer;
    ToolTip *m_toolTip = nullptr;
    QWidget *m_targetWidget = nullptr;
    int m_showDelayMs = 300;
    bool m_pointerInside = false;
};

class FQW_API AcrylicToolTip : public ToolTip
{
    Q_OBJECT

  public:
    explicit AcrylicToolTip(QWidget *parent = nullptr);
    explicit AcrylicToolTip(const QString &text, QWidget *parent = nullptr);
};

class FQW_API AcrylicToolTipFilter : public ToolTipFilter
{
    Q_OBJECT

  public:
    explicit AcrylicToolTipFilter(QWidget *parent, int showDelayMs = 300,
                                  ToolTipPosition position = ToolTipPosition::Top);
    explicit AcrylicToolTipFilter(const QString &text, ToolTipPosition position = ToolTipPosition::Top,
                                  QObject *parent = nullptr);
    explicit AcrylicToolTipFilter(const QString &text, int showDelayMs, ToolTipPosition position,
                                  QObject *parent = nullptr);

  protected:
    ToolTip *createToolTip(const QString &text, QWidget *parent) const override;
};

enum class ItemViewToolTipType
{
    List,
    Table,
    Tree
};

class FQW_API ItemViewToolTipDelegate : public QObject
{
    Q_OBJECT

  public:
    explicit ItemViewToolTipDelegate(QAbstractItemView *parent, int showDelayMs = 300,
                                     ItemViewToolTipType type = ItemViewToolTipType::Table);

    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option,
                   const QModelIndex &index);
    void setToolTipDelay(int delayMs);
    void setToolTipDuration(int durationMs);
    void hideToolTip();

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

  private:
    void showToolTip();
    QPoint calculatePosition(QAbstractItemView *view, const QRect &itemRect) const;

    QAbstractItemView *m_view = nullptr;
    ItemViewToolTipType m_type = ItemViewToolTipType::Table;
    ToolTip *m_toolTip = nullptr;
    QTimer m_timer;
    QString m_text;
    QModelIndex m_currentIndex;
    int m_showDelayMs = 300;
    int m_durationMs = -1;
    bool m_pointerInside = false;
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::ToolTipPosition)
Q_DECLARE_METATYPE(FluentQt::ItemViewToolTipType)
