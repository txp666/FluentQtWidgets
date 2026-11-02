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

    // Concentric rounded rects, each drawn over the ones inside it. Their alphas are chosen so the
    // composite is the shadow colour's alpha at the content's edge and falls off quadratically to
    // nothing blurRadius / 2 out, like the soft QGraphicsDropShadowEffect Python uses.
    // (Equal per-layer alphas would pile up to about twice the colour's alpha near the edge.)
    const QRectF baseRect = contentRect.translated(offset);
    const int layers = qBound(4, blurRadius / 2, 30);
    const qreal maxSpread = blurRadius / 2.0;
    const qreal peak = shadowColor.alphaF();
    auto composite = [&](int layer) { // The alpha wanted where layers layer..layers overlap
        const qreal remaining = static_cast<qreal>(layers - layer + 1) / layers;
        return peak * remaining * remaining;
    };

    for (int i = layers; i >= 1; --i) {
        const qreal spread = static_cast<qreal>(i) / layers * maxSpread;
        const qreal outer = i < layers ? composite(i + 1) : 0.0;
        const qreal alpha = 1.0 - (1.0 - composite(i)) / (1.0 - outer);
        QColor color = shadowColor;
        color.setAlphaF(qBound<qreal>(0.0, alpha, 1.0));
        painter->setBrush(color);
        painter->drawRoundedRect(baseRect.adjusted(-spread, -spread, spread, spread), radius + spread, radius + spread);
    }

    painter->restore();
}

} // namespace FluentQt::Private
