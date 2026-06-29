#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QPoint>
#include <QtCore/QPropertyAnimation>
#include <QtGui/QEnterEvent>
#include <QtGui/QColor>
#include <QtWidgets/QProxyStyle>
#include <QtWidgets/QSlider>

class QMouseEvent;

namespace FluentQt {

class FQW_API SliderHandle : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal radius READ radius WRITE setRadius)
    Q_PROPERTY(QColor lightHandleColor READ lightHandleColor WRITE setLightHandleColor)
    Q_PROPERTY(QColor darkHandleColor READ darkHandleColor WRITE setDarkHandleColor)

public:
    explicit SliderHandle(QWidget *parent = nullptr);

    qreal radius() const;
    QColor lightHandleColor() const;
    QColor darkHandleColor() const;
    void setRadius(qreal r);
    void setLightHandleColor(const QColor &color);
    void setDarkHandleColor(const QColor &color);
    void setHandleColor(const QColor &light, const QColor &dark);

signals:
    void pressed();
    void released();

protected:
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    void enterEvent(QEnterEvent *event) override;
#else
    void enterEvent(QEvent *event) override;
#endif
    void leaveEvent(QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private:
    void startAnimation(qreal targetRadius);

    qreal m_radius = 5.0;
    QColor m_lightHandleColor;
    QColor m_darkHandleColor;
    QPropertyAnimation *m_radiusAnimation = nullptr;
};

class FQW_API Slider : public QSlider
{
    Q_OBJECT
    Q_PROPERTY(QColor lightGrooveColor READ lightGrooveColor WRITE setLightGrooveColor)
    Q_PROPERTY(QColor darkGrooveColor READ darkGrooveColor WRITE setDarkGrooveColor)
    Q_PROPERTY(int grooveLength READ grooveLength)
    Q_PROPERTY(QPoint pressedPos READ pressedPos)

public:
    explicit Slider(QWidget *parent = nullptr);
    explicit Slider(Qt::Orientation orientation, QWidget *parent = nullptr);

    SliderHandle *handle() const;
    QPoint pressedPos() const;
    int grooveLength() const;
    QColor lightGrooveColor() const;
    QColor darkGrooveColor() const;
    void setLightGrooveColor(const QColor &color);
    void setDarkGrooveColor(const QColor &color);
    void setThemeColor(const QColor &light, const QColor &dark);
    void setOrientation(Qt::Orientation orientation);

signals:
    void clicked(int value);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void postInit();
    void adjustHandlePos();
    int posToValue(const QPoint &pos) const;
    void drawHorizonGroove(QPainter &painter);
    void drawVerticalGroove(QPainter &painter);

    SliderHandle *m_handle = nullptr;
    QColor m_lightGrooveColor;
    QColor m_darkGrooveColor;
    QPoint m_pressedPos;
    bool m_isDragging = false;
};

class FQW_API ClickableSlider : public QSlider
{
    Q_OBJECT

public:
    explicit ClickableSlider(QWidget *parent = nullptr);
    explicit ClickableSlider(Qt::Orientation orientation, QWidget *parent = nullptr);

signals:
    void clicked(int value);

protected:
    void mousePressEvent(QMouseEvent *event) override;
};

class FQW_API HollowHandleStyle : public QProxyStyle
{
public:
    struct Config
    {
        int grooveHeight = 3;
        QColor subPageColor{255, 255, 255};
        QColor addPageColor{255, 255, 255, 64};
        QColor handleColor{255, 255, 255};
        int handleRingWidth = 4;
        int handleHollowRadius = 6;
        int handleMargin = 4;
    };

    explicit HollowHandleStyle();
    explicit HollowHandleStyle(const Config &config);

    QRect subControlRect(ComplexControl cc, const QStyleOptionComplex *opt,
                         SubControl sc, const QWidget *widget) const override;
    void drawComplexControl(ComplexControl cc, const QStyleOptionComplex *opt,
                           QPainter *painter, const QWidget *widget) const override;

private:
    QSize handleSize() const;
    int sliderPositionFromValue(int min, int max, int val, int space) const;

    int m_grooveHeight = 3;
    QColor m_subPageColor{255, 255, 255};
    QColor m_addPageColor{255, 255, 255, 64};
    QColor m_handleColor{255, 255, 255};
    int m_handleRingWidth = 4;
    int m_handleHollowRadius = 6;
    int m_handleMargin = 4;
};

} // namespace FluentQt
