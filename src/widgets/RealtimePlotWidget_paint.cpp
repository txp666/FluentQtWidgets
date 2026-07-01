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

} // namespace

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

    painter.setPen(QPen(withAlpha(textColor(), 145), 1.2));
    painter.drawLine(QPointF(plot.left(), plot.top()), QPointF(plot.left(), plot.bottom()));
    painter.drawLine(QPointF(plot.left(), plot.bottom()), QPointF(plot.right(), plot.bottom()));

    const int pixelWidth = qMax(1, static_cast<int>(std::floor(plot.width())));
    int visibleCount = 0;
    QVector<HoverSample> hoverSamples;
    QVector<QPair<int, QPointF>> latestSamples;

    painter.save();
    painter.setClipRect(plot.adjusted(1, 1, -1, -1));

    for (int seriesIndex = 0; seriesIndex < m_series.size(); ++seriesIndex) {
        const PlotSeries &series = m_series.at(seriesIndex);
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
        const int targetPointCount =
            performanceMode ? qBound(3, pixelWidth * 2, visibleDataCount) : visibleDataCount;
        QVector<int> pointIndices;
        pointIndices.reserve(qMin(visibleDataCount, targetPointCount + 1));

        if (!performanceMode || visibleDataCount <= 3) {
            for (int i = firstIndex; i < lastIndex; ++i) {
                pointIndices.append(i);
            }
        } else {
            appendUniqueIndex(&pointIndices, firstIndex);

            const qreal bucketSize = static_cast<qreal>(visibleDataCount - 2) / (targetPointCount - 2);
            int anchorIndex = firstIndex;
            QPointF anchorPoint = mapToPlot(pointAt(seriesIndex, anchorIndex), plot, xMin, xMax, yMin, yMax);

            for (int bucket = 0; bucket < targetPointCount - 2; ++bucket) {
                int averageStart = firstIndex + 1 + qFloor((bucket + 1) * bucketSize);
                int averageEnd = firstIndex + 1 + qFloor((bucket + 2) * bucketSize);
                averageStart = qBound(firstIndex + 1, averageStart, lastIndex - 1);
                averageEnd = qBound(averageStart + 1, averageEnd, lastIndex);

                QPointF averagePoint;
                const int averageCount = averageEnd - averageStart;
                for (int i = averageStart; i < averageEnd; ++i) {
                    averagePoint += mapToPlot(pointAt(seriesIndex, i), plot, xMin, xMax, yMin, yMax);
                }
                averagePoint /= qMax(1, averageCount);

                int rangeStart = firstIndex + 1 + qFloor(bucket * bucketSize);
                int rangeEnd = firstIndex + 1 + qFloor((bucket + 1) * bucketSize);
                rangeStart = qBound(firstIndex + 1, rangeStart, lastIndex - 1);
                rangeEnd = qBound(rangeStart + 1, rangeEnd, lastIndex);

                int bestIndex = rangeStart;
                QPointF bestPoint = mapToPlot(pointAt(seriesIndex, bestIndex), plot, xMin, xMax, yMin, yMax);
                qreal bestArea = -1;
                for (int i = rangeStart; i < rangeEnd; ++i) {
                    const QPointF candidate = mapToPlot(pointAt(seriesIndex, i), plot, xMin, xMax, yMin, yMax);
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

            appendUniqueIndex(&pointIndices, lastIndex - 1);
        }

        QVector<QPointF> screenPoints;
        screenPoints.reserve(pointIndices.size());
        HoverSample nearest;
        nearest.seriesIndex = seriesIndex;
        for (int pointIndex : pointIndices) {
            const QPointF dataPoint = pointAt(seriesIndex, pointIndex);
            const QPointF screenPoint = mapToPlot(dataPoint, plot, xMin, xMax, yMin, yMax);
            screenPoints.append(screenPoint);
            if (m_hasHover) {
                const qreal distance = qAbs(screenPoint.x() - m_hoverPosition.x());
                if (distance < nearest.distance) {
                    nearest.distance = distance;
                    nearest.dataPoint = dataPoint;
                    nearest.plotPoint = screenPoint;
                }
            }
        }
        visibleCount += visibleDataCount;

        if (screenPoints.size() >= 2) {
            painter.setRenderHint(QPainter::Antialiasing, !performanceMode);
            if (m_fillVisible) {
                QPolygonF fillPolygon;
                fillPolygon.reserve(screenPoints.size() + 2);
                fillPolygon.append(QPointF(screenPoints.constFirst().x(), plot.bottom()));
                for (const QPointF &point : std::as_const(screenPoints)) {
                    fillPolygon.append(point);
                }
                fillPolygon.append(QPointF(screenPoints.constLast().x(), plot.bottom()));

                QLinearGradient gradient(plot.topLeft(), plot.bottomLeft());
                gradient.setColorAt(0, withAlpha(lineColor, performanceMode ? 34 : 48));
                gradient.setColorAt(1, withAlpha(lineColor, 0));
                painter.setPen(Qt::NoPen);
                painter.setBrush(gradient);
                painter.drawPolygon(fillPolygon);
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
        if (nearest.distance < std::numeric_limits<qreal>::max()) {
            hoverSamples.append(nearest);
        }
        if (!screenPoints.isEmpty()) {
            latestSamples.append(qMakePair(seriesIndex, screenPoints.constLast()));
        }
    }

    painter.setRenderHint(QPainter::Antialiasing, true);

    if (visibleCount == 0) {
        painter.setPen(QPen(withAlpha(textColor(), 55), 1.2, Qt::DashLine, Qt::RoundCap));
        const qreal centerY = plot.center().y();
        painter.drawLine(QPointF(plot.left() + 8, centerY), QPointF(plot.right() - 8, centerY));
    }

    if (m_crosshairVisible && m_hasHover && plot.contains(m_hoverPosition)) {
        const QPointF dataPoint = mapFromPlot(m_hoverPosition, plot, xMin, xMax, yMin, yMax);
        painter.setPen(QPen(withAlpha(textColor(), 120), 1, Qt::DashLine));
        painter.drawLine(QPointF(m_hoverPosition.x(), plot.top()), QPointF(m_hoverPosition.x(), plot.bottom()));
        painter.drawLine(QPointF(plot.left(), m_hoverPosition.y()), QPointF(plot.right(), m_hoverPosition.y()));

        QStringList labelLines{QStringLiteral("x=%1").arg(tickText(dataPoint.x()))};
        for (const HoverSample &sample : hoverSamples) {
            const PlotSeries &series = m_series.at(sample.seriesIndex);
            const QColor color = effectiveSeriesColor(sample.seriesIndex);
            painter.setPen(QPen(plotColor(), 3));
            painter.setBrush(color);
            painter.drawEllipse(sample.plotPoint, 4.5, 4.5);
            const QString name =
                series.name.isEmpty() ? QStringLiteral("Series %1").arg(sample.seriesIndex + 1) : series.name;
            labelLines.append(QStringLiteral("%1  %2").arg(name, tickText(sample.dataPoint.y())));
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
        const QString value = tickText(mapFromPlot(plotPoint, plot, xMin, xMax, yMin, yMax).y());
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
