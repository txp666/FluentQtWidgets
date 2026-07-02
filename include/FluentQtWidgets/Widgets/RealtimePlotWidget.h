#pragma once

#include <FluentQtWidgets/Global.h>

#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtGui/QColor>
#include <QtWidgets/QWidget>

class QEvent;
class QContextMenuEvent;
class QMouseEvent;
class QPainter;
class QPaintEvent;
class QTimer;
class QWheelEvent;

namespace FluentQt {

class FQW_API RealtimePlotWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(int capacity READ capacity WRITE setCapacity NOTIFY capacityChanged)
    Q_PROPERTY(int sampleCount READ sampleCount NOTIFY samplesChanged)
    Q_PROPERTY(int seriesCount READ seriesCount NOTIFY seriesChanged)
    Q_PROPERTY(int maximumVisiblePoints READ maximumVisiblePoints WRITE setMaximumVisiblePoints NOTIFY rangeChanged)
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
    Q_PROPERTY(int refreshRate READ refreshRate WRITE setRefreshRate NOTIFY refreshRateChanged)

  public:
    explicit RealtimePlotWidget(QWidget *parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    QVector<QPointF> points() const;
    QVector<QPointF> points(int seriesIndex) const;
    int capacity() const;
    int sampleCount() const;
    int seriesCount() const;
    int maximumVisiblePoints() const;
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
    int refreshRate() const;
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
    void setMaximumVisiblePoints(int count);
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
    void setRefreshRate(int framesPerSecond);
    void appendSample(qreal y);
    void appendSample(int seriesIndex, qreal y);
    void appendSamples(const QVector<qreal> &samples);
    void appendSamples(int seriesIndex, const QVector<qreal> &samples);
    void appendSamples(const QVector<QVector<qreal>> &seriesSamples);
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
    void refreshRateChanged(int refreshRate);
    void crosshairMoved(const QPointF &point);

  protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void leaveEvent(QEvent *event) override;

  private:
    struct PlotSeries
    {
        QString name;
        QColor color;
        QVector<QPointF> buffer;
        QVector<qreal> yBlockMinimums;
        QVector<qreal> yBlockMaximums;
        QVector<int> yBlockMinimumIndices;
        QVector<int> yBlockMaximumIndices;
        QVector<QPointF> renderCachePoints;
        QVector<QPointF> renderCacheFillPoints;
        int start = 0;
        int count = 0;
        qreal nextX = 0;
        quint64 dataRevision = 0;
        quint64 renderCacheRevision = 0;
        QRectF renderCachePlot;
        qreal renderCacheXMinimum = 0;
        qreal renderCacheXMaximum = 0;
        qreal renderCacheYMinimum = 0;
        qreal renderCacheYMaximum = 0;
        int renderCacheFirst = -1;
        int renderCacheLast = -1;
        int renderCachePixelWidth = 0;
        bool renderCachePerformanceMode = false;
        bool visible = true;
        bool xMonotonic = true;
    };

    bool hasSeries(int seriesIndex) const;
    void scheduleDataUpdate();
    void flushScheduledDataUpdate();
    int refreshInterval() const;
    bool shouldThrottleDataUpdates() const;
    void markSeriesRenderCacheDirty(PlotSeries *series);
    bool appendPointInternal(int seriesIndex, qreal x, qreal y);
    QPointF pointAt(int seriesIndex, int index) const;
    int nearestDataIndex(int seriesIndex, qreal x, int first, int last) const;
    void rebuildRenderCache(int seriesIndex, int first, int last, const QRectF &plot, qreal xMinimum,
                            qreal xMaximum, qreal yMinimum, qreal yMaximum, int pixelWidth,
                            bool performanceMode);
    void visibleIndexRange(int seriesIndex, qreal xMinimum, qreal xMaximum, int *first, int *last,
                           bool includeAdjacent) const;
    void resizeYRangeBlocks(PlotSeries *series);
    void ensureYRangeBlockCount(PlotSeries *series, int blockIndex);
    void rebuildYRangeBlock(PlotSeries *series, int blockIndex) const;
    void updateYRangeBlockAfterWrite(PlotSeries *series, int physicalIndex, qreal previousY, bool overwrote);
    bool isPhysicalIndexValid(const PlotSeries &series, int physicalIndex) const;
    void accumulatePhysicalYRange(const PlotSeries &series, int firstPhysical, int lastPhysical, qreal *minimum,
                                  qreal *maximum) const;
    void seriesYRange(int seriesIndex, int first, int last, qreal *minimum, qreal *maximum) const;
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
    void showContextMenu(const QPoint &globalPosition);
    void showAllData();
    QString exportDirectory() const;
    QString defaultExportPath(const QString &suffix) const;
    QString chooseExportPath(const QString &title, const QString &suffix, const QString &nameFilter) const;
    bool writeCsv(const QString &path) const;
    bool exportCsv();
    bool exportImage();

    QVector<PlotSeries> m_series;
    QVector<QRectF> m_legendToggleRects;
    QTimer *m_refreshTimer = nullptr;
    int m_capacity = 120000;
    int m_refreshRate = 60;
    int m_maximumVisiblePoints = 10000;
    qreal m_visibleSpan = 9999;
    qreal m_xMinimum = 0;
    qreal m_xMaximum = 9999;
    qreal m_yMinimum = -1;
    qreal m_yMaximum = 1;
    bool m_autoScroll = true;
    bool m_autoYRange = true;
    bool m_gridVisible = true;
    bool m_fillVisible = true;
    bool m_pointsVisible = false;
    bool m_crosshairVisible = true;
    bool m_legendVisible = true;
    bool m_dataUpdatePending = false;
    QColor m_curveColor;
    bool m_dragging = false;
    bool m_rightDragPending = false;
    bool m_rightScaling = false;
    bool m_suppressNextContextMenu = false;
    bool m_hasHover = false;
    QPointF m_hoverPosition;
    QPointF m_dragStartPosition;
    qreal m_dragStartXMinimum = 0;
    qreal m_dragStartXMaximum = 0;
    qreal m_dragStartYMinimum = 0;
    qreal m_dragStartYMaximum = 0;
    QPointF m_rightDragStartPosition;
    QPointF m_rightDragAnchor;
    qreal m_rightDragStartXMinimum = 0;
    qreal m_rightDragStartXMaximum = 0;
    qreal m_rightDragStartYMinimum = 0;
    qreal m_rightDragStartYMaximum = 0;
};

} // namespace FluentQt
