#pragma once

#include <QtCore/QEasingCurve>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/QtGlobal>

namespace FluentQt::Private {

// Python's FluentAnimation.createBezierCurve().
inline QEasingCurve fluentBezierCurve(qreal x1, qreal y1, qreal x2, qreal y2)
{
    QEasingCurve curve(QEasingCurve::BezierSpline);
    curve.addCubicBezierSegment(QPointF(x1, y1), QPointF(x2, y2), QPointF(1, 1));
    return curve;
}

constexpr int kScaleSlideDuration = 600;

// Python's ScaleSlideAnimation, the indicator geometry `elapsed` ms after the start. On the same
// level the indicator first stretches over 200 ms to cover both places, then shrinks over 400 ms
// into the new one (WinUI's squash and stretch); between levels it grows in place over 600 ms.
inline QRectF scaleSlideGeometry(const QRectF &start, const QRectF &end, int elapsed, Qt::Orientation orientation)
{
    const bool horizontal = orientation == Qt::Horizontal;
    const auto withLength = [horizontal](QRectF rect, const QPointF &topLeft, qreal length) {
        rect.moveTopLeft(topLeft);
        if (horizontal) {
            rect.setWidth(length);
        } else {
            rect.setHeight(length);
        }
        return rect;
    };

    const bool sameLevel = horizontal ? qAbs(start.y() - end.y()) < 1 : qAbs(start.x() - end.x()) < 1;
    if (!sameLevel) {
        const qreal dimension = horizontal ? end.width() : end.height();
        const bool nextAfter = horizontal ? end.x() > start.x() : end.y() > start.y();
        const QPointF from = end.topLeft() + (horizontal ? QPointF(nextAfter ? 0 : dimension, 0)
                                                         : QPointF(0, nextAfter ? 0 : dimension));
        const qreal progress = QEasingCurve(QEasingCurve::OutQuint).valueForProgress(qBound(0.0, elapsed / 600.0, 1.0));
        return withLength(end, from + (end.topLeft() - from) * progress, dimension * progress);
    }

    const qreal dimension = horizontal ? start.width() : start.height();
    const qreal from = horizontal ? start.x() : start.y();
    const qreal to = horizontal ? end.x() : end.y();
    const qreal middleLength = qAbs(to - from) + dimension;
    const bool forward = to > from;
    const QPointF startPos = start.topLeft();
    const QPointF endPos = end.topLeft();

    if (elapsed < 200) {
        const qreal progress = fluentBezierCurve(0.9, 0.1, 1, 0.2).valueForProgress(qMax(0, elapsed) / 200.0);
        const QPointF pos = forward ? startPos : startPos + (endPos - startPos) * progress;
        return withLength(start, pos, dimension + (middleLength - dimension) * progress);
    }

    const qreal progress = fluentBezierCurve(0.1, 0.9, 0.2, 1.0).valueForProgress(qMin(1.0, (elapsed - 200) / 400.0));
    const QPointF pos = forward ? startPos + (endPos - startPos) * progress : endPos;
    return withLength(start, pos, middleLength + (dimension - middleLength) * progress);
}

} // namespace FluentQt::Private
