#include <FluentQtWidgets/Widgets/RealtimePlotWidget.h>

#include <QtCore/QtMath>
#include <QtGui/QFontMetrics>
#include <QtGui/QLinearGradient>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPen>
#include <QtGui/QPolygonF>

#include <cmath>
#include <limits>
#include <utility>

namespace FluentQt {

namespace {

constexpr qreal kPaintMinimumRange = 1.0e-6;
constexpr int kRenderBlockSize = 256;

QColor withAlpha(QColor color, int alpha)
{
    color.setAlpha(alpha);
    return color;
}

int tickPrecision(qreal step)
{
    const qreal absoluteStep = qAbs(step);
    if (!std::isfinite(absoluteStep) || absoluteStep <= kPaintMinimumRange) {
        return 8;
    }

    if (absoluteStep >= 2.0) {
        return 0;
    }
    if (absoluteStep >= 1.0) {
        return 1;
    }
    return qBound(0, qCeil(-std::log10(absoluteStep)) + 1, 8);
}

QString tickText(qreal value, qreal step)
{
    const int precision = tickPrecision(step);
    QString text = QString::number(value, 'f', precision);
    while (text.contains(QLatin1Char('.')) && text.endsWith(QLatin1Char('0'))) {
        text.chop(1);
    }
    if (text.endsWith(QLatin1Char('.'))) {
        text.chop(1);
    }
    return text;
}

void appendUniqueIndex(QVector<int> *indices, int index)
{
    if (!indices || (!indices->isEmpty() && indices->constLast() == index)) {
        return;
    }
    indices->append(index);
}

struct HoverSample
{
    int seriesIndex = -1;
    QPointF dataPoint;
    QPointF plotPoint;
    qreal distance = std::numeric_limits<qreal>::max();
};

struct PeakBucket
{
    int firstIndex = -1;
    int lastIndex = -1;
    int minimumIndex = -1;
    int maximumIndex = -1;
    qreal minimum = std::numeric_limits<qreal>::max();
    qreal maximum = std::numeric_limits<qreal>::lowest();

    bool isValid() const { return firstIndex >= 0; }

    void add(int index, qreal value)
    {
        if (!isValid()) {
            firstIndex = index;
        }
        lastIndex = index;
        if (value < minimum) {
            minimum = value;
            minimumIndex = index;
        }
        if (value > maximum) {
            maximum = value;
            maximumIndex = index;
        }
    }
};

} // namespace

void RealtimePlotWidget::rebuildRenderCache(int seriesIndex, int first, int last, const QRectF &plot,
                                            qreal xMinimum, qreal xMaximum, qreal yMinimum, qreal yMaximum,
                                            int pixelWidth, bool performanceMode)
{
    if (!hasSeries(seriesIndex)) {
        return;
    }

    PlotSeries &series = m_series[seriesIndex];
    QVector<QPointF> &screenPoints = series.renderCachePoints;
    QVector<QPointF> &fillPoints = series.renderCacheFillPoints;
    screenPoints.clear();
    fillPoints.clear();

    const int visibleDataCount = last - first;
    const qreal xRange = qMax<qreal>(kPaintMinimumRange, xMaximum - xMinimum);
    const qreal yRange = qMax<qreal>(kPaintMinimumRange, yMaximum - yMinimum);
    const qreal xScale = plot.width() / xRange;
    const qreal yScale = plot.height() / yRange;
    const auto mapPoint = [&](const QPointF &point) {
        return QPointF(plot.left() + (point.x() - xMinimum) * xScale,
                       plot.bottom() - (point.y() - yMinimum) * yScale);
    };

    if (visibleDataCount > 0) {
        if (!performanceMode || visibleDataCount <= 3) {
            screenPoints.reserve(visibleDataCount);
            for (int i = first; i < last; ++i) {
                screenPoints.append(mapPoint(pointAt(seriesIndex, i)));
            }
        } else if (series.xMonotonic) {
            QVector<PeakBucket> buckets(qMax(1, pixelWidth));
            screenPoints.reserve(qMin(visibleDataCount, pixelWidth * 4 + 2));

            const auto screenColumn = [&](const QPointF &dataPoint, int *column) {
                const qreal screenX = plot.left() + (dataPoint.x() - xMinimum) * xScale;
                if (screenX < plot.left() || screenX > plot.right()) {
                    return false;
                }

                *column = qBound(0, static_cast<int>(std::floor(screenX - plot.left())), buckets.size() - 1);
                return true;
            };

            const auto addRawIndex = [&](int index) {
                const QPointF dataPoint = pointAt(seriesIndex, index);
                int column = -1;
                if (screenColumn(dataPoint, &column)) {
                    buckets[column].add(index, dataPoint.y());
                }
            };

            for (int i = first; i < last;) {
                if (m_capacity == 0) {
                    const int blockIndex = i / kRenderBlockSize;
                    const int blockFirst = blockIndex * kRenderBlockSize;
                    const int blockLast = blockFirst + kRenderBlockSize;
                    const bool fullBlock = i == blockFirst && blockLast <= last &&
                                           blockIndex >= 0 && blockIndex < series.yBlockMinimumIndices.size();
                    if (fullBlock) {
                        const QPointF firstPoint = series.buffer.at(blockFirst);
                        const QPointF lastPoint = series.buffer.at(blockLast - 1);
                        int firstColumn = -1;
                        int lastColumn = -1;
                        if (screenColumn(firstPoint, &firstColumn) && screenColumn(lastPoint, &lastColumn) &&
                            firstColumn == lastColumn) {
                            PeakBucket &bucket = buckets[firstColumn];
                            bucket.add(blockFirst, firstPoint.y());

                            const int minimumIndex = series.yBlockMinimumIndices.at(blockIndex);
                            if (minimumIndex >= blockFirst && minimumIndex < blockLast) {
                                bucket.add(minimumIndex, series.buffer.at(minimumIndex).y());
                            }

                            const int maximumIndex = series.yBlockMaximumIndices.at(blockIndex);
                            if (maximumIndex >= blockFirst && maximumIndex < blockLast) {
                                bucket.add(maximumIndex, series.buffer.at(maximumIndex).y());
                            }

                            bucket.add(blockLast - 1, lastPoint.y());
                            i = blockLast;
                            continue;
                        }
                    }
                }

                addRawIndex(i);
                ++i;
            }

            int lastAppendedIndex = -1;
            const auto appendIndex = [&](int index) {
                if (index < first || index >= last || index == lastAppendedIndex) {
                    return;
                }
                screenPoints.append(mapPoint(pointAt(seriesIndex, index)));
                lastAppendedIndex = index;
            };

            appendIndex(first);
            for (const PeakBucket &bucket : std::as_const(buckets)) {
                if (!bucket.isValid()) {
                    continue;
                }

                appendIndex(bucket.firstIndex);
                if (bucket.minimumIndex <= bucket.maximumIndex) {
                    appendIndex(bucket.minimumIndex);
                    appendIndex(bucket.maximumIndex);
                } else {
                    appendIndex(bucket.maximumIndex);
                    appendIndex(bucket.minimumIndex);
                }
                appendIndex(bucket.lastIndex);
            }
            appendIndex(last - 1);
        } else {
            const int targetPointCount = qBound(3, pixelWidth * 2, visibleDataCount);
            QVector<int> pointIndices;
            pointIndices.reserve(targetPointCount + 1);
            appendUniqueIndex(&pointIndices, first);

            const qreal bucketSize = static_cast<qreal>(visibleDataCount - 2) / (targetPointCount - 2);
            int anchorIndex = first;
            QPointF anchorPoint = mapPoint(pointAt(seriesIndex, anchorIndex));

            for (int bucket = 0; bucket < targetPointCount - 2; ++bucket) {
                int averageStart = first + 1 + qFloor((bucket + 1) * bucketSize);
                int averageEnd = first + 1 + qFloor((bucket + 2) * bucketSize);
                averageStart = qBound(first + 1, averageStart, last - 1);
                averageEnd = qBound(averageStart + 1, averageEnd, last);

                QPointF averagePoint;
                const int averageCount = averageEnd - averageStart;
                for (int i = averageStart; i < averageEnd; ++i) {
                    averagePoint += mapPoint(pointAt(seriesIndex, i));
                }
                averagePoint /= qMax(1, averageCount);

                int rangeStart = first + 1 + qFloor(bucket * bucketSize);
                int rangeEnd = first + 1 + qFloor((bucket + 1) * bucketSize);
                rangeStart = qBound(first + 1, rangeStart, last - 1);
                rangeEnd = qBound(rangeStart + 1, rangeEnd, last);

                int bestIndex = rangeStart;
                QPointF bestPoint = mapPoint(pointAt(seriesIndex, bestIndex));
                qreal bestArea = -1;
                for (int i = rangeStart; i < rangeEnd; ++i) {
                    const QPointF candidate = mapPoint(pointAt(seriesIndex, i));
                    const qreal area = qAbs((anchorPoint.x() - averagePoint.x()) *
                                                (candidate.y() - anchorPoint.y()) -
                                            (anchorPoint.x() - candidate.x()) *
                                                (averagePoint.y() - anchorPoint.y()));
                    if (area > bestArea) {
                        bestArea = area;
                        bestIndex = i;
                        bestPoint = candidate;
                    }
                }

                appendUniqueIndex(&pointIndices, bestIndex);
                anchorIndex = bestIndex;
                anchorPoint = bestPoint;
            }

            appendUniqueIndex(&pointIndices, last - 1);

            screenPoints.reserve(pointIndices.size());
            for (int pointIndex : std::as_const(pointIndices)) {
                screenPoints.append(mapPoint(pointAt(seriesIndex, pointIndex)));
            }
        }

        if (screenPoints.size() >= 2) {
            fillPoints.reserve(screenPoints.size() + 2);
            fillPoints.append(QPointF(screenPoints.constFirst().x(), plot.bottom()));
            for (const QPointF &point : std::as_const(screenPoints)) {
                fillPoints.append(point);
            }
            fillPoints.append(QPointF(screenPoints.constLast().x(), plot.bottom()));
        }
    }

    series.renderCacheRevision = series.dataRevision;
    series.renderCachePlot = plot;
    series.renderCacheXMinimum = xMinimum;
    series.renderCacheXMaximum = xMaximum;
    series.renderCacheYMinimum = yMinimum;
    series.renderCacheYMaximum = yMaximum;
    series.renderCacheFirst = first;
    series.renderCacheLast = last;
    series.renderCachePixelWidth = pixelWidth;
    series.renderCachePerformanceMode = performanceMode;
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
    painter.drawRect(plot);

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
        painter.setPen(QPen(gridColor(), 1));

        constexpr int xTicks = 6;
        constexpr int yTicks = 5;
        const qreal xTickStep = (xMax - xMin) / xTicks;
        const qreal yTickStep = (yMax - yMin) / yTicks;
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

        painter.setPen(withAlpha(textColor(), 190));
        for (int i = 0; i <= yTicks; ++i) {
            const qreal ratio = static_cast<qreal>(i) / yTicks;
            const qreal value = yMin + ratio * (yMax - yMin);
            const qreal y = plot.bottom() - ratio * plot.height();
            painter.drawText(QRectF(4, y - metrics.height() / 2.0, plot.left() - 10, metrics.height()),
                             Qt::AlignRight | Qt::AlignVCenter, tickText(value, yTickStep));
        }
        for (int i = 0; i <= xTicks; ++i) {
            const qreal ratio = static_cast<qreal>(i) / xTicks;
            const qreal value = xMin + ratio * (xMax - xMin);
            const qreal x = plot.left() + ratio * plot.width();
            painter.drawText(QRectF(x - 36, plot.bottom() + 7, 72, metrics.height()),
                             Qt::AlignHCenter | Qt::AlignVCenter, tickText(value, xTickStep));
        }
    }

    painter.setPen(QPen(withAlpha(textColor(), 145), 1.2));
    painter.drawLine(QPointF(plot.left(), plot.top()), QPointF(plot.left(), plot.bottom()));
    painter.drawLine(QPointF(plot.left(), plot.bottom()), QPointF(plot.right(), plot.bottom()));

    const int pixelWidth = qMax(1, static_cast<int>(std::floor(plot.width())));
    int visibleCount = 0;
    const bool hoverInsidePlot = m_crosshairVisible && m_hasHover && plot.contains(m_hoverPosition);
    const QPointF hoverDataPoint = hoverInsidePlot ? mapFromPlot(m_hoverPosition, plot, xMin, xMax, yMin, yMax)
                                                   : QPointF();
    QVector<HoverSample> hoverSamples;
    QVector<QPair<int, QPointF>> latestSamples;

    painter.save();
    painter.setClipRect(plot.adjusted(1, 1, -1, -1));

    for (int seriesIndex = 0; seriesIndex < m_series.size(); ++seriesIndex) {
        PlotSeries &series = m_series[seriesIndex];
        if (!series.visible || series.count == 0) {
            continue;
        }

        int firstIndex = 0;
        int lastIndex = 0;
        visibleIndexRange(seriesIndex, xMin, xMax, &firstIndex, &lastIndex, true);
        const int visibleDataCount = lastIndex - firstIndex;
        if (visibleDataCount <= 0) {
            continue;
        }

        const QColor lineColor = effectiveSeriesColor(seriesIndex);
        const bool performanceMode = visibleDataCount > qMax(1600, pixelWidth * 2);
        const bool cacheValid = series.renderCacheRevision == series.dataRevision &&
                                series.renderCacheFirst == firstIndex &&
                                series.renderCacheLast == lastIndex &&
                                series.renderCachePixelWidth == pixelWidth &&
                                series.renderCachePerformanceMode == performanceMode &&
                                series.renderCachePlot == plot &&
                                series.renderCacheXMinimum == xMin &&
                                series.renderCacheXMaximum == xMax &&
                                series.renderCacheYMinimum == yMin &&
                                series.renderCacheYMaximum == yMax;
        if (!cacheValid) {
            rebuildRenderCache(seriesIndex, firstIndex, lastIndex, plot, xMin, xMax, yMin, yMax, pixelWidth,
                               performanceMode);
        }
        const QVector<QPointF> &screenPoints = series.renderCachePoints;

        visibleCount += visibleDataCount;

        if (screenPoints.size() >= 2) {
            painter.setRenderHint(QPainter::Antialiasing, !performanceMode);
            if (m_fillVisible && !series.renderCacheFillPoints.isEmpty()) {
                QLinearGradient gradient(plot.topLeft(), plot.bottomLeft());
                gradient.setColorAt(0, withAlpha(lineColor, performanceMode ? 34 : 48));
                gradient.setColorAt(1, withAlpha(lineColor, 0));
                painter.setPen(Qt::NoPen);
                painter.setBrush(gradient);
                painter.drawPolygon(series.renderCacheFillPoints.constData(), series.renderCacheFillPoints.size());
            }

            painter.setPen(QPen(lineColor, performanceMode ? 1.35 : 2.0, Qt::SolidLine,
                                performanceMode ? Qt::FlatCap : Qt::RoundCap,
                                performanceMode ? Qt::MiterJoin : Qt::RoundJoin));
            painter.setBrush(Qt::NoBrush);
            painter.drawPolyline(screenPoints.constData(), screenPoints.size());
        } else if (screenPoints.size() == 1) {
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setPen(Qt::NoPen);
            painter.setBrush(lineColor);
            painter.drawEllipse(screenPoints.constFirst(), 2.4, 2.4);
        }

        if (m_pointsVisible && !performanceMode && screenPoints.size() <= 1000) {
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setPen(Qt::NoPen);
            painter.setBrush(lineColor);
            for (const QPointF &point : std::as_const(screenPoints)) {
                painter.drawEllipse(point, 2.4, 2.4);
            }
        }

        if (hoverInsidePlot) {
            const int nearestIndex = nearestDataIndex(seriesIndex, hoverDataPoint.x(), firstIndex, lastIndex);
            if (nearestIndex >= 0) {
                HoverSample nearest;
                nearest.seriesIndex = seriesIndex;
                nearest.dataPoint = pointAt(seriesIndex, nearestIndex);
                nearest.plotPoint = mapToPlot(nearest.dataPoint, plot, xMin, xMax, yMin, yMax);
                nearest.distance = qAbs(nearest.plotPoint.x() - m_hoverPosition.x());
                hoverSamples.append(nearest);
            }
        }

        int latestIndex = lastIndex - 1;
        if (series.xMonotonic) {
            while (latestIndex >= firstIndex && pointAt(seriesIndex, latestIndex).x() > xMax) {
                --latestIndex;
            }
        }
        if (latestIndex >= firstIndex && latestIndex < lastIndex) {
            latestSamples.append(
                qMakePair(seriesIndex, mapToPlot(pointAt(seriesIndex, latestIndex), plot, xMin, xMax, yMin, yMax)));
        }
    }

    painter.setRenderHint(QPainter::Antialiasing, true);

    if (visibleCount == 0) {
        painter.setPen(QPen(withAlpha(textColor(), 55), 1.2, Qt::DashLine, Qt::RoundCap));
        const qreal centerY = plot.center().y();
        painter.drawLine(QPointF(plot.left() + 8, centerY), QPointF(plot.right() - 8, centerY));
    }

    if (hoverInsidePlot) {
        painter.setPen(QPen(withAlpha(textColor(), 120), 1, Qt::DashLine));
        painter.drawLine(QPointF(m_hoverPosition.x(), plot.top()), QPointF(m_hoverPosition.x(), plot.bottom()));
        painter.drawLine(QPointF(plot.left(), m_hoverPosition.y()), QPointF(plot.right(), m_hoverPosition.y()));

        const qreal hoverXStep = (xMax - xMin) / 6.0;
        const qreal hoverYStep = (yMax - yMin) / 5.0;
        QStringList labelLines{QStringLiteral("x=%1").arg(tickText(hoverDataPoint.x(), hoverXStep))};
        for (const HoverSample &sample : hoverSamples) {
            const PlotSeries &series = m_series.at(sample.seriesIndex);
            const QColor color = effectiveSeriesColor(sample.seriesIndex);
            painter.setPen(QPen(plotColor(), 3));
            painter.setBrush(color);
            painter.drawEllipse(sample.plotPoint, 4.5, 4.5);
            const QString name =
                series.name.isEmpty() ? QStringLiteral("Series %1").arg(sample.seriesIndex + 1) : series.name;
            labelLines.append(QStringLiteral("%1  %2").arg(name, tickText(sample.dataPoint.y(), hoverYStep)));
        }

        qreal labelWidth = 0;
        for (const QString &line : labelLines) {
            labelWidth = qMax<qreal>(labelWidth, metrics.horizontalAdvance(line));
        }
        const QSizeF labelSize(labelWidth + 18, metrics.height() * labelLines.size() + 12);
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
        qreal y = labelRect.top() + 6 + metrics.ascent();
        for (int i = 0; i < labelLines.size(); ++i) {
            painter.setPen(i == 0 ? textColor() : effectiveSeriesColor(hoverSamples.value(i - 1).seriesIndex));
            painter.drawText(QPointF(labelRect.left() + 9, y), labelLines.at(i));
            y += metrics.height();
        }
    }

    painter.restore();

    for (const auto &sample : latestSamples) {
        const int seriesIndex = sample.first;
        const QPointF plotPoint = sample.second;
        if (plotPoint.y() < plot.top() || plotPoint.y() > plot.bottom()) {
            continue;
        }
        const QColor color = effectiveSeriesColor(seriesIndex);
        const QString value = tickText(mapFromPlot(plotPoint, plot, xMin, xMax, yMin, yMax).y(),
                                       (yMax - yMin) / 5.0);
        const QRectF tagRect(plot.right() + 6, plotPoint.y() - metrics.height() / 2.0 - 3,
                             qMin<qreal>(44, rect().right() - plot.right() - 8), metrics.height() + 6);
        if (tagRect.width() <= 12) {
            continue;
        }
        painter.setPen(Qt::NoPen);
        painter.setBrush(withAlpha(color, 34));
        painter.drawRoundedRect(tagRect, 3, 3);
        painter.setPen(color);
        painter.drawText(tagRect.adjusted(4, 0, -4, 0), Qt::AlignCenter,
                         metrics.elidedText(value, Qt::ElideRight, static_cast<int>(tagRect.width() - 8)));
    }
    drawLegend(&painter, plot);
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

    const qreal itemGap = 16;
    QVector<qreal> itemWidths;
    qreal totalWidth = 0;
    for (const PlotSeries &series : m_series) {
        const QString name = series.name.isEmpty() ? QStringLiteral("Series") : series.name;
        const qreal width = 28 + metrics.horizontalAdvance(name);
        itemWidths.append(width);
        totalWidth += width + itemGap;
    }
    totalWidth = qMax<qreal>(0, totalWidth - itemGap);

    const qreal y = plot.top() + 8;
    qreal x = plot.center().x() - totalWidth / 2.0;
    if (x < plot.left() + 8) {
        x = plot.left() + 8;
    }

    for (int i = 0; i < m_series.size(); ++i) {
        const PlotSeries &series = m_series.at(i);
        const QRectF itemRect(x, y, itemWidths.at(i), metrics.height() + 8);
        m_legendToggleRects[i] = itemRect;

        const QColor seriesColor = effectiveSeriesColor(i);
        const QColor displayColor = series.visible ? seriesColor : withAlpha(textColor(), 82);
        painter->setPen(QPen(displayColor, 2.2, Qt::SolidLine, Qt::RoundCap));
        const qreal lineY = itemRect.center().y();
        painter->drawLine(QPointF(itemRect.left() + 3, lineY), QPointF(itemRect.left() + 19, lineY));
        painter->setBrush(displayColor);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(itemRect.left() + 11, lineY), 3, 3);

        painter->setPen(series.visible ? textColor() : withAlpha(textColor(), 86));
        painter->drawText(QRectF(itemRect.left() + 26, itemRect.top(), itemRect.width() - 26, itemRect.height()),
                          Qt::AlignVCenter | Qt::AlignLeft, series.name);
        x += itemRect.width() + itemGap;
    }

    painter->restore();
}

} // namespace FluentQt
