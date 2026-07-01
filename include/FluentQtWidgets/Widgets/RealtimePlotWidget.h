#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QColor>
#include <QtWidgets/QWidget>

class QEvent;
class QMouseEvent;
class QPainter;
class QPaintEvent;
class QWheelEvent;

namespace FluentQt {

class FQW_API RealtimePlotWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int capacity READ capacity WRITE setCapacity NOTIFY capacityChanged)
    Q_PROPERTY(int sampleCount READ sampleCount NOTIFY samplesChanged)
    Q_PROPERTY(int seriesCount READ seriesCount NOTIFY seriesChanged)
    Q_PROPERTY(qreal visibleSpan READ visibleSpan WRITE setVisibleSpan NOTIFY rangeChanged)
    Q_PROPERTY(qreal xMinimum READ xMinimum WRITE setXMinimum NOTIFY rangeChanged)
    Q_PROPERTY(qreal xMaximum READ xMaximum WRITE setXMaximum NOTIFY rangeChanged)
    Q_PROPERTY(qreal yMinimum READ yMinimum WRITE setYMinimum NOTIFY rangeChanged)
    Q_PROPERTY(qreal yMaximum READ yMaximum WRITE setYMaximum NOTIFY rangeChanged)
    Q_PROPERTY(bool autoScroll READ autoScroll WRITE setAutoScroll NOTIFY interactionChanged)
    Q_PROPERTY(bool autoYRange READ autoYRange WRITE setAutoYRange NOTIFY rangeChanged)
    Q_PROPERTY(bool gridVisible READ isGridVisible WRITE setGridVisible NOTIFY appearanceChanged)
    Q_PROPERTY(bool fillVisible READ isFillVisible WRITE setFillVisible NOTIFY appearanceChanged)
    Q_PROPERTY(bool pointsVisible READ arePointsVisible WRITE setPointsVisible NOTIFY appearanceChanged)
    Q_PROPERTY(bool crosshairVisible READ isCrosshairVisible WRITE setCrosshairVisible NOTIFY appearanceChanged)
    Q_PROPERTY(bool legendVisible READ isLegendVisible WRITE setLegendVisible NOTIFY appearanceChanged)
    Q_PROPERTY(QColor curveColor READ curveColor WRITE setCurveColor NOTIFY appearanceChanged)

  public:
    explicit RealtimePlotWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    QVector<QPointF> points() const;
    QVector<QPointF> points(int seriesIndex) const;
    int capacity() const;
    int sampleCount() const;
    int seriesCount() const;
    qreal visibleSpan() const;
    qreal xMinimum() const;
    qreal xMaximum() const;
    qreal yMinimum() const;
    qreal yMaximum() const;
    bool autoScroll() const;
    bool autoYRange() const;
    bool isGridVisible() const;
    bool isFillVisible() const;
    bool arePointsVisible() const;
    bool isCrosshairVisible() const;
    bool isLegendVisible() const;
    QColor curveColor() const;
    QString seriesName(int seriesIndex) const;
    QColor seriesColor(int seriesIndex) const;
    bool isSeriesVisible(int seriesIndex) const;

  public slots:
    int addSeries(const QString &name = QString(), const QColor &color = QColor());
    void removeSeries(int seriesIndex);
    void clearSeries();
    void setSeriesName(int seriesIndex, const QString &name);
    void setSeriesColor(int seriesIndex, const QColor &color);
    void setSeriesVisible(int seriesIndex, bool visible);
    void setCapacity(int capacity);
    void setVisibleSpan(qreal span);
    void setXMinimum(qreal minimum);
    void setXMaximum(qreal maximum);
    void setXRange(qreal minimum, qreal maximum);
    void setYMinimum(qreal minimum);
    void setYMaximum(qreal maximum);
    void setYRange(qreal minimum, qreal maximum);
    void setAutoScroll(bool enabled);
    void setAutoYRange(bool enabled);
    void setGridVisible(bool visible);
    void setFillVisible(bool visible);
    void setPointsVisible(bool visible);
    void setCrosshairVisible(bool visible);
    void setLegendVisible(bool visible);
    void setCurveColor(const QColor &color);
    void appendSample(qreal y);
    void appendSample(int seriesIndex, qreal y);
    void appendSamples(const QVector<qreal> &samples);
    void appendSamples(int seriesIndex, const QVector<qreal> &samples);
    void appendPoint(qreal x, qreal y);
    void appendPoint(int seriesIndex, qreal x, qreal y);
    void setSamples(const QVector<qreal> &samples);
    void setSamples(int seriesIndex, const QVector<qreal> &samples);
    void clear();
    void resetView();

  signals:
    void samplesChanged();
    void capacityChanged(int capacity);
    void seriesChanged();
    void rangeChanged();
    void appearanceChanged();
    void interactionChanged();
    void crosshairMoved(const QPointF &point);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void leaveEvent(QEvent *event) override;

  private:
    struct PlotSeries
    {
        QString name;
        QColor color;
        QVector<QPointF> buffer;
        int start = 0;
        int count = 0;
        qreal nextX = 0;
        bool visible = true;
    };

    bool hasSeries(int seriesIndex) const;
    bool appendPointInternal(int seriesIndex, qreal x, qreal y);
    QPointF pointAt(int seriesIndex, int index) const;
    QRectF plotRect() const;
    qreal viewXMinimum() const;
    qreal viewXMaximum() const;
    void visibleYRange(qreal xMinimum, qreal xMaximum, qreal *minimum, qreal *maximum) const;
    QPointF mapToPlot(const QPointF &point, const QRectF &plot, qreal xMinimum, qreal xMaximum,
                      qreal yMinimum, qreal yMaximum) const;
    QPointF mapFromPlot(const QPointF &position, const QRectF &plot, qreal xMinimum, qreal xMaximum,
                        qreal yMinimum, qreal yMaximum) const;
    QColor panelColor() const;
    QColor plotColor() const;
    QColor textColor() const;
    QColor gridColor() const;
    QColor effectiveSeriesColor(int seriesIndex) const;
    void drawLegend(QPainter *painter, const QRectF &plot);

    QVector<PlotSeries> m_series;
    QVector<QRectF> m_legendToggleRects;
    int m_capacity = 120000;
    qreal m_visibleSpan = 600;
    qreal m_xMinimum = 0;
    qreal m_xMaximum = 600;
    qreal m_yMinimum = -1;
    qreal m_yMaximum = 1;
    bool m_autoScroll = true;
    bool m_autoYRange = true;
    bool m_gridVisible = true;
    bool m_fillVisible = true;
    bool m_pointsVisible = false;
    bool m_crosshairVisible = true;
    bool m_legendVisible = true;
    QColor m_curveColor;
    bool m_dragging = false;
    bool m_hasHover = false;
    QPointF m_hoverPosition;
    QPointF m_dragStartPosition;
    qreal m_dragStartXMinimum = 0;
    qreal m_dragStartXMaximum = 0;
};

} // namespace FluentQt
