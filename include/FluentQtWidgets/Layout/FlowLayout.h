#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QEasingCurve>
#include <QtCore/QList>
#include <QtCore/QRect>
#include <QtCore/QSize>
#include <QtWidgets/QLayout>

class QEvent;
class QLayoutItem;
class QParallelAnimationGroup;
class QPropertyAnimation;
class QTimer;
class QWidget;

namespace FluentQt {

class FQW_API FlowLayout : public QLayout
{
    Q_OBJECT
    Q_PROPERTY(int horizontalSpacing READ horizontalSpacing WRITE setHorizontalSpacing)
    Q_PROPERTY(int verticalSpacing READ verticalSpacing WRITE setVerticalSpacing)
    Q_PROPERTY(Qt::Alignment verticalAlignment READ verticalAlignment WRITE setVerticalAlignment)
    Q_PROPERTY(bool animationEnabled READ isAnimationEnabled WRITE setAnimationEnabled)
    Q_PROPERTY(bool tight READ isTight WRITE setTight)

  public:
    explicit FlowLayout(QWidget *parent = nullptr, bool animation = false, bool tight = false);
    explicit FlowLayout(bool animation, QWidget *parent = nullptr);
    FlowLayout(int margin, int horizontalSpacing, int verticalSpacing, QWidget *parent = nullptr,
               bool animation = false, bool tight = false);
    ~FlowLayout() override;

    void addItem(QLayoutItem *item) override;
    void insertItem(int index, QLayoutItem *item);
    void insertWidget(int index, QWidget *widget);
    QWidget *removeWidget(QWidget *widget);
    QWidget *takeWidgetAt(int index);
    void removeAllWidgets();
    void takeAllWidgets();

    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Alignment verticalAlignment() const;
    bool isAnimationEnabled() const;
    bool isTight() const;
    int animationDuration() const;
    QEasingCurve::Type animationEasing() const;

    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;

  public slots:
    void setHorizontalSpacing(int spacing);
    void setVerticalSpacing(int spacing);
    void setVerticalAlignment(Qt::Alignment alignment);
    void setAnimationEnabled(bool enabled);
    void setTight(bool tight);
    void setAnimation(int durationMs, QEasingCurve::Type easing = QEasingCurve::Linear);

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    virtual int doLayout(const QRect &rect, bool move);
    bool shouldSkipItem(QLayoutItem *item) const;
    QSize itemLayoutSize(QLayoutItem *item) const;
    int alignedItemY(QLayoutItem *item, int rowY, int rowHeight, int itemHeight) const;
    void setupAnimationForItem(QLayoutItem *item, int index = -1);
    void removeAnimationForWidget(QWidget *widget);

    QList<QLayoutItem *> m_items;
    QList<QPropertyAnimation *> m_animations;
    QParallelAnimationGroup *m_animationGroup = nullptr;
    QTimer *m_debounceTimer = nullptr;
    QWidget *m_parentWidget = nullptr;
    int m_horizontalSpacing = 10;
    int m_verticalSpacing = 10;
    Qt::Alignment m_verticalAlignment = Qt::AlignTop;
    int m_duration = 300;
    QEasingCurve::Type m_easing = QEasingCurve::Linear;
    bool m_animationEnabled = false;
    bool m_tight = false;
};

class FQW_API AdaptiveFlowLayout : public FlowLayout
{
    Q_OBJECT
    Q_PROPERTY(int widgetMinimumWidth READ widgetMinimumWidth WRITE setWidgetMinimumWidth)
    Q_PROPERTY(int widgetMaximumWidth READ widgetMaximumWidth WRITE setWidgetMaximumWidth)

  public:
    explicit AdaptiveFlowLayout(QWidget *parent = nullptr, bool animation = false, bool tight = false);
    explicit AdaptiveFlowLayout(bool animation, QWidget *parent = nullptr);
    AdaptiveFlowLayout(int margin, int horizontalSpacing, int verticalSpacing, QWidget *parent = nullptr,
                       bool animation = false, bool tight = false);

    int widgetMinimumWidth() const;
    int widgetMaximumWidth() const;

  public slots:
    void setWidgetMinimumWidth(int width);
    void setWidgetMaximumWidth(int width);
    void clearWidgetMaximumWidth();

  protected:
    int doLayout(const QRect &rect, bool move) override;

  private:
    int m_widgetMinimumWidth = 200;
    int m_widgetMaximumWidth = -1;
};

class FQW_API ExpandLayout : public QLayout
{
    Q_OBJECT

  public:
    explicit ExpandLayout(QWidget *parent = nullptr);
    ~ExpandLayout() override;

    void addWidget(QWidget *widget);
    void addItem(QLayoutItem *item) override;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int width) const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QSize minimumSize() const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QLayoutItem *takeAt(int index) override;

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

  private:
    int doLayout(const QRect &rect, bool move) const;
    QList<QLayoutItem *> m_items;
};

} // namespace FluentQt
