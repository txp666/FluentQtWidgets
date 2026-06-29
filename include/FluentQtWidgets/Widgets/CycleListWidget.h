#pragma once

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Global.h>

#include <QtCore/QElapsedTimer>
#include <QtCore/QSize>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QToolButton>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class QEnterEvent;
#endif

namespace FluentQt {

class SmoothScrollBar;

class FQW_API ScrollButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(FluentQt::FluentIcon fluentIcon READ fluentIcon WRITE setFluentIcon)
    Q_PROPERTY(bool isPressed READ isPressed)

  public:
    explicit ScrollButton(FluentIcon icon, QWidget *parent = nullptr);

    FluentIcon fluentIcon() const;
    bool isPressed() const;

  public slots:
    void setFluentIcon(FluentQt::FluentIcon icon);

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

  private:
    FluentIcon m_icon = FluentIcon::ArrowDown;
    bool m_isPressed = false;
};

class FQW_API CycleListWidget : public QListWidget
{
    Q_OBJECT
    Q_PROPERTY(QSize itemSize READ itemSize WRITE setItemSize)
    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)
    Q_PROPERTY(int visibleNumber READ visibleNumber WRITE setVisibleNumber)
    Q_PROPERTY(int scrollDuration READ scrollDuration WRITE setScrollDuration)
    Q_PROPERTY(bool cycle READ isCycle)
    Q_PROPERTY(bool scrollButtonRepeatEnabled READ isScrollButtonRepeatEnabled WRITE setScrollButtonRepeatEnabled)

  public:
    explicit CycleListWidget(const QStringList &items = {}, const QSize &itemSize = QSize(80, 32),
                             Qt::Alignment alignment = Qt::AlignCenter, QWidget *parent = nullptr);

    QSize itemSize() const;
    Qt::Alignment alignment() const;
    int visibleNumber() const;
    int scrollDuration() const;
    bool isCycle() const;
    bool isScrollButtonRepeatEnabled() const;
    int currentCycleIndex() const;
    QStringList originItems() const;
    ScrollButton *upButton() const;
    ScrollButton *downButton() const;
    SmoothScrollBar *verticalSmoothScrollBar() const;

  signals:
    void cycleCurrentItemChanged(QListWidgetItem *item);
    void currentItemChanged(QListWidgetItem *item);

  public slots:
    void setItems(const QStringList &items);
    void setItemSize(const QSize &size);
    void setAlignment(Qt::Alignment alignment);
    void setVisibleNumber(int visibleNumber);
    void setScrollDuration(int durationMs);
    void setSelectedItem(const QString &text);
    void setScrollButtonRepeatEnabled(bool enabled);
    void setCurrentIndex(int index);
    void scrollDown();
    void scrollUp();
    void scrollToItem(QListWidgetItem *item, QAbstractItemView::ScrollHint hint = QAbstractItemView::PositionAtCenter);

  protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

  private:
    void createItems(const QStringList &items);
    void addColumnItems(const QStringList &items, bool disabled = false);
    void setButtonsVisible(bool visible);
    void scrollWithAnimation(int index);
    void updateGeometryForVisibleItems();

    QSize m_itemSize = QSize(80, 32);
    Qt::Alignment m_alignment = Qt::AlignCenter;
    int m_visibleNumber = 9;
    int m_scrollDuration = 250;
    bool m_isCycle = false;
    bool m_scrollButtonRepeatEnabled = false;
    QStringList m_originItems;
    int m_currentIndex = 0;
    QElapsedTimer m_lastScrollTime;
    ScrollButton *m_upButton = nullptr;
    ScrollButton *m_downButton = nullptr;
    SmoothScrollBar *m_verticalSmoothScrollBar = nullptr;
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::ScrollButton *)
