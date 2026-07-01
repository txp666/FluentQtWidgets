#include <FluentQtWidgets/Widgets/RealtimePlotWidget.h>

#include <FluentQtWidgets/Theme.h>

#include <QtCore/QtMath>
#include <QtGui/QAction>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QWheelEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QSizePolicy>

#include <FluentQtWidgets/Widgets/Menu.h>

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

QPoint globalMousePosition(const QMouseEvent *event)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    return event->globalPosition().toPoint();
#else
    return event->globalPos();
#endif
}

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

    const QRectF plot = plotRect();
    if (event->button() == Qt::RightButton && plot.contains(position)) {
        qreal yMin = m_yMinimum;
        qreal yMax = m_yMaximum;
        visibleYRange(viewXMinimum(), viewXMaximum(), &yMin, &yMax);

        m_rightDragPending = true;
        m_rightScaling = false;
        m_suppressNextContextMenu = false;
        m_rightDragStartPosition = position;
        m_rightDragAnchor = mapFromPlot(position, plot, viewXMinimum(), viewXMaximum(), yMin, yMax);
        m_rightDragStartXMinimum = viewXMinimum();
        m_rightDragStartXMaximum = viewXMaximum();
        m_rightDragStartYMinimum = yMin;
        m_rightDragStartYMaximum = yMax;
        grabMouse();
        event->accept();
        return;
    }

    if (event->button() != Qt::LeftButton || !plot.contains(position)) {
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

    if (m_rightDragPending || m_rightScaling) {
        const QPointF delta = position - m_rightDragStartPosition;
        if (!m_rightScaling) {
            const QPoint roundedDelta(qRound(delta.x()), qRound(delta.y()));
            if (QPoint(0, 0).manhattanLength() == roundedDelta.manhattanLength() ||
                roundedDelta.manhattanLength() < QApplication::startDragDistance()) {
                event->accept();
                return;
            }
            m_rightScaling = true;
            m_suppressNextContextMenu = true;
        }

        const qreal xFactor = qBound<qreal>(0.02, std::pow(1.01, -delta.x()), 50.0);
        const qreal yFactor = qBound<qreal>(0.02, std::pow(1.01, delta.y()), 50.0);
        const qreal nextXMinimum = m_rightDragAnchor.x() - (m_rightDragAnchor.x() - m_rightDragStartXMinimum) * xFactor;
        const qreal nextXMaximum = m_rightDragAnchor.x() + (m_rightDragStartXMaximum - m_rightDragAnchor.x()) * xFactor;
        const qreal nextYMinimum = m_rightDragAnchor.y() - (m_rightDragAnchor.y() - m_rightDragStartYMinimum) * yFactor;
        const qreal nextYMaximum = m_rightDragAnchor.y() + (m_rightDragStartYMaximum - m_rightDragAnchor.y()) * yFactor;
        setXRange(nextXMinimum, nextXMaximum);
        setYRange(nextYMinimum, nextYMaximum);
        event->accept();
        return;
    }

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
    if (event->button() == Qt::RightButton && (m_rightDragPending || m_rightScaling)) {
        const bool shouldShowMenu = m_rightDragPending && !m_rightScaling;
        m_rightDragPending = false;
        m_rightScaling = false;
        if (mouseGrabber() == this) {
            releaseMouse();
        }
        if (shouldShowMenu) {
            m_suppressNextContextMenu = true;
            showContextMenu(globalMousePosition(event));
        }
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

void RealtimePlotWidget::contextMenuEvent(QContextMenuEvent *event)
{
    if (m_suppressNextContextMenu) {
        m_suppressNextContextMenu = false;
        event->accept();
        return;
    }

    if (!plotRect().contains(event->pos())) {
        QWidget::contextMenuEvent(event);
        return;
    }

    if (m_rightScaling) {
        event->accept();
        return;
    }
    if (m_rightDragPending) {
        event->accept();
        return;
    }

    showContextMenu(event->globalPos());
    event->accept();
}

void RealtimePlotWidget::showContextMenu(const QPoint &globalPosition)
{
    auto *menu = new CheckableMenu(this);
    menu->setAttribute(Qt::WA_DeleteOnClose);
    QAction *viewAllAction = menu->addAction(tr("View All"));
    QAction *autoRangeAction = menu->addAction(tr("Auto Range"));
    menu->addSeparator();

    QAction *autoScrollAction = menu->addCheckableAction(tr("Auto-scroll X"), m_autoScroll);
    QAction *autoYAction = menu->addCheckableAction(tr("Auto Y Range"), m_autoYRange);

    menu->addSeparator();
    QAction *gridAction = menu->addCheckableAction(tr("Grid"), m_gridVisible);
    QAction *fillAction = menu->addCheckableAction(tr("Fill under curves"), m_fillVisible);
    QAction *pointsAction = menu->addCheckableAction(tr("Points"), m_pointsVisible);
    QAction *crosshairAction = menu->addCheckableAction(tr("Crosshair"), m_crosshairVisible);
    QAction *legendAction = menu->addCheckableAction(tr("Legend"), m_legendVisible);

    connect(viewAllAction, &QAction::triggered, this, &RealtimePlotWidget::showAllData);
    connect(autoRangeAction, &QAction::triggered, this, &RealtimePlotWidget::resetView);
    connect(autoScrollAction, &QAction::triggered, this,
            [this, autoScrollAction]() { setAutoScroll(autoScrollAction->isChecked()); });
    connect(autoYAction, &QAction::triggered, this, [this, autoYAction]() {
        setAutoYRange(autoYAction->isChecked());
    });
    connect(gridAction, &QAction::triggered, this,
            [this, gridAction]() { setGridVisible(gridAction->isChecked()); });
    connect(fillAction, &QAction::triggered, this,
            [this, fillAction]() { setFillVisible(fillAction->isChecked()); });
    connect(pointsAction, &QAction::triggered, this,
            [this, pointsAction]() { setPointsVisible(pointsAction->isChecked()); });
    connect(crosshairAction, &QAction::triggered, this, [this, crosshairAction]() {
        setCrosshairVisible(crosshairAction->isChecked());
    });
    connect(legendAction, &QAction::triggered, this,
            [this, legendAction]() { setLegendVisible(legendAction->isChecked()); });

    menu->exec(globalPosition, true, MenuAnimationType::FadeInDropDown);
}

void RealtimePlotWidget::showAllData()
{
    qreal xMin = std::numeric_limits<qreal>::max();
    qreal xMax = std::numeric_limits<qreal>::lowest();
    qreal yMin = std::numeric_limits<qreal>::max();
    qreal yMax = std::numeric_limits<qreal>::lowest();

    for (int seriesIndex = 0; seriesIndex < m_series.size(); ++seriesIndex) {
        const PlotSeries &series = m_series.at(seriesIndex);
        if (!series.visible || series.count == 0) {
            continue;
        }
        for (int i = 0; i < series.count; ++i) {
            const QPointF point = pointAt(seriesIndex, i);
            xMin = qMin(xMin, point.x());
            xMax = qMax(xMax, point.x());
            yMin = qMin(yMin, point.y());
            yMax = qMax(yMax, point.y());
        }
    }

    if (xMin == std::numeric_limits<qreal>::max() || xMax == std::numeric_limits<qreal>::lowest()) {
        resetView();
        return;
    }

    const qreal xRange = qMax<qreal>(kMinimumRange, xMax - xMin);
    const qreal yRange = qMax<qreal>(kMinimumRange, yMax - yMin);
    const qreal xPadding = qMax<qreal>(0.5, xRange * 0.02);
    const qreal yPadding = qMax<qreal>(0.2, yRange * 0.12);
    const qreal xLeftPadding = xMin >= 0 ? qMin(xPadding, xMin) : xPadding;
    const qreal xLeft = xMin - xLeftPadding;

    m_xMinimum = xLeft;
    m_xMaximum = xMax;
    m_yMinimum = yMin - yPadding;
    m_yMaximum = yMax + yPadding;
    m_visibleSpan = qMax<qreal>(0, m_xMaximum - m_xMinimum);
    m_autoScroll = true;
    m_autoYRange = true;
    update();
    emit interactionChanged();
    emit rangeChanged();
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
    return QRectF(rect()).adjusted(58, 18, -58, -42);
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

} // namespace FluentQt
