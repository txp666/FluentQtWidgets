#include <FluentQtWidgets/Widgets/RealtimePlotWidget.h>

#include <FluentQtWidgets/Theme.h>

#include <QtCore/QtMath>
#include <QtGui/QFontMetrics>
#include <QtGui/QLinearGradient>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPen>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QSizePolicy>

#include <algorithm>
#include <cmath>
#include <limits>

namespace FluentQt {

namespace {

constexpr qreal kMinimumRange = 1.0e-6;

bool isFinite(qreal value)
{
    return std::isfinite(value);
}

QColor withAlpha(QColor color, int alpha)
{
    color.setAlpha(alpha);
    return color;
}

QString tickText(qreal value)
{
    const qreal absolute = qAbs(value);
    int precision = 2;
    if (absolute >= 100.0) {
        precision = 0;
    } else if (absolute >= 10.0) {
        precision = 1;
    }

    QString text = QString::number(value, 'f', precision);
    while (text.contains(QLatin1Char('.')) && text.endsWith(QLatin1Char('0'))) {
        text.chop(1);
    }
    if (text.endsWith(QLatin1Char('.'))) {
        text.chop(1);
    }
    return text;
}

QPointF mousePosition(const QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position();
#else
    return event->pos();
#endif
}

QPointF wheelPosition(const QWheelEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position();
#else
    return event->pos();
#endif
}

struct PlotBucket
{
    bool hasValue = false;
    qreal minimum = 0;
    qreal maximum = 0;
};

} // namespace

RealtimePlotWidget::RealtimePlotWidget(QWidget *parent) : QWidget(parent)
{
    addSeries(QStringLiteral("Series 1"));
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_StyledBackground, true);

    connect(ThemeManager::instance(), &ThemeManager::effectiveThemeChanged, this, [this](Theme) { update(); });
    connect(ThemeManager::instance(), &ThemeManager::accentColorChanged, this, [this](const QColor &) { update(); });
}

QSize RealtimePlotWidget::sizeHint() const { return QSize(680, 320); }

QSize RealtimePlotWidget::minimumSizeHint() const { return QSize(220, 140); }

QVector<QPointF> RealtimePlotWidget::points() const { return points(0); }

QVector<QPointF> RealtimePlotWidget::points(int seriesIndex) const
{
    QVector<QPointF> result;
    if (!hasSeries(seriesIndex)) {
        return result;
    }

    const PlotSeries &series = m_series.at(seriesIndex);
    result.reserve(series.count);
    for (int i = 0; i < series.count; ++i) {
        result.append(pointAt(seriesIndex, i));
    }
    return result;
}

int RealtimePlotWidget::capacity() const { return m_capacity; }

int RealtimePlotWidget::sampleCount() const
{
    int count = 0;
    for (const PlotSeries &series : m_series) {
        count = qMax(count, series.count);
    }
    return count;
}

int RealtimePlotWidget::seriesCount() const { return m_series.size(); }

qreal RealtimePlotWidget::visibleSpan() const { return m_visibleSpan; }

qreal RealtimePlotWidget::xMinimum() const { return m_xMinimum; }

qreal RealtimePlotWidget::xMaximum() const { return m_xMaximum; }

qreal RealtimePlotWidget::yMinimum() const { return m_yMinimum; }

qreal RealtimePlotWidget::yMaximum() const { return m_yMaximum; }

bool RealtimePlotWidget::autoScroll() const { return m_autoScroll; }

bool RealtimePlotWidget::autoYRange() const { return m_autoYRange; }

bool RealtimePlotWidget::isGridVisible() const { return m_gridVisible; }

bool RealtimePlotWidget::isFillVisible() const { return m_fillVisible; }

bool RealtimePlotWidget::arePointsVisible() const { return m_pointsVisible; }

bool RealtimePlotWidget::isCrosshairVisible() const { return m_crosshairVisible; }

bool RealtimePlotWidget::isLegendVisible() const { return m_legendVisible; }

QColor RealtimePlotWidget::curveColor() const { return effectiveSeriesColor(0); }

QString RealtimePlotWidget::seriesName(int seriesIndex) const
{
    return hasSeries(seriesIndex) ? m_series.at(seriesIndex).name : QString();
}

QColor RealtimePlotWidget::seriesColor(int seriesIndex) const
{
    return hasSeries(seriesIndex) ? effectiveSeriesColor(seriesIndex) : QColor();
}

bool RealtimePlotWidget::isSeriesVisible(int seriesIndex) const
{
    return hasSeries(seriesIndex) && m_series.at(seriesIndex).visible;
}

int RealtimePlotWidget::addSeries(const QString &name, const QColor &color)
{
    PlotSeries series;
    series.name = name.trimmed().isEmpty() ? QStringLiteral("Series %1").arg(m_series.size() + 1) : name;
    series.color = color;
    series.buffer.resize(m_capacity);
    m_series.append(series);
    m_legendToggleRects.resize(m_series.size());

    update();
    emit seriesChanged();
    return m_series.size() - 1;
}

void RealtimePlotWidget::removeSeries(int seriesIndex)
{
    if (!hasSeries(seriesIndex)) {
        return;
    }

    m_series.removeAt(seriesIndex);
    m_legendToggleRects.resize(m_series.size());
    update();
    emit seriesChanged();
    emit samplesChanged();
}

void RealtimePlotWidget::clearSeries()
{
    if (m_series.isEmpty()) {
        return;
    }

    m_series.clear();
    m_legendToggleRects.clear();
    update();
    emit seriesChanged();
    emit samplesChanged();
}

void RealtimePlotWidget::setSeriesName(int seriesIndex, const QString &name)
{
    if (!hasSeries(seriesIndex)) {
        return;
    }

    const QString normalized = name.trimmed().isEmpty() ? QStringLiteral("Series %1").arg(seriesIndex + 1) : name;
    if (m_series[seriesIndex].name == normalized) {
        return;
    }

    m_series[seriesIndex].name = normalized;
    update();
    emit seriesChanged();
}

void RealtimePlotWidget::setSeriesColor(int seriesIndex, const QColor &color)
{
    if (!hasSeries(seriesIndex) || m_series.at(seriesIndex).color == color) {
        return;
    }

    m_series[seriesIndex].color = color;
    update();
    emit appearanceChanged();
}

void RealtimePlotWidget::setSeriesVisible(int seriesIndex, bool visible)
{
    if (!hasSeries(seriesIndex) || m_series.at(seriesIndex).visible == visible) {
        return;
    }

    m_series[seriesIndex].visible = visible;
    update();
    emit seriesChanged();
    emit rangeChanged();
}

void RealtimePlotWidget::setCapacity(int capacity)
{
    const int boundedCapacity = qMax(2, capacity);
    if (m_capacity == boundedCapacity) {
        return;
    }

    QVector<QVector<QPointF>> currentPoints;
    currentPoints.reserve(m_series.size());
    for (int seriesIndex = 0; seriesIndex < m_series.size(); ++seriesIndex) {
        QVector<QPointF> seriesPoints = points(seriesIndex);
        if (seriesPoints.size() > boundedCapacity) {
            seriesPoints = seriesPoints.mid(seriesPoints.size() - boundedCapacity);
        }
        currentPoints.append(seriesPoints);
    }

    m_capacity = boundedCapacity;
    for (int seriesIndex = 0; seriesIndex < m_series.size(); ++seriesIndex) {
        PlotSeries &series = m_series[seriesIndex];
        series.buffer.clear();
        series.buffer.resize(m_capacity);
        series.start = 0;
        series.count = 0;
        for (const QPointF &point : currentPoints.at(seriesIndex)) {
            appendPointInternal(seriesIndex, point.x(), point.y());
        }
    }

    update();
    emit capacityChanged(m_capacity);
    emit samplesChanged();
}

void RealtimePlotWidget::setVisibleSpan(qreal span)
{
    const qreal boundedSpan = qMax<qreal>(1, span);
    if (qFuzzyCompare(m_visibleSpan + 1, boundedSpan + 1)) {
        return;
    }

    m_visibleSpan = boundedSpan;
    if (!m_autoScroll) {
        m_xMaximum = m_xMinimum + m_visibleSpan;
    }
    update();
    emit rangeChanged();
}

void RealtimePlotWidget::setXMinimum(qreal minimum)
{
    setXRange(minimum, qMax(minimum + kMinimumRange, m_xMaximum));
}

void RealtimePlotWidget::setXMaximum(qreal maximum)
{
    setXRange(qMin(m_xMinimum, maximum - kMinimumRange), maximum);
}

void RealtimePlotWidget::setXRange(qreal minimum, qreal maximum)
{
    if (!isFinite(minimum) || !isFinite(maximum)) {
        return;
    }
    if (maximum - minimum < kMinimumRange) {
        maximum = minimum + 1;
    }

    const bool changed = !qFuzzyCompare(m_xMinimum + 1, minimum + 1) ||
                         !qFuzzyCompare(m_xMaximum + 1, maximum + 1) || m_autoScroll;
    if (!changed) {
        return;
    }

    m_xMinimum = minimum;
    m_xMaximum = maximum;
    m_visibleSpan = qMax<qreal>(1, maximum - minimum);
    m_autoScroll = false;
    update();
    emit rangeChanged();
    emit interactionChanged();
}

void RealtimePlotWidget::setYMinimum(qreal minimum)
{
    setYRange(minimum, qMax(minimum + kMinimumRange, m_yMaximum));
}

void RealtimePlotWidget::setYMaximum(qreal maximum)
{
    setYRange(qMin(m_yMinimum, maximum - kMinimumRange), maximum);
}

void RealtimePlotWidget::setYRange(qreal minimum, qreal maximum)
{
    if (!isFinite(minimum) || !isFinite(maximum)) {
        return;
    }
    if (maximum - minimum < kMinimumRange) {
        maximum = minimum + 1;
    }

    const bool changed = !qFuzzyCompare(m_yMinimum + 1, minimum + 1) ||
                         !qFuzzyCompare(m_yMaximum + 1, maximum + 1) || m_autoYRange;
    if (!changed) {
        return;
    }

    m_yMinimum = minimum;
    m_yMaximum = maximum;
    m_autoYRange = false;
    update();
    emit rangeChanged();
}

void RealtimePlotWidget::setAutoScroll(bool enabled)
{
    if (m_autoScroll == enabled) {
        return;
    }

    m_autoScroll = enabled;
    update();
    emit interactionChanged();
    emit rangeChanged();
}

void RealtimePlotWidget::setAutoYRange(bool enabled)
{
    if (m_autoYRange == enabled) {
        return;
    }

    m_autoYRange = enabled;
    update();
    emit rangeChanged();
}

void RealtimePlotWidget::setGridVisible(bool visible)
{
    if (m_gridVisible == visible) {
        return;
    }

    m_gridVisible = visible;
    update();
    emit appearanceChanged();
}

void RealtimePlotWidget::setFillVisible(bool visible)
{
    if (m_fillVisible == visible) {
        return;
    }

    m_fillVisible = visible;
    update();
    emit appearanceChanged();
}

void RealtimePlotWidget::setPointsVisible(bool visible)
{
    if (m_pointsVisible == visible) {
        return;
    }

    m_pointsVisible = visible;
    update();
    emit appearanceChanged();
}

void RealtimePlotWidget::setCrosshairVisible(bool visible)
{
    if (m_crosshairVisible == visible) {
        return;
    }

    m_crosshairVisible = visible;
    if (!m_crosshairVisible) {
        m_hasHover = false;
    }
    update();
    emit appearanceChanged();
}

void RealtimePlotWidget::setLegendVisible(bool visible)
{
    if (m_legendVisible == visible) {
        return;
    }

    m_legendVisible = visible;
    update();
    emit appearanceChanged();
}

void RealtimePlotWidget::setCurveColor(const QColor &color)
{
    if (m_curveColor == color) {
        return;
    }

    m_curveColor = color;
    if (hasSeries(0)) {
        m_series[0].color = color;
    }
    update();
    emit appearanceChanged();
}

void RealtimePlotWidget::appendSample(qreal y)
{
    appendSample(0, y);
}

void RealtimePlotWidget::appendSample(int seriesIndex, qreal y)
{
    if (!hasSeries(seriesIndex) && seriesIndex == 0) {
        addSeries(QStringLiteral("Series 1"));
    }
    if (!hasSeries(seriesIndex)) {
        return;
    }

    const qreal x = m_series.at(seriesIndex).nextX;
    if (!appendPointInternal(seriesIndex, x, y)) {
        return;
    }
    m_series[seriesIndex].nextX = x + 1;
    update();
    emit samplesChanged();
}

void RealtimePlotWidget::appendSamples(const QVector<qreal> &samples)
{
    appendSamples(0, samples);
}

void RealtimePlotWidget::appendSamples(int seriesIndex, const QVector<qreal> &samples)
{
    if (!hasSeries(seriesIndex) && seriesIndex == 0) {
        addSeries(QStringLiteral("Series 1"));
    }
    if (!hasSeries(seriesIndex)) {
        return;
    }

    bool changed = false;
    for (qreal sample : samples) {
        const qreal x = m_series.at(seriesIndex).nextX;
        if (appendPointInternal(seriesIndex, x, sample)) {
            m_series[seriesIndex].nextX = x + 1;
            changed = true;
        }
    }

    if (!changed) {
        return;
    }
    update();
    emit samplesChanged();
}

void RealtimePlotWidget::appendPoint(qreal x, qreal y)
{
    appendPoint(0, x, y);
}

void RealtimePlotWidget::appendPoint(int seriesIndex, qreal x, qreal y)
{
    if (!hasSeries(seriesIndex) && seriesIndex == 0) {
        addSeries(QStringLiteral("Series 1"));
    }
    if (!appendPointInternal(seriesIndex, x, y)) {
        return;
    }
    if (x >= m_series.at(seriesIndex).nextX) {
        m_series[seriesIndex].nextX = x + 1;
    }
    update();
    emit samplesChanged();
}

void RealtimePlotWidget::setSamples(const QVector<qreal> &samples)
{
    setSamples(0, samples);
}

void RealtimePlotWidget::setSamples(int seriesIndex, const QVector<qreal> &samples)
{
    if (!hasSeries(seriesIndex) && seriesIndex == 0) {
        addSeries(QStringLiteral("Series 1"));
    }
    if (!hasSeries(seriesIndex)) {
        return;
    }

    PlotSeries &series = m_series[seriesIndex];
    series.start = 0;
    series.count = 0;
    series.nextX = 0;
    for (int i = 0; i < samples.size(); ++i) {
        appendPointInternal(seriesIndex, i, samples.at(i));
    }
    series.nextX = samples.size();
    update();
    emit samplesChanged();
}

void RealtimePlotWidget::clear()
{
    bool changed = false;
    for (PlotSeries &series : m_series) {
        if (series.count > 0) {
            changed = true;
        }
        series.start = 0;
        series.count = 0;
        series.nextX = 0;
    }

    if (!changed) {
        return;
    }
    m_hasHover = false;
    update();
    emit samplesChanged();
}

void RealtimePlotWidget::resetView()
{
    m_autoScroll = true;
    m_autoYRange = true;
    update();
    emit interactionChanged();
    emit rangeChanged();
}

void RealtimePlotWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    const QRectF outer = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    painter.setPen(Qt::NoPen);
    painter.setBrush(panelColor());
    painter.drawRoundedRect(outer, 8, 8);

    const QRectF plot = plotRect();
    if (plot.width() <= 2 || plot.height() <= 2) {
        return;
    }

    painter.setBrush(plotColor());
    painter.setPen(QPen(gridColor(), 1));
    painter.drawRoundedRect(plot, 6, 6);

    const qreal xMin = viewXMinimum();
    const qreal xMax = viewXMaximum();
    qreal yMin = m_yMinimum;
    qreal yMax = m_yMaximum;
    visibleYRange(xMin, xMax, &yMin, &yMax);

    const QFontMetrics metrics(font());
    painter.setFont(font());

    if (m_gridVisible) {
        painter.save();
        painter.setClipRect(plot.adjusted(1, 1, -1, -1));
        QPen gridPen(gridColor(), 1);
        painter.setPen(gridPen);

        constexpr int xTicks = 6;
        constexpr int yTicks = 5;
        for (int i = 0; i <= xTicks; ++i) {
            const qreal ratio = static_cast<qreal>(i) / xTicks;
            const qreal x = plot.left() + ratio * plot.width();
            painter.drawLine(QPointF(x, plot.top()), QPointF(x, plot.bottom()));
        }
        for (int i = 0; i <= yTicks; ++i) {
            const qreal ratio = static_cast<qreal>(i) / yTicks;
            const qreal y = plot.bottom() - ratio * plot.height();
            painter.drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        }
        painter.restore();

        painter.setPen(textColor());
        for (int i = 0; i <= yTicks; ++i) {
            const qreal ratio = static_cast<qreal>(i) / yTicks;
            const qreal value = yMin + ratio * (yMax - yMin);
            const qreal y = plot.bottom() - ratio * plot.height();
            painter.drawText(QRectF(4, y - metrics.height() / 2.0, plot.left() - 10, metrics.height()),
                             Qt::AlignRight | Qt::AlignVCenter, tickText(value));
        }
        for (int i = 0; i <= xTicks; ++i) {
            const qreal ratio = static_cast<qreal>(i) / xTicks;
            const qreal value = xMin + ratio * (xMax - xMin);
            const qreal x = plot.left() + ratio * plot.width();
            painter.drawText(QRectF(x - 36, plot.bottom() + 7, 72, metrics.height()),
                             Qt::AlignHCenter | Qt::AlignVCenter, tickText(value));
        }
    }

    const int pixelWidth = qMax(1, static_cast<int>(std::floor(plot.width())));
    int visibleCount = 0;

    painter.save();
    painter.setClipRect(plot.adjusted(1, 1, -1, -1));

    for (int seriesIndex = 0; seriesIndex < m_series.size(); ++seriesIndex) {
        const PlotSeries &series = m_series.at(seriesIndex);
        if (!series.visible || series.count == 0) {
            continue;
        }

        const QColor lineColor = effectiveSeriesColor(seriesIndex);
        const bool denseMode = series.count > pixelWidth * 2;

        if (!denseMode) {
            QPainterPath linePath;
            QPainterPath fillPath;
            QVector<QPointF> visiblePoints;
            visiblePoints.reserve(qMin(series.count, pixelWidth * 2 + 8));
            bool started = false;

            for (int i = 0; i < series.count; ++i) {
                const QPointF point = pointAt(seriesIndex, i);
                if (point.x() < xMin || point.x() > xMax) {
                    continue;
                }

                const QPointF mapped = mapToPlot(point, plot, xMin, xMax, yMin, yMax);
                if (!started) {
                    linePath.moveTo(mapped);
                    fillPath.moveTo(mapped.x(), plot.bottom());
                    fillPath.lineTo(mapped);
                    started = true;
                } else {
                    linePath.lineTo(mapped);
                    fillPath.lineTo(mapped);
                }
                visiblePoints.append(mapped);
                ++visibleCount;
            }

            if (visiblePoints.isEmpty()) {
                continue;
            }

            if (m_fillVisible) {
                fillPath.lineTo(visiblePoints.last().x(), plot.bottom());
                fillPath.closeSubpath();

                QLinearGradient gradient(plot.topLeft(), plot.bottomLeft());
                gradient.setColorAt(0, withAlpha(lineColor, 48));
                gradient.setColorAt(1, withAlpha(lineColor, 0));
                painter.fillPath(fillPath, gradient);
            }

            QPen curvePen(lineColor, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter.setPen(curvePen);
            painter.setBrush(Qt::NoBrush);
            painter.drawPath(linePath);

            if (m_pointsVisible && visiblePoints.size() <= 1000) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(lineColor);
                for (const QPointF &point : visiblePoints) {
                    painter.drawEllipse(point, 2.4, 2.4);
                }
            }
            continue;
        }

        QVector<PlotBucket> buckets(pixelWidth + 1);
        for (int i = 0; i < series.count; ++i) {
            const QPointF point = pointAt(seriesIndex, i);
            if (point.x() < xMin || point.x() > xMax) {
                continue;
            }

            const int column = qBound(0, static_cast<int>((point.x() - xMin) / (xMax - xMin) * pixelWidth), pixelWidth);
            PlotBucket &bucket = buckets[column];
            if (!bucket.hasValue) {
                bucket.hasValue = true;
                bucket.minimum = point.y();
                bucket.maximum = point.y();
            } else {
                bucket.minimum = qMin(bucket.minimum, point.y());
                bucket.maximum = qMax(bucket.maximum, point.y());
            }
            ++visibleCount;
        }

        if (m_fillVisible) {
            QPen fillPen(withAlpha(lineColor, 24), 1.0);
            painter.setPen(fillPen);
            for (int column = 0; column < buckets.size(); ++column) {
                const PlotBucket &bucket = buckets.at(column);
                if (!bucket.hasValue) {
                    continue;
                }
                const qreal x = plot.left() + column;
                const qreal topY = mapToPlot(QPointF(xMin, bucket.maximum), plot, xMin, xMax, yMin, yMax).y();
                painter.drawLine(QPointF(x, topY), QPointF(x, plot.bottom()));
            }
        }

        QPen curvePen(lineColor, 1.5, Qt::SolidLine, Qt::RoundCap);
        painter.setPen(curvePen);
        for (int column = 0; column < buckets.size(); ++column) {
            const PlotBucket &bucket = buckets.at(column);
            if (!bucket.hasValue) {
                continue;
            }
            const qreal x = plot.left() + column;
            const qreal y1 = mapToPlot(QPointF(xMin, bucket.minimum), plot, xMin, xMax, yMin, yMax).y();
            const qreal y2 = mapToPlot(QPointF(xMin, bucket.maximum), plot, xMin, xMax, yMin, yMax).y();
            painter.drawLine(QPointF(x, y1), QPointF(x, y2));
        }
    }

    if (visibleCount == 0) {
        QPen baselinePen(withAlpha(textColor(), 55), 1.2, Qt::DashLine, Qt::RoundCap);
        painter.setPen(baselinePen);
        const qreal centerY = plot.center().y();
        painter.drawLine(QPointF(plot.left() + 8, centerY), QPointF(plot.right() - 8, centerY));
    }

    if (m_crosshairVisible && m_hasHover && plot.contains(m_hoverPosition)) {
        const QPointF dataPoint = mapFromPlot(m_hoverPosition, plot, xMin, xMax, yMin, yMax);
        QPen crosshairPen(withAlpha(textColor(), 120), 1, Qt::DashLine);
        painter.setPen(crosshairPen);
        painter.drawLine(QPointF(m_hoverPosition.x(), plot.top()), QPointF(m_hoverPosition.x(), plot.bottom()));
        painter.drawLine(QPointF(plot.left(), m_hoverPosition.y()), QPointF(plot.right(), m_hoverPosition.y()));

        const QString label = QStringLiteral("%1, %2").arg(tickText(dataPoint.x()), tickText(dataPoint.y()));
        const QSize labelSize(metrics.horizontalAdvance(label) + 16, metrics.height() + 8);
        QPointF labelTopLeft(m_hoverPosition.x() + 10, m_hoverPosition.y() - labelSize.height() - 10);
        if (labelTopLeft.x() + labelSize.width() > plot.right() - 4) {
            labelTopLeft.setX(m_hoverPosition.x() - labelSize.width() - 10);
        }
        if (labelTopLeft.y() < plot.top() + 4) {
            labelTopLeft.setY(m_hoverPosition.y() + 10);
        }

        const QRectF labelRect(labelTopLeft, labelSize);
        painter.setPen(Qt::NoPen);
        painter.setBrush(withAlpha(panelColor(), 235));
        painter.drawRoundedRect(labelRect, 5, 5);
        painter.setPen(textColor());
        painter.drawText(labelRect, Qt::AlignCenter, label);
    }

    painter.restore();
    drawLegend(&painter, plot);
}

void RealtimePlotWidget::mousePressEvent(QMouseEvent *event)
{
    const QPointF position = mousePosition(event);
    if (event->button() == Qt::LeftButton) {
        for (int i = 0; i < m_legendToggleRects.size(); ++i) {
            if (m_legendToggleRects.at(i).contains(position)) {
                setSeriesVisible(i, !isSeriesVisible(i));
                event->accept();
                return;
            }
        }
    }

    if (event->button() != Qt::LeftButton || !plotRect().contains(position)) {
        QWidget::mousePressEvent(event);
        return;
    }

    m_dragging = true;
    m_dragStartPosition = position;
    m_dragStartXMinimum = viewXMinimum();
    m_dragStartXMaximum = viewXMaximum();
    event->accept();
}

void RealtimePlotWidget::mouseMoveEvent(QMouseEvent *event)
{
    const QRectF plot = plotRect();
    const QPointF position = mousePosition(event);

    if (m_dragging) {
        const qreal span = qMax<qreal>(kMinimumRange, m_dragStartXMaximum - m_dragStartXMinimum);
        const qreal delta = -(position.x() - m_dragStartPosition.x()) / qMax<qreal>(1, plot.width()) * span;
        setXRange(m_dragStartXMinimum + delta, m_dragStartXMaximum + delta);
        event->accept();
        return;
    }

    if (plot.contains(position)) {
        qreal yMin = m_yMinimum;
        qreal yMax = m_yMaximum;
        visibleYRange(viewXMinimum(), viewXMaximum(), &yMin, &yMax);

        m_hasHover = true;
        m_hoverPosition = position;
        const QPointF point = mapFromPlot(position, plot, viewXMinimum(), viewXMaximum(), yMin, yMax);
        emit crosshairMoved(point);
        update();
        event->accept();
        return;
    }

    if (m_hasHover) {
        m_hasHover = false;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void RealtimePlotWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_dragging) {
        m_dragging = false;
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void RealtimePlotWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && plotRect().contains(mousePosition(event))) {
        resetView();
        event->accept();
        return;
    }
    QWidget::mouseDoubleClickEvent(event);
}

void RealtimePlotWidget::wheelEvent(QWheelEvent *event)
{
    const QRectF plot = plotRect();
    const QPointF position = wheelPosition(event);
    if (!plot.contains(position) || event->angleDelta().y() == 0) {
        QWidget::wheelEvent(event);
        return;
    }

    qreal yMin = m_yMinimum;
    qreal yMax = m_yMaximum;
    visibleYRange(viewXMinimum(), viewXMaximum(), &yMin, &yMax);

    const qreal currentMin = viewXMinimum();
    const qreal currentMax = viewXMaximum();
    const qreal cursorX = mapFromPlot(position, plot, currentMin, currentMax, yMin, yMax).x();
    const qreal factor = event->angleDelta().y() > 0 ? 0.82 : 1.18;
    const qreal nextMin = cursorX - (cursorX - currentMin) * factor;
    const qreal nextMax = cursorX + (currentMax - cursorX) * factor;
    setXRange(nextMin, nextMax);
    event->accept();
}

void RealtimePlotWidget::leaveEvent(QEvent *event)
{
    if (m_hasHover) {
        m_hasHover = false;
        update();
    }
    QWidget::leaveEvent(event);
}

bool RealtimePlotWidget::hasSeries(int seriesIndex) const
{
    return seriesIndex >= 0 && seriesIndex < m_series.size();
}

bool RealtimePlotWidget::appendPointInternal(int seriesIndex, qreal x, qreal y)
{
    if (!hasSeries(seriesIndex) || !isFinite(x) || !isFinite(y) || m_capacity < 2) {
        return false;
    }

    PlotSeries &series = m_series[seriesIndex];
    const int index = series.count < m_capacity ? (series.start + series.count) % m_capacity : series.start;
    series.buffer[index] = QPointF(x, y);
    if (series.count < m_capacity) {
        ++series.count;
    } else {
        series.start = (series.start + 1) % m_capacity;
    }
    return true;
}

QPointF RealtimePlotWidget::pointAt(int seriesIndex, int index) const
{
    const PlotSeries &series = m_series.at(seriesIndex);
    return series.buffer.at((series.start + index) % m_capacity);
}

QRectF RealtimePlotWidget::plotRect() const
{
    return QRectF(rect()).adjusted(52, 18, -18, -34);
}

qreal RealtimePlotWidget::viewXMinimum() const
{
    if (m_autoScroll) {
        return viewXMaximum() - m_visibleSpan;
    }
    return m_xMinimum;
}

qreal RealtimePlotWidget::viewXMaximum() const
{
    if (!m_autoScroll) {
        return m_xMaximum;
    }

    qreal latest = std::numeric_limits<qreal>::lowest();
    for (int seriesIndex = 0; seriesIndex < m_series.size(); ++seriesIndex) {
        const PlotSeries &series = m_series.at(seriesIndex);
        if (!series.visible || series.count == 0) {
            continue;
        }
        latest = qMax(latest, pointAt(seriesIndex, series.count - 1).x());
    }
    if (latest == std::numeric_limits<qreal>::lowest()) {
        return m_visibleSpan;
    }
    return latest;
}

void RealtimePlotWidget::visibleYRange(qreal xMinimum, qreal xMaximum, qreal *minimum, qreal *maximum) const
{
    if (!m_autoYRange) {
        *minimum = m_yMinimum;
        *maximum = m_yMaximum;
        return;
    }

    qreal yMin = std::numeric_limits<qreal>::max();
    qreal yMax = std::numeric_limits<qreal>::lowest();
    for (int seriesIndex = 0; seriesIndex < m_series.size(); ++seriesIndex) {
        const PlotSeries &series = m_series.at(seriesIndex);
        if (!series.visible) {
            continue;
        }
        for (int i = 0; i < series.count; ++i) {
            const QPointF point = pointAt(seriesIndex, i);
            if (point.x() < xMinimum || point.x() > xMaximum) {
                continue;
            }
            yMin = qMin(yMin, point.y());
            yMax = qMax(yMax, point.y());
        }
    }

    if (yMin == std::numeric_limits<qreal>::max() || yMax == std::numeric_limits<qreal>::lowest()) {
        *minimum = m_yMinimum;
        *maximum = m_yMaximum;
        return;
    }

    const qreal range = qMax<qreal>(kMinimumRange, yMax - yMin);
    const qreal padding = qMax<qreal>(0.2, range * 0.12);
    *minimum = yMin - padding;
    *maximum = yMax + padding;
}

QPointF RealtimePlotWidget::mapToPlot(const QPointF &point, const QRectF &plot, qreal xMinimum, qreal xMaximum,
                                      qreal yMinimum, qreal yMaximum) const
{
    const qreal xRange = qMax<qreal>(kMinimumRange, xMaximum - xMinimum);
    const qreal yRange = qMax<qreal>(kMinimumRange, yMaximum - yMinimum);
    const qreal x = plot.left() + (point.x() - xMinimum) / xRange * plot.width();
    const qreal y = plot.bottom() - (point.y() - yMinimum) / yRange * plot.height();
    return QPointF(x, y);
}

QPointF RealtimePlotWidget::mapFromPlot(const QPointF &position, const QRectF &plot, qreal xMinimum, qreal xMaximum,
                                        qreal yMinimum, qreal yMaximum) const
{
    const qreal xRange = qMax<qreal>(kMinimumRange, xMaximum - xMinimum);
    const qreal yRange = qMax<qreal>(kMinimumRange, yMaximum - yMinimum);
    const qreal x = xMinimum + (position.x() - plot.left()) / qMax<qreal>(1, plot.width()) * xRange;
    const qreal y = yMinimum + (plot.bottom() - position.y()) / qMax<qreal>(1, plot.height()) * yRange;
    return QPointF(x, y);
}

QColor RealtimePlotWidget::panelColor() const
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(32, 32, 32) : QColor(255, 255, 255);
}

QColor RealtimePlotWidget::plotColor() const
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(18, 18, 18) : QColor(248, 250, 252);
}

QColor RealtimePlotWidget::textColor() const
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(243, 243, 243, 190)
                                                                     : QColor(31, 31, 31, 170);
}

QColor RealtimePlotWidget::gridColor() const
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(255, 255, 255, 24)
                                                                     : QColor(0, 0, 0, 20);
}

QColor RealtimePlotWidget::effectiveSeriesColor(int seriesIndex) const
{
    if (hasSeries(seriesIndex) && m_series.at(seriesIndex).color.isValid()) {
        return m_series.at(seriesIndex).color;
    }

    const QVector<QColor> palette{
        ThemeManager::instance()->accentColor(),
        QColor(22, 163, 74),
        QColor(245, 158, 11),
        QColor(239, 68, 68),
        QColor(139, 92, 246),
        QColor(14, 165, 233),
    };
    return palette.at(seriesIndex < 0 ? 0 : seriesIndex % palette.size());
}

void RealtimePlotWidget::drawLegend(QPainter *painter, const QRectF &plot)
{
    m_legendToggleRects.fill(QRectF());
    if (!m_legendVisible || m_series.isEmpty()) {
        return;
    }

    painter->save();
    QFont legendFont = font();
    legendFont.setPointSize(qMax(8, legendFont.pointSize() - 1));
    painter->setFont(legendFont);
    const QFontMetrics metrics(legendFont);

    qreal contentWidth = 0;
    for (const PlotSeries &series : m_series) {
        contentWidth = qMax<qreal>(contentWidth, metrics.horizontalAdvance(series.name));
    }

    const qreal rowHeight = qMax<qreal>(22, metrics.height() + 8);
    const qreal legendWidth = qMin<qreal>(plot.width() - 24, contentWidth + 46);
    const qreal legendHeight = 10 + rowHeight * m_series.size();
    const QRectF legendRect(plot.right() - legendWidth - 10, plot.top() + 10, legendWidth, legendHeight);

    painter->setPen(QPen(gridColor(), 1));
    painter->setBrush(withAlpha(panelColor(), 232));
    painter->drawRoundedRect(legendRect, 6, 6);

    for (int i = 0; i < m_series.size(); ++i) {
        const PlotSeries &series = m_series.at(i);
        const qreal y = legendRect.top() + 5 + i * rowHeight;
        const QRectF itemRect(legendRect.left() + 6, y, legendRect.width() - 12, rowHeight);
        const QRectF boxRect(itemRect.left() + 3, itemRect.center().y() - 5.5, 11, 11);
        m_legendToggleRects[i] = itemRect;

        const QColor seriesColor = effectiveSeriesColor(i);
        painter->setPen(QPen(series.visible ? seriesColor : withAlpha(textColor(), 90), 1.4));
        painter->setBrush(series.visible ? withAlpha(seriesColor, 54) : Qt::NoBrush);
        painter->drawRoundedRect(boxRect, 2, 2);

        if (series.visible) {
            QPen checkPen(seriesColor, 1.6, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
            painter->setPen(checkPen);
            QPainterPath checkPath;
            checkPath.moveTo(boxRect.left() + 2.2, boxRect.center().y());
            checkPath.lineTo(boxRect.left() + 4.8, boxRect.bottom() - 2.2);
            checkPath.lineTo(boxRect.right() - 2.0, boxRect.top() + 2.4);
            painter->drawPath(checkPath);
        }

        painter->setPen(series.visible ? textColor() : withAlpha(textColor(), 80));
        painter->drawText(QRectF(boxRect.right() + 8, itemRect.top(), itemRect.width() - 22, itemRect.height()),
                          Qt::AlignVCenter | Qt::AlignLeft, metrics.elidedText(series.name, Qt::ElideRight,
                                                                                qMax(1, static_cast<int>(itemRect.width() - 28))));
    }

    painter->restore();
}

} // namespace FluentQt
