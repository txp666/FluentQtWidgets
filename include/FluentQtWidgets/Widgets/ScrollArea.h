#pragma once

#include <FluentQtWidgets/FluentIcon.h>
#include <FluentQtWidgets/Global.h>
#include <FluentQtWidgets/Theme.h>

#include <QtCore/QEasingCurve>
#include <QtCore/QObject>
#include <QtCore/QtGlobal>
#include <QtGui/QColor>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QWidget>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
class QEnterEvent;
#endif
class QKeyEvent;
class QMouseEvent;
class QPaintEvent;
class QPropertyAnimation;
class QResizeEvent;
class QWheelEvent;
class QAbstractScrollArea;
class QScrollBar;

namespace FluentQt {

class SmoothScrollDelegate;

enum class ScrollBarHandleDisplayMode
{
    Always,
    OnHover
};

class FQW_API ArrowButton : public QToolButton
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(QColor lightColor READ lightColor WRITE setLightColor)
    Q_PROPERTY(QColor darkColor READ darkColor WRITE setDarkColor)

  public:
    explicit ArrowButton(FluentIcon icon, QWidget *parent = nullptr);

    FluentIcon iconType() const;
    qreal opacity() const;
    QColor lightColor() const;
    QColor darkColor() const;

  public slots:
    void setOpacity(qreal opacity);
    void setLightColor(const QColor &color);
    void setDarkColor(const QColor &color);

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    FluentIcon m_icon = FluentIcon::ArrowDown;
    qreal m_opacity = 1.0;
    QColor m_lightColor = QColor(0, 0, 0, 114);
    QColor m_darkColor = QColor(255, 255, 255, 139);
};

class FQW_API ScrollBarGroove : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged)
    Q_PROPERTY(QColor lightBackgroundColor READ lightBackgroundColor WRITE setLightBackgroundColor)
    Q_PROPERTY(QColor darkBackgroundColor READ darkBackgroundColor WRITE setDarkBackgroundColor)

  public:
    explicit ScrollBarGroove(Qt::Orientation orientation, QWidget *parent = nullptr);

    Qt::Orientation orientation() const;
    qreal opacity() const;
    QColor lightBackgroundColor() const;
    QColor darkBackgroundColor() const;
    ArrowButton *upButton() const;
    ArrowButton *downButton() const;

  signals:
    void opacityChanged(qreal opacity);

  public slots:
    void setOpacity(qreal opacity);
    void setLightBackgroundColor(const QColor &color);
    void setDarkBackgroundColor(const QColor &color);
    void fadeIn();
    void fadeOut();

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    Qt::Orientation m_orientation = Qt::Vertical;
    qreal m_opacity = 1.0;
    QColor m_lightBackgroundColor = QColor(252, 252, 252, 217);
    QColor m_darkBackgroundColor = QColor(44, 44, 44, 245);
    ArrowButton *m_upButton = nullptr;
    ArrowButton *m_downButton = nullptr;
    QPropertyAnimation *m_opacityAnimation = nullptr;
};

class FQW_API ScrollBarHandle : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(QColor lightColor READ lightColor WRITE setLightColor)
    Q_PROPERTY(QColor darkColor READ darkColor WRITE setDarkColor)

  public:
    explicit ScrollBarHandle(Qt::Orientation orientation, QWidget *parent = nullptr);

    Qt::Orientation orientation() const;
    qreal opacity() const;
    QColor lightColor() const;
    QColor darkColor() const;

  public slots:
    void setOpacity(qreal opacity);
    void setLightColor(const QColor &color);
    void setDarkColor(const QColor &color);
    void fadeIn();
    void fadeOut();

  protected:
    void paintEvent(QPaintEvent *event) override;

  private:
    Qt::Orientation m_orientation = Qt::Vertical;
    qreal m_opacity = 1.0;
    QColor m_lightColor = QColor(0, 0, 0, 114);
    QColor m_darkColor = QColor(255, 255, 255, 139);
    QPropertyAnimation *m_opacityAnimation = nullptr;
};

class FQW_API ScrollBar : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int val READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool forceHidden READ isForceHidden WRITE setForceHidden)
    Q_PROPERTY(qreal paintOpacity READ paintOpacity WRITE setPaintOpacity)

  public:
    explicit ScrollBar(QWidget *parent = nullptr);
    explicit ScrollBar(Qt::Orientation orientation, QWidget *parent = nullptr);

    int value() const;
    int minimum() const;
    int maximum() const;
    int pageStep() const;
    int singleStep() const;
    Qt::Orientation orientation() const;
    bool isSliderDown() const;
    ScrollBarHandleDisplayMode handleDisplayMode() const;
    bool isForceHidden() const;
    qreal paintOpacity() const;
    ScrollBarGroove *groove() const;
    ScrollBarHandle *handle() const;
    ArrowButton *upButton() const;
    ArrowButton *downButton() const;

    void attachToScrollArea(QAbstractScrollArea *area, Qt::Orientation orientation);
    QColor handleColor(Theme theme = Theme::Auto) const;
    QColor arrowColor(Theme theme = Theme::Auto) const;
    QColor grooveColor(Theme theme = Theme::Auto) const;

  signals:
    void rangeChanged(int minimum, int maximum);
    void valueChanged(int value);
    void sliderPressed();
    void sliderReleased();
    void sliderMoved();

  public slots:
    void setValue(int value);
    void setMinimum(int minimum);
    void setMaximum(int maximum);
    void setRange(int minimum, int maximum);
    void setPageStep(int step);
    void setSingleStep(int step);
    void setSliderDown(bool down);
    void setHandleDisplayMode(FluentQt::ScrollBarHandleDisplayMode mode);
    void setForceHidden(bool hidden);
    void setPaintOpacity(qreal opacity);
    void setHandleColor(const QColor &light, const QColor &dark);
    void setArrowColor(const QColor &light, const QColor &dark);
    void setGrooveColor(const QColor &light, const QColor &dark);
    void expand();
    void collapse();

  protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

  private:
    void init();
    void initOverlay();
    void updateExpanded(bool expanded);
    void adjustOverlayGeometry(const QSize &size);
    void syncFromPartner(int value);
    void fadeTo(qreal opacity);
    void updateCustomStyleSheet();
    void updateVisibility();
    void updateHandleGeometry();
    int grooveLength() const;
    int slideLength() const;
    int handleLength() const;
    QRect handleRect() const;
    QRect subLineRect() const;
    QRect addLineRect() const;
    bool isSlideRegion(const QPoint &pos) const;
    int valueFromHandleOffset(int offset) const;

    Qt::Orientation m_orientation = Qt::Vertical;
    int m_minimum = 0;
    int m_maximum = 0;
    int m_value = 0;
    int m_pageStep = 50;
    int m_singleStep = 1;
    int m_padding = 14;
    bool m_sliderDown = false;
    QPoint m_pressedPos;
    ScrollBarHandleDisplayMode m_handleDisplayMode = ScrollBarHandleDisplayMode::Always;
    bool m_forceHidden = false;
    bool m_overlayMode = false;
    bool m_isEnter = false;
    qreal m_paintOpacity = 1.0;
    QColor m_lightHandleColor = QColor(0, 0, 0, 114);
    QColor m_darkHandleColor = QColor(255, 255, 255, 139);
    QColor m_lightArrowColor = QColor(0, 0, 0, 114);
    QColor m_darkArrowColor = QColor(255, 255, 255, 139);
    QColor m_lightGrooveColor = QColor(252, 252, 252, 217);
    QColor m_darkGrooveColor = QColor(44, 44, 44, 245);
    ScrollBarGroove *m_groove = nullptr;
    ScrollBarHandle *m_handle = nullptr;
    QAbstractScrollArea *m_scrollArea = nullptr;
    QScrollBar *m_partnerBar = nullptr;
    QPropertyAnimation *m_opacityAnimation = nullptr;
};

class FQW_API SmoothScrollBar : public ScrollBar
{
    Q_OBJECT
    Q_PROPERTY(int val READ value WRITE setAnimatedValue)

  public:
    explicit SmoothScrollBar(QWidget *parent = nullptr);
    explicit SmoothScrollBar(Qt::Orientation orientation, QWidget *parent = nullptr);

    int scrollAnimationDuration() const;
    QEasingCurve::Type scrollAnimationEasing() const;

    using ScrollBar::setValue;

  public slots:
    void setScrollAnimation(int durationMs, QEasingCurve::Type easing = QEasingCurve::OutCubic);
    void setAnimatedValue(int value);
    void setValueWithAnimation(int value, bool useAnimation = true);
    void scrollValue(int delta, bool useAnimation = true);
    void scrollTo(int value, bool useAnimation = true);
    void resetValue(int value);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

  private:
    void applyValue(int value, bool useAnimation);

    int m_scrollAnimationDuration = 500;
    QEasingCurve::Type m_scrollAnimationEasing = QEasingCurve::OutCubic;
    int m_accumulatedValue = 0;
    QPropertyAnimation *m_scrollAnimation = nullptr;
};

class FQW_API ScrollArea : public QScrollArea
{
    Q_OBJECT
    Q_PROPERTY(FluentQt::ScrollBar *vScrollBar READ vScrollBar)
    Q_PROPERTY(FluentQt::ScrollBar *hScrollBar READ hScrollBar)
    Q_PROPERTY(FluentQt::SmoothScrollDelegate *scrollDelegate READ scrollDelegate)
    Q_PROPERTY(FluentQt::SmoothScrollDelegate *scrollDelagate READ scrollDelagate)

  public:
    explicit ScrollArea(QWidget *parent = nullptr);

    ScrollBar *verticalFluentScrollBar() const;
    ScrollBar *horizontalFluentScrollBar() const;
    ScrollBar *vScrollBar() const;
    ScrollBar *hScrollBar() const;
    SmoothScrollDelegate *scrollDelegate() const;
    SmoothScrollDelegate *scrollDelagate() const;

  public slots:
    void enableTransparentBackground(bool enabled = true);
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);

  private:
    void init();

  protected:
    ScrollBar *m_verticalScrollBar = nullptr;
    ScrollBar *m_horizontalScrollBar = nullptr;
    SmoothScrollDelegate *m_scrollDelegate = nullptr;
};

class FQW_API SingleDirectionScrollArea : public ScrollArea
{
    Q_OBJECT
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation)

  public:
    explicit SingleDirectionScrollArea(QWidget *parent = nullptr);
    explicit SingleDirectionScrollArea(Qt::Orientation orientation, QWidget *parent = nullptr);

    Qt::Orientation orientation() const;

  public slots:
    void setOrientation(Qt::Orientation orientation);

  protected:
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

  private:
    void updatePolicies();

    Qt::Orientation m_orientation = Qt::Vertical;
};

class FQW_API SmoothScrollArea : public ScrollArea
{
    Q_OBJECT
    Q_PROPERTY(FluentQt::SmoothScrollDelegate *delegate READ delegate)

  public:
    explicit SmoothScrollArea(QWidget *parent = nullptr);

    SmoothScrollDelegate *delegate() const;
    int scrollAnimationDuration(Qt::Orientation orientation) const;
    QEasingCurve::Type scrollAnimationEasing(Qt::Orientation orientation) const;

  public slots:
    void setScrollAnimation(Qt::Orientation orientation, int durationMs,
                            QEasingCurve::Type easing = QEasingCurve::OutCubic);

  private:
    SmoothScrollDelegate *m_smoothDelegate = nullptr;
};

} // namespace FluentQt

Q_DECLARE_METATYPE(FluentQt::ScrollBarHandleDisplayMode)
