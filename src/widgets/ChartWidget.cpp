#include <FluentQtWidgets/Widgets/ChartWidget.h>

#include <FluentQtWidgets/Theme.h>

#include <QtCore/QEasingCurve>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonParseError>
#include <QtCore/QLineF>
#include <QtCore/QVariantAnimation>
#include <QtGui/QFontMetrics>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtGui/QPen>
#include <QtWidgets/QSizePolicy>

#include <algorithm>
#include <cmath>
#include <limits>

namespace FluentQt {

namespace {

constexpr qreal kPi = 3.14159265358979323846;
constexpr qreal kHalfPi = kPi / 2.0;

struct ChartSeries
{
    QString name;
    QString type;
    QVector<qreal> values;
    QVector<QPointF> points;
    bool area = false;
    int yAxisIndex = 0;
};

QColor withAlpha(QColor color, int alpha)
{
    color.setAlpha(alpha);
    return color;
}

QColor withOpacity(QColor color, qreal opacity)
{
    color.setAlpha(qBound(0, qRound(color.alpha() * opacity), 255));
    return color;
}

qreal easedProgress(qreal progress)
{
    const qreal bounded = qBound<qreal>(0.0, progress, 1.0);
    return 1.0 - std::pow(1.0 - bounded, 3.0);
}

qreal stagedProgress(qreal progress, int index, int count)
{
    if (count <= 1) {
        return easedProgress(progress);
    }

    const qreal delayWindow = 0.42;
    const qreal delay = delayWindow * qBound(0, index, count - 1) / qMax(1, count - 1);
    const qreal local = (progress - delay) / (1.0 - delayWindow);
    return easedProgress(local);
}

QRectF scaledFromCenter(const QRectF &rect, qreal scale)
{
    const qreal bounded = qBound<qreal>(0.0, scale, 1.0);
    const QSizeF size(rect.width() * bounded, rect.height() * bounded);
    return QRectF(rect.center() - QPointF(size.width() / 2.0, size.height() / 2.0), size);
}

QPointF mousePosition(const QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->position();
#else
    return event->pos();
#endif
}

QColor panelColor()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(32, 32, 32) : QColor(249, 249, 249);
}

QColor plotColor()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(39, 39, 39) : QColor(255, 255, 255);
}

QColor textColor()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(243, 243, 243) : QColor(31, 31, 31);
}

QColor mutedTextColor()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(180, 180, 180) : QColor(96, 96, 96);
}

QColor gridColor()
{
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(255, 255, 255, 34) : QColor(0, 0, 0, 22);
}

QVector<QColor> palette()
{
    return {ThemeManager::instance()->accentColor(), QColor(22, 163, 74), QColor(245, 158, 11),
            QColor(239, 68, 68), QColor(139, 92, 246), QColor(14, 165, 233)};
}

QJsonObject objectValue(const QJsonObject &object, const QString &key)
{
    return object.value(key).toObject();
}

QJsonArray arrayValue(const QJsonObject &object, const QString &key)
{
    return object.value(key).toArray();
}

QJsonValue arrayAt(const QJsonArray &array, int index)
{
    return index >= 0 && index < array.size() ? array.at(index) : QJsonValue();
}

QJsonObject objectAt(const QJsonArray &array, int index)
{
    return arrayAt(array, index).toObject();
}

QJsonObject axisObject(const QJsonObject &option, const QString &key, int index = 0)
{
    const QJsonValue value = option.value(key);
    if (value.isArray()) {
        return objectAt(value.toArray(), index);
    }
    return value.toObject();
}

QString optionTitle(const QJsonObject &option)
{
    return objectValue(option, QStringLiteral("title")).value(QStringLiteral("text")).toString();
}

QStringList categoryLabels(const QJsonObject &option, const QString &axisKey)
{
    QStringList labels;
    const QJsonArray data = arrayValue(axisObject(option, axisKey), QStringLiteral("data"));
    labels.reserve(data.size());
    for (const QJsonValue &value : data) {
        labels.append(value.toVariant().toString());
    }
    return labels;
}

QVector<qreal> numericValues(const QJsonArray &array)
{
    QVector<qreal> values;
    values.reserve(array.size());
    for (const QJsonValue &value : array) {
        if (value.isArray()) {
            const QJsonArray point = value.toArray();
            values.append(point.size() > 1 ? point.at(1).toDouble() : 0.0);
        } else if (value.isObject()) {
            values.append(value.toObject().value(QStringLiteral("value")).toDouble());
        } else {
            values.append(value.toDouble());
        }
    }
    return values;
}

QVector<QPointF> pointValues(const QJsonArray &array)
{
    QVector<QPointF> points;
    points.reserve(array.size());
    for (const QJsonValue &value : array) {
        if (!value.isArray()) {
            continue;
        }
        const QJsonArray point = value.toArray();
        if (point.size() >= 2) {
            points.append(QPointF(point.at(0).toDouble(), point.at(1).toDouble()));
        }
    }
    return points;
}

QVector<ChartSeries> seriesList(const QJsonObject &option)
{
    QVector<ChartSeries> result;
    const QJsonArray seriesArray = arrayValue(option, QStringLiteral("series"));
    result.reserve(seriesArray.size());
    for (const QJsonValue &value : seriesArray) {
        const QJsonObject object = value.toObject();
        ChartSeries series;
        series.name = object.value(QStringLiteral("name")).toString();
        series.type = object.value(QStringLiteral("type")).toString(QStringLiteral("line"));
        series.values = numericValues(arrayValue(object, QStringLiteral("data")));
        series.points = pointValues(arrayValue(object, QStringLiteral("data")));
        series.area = object.contains(QStringLiteral("areaStyle"));
        series.yAxisIndex = object.value(QStringLiteral("yAxisIndex")).toInt();
        result.append(series);
    }
    return result;
}

QString formatNumber(qreal value)
{
    const qreal absolute = qAbs(value);
    int precision = absolute >= 100.0 ? 0 : absolute >= 10.0 ? 1 : 2;
    QString text = QString::number(value, 'f', precision);
    while (text.contains(QLatin1Char('.')) && text.endsWith(QLatin1Char('0'))) {
        text.chop(1);
    }
    if (text.endsWith(QLatin1Char('.'))) {
        text.chop(1);
    }
    return text;
}

QRectF chartAreaRect(const QRectF &rect, const QString &title, bool hasBottomLegend = false)
{
    const qreal top = title.isEmpty() ? 24.0 : 56.0;
    const qreal bottom = hasBottomLegend ? 62.0 : 44.0;
    return rect.adjusted(58.0, top, -28.0, -bottom);
}

qreal niceMaximum(qreal value)
{
    if (value <= 0.0 || !std::isfinite(value)) {
        return 1.0;
    }
    const qreal power = std::pow(10.0, std::floor(std::log10(value)));
    const qreal normalized = value / power;
    qreal step = 1.0;
    if (normalized > 5.0) {
        step = 10.0;
    } else if (normalized > 2.0) {
        step = 5.0;
    } else if (normalized > 1.0) {
        step = 2.0;
    }
    return step * power;
}

void drawTitle(QPainter *painter, const QRectF &rect, const QString &title)
{
    if (title.isEmpty()) {
        return;
    }
    QFont font = painter->font();
    font.setBold(true);
    font.setPointSize(font.pointSize() + 1);
    painter->setFont(font);
    painter->setPen(textColor());
    painter->drawText(rect.adjusted(16, 16, -16, 0), Qt::AlignHCenter | Qt::AlignTop, title);
}

void drawLegend(QPainter *painter, const QRectF &rect, const QVector<ChartSeries> &series)
{
    if (series.size() < 2) {
        return;
    }

    const QVector<QColor> colors = palette();
    QFontMetrics metrics(painter->font());
    qreal totalWidth = 0;
    for (int i = 0; i < series.size(); ++i) {
        totalWidth += 22 + metrics.horizontalAdvance(series.at(i).name.isEmpty()
                                                         ? QStringLiteral("Series %1").arg(i + 1)
                                                         : series.at(i).name) + 16;
    }

    qreal x = rect.center().x() - totalWidth / 2.0;
    const qreal y = rect.bottom() - 28.0;
    painter->setPen(mutedTextColor());
    for (int i = 0; i < series.size(); ++i) {
        const QString name = series.at(i).name.isEmpty() ? QStringLiteral("Series %1").arg(i + 1) : series.at(i).name;
        painter->setBrush(colors.at(i % colors.size()));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(QRectF(x, y + 6, 12, 4), 2, 2);
        painter->setPen(mutedTextColor());
        painter->drawText(QPointF(x + 18, y + metrics.ascent()), name);
        x += 22 + metrics.horizontalAdvance(name) + 16;
    }
}

void drawTooltip(QPainter *painter, const QRectF &bounds, QPointF anchor, const QStringList &lines)
{
    if (lines.isEmpty()) {
        return;
    }

    QFontMetrics metrics(painter->font());
    qreal width = 0;
    for (const QString &line : lines) {
        width = qMax<qreal>(width, metrics.horizontalAdvance(line));
    }

    const qreal padding = 9.0;
    const QSizeF size(width + padding * 2.0, metrics.height() * lines.size() + padding * 2.0);
    QPointF topLeft(anchor.x() + 14.0, anchor.y() - size.height() - 12.0);
    if (topLeft.x() + size.width() > bounds.right() - 8.0) {
        topLeft.setX(anchor.x() - size.width() - 14.0);
    }
    if (topLeft.y() < bounds.top() + 8.0) {
        topLeft.setY(anchor.y() + 14.0);
    }
    topLeft.setX(qBound(bounds.left() + 8.0, topLeft.x(), bounds.right() - size.width() - 8.0));
    topLeft.setY(qBound(bounds.top() + 8.0, topLeft.y(), bounds.bottom() - size.height() - 8.0));

    const QRectF tooltipRect(topLeft, size);
    painter->save();
    painter->setPen(ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(255, 255, 255, 36)
                                                                               : QColor(0, 0, 0, 24));
    painter->setBrush(ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QColor(45, 45, 45, 238)
                                                                                : QColor(255, 255, 255, 244));
    painter->drawRoundedRect(tooltipRect, 6, 6);
    painter->setPen(textColor());
    qreal y = tooltipRect.top() + padding + metrics.ascent();
    for (const QString &line : lines) {
        painter->drawText(QPointF(tooltipRect.left() + padding, y), line);
        y += metrics.height();
    }
    painter->restore();
}

void drawInlineLegend(QPainter *painter, const QRectF &rect, const QStringList &labels, const QStringList &values,
                      const QVector<QColor> &colors, qreal progress)
{
    if (labels.isEmpty()) {
        return;
    }

    QFont legendFont = painter->font();
    legendFont.setPointSize(qMax(8, legendFont.pointSize() - 1));
    painter->setFont(legendFont);
    QFontMetrics metrics(legendFont);

    QVector<qreal> itemWidths;
    itemWidths.reserve(labels.size());
    qreal totalWidth = 0;
    for (int i = 0; i < labels.size(); ++i) {
        const QString text = values.value(i).isEmpty() ? labels.at(i) : QStringLiteral("%1  %2").arg(labels.at(i), values.at(i));
        const qreal width = 18 + metrics.horizontalAdvance(text) + 18;
        itemWidths.append(width);
        totalWidth += width;
    }

    qreal x = rect.center().x() - totalWidth / 2.0;
    const qreal y = rect.bottom() - 28.0;
    for (int i = 0; i < labels.size(); ++i) {
        const QString text = values.value(i).isEmpty() ? labels.at(i) : QStringLiteral("%1  %2").arg(labels.at(i), values.at(i));
        const QColor color = withOpacity(colors.at(i % colors.size()), progress);
        painter->setPen(Qt::NoPen);
        painter->setBrush(color);
        painter->drawEllipse(QPointF(x + 6, y + 8), 4, 4);
        painter->setPen(withOpacity(mutedTextColor(), progress));
        painter->drawText(QPointF(x + 16, y + metrics.ascent()), text);
        x += itemWidths.at(i);
    }
}

void drawFrame(QPainter *painter, const QRectF &plot)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(plotColor());
    painter->drawRoundedRect(plot.adjusted(-8, -8, 8, 8), 6, 6);
}

void drawGrid(QPainter *painter, const QRectF &plot, qreal maximum, const QStringList &categories)
{
    painter->setPen(QPen(gridColor(), 1));
    painter->setBrush(Qt::NoBrush);
    QFontMetrics metrics(painter->font());

    constexpr int tickCount = 4;
    for (int i = 0; i <= tickCount; ++i) {
        const qreal ratio = static_cast<qreal>(i) / tickCount;
        const qreal y = plot.bottom() - ratio * plot.height();
        painter->drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
        painter->setPen(mutedTextColor());
        const QString label = formatNumber(maximum * ratio);
        painter->drawText(QRectF(0, y - metrics.height() / 2.0, plot.left() - 10, metrics.height()),
                          Qt::AlignRight | Qt::AlignVCenter, label);
        painter->setPen(QPen(gridColor(), 1));
    }

    if (!categories.isEmpty()) {
        painter->setPen(mutedTextColor());
        const int stride = qMax(1, categories.size() / 7);
        for (int i = 0; i < categories.size(); i += stride) {
            const qreal x = categories.size() == 1
                                ? plot.center().x()
                                : plot.left() + (static_cast<qreal>(i) + 0.5) * plot.width() / categories.size();
            painter->drawText(QRectF(x - 38, plot.bottom() + 8, 76, metrics.height()),
                              Qt::AlignHCenter | Qt::AlignTop, categories.at(i));
        }
    }
}

QPointF mapCategoryPoint(const QRectF &plot, int index, int count, qreal value, qreal maximum)
{
    const qreal x = count <= 1 ? plot.center().x()
                               : plot.left() + static_cast<qreal>(index) * plot.width() / qMax(1, count - 1);
    const qreal y = plot.bottom() - (value / maximum) * plot.height();
    return QPointF(x, y);
}

void drawAxisCharts(QPainter *painter, const QRectF &rect, const QJsonObject &option, qreal animationProgress,
                    bool hasHover, const QPointF &hoverPosition)
{
    QVector<ChartSeries> series = seriesList(option);
    if (series.isEmpty()) {
        return;
    }
    const qreal progress = easedProgress(animationProgress);
    const bool scatterOnly = std::all_of(series.cbegin(), series.cend(), [](const ChartSeries &item) {
        return item.type == QStringLiteral("scatter");
    });

    if (scatterOnly) {
        qreal xMinimum = std::numeric_limits<qreal>::max();
        qreal xMaximum = std::numeric_limits<qreal>::lowest();
        qreal yMinimum = std::numeric_limits<qreal>::max();
        qreal yMaximum = std::numeric_limits<qreal>::lowest();
        for (const ChartSeries &item : series) {
            for (const QPointF &point : item.points) {
                xMinimum = qMin(xMinimum, point.x());
                xMaximum = qMax(xMaximum, point.x());
                yMinimum = qMin(yMinimum, point.y());
                yMaximum = qMax(yMaximum, point.y());
            }
        }
        if (!std::isfinite(xMinimum) || !std::isfinite(yMinimum)) {
            return;
        }
        if (xMinimum > 0) {
            xMinimum = 0;
        }
        if (yMinimum > 0) {
            yMinimum = 0;
        }
        xMaximum = niceMaximum(xMaximum);
        yMaximum = niceMaximum(yMaximum);
        if (qFuzzyCompare(xMinimum, xMaximum)) {
            xMaximum = xMinimum + 1;
        }
        if (qFuzzyCompare(yMinimum, yMaximum)) {
            yMaximum = yMinimum + 1;
        }

        const QRectF plot = chartAreaRect(rect, optionTitle(option), series.size() > 1);
        drawFrame(painter, plot);
        QFontMetrics metrics(painter->font());
        painter->setPen(QPen(gridColor(), 1));
        constexpr int tickCount = 4;
        for (int i = 0; i <= tickCount; ++i) {
            const qreal ratio = static_cast<qreal>(i) / tickCount;
            const qreal x = plot.left() + ratio * plot.width();
            const qreal y = plot.bottom() - ratio * plot.height();
            painter->drawLine(QPointF(plot.left(), y), QPointF(plot.right(), y));
            painter->drawLine(QPointF(x, plot.top()), QPointF(x, plot.bottom()));
            painter->setPen(mutedTextColor());
            painter->drawText(QRectF(0, y - metrics.height() / 2.0, plot.left() - 10, metrics.height()),
                              Qt::AlignRight | Qt::AlignVCenter,
                              formatNumber(yMinimum + (yMaximum - yMinimum) * ratio));
            painter->drawText(QRectF(x - 34, plot.bottom() + 8, 68, metrics.height()), Qt::AlignCenter,
                              formatNumber(xMinimum + (xMaximum - xMinimum) * ratio));
            painter->setPen(QPen(gridColor(), 1));
        }

        const QString xAxisName = axisObject(option, QStringLiteral("xAxis")).value(QStringLiteral("name")).toString();
        const QString yAxisName = axisObject(option, QStringLiteral("yAxis")).value(QStringLiteral("name")).toString();
        painter->setPen(mutedTextColor());
        if (!xAxisName.isEmpty()) {
            painter->drawText(QRectF(plot.right() - 120, plot.bottom() + metrics.height() + 10, 120, metrics.height()),
                              Qt::AlignRight, xAxisName);
        }
        if (!yAxisName.isEmpty()) {
            painter->drawText(QRectF(plot.left(), plot.top() - metrics.height() - 8, 160, metrics.height()),
                              Qt::AlignLeft, yAxisName);
        }

        const QVector<QColor> colors = palette();
        QPointF nearestScreenPoint;
        QPointF nearestDataPoint;
        QString nearestName;
        QColor nearestColor;
        qreal nearestDistance = std::numeric_limits<qreal>::max();
        for (int seriesIndex = 0; seriesIndex < series.size(); ++seriesIndex) {
            const ChartSeries &item = series.at(seriesIndex);
            const QColor color = colors.at(seriesIndex % colors.size());
            painter->setPen(Qt::NoPen);
            for (int i = 0; i < item.points.size(); ++i) {
                const QPointF &point = item.points.at(i);
                const qreal itemProgress = stagedProgress(animationProgress, i, item.points.size());
                const qreal x = plot.left() + (point.x() - xMinimum) / (xMaximum - xMinimum) * plot.width();
                const qreal y = plot.bottom() - (point.y() - yMinimum) / (yMaximum - yMinimum) * plot.height();
                painter->setBrush(withOpacity(color, qMax<qreal>(0.1, itemProgress)));
                painter->drawEllipse(QPointF(x, y), 5.5 * itemProgress, 5.5 * itemProgress);
                const qreal distance = std::hypot(hoverPosition.x() - x, hoverPosition.y() - y);
                if (hasHover && progress > 0.98 && distance < nearestDistance) {
                    nearestDistance = distance;
                    nearestScreenPoint = QPointF(x, y);
                    nearestDataPoint = point;
                    nearestName = item.name.isEmpty() ? QStringLiteral("Series %1").arg(seriesIndex + 1) : item.name;
                    nearestColor = color;
                }
            }
        }
        if (hasHover && progress > 0.98 && nearestDistance <= 18.0) {
            painter->setPen(QPen(plotColor(), 4));
            painter->setBrush(nearestColor);
            painter->drawEllipse(nearestScreenPoint, 7, 7);
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(withAlpha(nearestColor, 110), 2));
            painter->drawEllipse(nearestScreenPoint, 12, 12);
            drawTooltip(painter, rect, nearestScreenPoint,
                        {nearestName, QStringLiteral("%1: %2").arg(xAxisName.isEmpty() ? QStringLiteral("x") : xAxisName,
                                                                   formatNumber(nearestDataPoint.x())),
                         QStringLiteral("%1: %2").arg(yAxisName.isEmpty() ? QStringLiteral("y") : yAxisName,
                                                       formatNumber(nearestDataPoint.y()))});
        }
        drawLegend(painter, rect, series);
        return;
    }

    QStringList categories = categoryLabels(option, QStringLiteral("xAxis"));
    int categoryCount = categories.size();
    for (const ChartSeries &item : series) {
        categoryCount = qMax(categoryCount, item.values.size());
    }
    if (categories.isEmpty()) {
        for (int i = 0; i < categoryCount; ++i) {
            categories.append(QString::number(i + 1));
        }
    }

    qreal maximum = 0;
    for (const ChartSeries &item : series) {
        for (qreal value : item.values) {
            maximum = qMax(maximum, value);
        }
        for (const QPointF &point : item.points) {
            maximum = qMax(maximum, point.y());
        }
    }
    maximum = niceMaximum(maximum);

    const QRectF plot = chartAreaRect(rect, optionTitle(option), series.size() > 1);
    drawFrame(painter, plot);
    drawGrid(painter, plot, maximum, categories);

    const QVector<QColor> colors = palette();
    QVector<int> barSeriesIndexes;
    for (int i = 0; i < series.size(); ++i) {
        if (series.at(i).type == QStringLiteral("bar")) {
            barSeriesIndexes.append(i);
        }
    }

    if (!barSeriesIndexes.isEmpty() && categoryCount > 0) {
        const qreal groupWidth = plot.width() / categoryCount;
        const qreal barWidth = qMin<qreal>(34.0, groupWidth * 0.72 / barSeriesIndexes.size());
        for (int groupIndex = 0; groupIndex < barSeriesIndexes.size(); ++groupIndex) {
            const int seriesIndex = barSeriesIndexes.at(groupIndex);
            const ChartSeries &item = series.at(seriesIndex);
            painter->setPen(Qt::NoPen);
            painter->setBrush(colors.at(seriesIndex % colors.size()));
            for (int i = 0; i < item.values.size(); ++i) {
                const qreal itemProgress = stagedProgress(animationProgress, i + groupIndex, item.values.size() + barSeriesIndexes.size());
                const qreal center = plot.left() + (i + 0.5) * groupWidth;
                const qreal x = center - (barWidth * barSeriesIndexes.size()) / 2.0 + groupIndex * barWidth;
                const qreal targetHeight = item.values.at(i) / maximum * plot.height();
                const qreal height = targetHeight * itemProgress;
                painter->drawRoundedRect(QRectF(x, plot.bottom() - height, barWidth * 0.82, height), 2, 2);
            }
        }
    }

    for (int seriesIndex = 0; seriesIndex < series.size(); ++seriesIndex) {
        const ChartSeries &item = series.at(seriesIndex);
        if (item.type != QStringLiteral("line")) {
            continue;
        }

        QPainterPath line;
        QPainterPath area;
        for (int i = 0; i < item.values.size(); ++i) {
            const QPointF point = mapCategoryPoint(plot, i, item.values.size(), item.values.at(i), maximum);
            if (i == 0) {
                line.moveTo(point);
                area.moveTo(point.x(), plot.bottom());
                area.lineTo(point);
            } else {
                line.lineTo(point);
                area.lineTo(point);
            }
        }
        if (!item.values.isEmpty()) {
            area.lineTo(mapCategoryPoint(plot, item.values.size() - 1, item.values.size(), item.values.last(), maximum)
                            .x(),
                        plot.bottom());
            area.closeSubpath();
        }

        const QColor color = colors.at(seriesIndex % colors.size());
        painter->save();
        painter->setClipRect(plot.adjusted(-2, -8, -(plot.width() * (1.0 - progress)), 8));
        if (item.area) {
            painter->setPen(Qt::NoPen);
            painter->setBrush(withAlpha(color, ThemeManager::instance()->effectiveTheme() == Theme::Dark ? 54 : 42));
            painter->drawPath(area);
        }
        painter->setPen(QPen(color, 2.2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(Qt::NoBrush);
        painter->drawPath(line);
        painter->restore();
    }

    for (int seriesIndex = 0; seriesIndex < series.size(); ++seriesIndex) {
        const ChartSeries &item = series.at(seriesIndex);
        if (item.type != QStringLiteral("scatter")) {
            continue;
        }

        qreal xMinimum = std::numeric_limits<qreal>::max();
        qreal xMaximum = std::numeric_limits<qreal>::lowest();
        qreal yMinimum = std::numeric_limits<qreal>::max();
        qreal yMaximum = std::numeric_limits<qreal>::lowest();
        for (const QPointF &point : item.points) {
            xMinimum = qMin(xMinimum, point.x());
            xMaximum = qMax(xMaximum, point.x());
            yMinimum = qMin(yMinimum, point.y());
            yMaximum = qMax(yMaximum, point.y());
        }
        if (item.points.isEmpty()) {
            continue;
        }
        if (qFuzzyCompare(xMinimum, xMaximum)) {
            xMaximum = xMinimum + 1;
        }
        if (qFuzzyCompare(yMinimum, yMaximum)) {
            yMaximum = yMinimum + 1;
        }
        painter->setPen(Qt::NoPen);
        QPointF nearestScreenPoint;
        QPointF nearestDataPoint;
        qreal nearestDistance = std::numeric_limits<qreal>::max();
        for (int i = 0; i < item.points.size(); ++i) {
            const QPointF &point = item.points.at(i);
            const qreal itemProgress = stagedProgress(animationProgress, i, item.points.size());
            painter->setBrush(withOpacity(colors.at(seriesIndex % colors.size()), qMax<qreal>(0.1, itemProgress)));
            const qreal x = plot.left() + (point.x() - xMinimum) / (xMaximum - xMinimum) * plot.width();
            const qreal y = plot.bottom() - (point.y() - yMinimum) / (yMaximum - yMinimum) * plot.height();
            const qreal radius = 5.5 * itemProgress;
            painter->drawEllipse(QPointF(x, y), radius, radius);
            const qreal distance = std::hypot(hoverPosition.x() - x, hoverPosition.y() - y);
            if (hasHover && progress > 0.98 && distance < nearestDistance) {
                nearestDistance = distance;
                nearestScreenPoint = QPointF(x, y);
                nearestDataPoint = point;
            }
        }
        if (hasHover && progress > 0.98 && nearestDistance <= 16.0) {
            const QColor color = colors.at(seriesIndex % colors.size());
            painter->setPen(QPen(plotColor(), 4));
            painter->setBrush(color);
            painter->drawEllipse(nearestScreenPoint, 7, 7);
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(withAlpha(color, 110), 2));
            painter->drawEllipse(nearestScreenPoint, 12, 12);
            const QString name = item.name.isEmpty() ? QStringLiteral("Series %1").arg(seriesIndex + 1) : item.name;
            drawTooltip(painter, rect, nearestScreenPoint,
                        {name, QStringLiteral("x: %1").arg(formatNumber(nearestDataPoint.x())),
                         QStringLiteral("y: %1").arg(formatNumber(nearestDataPoint.y()))});
        }
    }

    if (hasHover && progress > 0.98 && plot.contains(hoverPosition) && categoryCount > 0) {
        const qreal groupWidth = plot.width() / categoryCount;
        const int index =
            qBound(0, static_cast<int>((hoverPosition.x() - plot.left()) / groupWidth), categoryCount - 1);
        const qreal x = plot.left() + (index + 0.5) * groupWidth;
        painter->setPen(QPen(gridColor(), 1, Qt::DashLine));
        painter->drawLine(QPointF(x, plot.top()), QPointF(x, plot.bottom()));

        QStringList lines;
        if (index < categories.size()) {
            lines.append(categories.at(index));
        }
        const QVector<QColor> chartColors = palette();
        QPointF tooltipAnchor(x, hoverPosition.y());
        for (int seriesIndex = 0; seriesIndex < series.size(); ++seriesIndex) {
            const ChartSeries &item = series.at(seriesIndex);
            if (item.type == QStringLiteral("scatter") || index >= item.values.size()) {
                continue;
            }
            const qreal value = item.values.at(index);
            const QPointF point(x, plot.bottom() - value / maximum * plot.height());
            const QColor color = chartColors.at(seriesIndex % chartColors.size());
            painter->setPen(QPen(plotColor(), 3));
            painter->setBrush(color);
            painter->drawEllipse(point, 4.5, 4.5);
            const QString name = item.name.isEmpty() ? QStringLiteral("Series %1").arg(seriesIndex + 1) : item.name;
            lines.append(QStringLiteral("%1: %2").arg(name, formatNumber(value)));
            tooltipAnchor.setY(qMin(tooltipAnchor.y(), point.y()));
        }
        if (lines.size() > 1) {
            drawTooltip(painter, rect, tooltipAnchor, lines);
        }
    }

    drawLegend(painter, rect, series);
}

void drawPieChart(QPainter *painter, const QRectF &rect, const QJsonObject &option, qreal animationProgress,
                  bool hasHover, const QPointF &hoverPosition)
{
    const QJsonObject item = objectAt(arrayValue(option, QStringLiteral("series")), 0);
    const QJsonArray data = arrayValue(item, QStringLiteral("data"));
    if (data.isEmpty()) {
        return;
    }

    qreal total = 0;
    for (const QJsonValue &value : data) {
        total += value.toObject().value(QStringLiteral("value")).toDouble();
    }
    if (total <= 0) {
        return;
    }

    const QRectF plot = rect.adjusted(40, optionTitle(option).isEmpty() ? 28 : 64, -40, -58);
    const qreal radius = qMin(plot.width(), plot.height()) / 2.0;
    const QRectF pieRect(plot.center().x() - radius, plot.center().y() - radius, radius * 2, radius * 2);
    const QVector<QColor> colors = palette();
    const qreal progress = easedProgress(animationProgress);
    const QRectF animatedPieRect = scaledFromCenter(pieRect, 0.72 + 0.28 * progress);
    const qreal animatedRadius = radius * (0.72 + 0.28 * progress);
    const QPointF center = animatedPieRect.center();
    const qreal innerRadius = animatedRadius * 0.34;
    int hoveredIndex = -1;
    if (hasHover && progress > 0.98) {
        const QPointF delta = hoverPosition - center;
        const qreal distance = std::hypot(delta.x(), delta.y());
        if (distance >= innerRadius && distance <= animatedRadius) {
            const qreal angle = std::atan2(delta.y(), delta.x()) * 180.0 / kPi;
            const qreal clockwiseFromTop = std::fmod(angle + 90.0 + 360.0, 360.0);
            qreal accumulated = 0;
            for (int i = 0; i < data.size(); ++i) {
                const qreal span = data.at(i).toObject().value(QStringLiteral("value")).toDouble() / total * 360.0;
                if (clockwiseFromTop >= accumulated && clockwiseFromTop < accumulated + span) {
                    hoveredIndex = i;
                    break;
                }
                accumulated += span;
            }
        }
    }

    int startAngle = 90 * 16;
    QStringList labels;
    QStringList values;
    QVector<QLineF> leaderLines;
    QVector<QPointF> labelPoints;
    QStringList sliceTexts;
    qreal accumulatedDegrees = 0;
    for (int i = 0; i < data.size(); ++i) {
        const QJsonObject object = data.at(i).toObject();
        const QString label = object.value(QStringLiteral("name")).toString(QStringLiteral("Item %1").arg(i + 1));
        const qreal rawValue = object.value(QStringLiteral("value")).toDouble();
        const qreal sliceDegrees = rawValue / total * 360.0;
        labels.append(label);
        values.append(formatNumber(rawValue));
        const int span = qRound(-sliceDegrees * 16.0 * progress);
        painter->setPen(Qt::NoPen);
        painter->setBrush(withOpacity(colors.at(i % colors.size()), qMax<qreal>(0.12, progress)));
        const QRectF sliceRect = i == hoveredIndex ? animatedPieRect.adjusted(-5, -5, 5, 5) : animatedPieRect;
        painter->drawPie(sliceRect, startAngle, span);
        startAngle += span;

        const qreal midRadians = (accumulatedDegrees + sliceDegrees / 2.0) * kPi / 180.0;
        const QPointF edge = center + QPointF(std::sin(midRadians) * animatedRadius * 0.78,
                                              -std::cos(midRadians) * animatedRadius * 0.78);
        const QPointF outer = center + QPointF(std::sin(midRadians) * animatedRadius * 1.08,
                                               -std::cos(midRadians) * animatedRadius * 1.08);
        const qreal side = outer.x() >= center.x() ? 1.0 : -1.0;
        const QPointF labelPoint = outer + QPointF(20.0 * side, 0);
        leaderLines.append(QLineF(edge, outer));
        labelPoints.append(labelPoint);
        sliceTexts.append(QStringLiteral("%1  %2%").arg(label).arg(QString::number(rawValue / total * 100.0, 'f', 1)));
        accumulatedDegrees += sliceDegrees;
    }

    painter->setBrush(panelColor());
    painter->drawEllipse(animatedPieRect.adjusted(animatedRadius * 0.34, animatedRadius * 0.34,
                                                 -animatedRadius * 0.34, -animatedRadius * 0.34));
    if (progress > 0.82) {
        const qreal labelOpacity = (progress - 0.82) / 0.18;
        QFont labelFont = painter->font();
        labelFont.setPointSize(qMax(8, labelFont.pointSize() - 1));
        painter->setFont(labelFont);
        QFontMetrics metrics(labelFont);
        for (int i = 0; i < leaderLines.size(); ++i) {
            const QColor color = withOpacity(colors.at(i % colors.size()), labelOpacity);
            painter->setPen(QPen(color, 1.2));
            painter->drawLine(leaderLines.at(i));
            painter->drawLine(leaderLines.at(i).p2(), labelPoints.at(i));
            const QPointF labelPoint = labelPoints.at(i);
            const bool rightSide = labelPoint.x() >= center.x();
            const QRectF textRect(rightSide ? labelPoint.x() + 4 : labelPoint.x() - 118,
                                  labelPoint.y() - metrics.height() / 2.0, 114, metrics.height());
            painter->setPen(withOpacity(mutedTextColor(), labelOpacity));
            painter->drawText(textRect, rightSide ? Qt::AlignLeft | Qt::AlignVCenter : Qt::AlignRight | Qt::AlignVCenter,
                              sliceTexts.at(i));
        }
    }
    drawInlineLegend(painter, rect, labels, values, colors, progress);
    if (hoveredIndex >= 0) {
        drawTooltip(painter, rect, hoverPosition, {labels.at(hoveredIndex), values.at(hoveredIndex)});
    }
}

void drawGauge(QPainter *painter, const QRectF &rect, const QJsonObject &option, qreal animationProgress)
{
    const QJsonObject item = objectAt(arrayValue(option, QStringLiteral("series")), 0);
    const QJsonObject data = objectAt(arrayValue(item, QStringLiteral("data")), 0);
    const qreal value = qBound<qreal>(0, data.value(QStringLiteral("value")).toDouble(), 100);
    const QString name = data.value(QStringLiteral("name")).toString();

    const QRectF plot = rect.adjusted(44, optionTitle(option).isEmpty() ? 30 : 64, -44, -30);
    const qreal radius = qMin(plot.width() * 0.46, plot.height() * 0.72);
    const QPointF center(plot.center().x(), plot.top() + radius + 18);
    const QRectF arcRect(center.x() - radius, center.y() - radius, radius * 2, radius * 2);
    const qreal progress = easedProgress(animationProgress);
    const qreal startDegrees = 210.0;
    const qreal sweepDegrees = 240.0;
    const qreal animatedValue = value * progress;
    const QVector<QPair<qreal, QColor>> sections{
        {0.35, QColor(34, 197, 94)},
        {0.75, ThemeManager::instance()->accentColor()},
        {1.0, QColor(239, 68, 68)},
    };

    painter->setPen(QPen(gridColor(), 14, Qt::SolidLine, Qt::RoundCap));
    painter->drawArc(arcRect, qRound(startDegrees * 16.0), qRound(-sweepDegrees * 16.0));

    qreal sectionStart = 0.0;
    for (const auto &section : sections) {
        const qreal sectionEnd = section.first;
        const qreal visibleEnd = qMin(sectionEnd, animatedValue / 100.0);
        if (visibleEnd > sectionStart) {
            const qreal start = startDegrees - sweepDegrees * sectionStart;
            const qreal span = -sweepDegrees * (visibleEnd - sectionStart);
            painter->setPen(QPen(section.second, 14, Qt::SolidLine, Qt::RoundCap));
            painter->drawArc(arcRect, qRound(start * 16.0), qRound(span * 16.0));
        }
        sectionStart = sectionEnd;
    }

    QFont tickFont = painter->font();
    tickFont.setPointSize(qMax(8, tickFont.pointSize() - 1));
    painter->setFont(tickFont);
    QFontMetrics tickMetrics(tickFont);
    for (int i = 0; i <= 10; ++i) {
        const qreal ratio = static_cast<qreal>(i) / 10.0;
        const qreal degrees = startDegrees - sweepDegrees * ratio;
        const qreal radians = degrees * kPi / 180.0;
        const QPointF outer = center + QPointF(std::cos(radians) * radius * 0.94, -std::sin(radians) * radius * 0.94);
        const QPointF inner = center + QPointF(std::cos(radians) * radius * 0.84, -std::sin(radians) * radius * 0.84);
        painter->setPen(QPen(gridColor(), i % 5 == 0 ? 2.0 : 1.0, Qt::SolidLine, Qt::RoundCap));
        painter->drawLine(inner, outer);
        if (i % 5 == 0) {
            const QPointF labelPoint =
                center + QPointF(std::cos(radians) * radius * 0.68, -std::sin(radians) * radius * 0.68);
            painter->setPen(mutedTextColor());
            painter->drawText(QRectF(labelPoint.x() - 22, labelPoint.y() - tickMetrics.height() / 2.0, 44,
                                     tickMetrics.height()),
                              Qt::AlignCenter, QString::number(i * 10));
        }
    }

    const qreal needleAngle = (startDegrees - sweepDegrees * value / 100.0 * progress) * kPi / 180.0;
    const QPointF needleEnd =
        center + QPointF(std::cos(needleAngle) * radius * 0.62, -std::sin(needleAngle) * radius * 0.62);
    const QPointF tail =
        center - QPointF(std::cos(needleAngle) * radius * 0.12, -std::sin(needleAngle) * radius * 0.12);
    const QPointF normal(-std::sin(needleAngle), -std::cos(needleAngle));
    QPolygonF needle;
    needle << needleEnd << center + normal * 5.0 << tail << center - normal * 5.0;
    painter->setPen(Qt::NoPen);
    painter->setBrush(ThemeManager::instance()->accentColor());
    painter->drawPolygon(needle);
    painter->setBrush(plotColor());
    painter->drawEllipse(center, 7, 7);
    painter->setBrush(ThemeManager::instance()->accentColor());
    painter->drawEllipse(center, 4, 4);

    QFont font = painter->font();
    font.setPointSize(font.pointSize() + 14);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(textColor());
    painter->drawText(QRectF(center.x() - 80, center.y() + radius * 0.22, 160, 48), Qt::AlignCenter,
                      QStringLiteral("%1%").arg(qRound(animatedValue)));
    font.setPointSize(font.pointSize() - 14);
    font.setBold(false);
    painter->setFont(font);
    painter->setPen(mutedTextColor());
    painter->drawText(QRectF(center.x() - 110, center.y() + radius * 0.50, 220, 28), Qt::AlignCenter, name);
}

void drawRadar(QPainter *painter, const QRectF &rect, const QJsonObject &option, qreal animationProgress,
               bool hasHover, const QPointF &hoverPosition)
{
    const QJsonArray indicators = arrayValue(objectValue(option, QStringLiteral("radar")), QStringLiteral("indicator"));
    const QJsonObject item = objectAt(arrayValue(option, QStringLiteral("series")), 0);
    const QJsonArray data = arrayValue(item, QStringLiteral("data"));
    if (indicators.size() < 3 || data.isEmpty()) {
        return;
    }

    const QRectF plot = rect.adjusted(54, optionTitle(option).isEmpty() ? 30 : 66, -54, -54);
    const QPointF center = plot.center();
    const qreal radius = qMin(plot.width(), plot.height()) / 2.0;
    const QVector<QColor> colors = palette();
    const qreal progress = easedProgress(animationProgress);

    painter->setPen(QPen(gridColor(), 1));
    painter->setBrush(Qt::NoBrush);
    for (int ring = 1; ring <= 4; ++ring) {
        QPolygonF polygon;
        const qreal r = radius * ring / 4.0;
        for (int i = 0; i < indicators.size(); ++i) {
            const qreal angle = -kHalfPi + i * 2.0 * kPi / indicators.size();
            polygon.append(center + QPointF(std::cos(angle) * r, std::sin(angle) * r));
        }
        painter->drawPolygon(polygon);
    }

    QFontMetrics metrics(painter->font());
    for (int i = 0; i < indicators.size(); ++i) {
        const qreal angle = -kHalfPi + i * 2.0 * kPi / indicators.size();
        const QPointF end = center + QPointF(std::cos(angle) * radius, std::sin(angle) * radius);
        painter->drawLine(center, end);
        const QString label = indicators.at(i).toObject().value(QStringLiteral("name")).toString();
        painter->setPen(mutedTextColor());
        painter->drawText(QRectF(end.x() - 48, end.y() - metrics.height() / 2.0, 96, metrics.height()), Qt::AlignCenter,
                          label);
        painter->setPen(QPen(gridColor(), 1));
    }

    QStringList legendLabels;
    QStringList legendValues;
    QPointF nearestVertex;
    QStringList nearestLines;
    QColor nearestColor;
    qreal nearestDistance = std::numeric_limits<qreal>::max();
    for (int dataIndex = 0; dataIndex < data.size(); ++dataIndex) {
        const QJsonObject dataObject = data.at(dataIndex).toObject();
        const QJsonArray values = arrayValue(dataObject, QStringLiteral("value"));
        const QString seriesName =
            dataObject.value(QStringLiteral("name")).toString(QStringLiteral("Series %1").arg(dataIndex + 1));
        legendLabels.append(seriesName);
        qreal average = 0;
        for (const QJsonValue &value : values) {
            average += value.toDouble();
        }
        if (!values.isEmpty()) {
            average /= values.size();
        }
        legendValues.append(formatNumber(average));
        QPolygonF polygon;
        for (int i = 0; i < qMin(indicators.size(), values.size()); ++i) {
            const qreal maximum = indicators.at(i).toObject().value(QStringLiteral("max")).toDouble(100);
            const qreal ratio = maximum > 0 ? values.at(i).toDouble() / maximum * progress : 0;
            const qreal angle = -kHalfPi + i * 2.0 * kPi / indicators.size();
            polygon.append(center + QPointF(std::cos(angle) * radius * ratio, std::sin(angle) * radius * ratio));
        }
        const QColor color = colors.at(dataIndex % colors.size());
        painter->setBrush(withAlpha(color, qRound(42 * progress)));
        painter->setPen(QPen(withOpacity(color, qMax<qreal>(0.12, progress)), 2, Qt::SolidLine, Qt::RoundCap,
                             Qt::RoundJoin));
        painter->drawPolygon(polygon);

        if (hasHover && progress > 0.98) {
            for (int i = 0; i < polygon.size(); ++i) {
                const QPointF vertex = polygon.at(i);
                const qreal distance = std::hypot(hoverPosition.x() - vertex.x(), hoverPosition.y() - vertex.y());
                if (distance < nearestDistance) {
                    nearestDistance = distance;
                    nearestVertex = vertex;
                    nearestColor = color;
                    nearestLines = {seriesName,
                                    indicators.at(i).toObject().value(QStringLiteral("name")).toString(),
                                    formatNumber(values.at(i).toDouble())};
                }
            }
        }
    }
    if (hasHover && progress > 0.98 && nearestDistance <= 18.0) {
        painter->setPen(QPen(plotColor(), 4));
        painter->setBrush(nearestColor);
        painter->drawEllipse(nearestVertex, 5.5, 5.5);
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(withAlpha(nearestColor, 110), 2));
        painter->drawEllipse(nearestVertex, 11, 11);
        drawTooltip(painter, rect, nearestVertex, nearestLines);
    }
    drawInlineLegend(painter, rect, legendLabels, legendValues, colors, progress);
}

void drawHeatmap(QPainter *painter, const QRectF &rect, const QJsonObject &option, qreal animationProgress,
                 bool hasHover, const QPointF &hoverPosition)
{
    const QStringList xLabels = categoryLabels(option, QStringLiteral("xAxis"));
    const QStringList yLabels = categoryLabels(option, QStringLiteral("yAxis"));
    const QJsonObject item = objectAt(arrayValue(option, QStringLiteral("series")), 0);
    const QJsonArray data = arrayValue(item, QStringLiteral("data"));
    if (xLabels.isEmpty() || yLabels.isEmpty()) {
        return;
    }

    const QRectF plot = chartAreaRect(rect, optionTitle(option));
    drawFrame(painter, plot);
    const qreal cellWidth = plot.width() / xLabels.size();
    const qreal cellHeight = plot.height() / yLabels.size();
    const QColor accent = ThemeManager::instance()->accentColor();

    qreal maximum = 1;
    for (const QJsonValue &value : data) {
        const QJsonArray cell = value.toArray();
        if (cell.size() >= 3) {
            maximum = qMax(maximum, cell.at(2).toDouble());
        }
    }

    painter->setPen(panelColor());
    QRectF hoveredCellRect;
    QStringList hoverLines;
    for (int i = 0; i < data.size(); ++i) {
        const QJsonValue &value = data.at(i);
        const QJsonArray cell = value.toArray();
        if (cell.size() < 3) {
            continue;
        }
        const int x = cell.at(0).toInt();
        const int y = cell.at(1).toInt();
        const qreal itemProgress = stagedProgress(animationProgress, i, data.size());
        const qreal ratio = qBound<qreal>(0, cell.at(2).toDouble() / maximum, 1);
        painter->setBrush(withAlpha(accent, qRound((35 + ratio * 210) * itemProgress)));
        const QRectF cellRect(plot.left() + x * cellWidth + 2, plot.top() + y * cellHeight + 2, cellWidth - 4,
                              cellHeight - 4);
        const QRectF animatedCellRect = scaledFromCenter(cellRect, itemProgress);
        painter->drawRoundedRect(animatedCellRect, 3, 3);

        if (itemProgress > 0.72) {
            painter->setPen(withOpacity(ratio > 0.58 ? QColor(255, 255, 255) : textColor(),
                                        (itemProgress - 0.72) / 0.28));
            painter->drawText(animatedCellRect, Qt::AlignCenter, formatNumber(cell.at(2).toDouble()));
            painter->setPen(panelColor());
        }

        if (hasHover && itemProgress > 0.98 && animatedCellRect.contains(hoverPosition)) {
            hoveredCellRect = animatedCellRect;
            hoverLines = {xLabels.value(x), yLabels.value(y), formatNumber(cell.at(2).toDouble())};
        }
    }

    if (!hoveredCellRect.isNull()) {
        painter->setPen(QPen(ThemeManager::instance()->accentColor(), 2));
        painter->setBrush(Qt::NoBrush);
        painter->drawRoundedRect(hoveredCellRect.adjusted(-1, -1, 1, 1), 4, 4);
        drawTooltip(painter, rect, hoveredCellRect.center(), hoverLines);
    }

    QFontMetrics metrics(painter->font());
    painter->setPen(mutedTextColor());
    for (int i = 0; i < xLabels.size(); ++i) {
        painter->drawText(QRectF(plot.left() + i * cellWidth, plot.bottom() + 8, cellWidth, metrics.height()),
                          Qt::AlignCenter, xLabels.at(i));
    }
    for (int i = 0; i < yLabels.size(); ++i) {
        painter->drawText(QRectF(0, plot.top() + i * cellHeight + cellHeight / 2.0 - metrics.height() / 2.0,
                                 plot.left() - 12, metrics.height()),
                          Qt::AlignRight | Qt::AlignVCenter, yLabels.at(i));
    }
}

QString primaryType(const QJsonObject &option)
{
    return objectAt(arrayValue(option, QStringLiteral("series")), 0).value(QStringLiteral("type")).toString();
}

} // namespace

ChartWidget::ChartWidget(QWidget *parent) : QWidget(parent)
{
    init();
}

ChartWidget::ChartWidget(const QJsonObject &option, QWidget *parent)
    : QWidget(parent)
    , m_option(option)
{
    init();
}

QJsonObject ChartWidget::option() const { return m_option; }

QString ChartWidget::chartTheme() const { return m_chartTheme; }

QSize ChartWidget::sizeHint() const { return QSize(520, 320); }

void ChartWidget::setOption(const QJsonObject &option)
{
    if (m_option == option) {
        return;
    }
    m_option = option;
    applyPendingOption();
    emit optionChanged(m_option);
}

void ChartWidget::setOptionJson(const QString &json)
{
    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        return;
    }
    setOption(document.object());
}

void ChartWidget::setChartTheme(const QString &theme)
{
    const QString normalized = theme.trimmed().isEmpty() ? QStringLiteral("auto") : theme.trimmed().toLower();
    if (m_chartTheme == normalized) {
        return;
    }
    m_chartTheme = normalized;
    reload();
    emit chartThemeChanged(m_chartTheme);
}

void ChartWidget::reload()
{
    renderChart();
}

void ChartWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    if (m_pendingRenderAnimation) {
        m_pendingRenderAnimation = false;
        startRenderAnimation();
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.fillRect(rect(), panelColor());

    const QRectF bounds = QRectF(rect()).adjusted(10, 10, -10, -10);
    const QString title = optionTitle(m_option);
    drawTitle(&painter, bounds, title);

    const QString type = primaryType(m_option);
    if (type == QStringLiteral("pie")) {
        drawPieChart(&painter, bounds, m_option, m_animationProgress, m_hasHover, m_hoverPosition);
    } else if (type == QStringLiteral("gauge")) {
        drawGauge(&painter, bounds, m_option, m_animationProgress);
    } else if (type == QStringLiteral("radar")) {
        drawRadar(&painter, bounds, m_option, m_animationProgress, m_hasHover, m_hoverPosition);
    } else if (type == QStringLiteral("heatmap")) {
        drawHeatmap(&painter, bounds, m_option, m_animationProgress, m_hasHover, m_hoverPosition);
    } else {
        drawAxisCharts(&painter, bounds, m_option, m_animationProgress, m_hasHover, m_hoverPosition);
    }
}

void ChartWidget::mouseMoveEvent(QMouseEvent *event)
{
    m_hoverPosition = mousePosition(event);
    if (!m_hasHover) {
        m_hasHover = true;
    }
    update();
    QWidget::mouseMoveEvent(event);
}

void ChartWidget::leaveEvent(QEvent *event)
{
    if (m_hasHover) {
        m_hasHover = false;
        update();
    }
    QWidget::leaveEvent(event);
}

void ChartWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    resizeChart();
}

void ChartWidget::init()
{
    setAttribute(Qt::WA_StyledBackground, true);
    setMouseTracking(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_animation = new QVariantAnimation(this);
    m_animation->setDuration(900);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        m_animationProgress = value.toReal();
        update();
    });
    connect(m_animation, &QVariantAnimation::finished, this, [this]() {
        m_animationProgress = 1.0;
        update();
        emit loadFinished(true);
    });

    connect(ThemeManager::instance(), &ThemeManager::effectiveThemeChanged, this, [this](Theme) {
        if (m_chartTheme == QStringLiteral("auto")) {
            update();
        }
    });
    connect(ThemeManager::instance(), &ThemeManager::accentColorChanged, this, [this](const QColor &) { update(); });

    m_animationProgress = 0.0;
}

void ChartWidget::renderChart()
{
    m_pendingRenderAnimation = true;
    update();
}

void ChartWidget::applyPendingOption()
{
    renderChart();
}

void ChartWidget::resizeChart()
{
    update();
}

void ChartWidget::startRenderAnimation()
{
    if (!m_animation) {
        m_animationProgress = 1.0;
        update();
        emit loadFinished(true);
        return;
    }

    m_animation->stop();
    m_pendingRenderAnimation = false;
    m_animationProgress = 0.0;
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->start();
    update();
}

QString ChartWidget::resolvedChartTheme() const
{
    if (m_chartTheme != QStringLiteral("auto")) {
        return m_chartTheme;
    }
    return ThemeManager::instance()->effectiveTheme() == Theme::Dark ? QStringLiteral("dark") : QStringLiteral("light");
}

} // namespace FluentQt
