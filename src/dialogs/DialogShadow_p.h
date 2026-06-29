#pragma once

#include <QtCore/QPoint>
#include <QtCore/QRectF>
#include <QtGui/QColor>
#include <QtGui/QPainter>

namespace FluentQt::Private {

inline void paintFluentWindowShadow(QPainter *painter, const QRectF &contentRect, int blurRadius,
                                    const QPoint &offset, const QColor &shadowColor, qreal radius = 9.0)
{
    if (!painter || blurRadius <= 0 || shadowColor.alpha() <= 0 || contentRect.isEmpty()) {
        return;
    }

    painter->save();
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setPen(Qt::NoPen);

    const QRectF baseRect = contentRect.translated(offset);
    const int layers = qBound(4, blurRadius / 4, 14);
    const qreal maxSpread = blurRadius / 2.0;

    for (int i = layers; i >= 1; --i) {
        const qreal progress = static_cast<qreal>(i) / layers;
        const qreal spread = progress * maxSpread;
        QColor color = shadowColor;
        const int alpha = qRound(shadowColor.alpha() * (1.0 - progress) * (1.0 - progress) + 1);
        color.setAlpha(qBound(0, alpha, shadowColor.alpha()));
        painter->setBrush(color);
        painter->drawRoundedRect(baseRect.adjusted(-spread, -spread, spread, spread),
                                 radius + spread, radius + spread);
    }

    painter->restore();
}

} // namespace FluentQt::Private
